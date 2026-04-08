#include "pch.h"
#include "hacks_core.h"
#include "hacks_vars.h"
#include "win32_utils.h"

namespace
{
FORCEINLINE void UninstallWindowHook(HHOOK& hook)
{
    if (hook != nullptr)
    {
        UnhookWindowsHookEx(hook);
        hook = nullptr;
    }
}

FORCEINLINE INT HitTestToWMSZ(INT hittest)
{
    switch (hittest)
    {
    case HTLEFT:
        return WMSZ_LEFT;
    case HTTOP:
        return WMSZ_TOP;
    case HTRIGHT:
        return WMSZ_RIGHT;
    case HTBOTTOM:
        return WMSZ_BOTTOM;
    case HTTOPLEFT:
        return WMSZ_TOPLEFT;
    case HTTOPRIGHT:
        return WMSZ_TOPRIGHT;
    case HTBOTTOMLEFT:
        return WMSZ_BOTTOMLEFT;
    case HTBOTTOMRIGHT:
        return WMSZ_BOTTOMRIGHT;
    default:
        break;
    }
    return 0;
}

static HBRUSH g_hBackgroundBrush = nullptr;
static COLORREF g_lastBgColor = CLR_INVALID;

FORCEINLINE void UpdateBackgroundBrush()
{
    COLORREF currentColor = Utility::GetFoobarBackgroundColor();
    if (currentColor != g_lastBgColor)
    {
        if (g_hBackgroundBrush) DeleteObject(g_hBackgroundBrush);
        g_hBackgroundBrush = CreateSolidBrush(currentColor);
        g_lastBgColor = currentColor;
        console::printf("[OpenHacks] Global background brush updated to R=%d G=%d B=%d",
                      GetRValue(currentColor), GetGValue(currentColor), GetBValue(currentColor));
    }
}

} // namespace

bool OpenHacksCore::InstallWindowHooks()
{
    if (InstallWindowHooksInternal())
        return true;

    mInstallHooksWin32Error = GetLastError();
    mInitErrors |= HooksInstallError;
    UninstallWindowHooks();
    return false;
}

bool OpenHacksCore::InstallWindowHooksInternal()
{
    HINSTANCE hmod = core_api::get_my_instance();
    const DWORD threadId = GetCurrentThreadId();

    mCallWndHook = SetWindowsHookExW(WH_CALLWNDPROC, StaticOpenHacksCallWndProc, hmod, threadId);
    if (mCallWndHook == nullptr)
        return false;

    mGetMsgHook = SetWindowsHookExW(WH_GETMESSAGE, StaticOpenHacksGetMessageProc, hmod, threadId);
    if (mGetMsgHook == nullptr)
        return false;

    return true;
}

void OpenHacksCore::UninstallWindowHooks()
{
    UninstallWindowHook(mGetMsgHook);
    UninstallWindowHook(mCallWndHook);
}

LRESULT OpenHacksCore::OpenHacksCallWndProc(int code, WPARAM wp, LPARAM lp)
{
    if (code >= HC_ACTION)
    {
        const auto pcwps = reinterpret_cast<PCWPSTRUCT>(lp);
        switch (pcwps->message)
        {
        case WM_NCCREATE:
        {
            if (mMainWindow == nullptr)
            {
                wchar_t className[MAX_PATH] = {};
                GetClassNameW(pcwps->hwnd, className, ARRAYSIZE(className));
                if (className == kDUIMainWindowClassName)
                {
                    console::printf("[OpenHacks] WM_NCCREATE: Initializing background...");
                    
                    UpdateBackgroundBrush();
                    
                    SetClassLongPtr(pcwps->hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)g_hBackgroundBrush);
                    
                    LONG style = GetWindowLong(pcwps->hwnd, GWL_STYLE);
                    if (!(style & WS_CLIPCHILDREN))
                    {
                        SetWindowLong(pcwps->hwnd, GWL_STYLE, style | WS_CLIPCHILDREN);
                        console::printf("[OpenHacks] WS_CLIPCHILDREN added to prevent background leak");
                    }
                }
            }
            break;
        }

        case WM_CREATE:
        {
            if (mMainWindow == nullptr)
            {
                wchar_t className[MAX_PATH] = {};
                GetClassNameW(pcwps->hwnd, className, ARRAYSIZE(className));
                if (className == kDUIMainWindowClassName)
                {
                    mMainWindow = pcwps->hwnd;
                    mMainWindowOriginProc = (WNDPROC)SetWindowLongPtr(pcwps->hwnd, GWLP_WNDPROC, (LONG_PTR)StaticOpenHacksMainWindowProc);
                    OpenHacksVars::DPI = Utility::GetDPI(mMainMenuWindow);
                    
                    console::printf("[OpenHacks] Window subclassed at WM_CREATE");

                    UpdateBackgroundBrush();
                    SetClassLongPtr(pcwps->hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)g_hBackgroundBrush);

                    RedrawWindow(pcwps->hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
                }
            }
            break;
        }

        default:
            break;
        }
    }

    return CallNextHookEx(mCallWndHook, code, wp, lp);
}

LRESULT OpenHacksCore::OpenHacksGetMessageProc(int code, WPARAM wp, LPARAM lp)
{
    if (code >= HC_ACTION && (UINT)wp == PM_REMOVE)
    {
        auto msg = (LPMSG)(lp);
        if (IsMainOrChildWindow(msg->hwnd))
        {
            switch (msg->message)
            {
            case WM_MOUSEMOVE:
                OnHookMouseMove(msg);
                break;

            case WM_LBUTTONDOWN:
                OnHookLButtonDown(msg);
                break;

            case WM_LBUTTONDBLCLK:
                OnHookLButtonDblClk(msg);
                break;

            default:
                break;
            }
        }
    }

    return CallNextHookEx(mGetMsgHook, code, wp, lp);
}

