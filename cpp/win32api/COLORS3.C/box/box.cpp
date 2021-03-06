// box.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "box.h"
#include <commdlg.h>

int APIENTRY wWinMain(_In_     HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_     LPWSTR    lpCmdLine,
                      _In_     int       nCmdShow)
{
    static CHOOSECOLOR cc;
    static COLORREF crCustColors[16];
    cc.lStructSize    = sizeof(CHOOSECOLOR);
    cc.hwndOwner      = NULL;
    cc.hInstance      = NULL;
    cc.rgbResult      = RGB(0x80, 0x80, 0x80);
    cc.lpCustColors   = crCustColors;
    cc.Flags          = CC_RGBINIT | CC_FULLOPEN;
    cc.lCustData      = 0;
    cc.lpfnHook       = NULL;
    cc.lpTemplateName = NULL;

    return ChooseColor(&cc);
}
