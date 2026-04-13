#include "pch.h"
#include "hacks_core.h"
#include "hacks_vars.h"

LRESULT OpenHacksCore::OpenHacksStatusBarProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_NCDESTROY:
        SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)mStatusBarOriginProc);
        break;

    case WM_WINDOWPOSCHANGING:
        if (auto wpos = (PWINDOWPOS)lp)
        {
            if (OpenHacksVars::ShowStatusBar == false)
            {
                wpos->cy = 0;
                return 0;
            }
        }
        break;

    default:
        break;
    }

    return CallWindowProc(mStatusBarOriginProc, wnd, msg, wp, lp);
}