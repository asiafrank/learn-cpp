// Box.cpp : Defines the entry point for the application.
// Chapter 6 exercises 以 Box.cpp 为基础，做修改。

#include "stdafx.h"
#include "Box.h"

#include "../Common/d3dApp.h"
#include "../Common/MathHelper.h"
#include "../Common/UploadBuffer.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

struct ObjectConstants
{
    XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};

class BoxApp : public D3DApp
{
public:
    BoxApp(HINSTANCE hInstance);
    BoxApp(const BoxApp& rhs) = delete;
    BoxApp& operator=(const BoxApp& rhs) = delete;
    ~BoxApp();

    virtual bool Initialize() override;
private:
    virtual void OnResize() override;
    virtual void Update(const GameTimer& gt) override;
    virtual void Draw(const GameTimer& gt) override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

    void BuildDescriptorHeaps();
    void BuildConstantBuffers();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildBoxGeometry();
    void BuildPSO();

private:
    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr; // constant buffer view heap

    std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr; // object constant buffer
    std::unique_ptr<MeshGeometry> mGeo = nullptr;

    ComPtr<ID3DBlob> mvsByteCode = nullptr;
    ComPtr<ID3DBlob> mpsByteCode = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

    ComPtr<ID3D12PipelineState> mPSO = nullptr;

    XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    // 这里的 mTheta 和 mPhi 是初始时，摄像机对准的角度
    float mTheta = 1.5f*XM_PI; // 左右角度
    float mPhi = XM_PIDIV4;    // 上下角度
    // 摄像机到目标原点的距离，也相当于屏幕横坐标的大小;控制半径，能让立方体放大缩小，半径越大，立方体越小
    float mRadius = 10.0f;

    POINT mLastMousePos;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        BoxApp theApp(hInstance);
        if (!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

BoxApp::BoxApp(HINSTANCE hInstance) : D3DApp(hInstance)
{}

BoxApp::~BoxApp()
{}

bool BoxApp::Initialize()
{
    if (!D3DApp::Initialize())
        return false;

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    BuildDescriptorHeaps();
    BuildConstantBuffers();
    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildBoxGeometry();
    BuildPSO();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();
    return true;
}

void BoxApp::OnResize()
{
    D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection
    // matrix.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void BoxApp::Update(const GameTimer& gt)
{
    // Convert Spherical to Cartesian coordinates
    float x = mRadius * sinf(mPhi) * cosf(mTheta); // 0
    float z = mRadius * sinf(mPhi) * sinf(mTheta); // 5*(-1/sqrt(2)) = -3.53....
    float y = mRadius * cosf(mPhi);                // 5*(1/sqrt(2))

                                                   // Build the view matrix.
    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f); // 摄像机位置
    XMVECTOR target = XMVectorZero();          // 目标坐标
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, view);

    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    XMMATRIX worldViewProj = world * view * proj;

    // Update the constant buffer with the latest worldViewProj matrix.
    ObjectConstants objConstants;
    XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
    mObjectCB->CopyData(0, objConstants);
}

void BoxApp::Draw(const GameTimer& gt)
{
    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists
    // have finished execution on the GPU.
    ThrowIfFailed(mDirectCmdListAlloc->Reset());

    // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    // Indicate a state transition on the resource usage.
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(),
        Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

    ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    mCommandList->IASetVertexBuffers(0, 1, &mGeo->VertexBufferView());
    mCommandList->IASetIndexBuffer(&mGeo->IndexBufferView());
    mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

    SubmeshGeometry trianglemesh = mGeo->DrawArgs["triangle"];
    mCommandList->DrawIndexedInstanced(trianglemesh.IndexCount, 1, 
        trianglemesh.StartIndexLocation, trianglemesh.BaseVertexLocation, 0);

    SubmeshGeometry boxmesh = mGeo->DrawArgs["box"];
    mCommandList->DrawIndexedInstanced(boxmesh.IndexCount, 1,
        boxmesh.StartIndexLocation, boxmesh.BaseVertexLocation, 0);

    // Indicate a state transition on the resource usage.
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
    ThrowIfFailed(mCommandList->Close());

    // Add the command list to the queue for execution.
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // swap the back and front buffers
    ThrowIfFailed(mSwapChain->Present(0, 0));
    mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    // Wait until frame commands are complete. This waiting is inefficient and is
    // done for simplicity. Later we will show how to organize our rendering code
    // so we do not have to wait per frame.
    FlushCommandQueue();
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        // Update angles based on input to orbit camera around box.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if ((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.005 unit in the scene.
        float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void BoxApp::BuildDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap)));
}

