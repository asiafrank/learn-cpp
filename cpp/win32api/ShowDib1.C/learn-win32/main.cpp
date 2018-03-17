// box.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "main.h"
#include "dibfile.h"

PCWSTR szAppName = L"ShowDib1";

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(_In_     HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_     LPWSTR    lpCmdLine,
                      _In_     int       nCmdShow)
{
    HACCEL        hAccel;
    HWND          hwnd;
    MSG           msg;
    WNDCLASS      wndclass;

    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName  = szAppName;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, L"This program requires Windows NT!",
                   szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName, L"DIB Headers",
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, 
                        CW_USEDEFAULT, CW_USEDEFAULT, 
                        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    hAccel = LoadAccelerators(hInstance, szAppName);
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(hwnd, hAccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BITMAPFILEHEADER *pbmfh;
    static BITMAPINFO       *pbmi;
    static BYTE             *pBits;
    static int              cxClient, cyClient, cxDib, cyDib;
    static TCHAR            szFileName[MAX_PATH], szTitleName[MAX_PATH];
    BOOL                    bSuccess;
    HDC                     hdc;
    PAINTSTRUCT             ps;

    switch (message)
    {
    case WM_CREATE:
        DibFileInitialize(hwnd);
        return 0;
    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        return 0;
    case WM_INITMENUPOPUP:
        EnableMenuItem((HMENU)wParam, IDM_FILE_SAVE, pbmfh ? MF_ENABLED : MF_GRAYED);
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_FILE_OPEN:
            // Show the File Open dialog box
            if (!DibFileOpenDlg(hwnd, szFileName, szTitleName))
                return 0;

            // If there's an existing DIB, free the memory
            if (pbmfh)
            {
                free(pbmfh);
                pbmfh = NULL;
            }

            // Load the entire DIB int memory
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            ShowCursor(TRUE);

            pbmfh = DibLoadImage(szFileName);
            ShowCursor(FALSE);
            SetCursor(LoadCursor(NULL, IDC_ARROW));

            // Invalidate the client area for later update
            InvalidateRect(hwnd, NULL, TRUE);

            if (pbmfh == NULL)
            {
                MessageBox(hwnd, L"Cannot load DIB file", szAppName, 0);
                return 0;
            }

            // Get pointers to the info structure & the bits
            pbmi  = (BITMAPINFO*)(pbmfh + 1);
            pBits = (BYTE*)pbmfh + pbmfh->bfOffBits;

            // Get the DIB width and height
            if (pbmi->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
            {
                cxDib = ((BITMAPCOREHEADER*)pbmi)->bcWidth;
                cyDib = ((BITMAPCOREHEADER*)pbmi)->bcHeight;
            }
            else
            {
                cxDib = pbmi->bmiHeader.biWidth;
                cyDib = abs(pbmi->bmiHeader.biHeight);
            }
            return 0;
        case IDM_FILE_SAVE:
            // Show the File Sae dialog box
            if (!DibFileSaveDlg(hwnd, szFileName, szTitleName))
                return 0;

            // Save the DIB to memory
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            ShowCursor(TRUE);

            bSuccess = DibSaveImage(szFileName, pbmfh);
            ShowCursor(FALSE);
            SetCursor(LoadCursor(NULL, IDC_ARROW));

            if (!bSuccess)
                MessageBox(hwnd, L"Cannot save DIB file", szAppName, 0);
            return 0;
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        if (pbmfh)
            SetDIBitsToDevice(hdc, 
                0,      // xDst
                0,      // yDst
                cxDib,  // cxSrc
                cyDib,  // cySrc
                0,      // xSrc
                0,      // ySrc
                0,      // first scan line
                cyDib,  // number of scan lines
                pBits, 
                pbmi, 
                DIB_RGB_COLORS);
        EndPaint(hwnd, &ps);
        return 0;
    case WM_DESTROY:
        if (pbmfh)
            free(pbmfh);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}