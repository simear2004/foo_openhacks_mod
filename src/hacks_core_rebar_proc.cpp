#include "pch.h"
#include "hacks_core.h"
#include "hacks_vars.h"

LRESULT OpenHacksCore::OpenHacksReBarProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case RB_SHOWBAND:
    {
        if (mMainMenuWindow == nullptr || OpenHacksVars::ShowMainMenu == true)
            break;
        REBARBANDINFO rebarInfo = {};
        rebarInfo.cbSize = sizeof(rebarInfo);
        rebarInfo.fMask = RBBIM_CHILD;
        SendMessage(wnd, RB_GETBANDINFO, wp, (LPARAM)&rebarInfo);
        if (mMainMenuWindow == rebarInfo.hwndChild)
            lp = 0; // alter show flag anyway
        break;
    }

    case WM_NCDESTROY:
        SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)mReBarOriginProc);
        break;

    default:
        break;
    }

    return CallWindowProc(mReBarOriginProc, wnd, msg, wp, lp);
}
