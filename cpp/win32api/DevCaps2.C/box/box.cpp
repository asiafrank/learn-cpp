// box.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "box.h"
#include <winspool.h>

#define IDM_DEVMODE 1000

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DoBasicInfo(HDC, HDC, int, int);
void DoOtherInfo(HDC, HDC, int, int);
void DoBitCodedCaps(HDC, HDC, int, int, int);

struct BITS
{
    int iMask;
    PCWCH szDesc;
};

int APIENTRY wWinMain(_In_     HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_     LPWSTR    lpCmdLine,
                      _In_     int       nCmdShow)
{
    static TCHAR szAppName[] = L"DevCaps2";
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
    wndclass.lpszMenuName  = szAppName;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, L"This program requires Windows NT!", 
                   szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName, NULL,
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static TCHAR szDevice[32], szWindowText[64];
    static int cxChar, cyChar, nCurrentDevice = IDM_SCREEN,
           nCurrentInfo = IDM_BASIC;
    static DWORD dwNeeded, dwReturned;
    static PRINTER_INFO_4 *pinfo4;
    static PRINTER_INFO_5 *pinfo5;
    DWORD i;
    HDC hdc, hdcInfo;
    HMENU hMenu;
    HANDLE hPrint;
    PAINTSTRUCT ps;
    TEXTMETRIC tm;

    switch (message)
    {
    case WM_CREATE:
        hdc = GetDC(hwnd);
        SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
        GetTextMetrics(hdc, &tm);
        cxChar = tm.tmAveCharWidth;
        cyChar = tm.tmHeight + tm.tmExternalLeading;
        ReleaseDC(hwnd, hdc);
        // fall through
    case WM_SETTINGCHANGE:
        hMenu = GetSubMenu(GetMenu(hwnd), 0);

        while (GetMenuItemCount(hMenu) > 1)
        {
            DeleteMenu(hMenu, 1, MF_BYPOSITION);
        }

        // Get a list of all local and remote printers
        // 
        // First, find out how large an array we need; this
        // call will fail, leaving the required size in dwNeeded
        //
        // Next, allocate space for the info array and fill it
        // Put the printer names on the menu

        // GetVersion is deprecated, so remove the if else statement
        EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, NULL, 
                        0, &dwNeeded, &dwReturned);
        pinfo4 = (PRINTER_INFO_4*)malloc(dwNeeded);

        EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, (PBYTE)pinfo4, 
                        dwNeeded, &dwNeeded, &dwReturned);

        for (i = 0; i < dwReturned; i++)
        {
            AppendMenu(hMenu, (i+1) % 16 ? 0 : MF_MENUBARBREAK, i + 1, 
                       pinfo4[i].pPrinterName);
        }
        free(pinfo4);

        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenu, 0, IDM_DEVMODE, L"Properties");

        wParam = IDM_SCREEN;
        // fall through
    case WM_COMMAND:
        hMenu = GetMenu(hwnd);

        if (LOWORD(wParam) == IDM_SCREEN || // IDM_SCREEN & Printers
            LOWORD(wParam) < IDM_DEVMODE)
        {
            CheckMenuItem(hMenu, nCurrentDevice, MF_UNCHECKED);
            nCurrentDevice = LOWORD(wParam);
            CheckMenuItem(hMenu, nCurrentDevice, MF_CHECKED);
        }
        else if (LOWORD(wParam) == IDM_DEVMODE) // Properties selection
        {
            GetMenuString(hMenu, nCurrentDevice, szDevice, 
                          sizeof(szDevice) / sizeof(TCHAR), MF_BYCOMMAND);

            if (OpenPrinter(szDevice, &hPrint, NULL))
            {
                PrinterProperties(hwnd, hPrint);
                ClosePrinter(hPrint);
            }
        }
        else // info menu items
        {
            CheckMenuItem(hMenu, nCurrentInfo, MF_UNCHECKED);
            nCurrentInfo = LOWORD(wParam);
            CheckMenuItem(hMenu, nCurrentInfo, MF_CHECKED);
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    case WM_INITMENUPOPUP:
        if (lParam == 0)
            EnableMenuItem(GetMenu(hwnd), IDM_DEVMODE, 
                           nCurrentDevice == IDM_SCREEN 
                           ? MF_GRAYED 
                           : MF_ENABLED);
        return 0;
    case WM_PAINT:
        lstrcpy(szWindowText, L"Device Capabilities: ");

        if (nCurrentDevice == IDM_SCREEN)
        {
            lstrcpy(szDevice, L"DISPLAY");
            hdcInfo = CreateIC(szDevice, NULL, NULL, NULL);
        }
        else
        {
            hMenu = GetMenu(hwnd);
            GetMenuString(hMenu, nCurrentDevice, szDevice, sizeof(szDevice), MF_BYCOMMAND);
            hdcInfo = CreateIC(NULL, szDevice, NULL, NULL);
        }

        lstrcat(szWindowText, szDevice);
        SetWindowText(hwnd, szWindowText);

        hdc = BeginPaint(hwnd, &ps);
        SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));

        if (hdcInfo)
        {
            switch (nCurrentInfo)
            {
            case IDM_BASIC:
                DoBasicInfo(hdc, hdcInfo, cxChar, cyChar);
                break;
            case IDM_OTHER:
                DoOtherInfo(hdc, hdcInfo, cxChar, cyChar);
                break;
            case IDM_CURVE:
            case IDM_LINE:
            case IDM_POLY:
            case IDM_TEXT:
                DoBitCodedCaps(hdc, hdcInfo, cxChar, cyChar, nCurrentInfo - IDM_CURVE);
                break;
            }
            DeleteDC(hdcInfo);
        }
        EndPaint(hwnd, &ps);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void DoBasicInfo(HDC hdc, HDC hdcInfo, int cxChar, int cyChar)
{
    static struct
    {
        int nIndex;
        PCWCH szDesc;
    } info[] =
    {
        {HORZSIZE,        L"HORZSIZE Width in millimeters:"},
        {VERTSIZE,        L"VERTSIZE Height in millimeters:"},
        {HORZRES,         L"HORZRES Width in pixels:"},
        {VERTRES,         L"VERTRES Height in raster lines:"},
        {BITSPIXEL,       L"BITSPIXEL Color bits per pixel:"},
        {PLANES,          L"PLANES Number of color planes:"},
        {NUMBRUSHES,      L"NUMBRUSHES Number of device brushes:"},
        {NUMPENS,         L"NUMPENS Number of device pens:"},
        {NUMMARKERS,      L"NUMMARKERS Number of device markers:"},
        {NUMFONTS,        L"NUMFONTS Number of device fonts:"},
        {NUMCOLORS,       L"NUMCOLORS Number of device colors:"},
        {PDEVICESIZE,     L"PDEVICESIZE size of device structure:"},
        {ASPECTX,         L"ASPECTX Relative width of pixel:"},
        {ASPECTY,         L"ASPECTY Relative height of pixel:"},
        {ASPECTXY,        L"ASPECTXY Relative height of pixel:"},
        {LOGPIXELSX,      L"LOGPIXELSX Horizontal dots per inch:"},
        {LOGPIXELSY,      L"LOGPIXELSY Vertical dots per inch:"},
        {SIZEPALETTE,     L"SIZEPALETTE Number of palette entries:"},
        {COLORRES,        L"COLORRES Actual color resolution:"},
        {PHYSICALWIDTH,   L"PHYSICALWIDTH Printer page pixel width:"},
        {PHYSICALHEIGHT,  L"PHYSICALHEIGHT Printer page pixel height:"},
        {PHYSICALOFFSETX, L"PHYSICALOFFSETX Printer page x offset:"},
        {PHYSICALOFFSETY, L"PHYSICALOFFSETY Printer page y offset:"}
    };
    int i;
    TCHAR szBuffer[80];

    for (i = 0; i < sizeof(info) / sizeof(info[0]); i++)
    {
        TextOut(hdc, cxChar, (i + 1) * cyChar, szBuffer, 
                wsprintf(szBuffer, L"%-45s%8d", info[i].szDesc, 
                         GetDeviceCaps(hdcInfo, info[i].nIndex)));
    }
}

