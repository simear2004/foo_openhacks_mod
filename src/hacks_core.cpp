#include "pch.h"
#include "hacks_core.h"
#include "hacks_vars.h"
#include "win32_utils.h"

OpenHacksCore& OpenHacksCore::Get()
{
    static OpenHacksCore core;
    return core;
}

void OpenHacksCore::Initialize()
{
    // check init errors.
    if (mInitErrors != NoError)
    {
        pfc::string8_fast errorMessage;
        if (mInitErrors & IncompatibleComponentInstalled)
        {
            errorMessage << "\nOpenHacks is not compatible with UIHacks.";
        }

        if (mInitErrors & HooksInstallError)
        {
            errorMessage << "\nfailed to install windows hook: " << format_win32_error(mInstallHooksWin32Error) << "(0x"
                         << pfc::format_hex(mInstallHooksWin32Error, 8) << ")";
        }

        popup_message_v2::g_complain(core_api::get_main_window(), "OpenHacks init failed", errorMessage);
        return;
    }

    if (HWND window = core_api::get_main_window())
    {
        ApplyMainWindowFrameStyle(static_cast<WindowFrameStyle>((int32_t)OpenHacksVars::MainWindowFrameStyle));

        if (HWND rebarWindow = FindWindowExW(window, nullptr, kDUIRebarWindowClassName.data(), nullptr))
        {
            mRebarWindow = rebarWindow;
            mReBarOriginProc = (WNDPROC)SetWindowLongPtr(mRebarWindow, GWLP_WNDPROC, (LONG_PTR)StaticOpenHacksReBarProc);
            mMainMenuWindow = FindWindowExW(mRebarWindow, nullptr, kDUIMainMenuBandClassName.data(), nullptr);

            if (OpenHacksVars::ShowMainMenu == false)
            {
                ShowOrHideMenuBar(false);
            }
        }

        if (HWND statusBar = FindWindowExW(window, nullptr, kDUIStatusBarClassName.data(), nullptr))
        {
            mStatusBar = statusBar;
            mStatusBarOriginProc = (WNDPROC)SetWindowLongPtr(statusBar, GWLP_WNDPROC, (LONG_PTR)StaticOpenHacksStatusBarProc);
        }

        // always send WM_SIZE in order to update rectangle stat internal.
        SendMessage(window, WM_SIZE, 0, 0);
    }
}

void OpenHacksCore::Finalize()
{
    UninstallWindowHooks();
}

bool OpenHacksCore::IsMainOrChildWindow(HWND wnd)
{
    return wnd == mMainWindow || IsChild(mMainWindow, wnd);
}

POINT OpenHacksCore::GetBorderMetrics()
{
    const int32_t cx = Utility::GetSystemMetricsForDpi(SM_CXFRAME, OpenHacksVars::DPI) + Utility::GetSystemMetricsForDpi(SM_CXPADDEDBORDER, OpenHacksVars::DPI);
    const int32_t cy = Utility::GetSystemMetricsForDpi(SM_CYFRAME, OpenHacksVars::DPI) + Utility::GetSystemMetricsForDpi(SM_CXPADDEDBORDER, OpenHacksVars::DPI);
    return POINT{cx, cy};
}

Rect OpenHacksCore::GetRectForNonSizing()
{
    Rect rect;
    GetWindowRect(mMainWindow, &rect);
    const auto border = GetBorderMetrics();
    return rect.Inflate(-border.x, -border.y);
}

void OpenHacksCore::ToggleStatusBar()
{
    if (mStatusBar != nullptr)
    {
        OpenHacksVars::ToggleShowStatusBar();
        ShowOrHideStatusBar(OpenHacksVars::ShowStatusBar);
    }
}

void OpenHacksCore::ToggleMenuBar()
{
    OpenHacksVars::ToggleShowMainMenu();
    if (!ShowOrHideMenuBar(OpenHacksVars::ShowMainMenu))
        OpenHacksVars::ToggleShowMainMenu();
}

void OpenHacksCore::ShowOrHideStatusBar(bool value)
{
    if (mStatusBar == nullptr)
        return;
    SendMessage(core_api::get_main_window(), WM_SIZE, 0, 0);
}

bool OpenHacksCore::ShowOrHideMenuBar(bool value)
{
    if (mRebarWindow == nullptr || mMainMenuWindow == nullptr)
        return false;

    if (IsMenuBarVisible() == value)
        return true;

    const UINT bandCount = (UINT)SendMessage(mRebarWindow, RB_GETBANDCOUNT, 0, 0);
    for (UINT i = 0; i < bandCount; ++i)
    {
        REBARBANDINFO rebarInfo = {};
        rebarInfo.cbSize = sizeof(rebarInfo);
        rebarInfo.fMask = RBBIM_CHILD;
        SendMessage(mRebarWindow, RB_GETBANDINFO, (WPARAM)i, (LPARAM)&rebarInfo);
        if (mMainMenuWindow == rebarInfo.hwndChild)
        {
            SendMessage(mRebarWindow, RB_SHOWBAND, (WPARAM)i, (LPARAM)value);
            return true;
        }
    }

    return false;
}

bool OpenHacksCore::CheckIncompatibleComponents()
{
    static wstring_view_t incompatibleComponents[] = {
        L"foo_ui_hacks.dll",
    };

    for (const auto& fileName : incompatibleComponents)
    {
        if (GetModuleHandleW(fileName.data()))
        {
            mInitErrors |= IncompatibleComponentInstalled;
            break;
        }
    }

    return mInitErrors == NoError;
}

void OpenHacksCore::ApplyMainWindowFrameStyle(WindowFrameStyle newStyle)
{
    HWND mainWindow = core_api::get_main_window();

    // Check if window is maximized before style change
    WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
    GetWindowPlacement(mainWindow, &wp);
    bool wasMaximized = (wp.showCmd == SW_SHOWMAXIMIZED);

    const LONG currentStyle = static_cast<LONG>(GetWindowLongPtr(mainWindow, GWL_STYLE));
    LONG style = currentStyle;
    switch (newStyle)
    {
    case WindowFrameStyle::Default:
        style |= (WS_CAPTION | WS_THICKFRAME);
        break;

    case WindowFrameStyle::NoCaption:
        style |= WS_THICKFRAME;
        style &= ~(WS_CAPTION);
        break;

    case WindowFrameStyle::NoBorder:
        style &= ~(WS_CAPTION | WS_THICKFRAME);
        break;

    default:
        break;
    }

    if (currentStyle == style)
        return;

    SetWindowLongPtr(mainWindow, GWL_STYLE, style);

    if (newStyle == WindowFrameStyle::NoBorder)
        Utility::EnableWindowShadow(mainWindow, true);

    // notify frame changes
    SetWindowPos(mainWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    // Fix: Force fullscreen for maximized NoBorder windows to cover taskbar
    if (wasMaximized && newStyle == WindowFrameStyle::NoBorder)
    {
        if (HMONITOR monitor = MonitorFromWindow(mainWindow, MONITOR_DEFAULTTONEAREST))
        {
            MONITORINFO mi = { sizeof(MONITORINFO) };
            if (GetMonitorInfo(monitor, &mi))
            {
                SetWindowPos(mainWindow, HWND_TOP,
                    mi.rcMonitor.left, mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
    }
}
