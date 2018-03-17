// box.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "box.h"

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_     HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_     LPWSTR    lpCmdLine,
                      _In_     int       nCmdShow)
{
    static TCHAR szAppName[] = L"NoPopUps";
    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass;

    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, L"This program requires Windows NT!", 
                   szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName, L"The Poor-Person's Menu",
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HMENU hMenuMain, hMenuEdit, hMenuFile;
    HINSTANCE hInstance;
    switch (message)
    {
    case WM_CREATE:
        hInstance = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

        hMenuMain = LoadMenu(hInstance, L"MenuMain");
        hMenuFile = LoadMenu(hInstance, L"MenuFile");
        hMenuEdit = LoadMenu(hInstance, L"MenuEdit");

        SetMenu(hwnd, hMenuMain);
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_MAIN:
            SetMenu(hwnd, hMenuMain);
            return 0;
        case IDM_FILE:
            SetMenu(hwnd, hMenuFile);
            return 0;
        case IDM_EDIT:
            SetMenu(hwnd, hMenuEdit);
            return 0;
        case IDM_FILE_NEW:
        case IDM_FILE_OPEN:
        case IDM_FILE_SAVE:
        case IDM_FILE_SAVE_AS:
        case IDM_EDIT_UNDO:
        case IDM_EDIT_CUT:
        case IDM_EDIT_COPY:
        case IDM_EDIT_PASTE:
        case IDM_EDIT_CLEAR:
            MessageBeep(0);
            return 0;
        }
        break;
    case WM_DESTROY:
        SetMenu(hwnd, hMenuMain);
        DestroyMenu(hMenuFile);
        DestroyMenu(hMenuEdit);

        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