void OpenHacksCore::OnHookMouseMove(LPMSG msg)
{
    if (OpenHacksVars::MainWindowFrameStyle != WindowFrameStyleNoBorder)
        return;

    GUITHREADINFO threadInfo = {};
    threadInfo.cbSize = sizeof(threadInfo);
    if (GetGUIThreadInfo(GetCurrentThreadId(), &threadInfo))
    {
        if (threadInfo.hwndCapture != nullptr && threadInfo.hwndCapture != mMainWindow)
            return;

        if (threadInfo.flags & (GUI_INMENUMODE | GUI_INMOVESIZE | GUI_POPUPMENUMODE | GUI_SYSTEMMENUMODE))
            return;

        const DWORD messagePos = GetMessagePos();
        const POINT pt = {GET_X_LPARAM(messagePos), GET_Y_LPARAM(messagePos)};
        const Rect rectForNonSizeing = GetRectForNonSizing();
        if (rectForNonSizeing.IsPointIn(pt))
        {
            if (mRequireRevertCursor)
            {
                mRequireRevertCursor = false;
                SendMessage(mMainWindow, WM_SETCURSOR, (WPARAM)mMainWindow, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
            }

            return;
        }

        const int32_t hittest = (int32_t)SendMessage(mMainWindow, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));
        if (hittest != HTCLIENT)
        {
            mRequireRevertCursor = true;
            SendMessage(mMainWindow, WM_SETCURSOR, (WPARAM)mMainWindow, MAKELPARAM(hittest, WM_MOUSEMOVE));
            msg->message = WM_NULL;
        }
    }
}

void OpenHacksCore::OnHookLButtonDown(LPMSG msg)
{
    if (OpenHacksVars::MainWindowFrameStyle == WindowFrameStyleDefault)
        return;

    GUITHREADINFO threadInfo = {};
    threadInfo.cbSize = sizeof(threadInfo);
    if (GetGUIThreadInfo(GetCurrentThreadId(), &threadInfo))
    {
        if (threadInfo.flags & (GUI_INMENUMODE | GUI_POPUPMENUMODE | GUI_SYSTEMMENUMODE))
            return;

        const DWORD messagePos = GetMessagePos();
        const POINT pt = {GET_X_LPARAM(messagePos), GET_Y_LPARAM(messagePos)};

        // Check if click is in pseudo-caption area
        const auto& pseudoCaption = OpenHacksVars::PseudoCaptionSettings.get_value();
        Rect rectPseudoCaption = pseudoCaption.ToRect(mMainWindow);
        if (rectPseudoCaption.IsPointIn(pt))
        {
            // Disable drag when maximized or fullscreen
            if (Utility::IsMaximized(mMainWindow) || mSavedWindowState.has_value() || Utility::IsFullscreen(mMainWindow))
            {
                // Do nothing - disable dragging in maximized/fullscreen state
                return;
            }
            
            // Normal case: start moving the window
            SendMessage(mMainWindow, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
            msg->message = WM_NULL;
            return;
        }

        if (OpenHacksVars::MainWindowFrameStyle == WindowFrameStyleNoBorder)
        {
            // simulate resizing
            const Rect rectForNonSizeing = GetRectForNonSizing();
            if (!rectForNonSizeing.IsPointIn(pt))
            {
                bool isInMoveSize = (threadInfo.flags & GUI_INMOVESIZE) != 0;
                
                if (isInMoveSize) return;
                
                const int32_t hittest = (int32_t)SendMessage(mMainWindow, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));
                if (hittest != HTCLIENT)
                {
                    SendMessage(mMainWindow, WM_SETCURSOR, (WPARAM)mMainWindow, MAKELPARAM(hittest, WM_MOUSEMOVE));
                    SendMessage(mMainWindow, WM_SYSCOMMAND, SC_SIZE | HitTestToWMSZ(hittest), MAKELPARAM(pt.x, pt.y));
                    msg->message = WM_NULL;
                    return;
                }
            }
        }
    }
}

void OpenHacksCore::OnHookLButtonDblClk(LPMSG msg)
{
    if (OpenHacksVars::MainWindowFrameStyle == WindowFrameStyleDefault)
        return;

    GUITHREADINFO threadInfo = {};
    threadInfo.cbSize = sizeof(threadInfo);
    if (GetGUIThreadInfo(GetCurrentThreadId(), &threadInfo))
    {
        if (threadInfo.flags & (GUI_INMENUMODE | GUI_POPUPMENUMODE | GUI_SYSTEMMENUMODE))
            return;

        const DWORD messagePos = GetMessagePos();
        const POINT pt = {GET_X_LPARAM(messagePos), GET_Y_LPARAM(messagePos)};

        // Check if double-click is in pseudo-caption area
        const auto& pseudoCaption = OpenHacksVars::PseudoCaptionSettings.get_value();
        Rect rectPseudoCaption = pseudoCaption.ToRect(mMainWindow);
        if (rectPseudoCaption.IsPointIn(pt))
        {
            // If maximized or has saved state (custom maximize), restore it
            if (Utility::IsMaximized(mMainWindow) || mSavedWindowState.has_value())
            {
                Restore();
                msg->message = WM_NULL;
                return;
            }
            
            // If fullscreen, exit fullscreen
            if (Utility::IsFullscreen(mMainWindow))
            {
                ExitFullscreen();
                msg->message = WM_NULL;
                return;
            }
            
            // Normal case: maximize the window
            Maximize();
            msg->message = WM_NULL;
            return;
        }
    }
}
