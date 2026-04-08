#include "pch.h"
#include "hacks_core.h"
#include "hacks_vars.h"

LRESULT OpenHacksCore::OpenHacksReBarProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_WINDOWPOSCHANGING:
    {
        if (mMainMenuWindow != nullptr && OpenHacksVars::ShowMainMenu == false)
        {
            const UINT bandCount = (UINT)SendMessage(wnd, RB_GETBANDCOUNT, 0, 0);
            for (UINT i = 0; i < bandCount; ++i)
            {
                REBARBANDINFO rebarInfo = {};
                rebarInfo.cbSize = sizeof(rebarInfo);
                rebarInfo.fMask = RBBIM_CHILD | RBBIM_STYLE;
                SendMessage(wnd, RB_GETBANDINFO, (WPARAM)i, (LPARAM)&rebarInfo);
                if (mMainMenuWindow == rebarInfo.hwndChild)
                {
                    if (!(rebarInfo.fStyle & RBBS_HIDDEN))
                    {
                        SendMessage(wnd, RB_SHOWBAND, (WPARAM)i, (LPARAM)FALSE);
                    }
                    break;
                }
            }
        }
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