void DoOtherInfo(HDC hdc, HDC hdcInfo, int cxChar, int cyChar)
{
    static BITS clip[] =
    {
        CP_RECTANGLE, L"CP_RECTANGLE Can Clip To Rectangle:"
    };

    static BITS raster[] =
    {
        {RC_BITBLT,       L"RC_BITBLT Capable of simple BitBlt:"},
        {RC_BANDING,      L"RC_BANDING Requires handing support:"},
        {RC_SCALING,      L"RC_SCALING Requires scaling support:"},
        {RC_BITMAP64,     L"RC_BITMAP64 Supports bitmaps >64K:"},
        {RC_GDI20_OUTPUT, L"RC_GDI20_OUTPUT Has 2.0 output calls:"},
        {RC_DI_BITMAP,    L"RC_DI_BITMAP Supports a palette:"},
        {RC_PALETTE,      L"RC_PALETTE Supports a palette:"},
        {RC_DIBTODEV,     L"RC_DIBTODEV Supports bitmap conversion:"},
        {RC_BIGFONT,      L"RC_BIGFONT Supports fonts >64K:"},
        {RC_STRETCHBLT,   L"RC_STRETCHBLT Supports StretchBlt:"},
        {RC_FLOODFILL,    L"RC_FLOODFILL Supports FloodFill:"},
        {RC_STRETCHDIB,   L"RC_STRETCHDIB Supports StretchDIBits:"}
    };

    static PCWCH szTech[] =
    {
        L"DT_PLOTTER (Vector plotter)",
        L"DT_RASDISPLAY (Raster display)",
        L"DT_RASPRINTER (Raster printer)",
        L"DT_RASCAMERA (Raster camera)",
        L"DT_CHARSTREAM (Character stream)",
        L"DT_METAFILE (Metafile)",
        L"DT_DISPFILE (Display file)"
    };

    int i;
    TCHAR szBuffer[80];

    TextOut(hdc, cxChar, cyChar, szBuffer, 
            wsprintf(szBuffer, L"%-24s%04XH", 
                     L"DRIVERVERSION:", GetDeviceCaps(hdcInfo, DRIVERVERSION)));

    TextOut(hdc, cxChar, 2 * cyChar, szBuffer, 
            wsprintf(szBuffer, L"%-24s%-40s", 
                     L"TECHNOLOGY:", szTech[GetDeviceCaps(hdcInfo, TECHNOLOGY)]));

    TextOut(hdc, cxChar, 4 * cyChar, szBuffer, 
            wsprintf(szBuffer, L"CLIPCAPS (Clipping capabilities)"));

    for (i = 0; i < sizeof(clip) / sizeof(clip[0]); i++)
    {
        TextOut(hdc, 9 * cxChar, (i + 6) * cyChar, szBuffer, 
                wsprintf(szBuffer, L"%-45s %3s", clip[i].szDesc, 
                         GetDeviceCaps(hdcInfo, CLIPCAPS) & clip[i].iMask 
                             ? L"Yes" 
                             : L"No"));
    }

    TextOut(hdc, cxChar, 8 * cyChar, szBuffer, 
            wsprintf(szBuffer, L"RASTERCAPS (Raster capabilities)"));

    for (i = 0; i < sizeof(raster) / sizeof(raster[0]); i++)
    {
        TextOut(hdc, 9 * cxChar, (i + 10) * cyChar, szBuffer, 
                wsprintf(szBuffer, L"%-45s %3s", raster[i].szDesc, 
                         GetDeviceCaps(hdcInfo, RASTERCAPS) & raster[i].iMask 
                             ? L"Yes" 
                             : L"No"));
    }
}