void BoxApp::BuildConstantBuffers()
{
    mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);

    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
    // Offset to the ith object constant buffer in the buffer.
    int boxCBufIndex = 0;
    cbAddress += boxCBufIndex * objCBByteSize;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = cbAddress;
    cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    md3dDevice->CreateConstantBufferView(&cbvDesc, mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void BoxApp::BuildRootSignature()
{
    // Shader programs typically require resources as input (constant buffers,
    // textures, samplers). The root signature defines the resources the shader
    // programs expect. If we think of the shader programs as a function, and 
    // the input resources as function parameters, then the root signature can be
    // thought of as defining the function signature.

    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[1];

    // Create a single descriptor table of CBVs.
    CD3DX12_DESCRIPTOR_RANGE cbvTable;
    cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // create a root signature with a single slot which points to a descriptor range 
    // consisting of a single constant buffer
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(md3dDevice->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&mRootSignature)));
}

void BoxApp::BuildShadersAndInputLayout()
{
    HRESULT hr = S_OK;

    mvsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
    mpsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

    mInputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void BoxApp::BuildBoxGeometry()
{

    // TODO: exercises 也写道 test/d3d 中
    // TODO: 画三角形，四边形，圆
    std::array<Vertex, 11> vertices = {
        // 正立方体，x轴左移2位
        Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),  // 0
        Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),  // 1
        Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),    // 2
        Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),  // 3
        Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),   // 4
        Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }), // 5
        Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),   // 6
        Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) }),// 7
        // 三角形
        Vertex({ XMFLOAT3(2.0f, 1.0f, -2.0f), XMFLOAT4(Colors::Magenta) }), // 0
        Vertex({ XMFLOAT3(2.0f, +3.0f,-2.0f), XMFLOAT4(Colors::Blue) }),    // 1
        Vertex({ XMFLOAT3(+3.0f, +3.0f, -2.0f), XMFLOAT4(Colors::Yellow) }) // 2
    };

    std::array<std::uint16_t, 39> indices = {
        // 正立方体面
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7, // indexCount:36

        /*
        因为 DrawIndexedInstanced 里定义了 BaseVertexLocation,
        trianglemesh.BaseVertexLocation = 8,
        所以索引又从 0 开始来描绘三角形。即这里的 0,1,2 代表 vertices[8],[9],[10]
        */
        // 三角形
        0, 1, 2  // indexCount: 3
    };

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    mGeo = std::make_unique<MeshGeometry>();
    mGeo->Name = "Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &mGeo->VertexBufferCPU));
    CopyMemory(mGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &mGeo->IndexBufferCPU));
    CopyMemory(mGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    mGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, mGeo->VertexBufferUploader);

    mGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, mGeo->IndexBufferUploader);

    mGeo->VertexByteStride = sizeof(Vertex);
    mGeo->VertexBufferByteSize = vbByteSize;
    mGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
    mGeo->IndexBufferByteSize = ibByteSize;

    SubmeshGeometry boxmesh;
    boxmesh.IndexCount = (UINT)36;
    boxmesh.StartIndexLocation = 0;
    boxmesh.BaseVertexLocation = 0;

    SubmeshGeometry trianglemesh;
    trianglemesh.IndexCount = (UINT)3;
    trianglemesh.StartIndexLocation = 36;
    trianglemesh.BaseVertexLocation = 8;


    mGeo->DrawArgs["box"] = boxmesh;
    mGeo->DrawArgs["triangle"] = trianglemesh;
}

void BoxApp::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    psoDesc.pRootSignature = mRootSignature.Get();
    psoDesc.VS = {
        reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
        mvsByteCode->GetBufferSize()
    };
    psoDesc.PS = {
        reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
        mpsByteCode->GetBufferSize()
    };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = mBackBufferFormat;
    psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    psoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}