void DoBitCodedCaps(HDC hdc, HDC hdcInfo, int cxChar, int cyChar, int iType)
{
    static BITS curves[] =
    {
        CC_CIRCLES,    L"CC_CIRCLES Can do circles:", 
        CC_PIE,        L"CC_PIE Can do pie wedges:",
        CC_CHORD,      L"CC_CHORD Can do chord arcs:",
        CC_ELLIPSES,   L"CC_ELLIPSES Can do ellipses:",
        CC_WIDE,       L"CC_WIDE Can do wide borders:",
        CC_STYLED,     L"CC_STYLED Can do styled borders:",
        CC_WIDESTYLED, L"CC_WIDESTYLED Can do wide and styled borders:",
        CC_INTERIORS,  L"CC_INTERIORS Can do interiors:"
    };

    static BITS lines[] =
    {
        LC_POLYLINE,   L"LC_POLYLINE Can do polyline:",
        LC_MARKER,     L"LC_MARKER Can do markers:",
        LC_POLYMARKER, L"LC_POLYMARKER Can do polymarkers:",
        LC_WIDE,       L"LC_WIDE Can do wide lines:",
        LC_STYLED,     L"LC_STYLED Can do styled lines:",
        LC_WIDESTYLED, L"LC_WIDESTYLED Can do wide and styled lines:",
        LC_INTERIORS,  L"LC_INTERIORS Can do interiors:"
    };

    static BITS poly[] =
    {
        PC_POLYGON,     L"PC_POLYGON Can do alternate fill polygon:",
        PC_RECTANGLE,   L"PC_RECTANGLE Can do rectangle:",
        PC_WINDPOLYGON, L"PC_WINDPOLYGON Can do winding number fill polygon:",
        PC_SCANLINE,    L"PC_SCANLINE Can do scanlines:",
        PC_WIDE,        L"PC_WIDE Can do wide borders:",
        PC_STYLED,      L"PC_STYLED Can do styled borders:",
        PC_WIDESTYLED,  L"PC_WIDESTYLED Can do wide and styled borders:",
        PC_INTERIORS,   L"PC_INTERIORS Can do interiors:"
    };

    static BITS text[] =
    {
        TC_OP_CHARACTER, L"TC_OP_CHARACTER Can do character output precision:",
        TC_OP_STROKE,    L"TC_OP_STROKE Can do stroke output precision:",
        TC_CP_STROKE,    L"TC_CP_STROKE Can do stroke clip precision:",
        TC_CR_90,        L"TC_CP_90 Can do 90 degree character rotation:",
        TC_CR_ANY,       L"TC_CR_ANY Can do any character rotation:",
        TC_SF_X_YINDEP,  L"TC_SF_X_YINDEP Can do scaling independent of X and Y:",
        TC_SA_DOUBLE,    L"TC_SA_DOUBLE Can do doubled character for scaling:",
        TC_SA_INTEGER,   L"TC_SA_INTEGER Can do integer multiples for scaling:",
        TC_SA_CONTIN,    L"TC_SA_CONTIN Can do any multiples for exact scaling:",
        TC_EA_DOUBLE,    L"TC_EA_DOUBLE Can do double weight characters:",
        TC_IA_ABLE,      L"TC_IA_ABLE Can do italicizing:",
        TC_UA_ABLE,      L"TC_UA_ABLE Can do underlining:",
        TC_SO_ABLE,      L"TC_SO_ABLE Can do strikeouts:",
        TC_RA_ABLE,      L"TC_RA_ABLE Can do raster fonts:",
        TC_VA_ABLE,      L"TC_VA_ABLE Can do vector fonts:"
    };

    static struct
    {
        int iIndex;
        PCWCH szTitle;
        BITS (*pbits)[];
        int iSize;
    } bitinfo[] =
    {
        CURVECAPS,     L"CURVCAPS (Curve Capabilities)",
            (BITS (*)[]) curves, sizeof(curves) / sizeof(curves[0]),
        LINECAPS,      L"LINECAPS (Line Capabilities)",
            (BITS (*)[]) lines,  sizeof(lines) / sizeof(lines[0]),
        POLYGONALCAPS, L"POLYGONALCAPS (Polygonal Capabilities)",
            (BITS (*)[]) poly,   sizeof(poly) / sizeof(poly[0]),
        TEXTCAPS,      L"TEXTCAPS (Text Capailites)",
            (BITS (*)[]) text,   sizeof(text) / sizeof(text[0])
    };

    static TCHAR szBuffer[80];
    BITS (*pbits)[] = bitinfo[iType].pbits;
    int i, iDevCaps = GetDeviceCaps(hdcInfo, bitinfo[iType].iIndex);

    TextOut(hdc, cxChar, cyChar, 
            bitinfo[iType].szTitle, lstrlen(bitinfo[iType].szTitle));
    for (i = 0; i < bitinfo[iType].iSize; i++)
    {
        TextOut(hdc, cxChar, (i + 3) * cyChar, szBuffer, 
                wsprintf(szBuffer, L"%-55s %3s", 
                         (*pbits)[i].szDesc, 
                         iDevCaps & (*pbits)[i].iMask 
                             ? L"Yes" 
                             : L"No"));
    }

}