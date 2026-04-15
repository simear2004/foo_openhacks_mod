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
            errorMessage << "\nOpenHacksMod is not compatible with UIHacks.";
        }

        if (mInitErrors & HooksInstallError)
        {
            errorMessage << "\nfailed to install windows hook: " << format_win32_error(mInstallHooksWin32Error) << "(0x"
                         << pfc::format_hex(mInstallHooksWin32Error, 8) << ")";
        }

        popup_message_v2::g_complain(core_api::get_main_window(), "OpenHacksMod init failed", errorMessage);
        return;
    }

    if (HWND window = core_api::get_main_window())
    {
        // Restore saved window state from persistent storage
        auto& savedWindowData = OpenHacksVars::SavedWindowState.get_value();
        if (savedWindowData.wp.rcNormalPosition.right > savedWindowData.wp.rcNormalPosition.left &&
            savedWindowData.wp.rcNormalPosition.bottom > savedWindowData.wp.rcNormalPosition.top)
        {
            mSavedWindowState = savedWindowData.ToWindowState();
        }

        if (mSavedWindowState.has_value() && mSavedWindowState->fullscreen)
        {
            WindowState state = {};
            Utility::EnterFullscreen(window, state);
        }
        else
        {
            auto newStyle = static_cast<WindowFrameStyle>((int32_t)OpenHacksVars::MainWindowFrameStyle);
            if (mSavedWindowState.has_value() && (newStyle == WindowFrameStyleNoCaption))
                newStyle = WindowFrameStyleNoBorder;

            ApplyMainWindowFrameStyle(newStyle);
        }

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

        if (mUsedCompositedStyle)
        {
            LONG exStyle = GetWindowLong(window, GWL_EXSTYLE);
            SetWindowLong(window, GWL_EXSTYLE, exStyle & ~(WS_EX_COMPOSITED | WS_EX_LAYERED));
            mUsedCompositedStyle = false;
            
            RedrawWindow(window, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        }
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
    const int32_t cxFrame = Utility::GetSystemMetricsForDpi(SM_CXFRAME, OpenHacksVars::DPI);
    const int32_t cyFrame = Utility::GetSystemMetricsForDpi(SM_CYFRAME, OpenHacksVars::DPI);
    const int32_t cxPadding = Utility::GetSystemMetricsForDpi(SM_CXPADDEDBORDER, OpenHacksVars::DPI);
    
    int32_t cx = cxFrame;
    int32_t cy = cyFrame;

    if (OpenHacksVars::MainWindowFrameStyle == WindowFrameStyleNoCaption)
    {
        cx = cxFrame + cxPadding;
        cy = cyFrame + cxPadding;
    }

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
    Utility::ApplyWindowFrameStyle(mainWindow, newStyle);
    
    // Handle shadow for custom styles
    if (newStyle != WindowFrameStyleDefault)
    {
        // Check if window is maximized or in custom maximize state
        const bool isMaximized = Utility::IsMaximized(mainWindow) || mSavedWindowState.has_value();
        
        // If maximized, disable shadow to match Windows behavior; otherwise enable it
        Utility::EnableWindowShadow(mainWindow, !isMaximized);
    }
}

void OpenHacksCore::Maximize()
{
    HWND mainWindow = core_api::get_main_window();
    
    console::printf("=== Maximize() START ===");
    
    if (!mSavedWindowState.has_value())
    {
        console::printf("Maximize: mSavedWindowState was empty, creating new state");
        mSavedWindowState.emplace();
    }
    else
    {
        console::printf("Maximize: mSavedWindowState already exists");
    }
    
    console::printf("Maximize: Before Utility::Maximize - wp.rcNormalPosition: left=%d, top=%d, right=%d, bottom=%d (width=%d, height=%d)",
        mSavedWindowState->wp.rcNormalPosition.left,
        mSavedWindowState->wp.rcNormalPosition.top,
        mSavedWindowState->wp.rcNormalPosition.right,
        mSavedWindowState->wp.rcNormalPosition.bottom,
        mSavedWindowState->wp.rcNormalPosition.right - mSavedWindowState->wp.rcNormalPosition.left,
        mSavedWindowState->wp.rcNormalPosition.bottom - mSavedWindowState->wp.rcNormalPosition.top);
    
    Utility::Maximize(mainWindow, mSavedWindowState.value());
    
    console::printf("Maximize: After Utility::Maximize - wp.rcNormalPosition: left=%d, top=%d, right=%d, bottom=%d (width=%d, height=%d)",
        mSavedWindowState->wp.rcNormalPosition.left,
        mSavedWindowState->wp.rcNormalPosition.top,
        mSavedWindowState->wp.rcNormalPosition.right,
        mSavedWindowState->wp.rcNormalPosition.bottom,
        mSavedWindowState->wp.rcNormalPosition.right - mSavedWindowState->wp.rcNormalPosition.left,
        mSavedWindowState->wp.rcNormalPosition.bottom - mSavedWindowState->wp.rcNormalPosition.top);
    
    OpenHacksVars::SavedWindowState.get_value().FromWindowState(mSavedWindowState.value());
    
    console::printf("=== Maximize() END ===");
}

void OpenHacksCore::Restore()
{
    HWND mainWindow = core_api::get_main_window();
    
    console::printf("=== Restore() START ===");
    
    if (Utility::IsMinimized(mainWindow))
    {
        console::printf("Restore: Window is minimized, calling ShowWindow(SW_RESTORE)");
        ShowWindow(mainWindow, SW_RESTORE);
    }
    else if (mSavedWindowState.has_value())
    {
        console::printf("Restore: mSavedWindowState exists, calling Utility::Restore");
        console::printf("Restore: Saved wp.rcNormalPosition: left=%d, top=%d, right=%d, bottom=%d (width=%d, height=%d)",
            mSavedWindowState->wp.rcNormalPosition.left,
            mSavedWindowState->wp.rcNormalPosition.top,
            mSavedWindowState->wp.rcNormalPosition.right,
            mSavedWindowState->wp.rcNormalPosition.bottom,
            mSavedWindowState->wp.rcNormalPosition.right - mSavedWindowState->wp.rcNormalPosition.left,
            mSavedWindowState->wp.rcNormalPosition.bottom - mSavedWindowState->wp.rcNormalPosition.top);
        
        RECT beforeRect;
        GetWindowRect(mainWindow, &beforeRect);
        console::printf("Restore: Before restore - window rect: left=%d, top=%d, right=%d, bottom=%d (width=%d, height=%d)",
            beforeRect.left, beforeRect.top, beforeRect.right, beforeRect.bottom,
            beforeRect.right - beforeRect.left, beforeRect.bottom - beforeRect.top);
        
        Utility::Restore(mainWindow, mSavedWindowState.value());
        
        RECT afterRect;
        GetWindowRect(mainWindow, &afterRect);
        console::printf("Restore: After restore - window rect: left=%d, top=%d, right=%d, bottom=%d (width=%d, height=%d)",
            afterRect.left, afterRect.top, afterRect.right, afterRect.bottom,
            afterRect.right - afterRect.left, afterRect.bottom - afterRect.top);
        
        mSavedWindowState.reset();
        OpenHacksVars::SavedWindowState.get_value() = WindowStateData();
    }
    else
    {
        console::printf("Restore: mSavedWindowState is empty, calling ShowWindow(SW_RESTORE)");
        ShowWindow(mainWindow, SW_RESTORE);
    }
    
    console::printf("=== Restore() END ===");
}

bool OpenHacksCore::IsMaximized()
{
    return mSavedWindowState.has_value() || Utility::IsMaximized(core_api::get_main_window());
}

bool OpenHacksCore::IsMinimized()
{
    return Utility::IsMinimized(core_api::get_main_window());
}

void OpenHacksCore::EnterFullscreen()
{
    HWND mainWindow = core_api::get_main_window();
    
    console::printf("=== EnterFullscreen() START ===");
    
    if (!mSavedWindowState.has_value())
    {
        console::printf("EnterFullscreen: Creating new state");
        WindowState newState;
        newState.style = static_cast<DWORD>(GetWindowLongPtr(mainWindow, GWL_STYLE));
        GetWindowPlacement(mainWindow, &newState.wp);
        mSavedWindowState = newState;
        
        console::printf("EnterFullscreen: Created - wp.rcNormalPosition: left=%d, top=%d, right=%d, bottom=%d",
            mSavedWindowState->wp.rcNormalPosition.left,
            mSavedWindowState->wp.rcNormalPosition.top,
            mSavedWindowState->wp.rcNormalPosition.right,
            mSavedWindowState->wp.rcNormalPosition.bottom);
    }
    else
    {
        console::printf("EnterFullscreen: Preserving existing wp");
        console::printf("EnterFullscreen: Existing wp.rcNormalPosition: left=%d, top=%d, right=%d, bottom=%d",
            mSavedWindowState->wp.rcNormalPosition.left,
            mSavedWindowState->wp.rcNormalPosition.top,
            mSavedWindowState->wp.rcNormalPosition.right,
            mSavedWindowState->wp.rcNormalPosition.bottom);
    }
    
    mSavedWindowState->fullscreen = true;
    
    console::printf("EnterFullscreen: Before Utility::EnterFullscreen - wp.rcNormalPosition: left=%d, top=%d, right=%d, bottom=%d",
        mSavedWindowState->wp.rcNormalPosition.left,
        mSavedWindowState->wp.rcNormalPosition.top,
        mSavedWindowState->wp.rcNormalPosition.right,
        mSavedWindowState->wp.rcNormalPosition.bottom);

    Utility::EnterFullscreen(mainWindow, mSavedWindowState.value());
    
    console::printf("EnterFullscreen: After Utility::EnterFullscreen - wp.rcNormalPosition: left=%d, top=%d, right=%d, bottom=%d",
        mSavedWindowState->wp.rcNormalPosition.left,
        mSavedWindowState->wp.rcNormalPosition.top,
        mSavedWindowState->wp.rcNormalPosition.right,
        mSavedWindowState->wp.rcNormalPosition.bottom);

    OpenHacksVars::SavedWindowState.get_value().FromWindowState(mSavedWindowState.value());
    
    console::printf("EnterFullscreen: Final - fullscreen=%s, wp.rcNormalPosition: left=%d, top=%d, right=%d, bottom=%d",
        mSavedWindowState->fullscreen ? "true" : "false",
        mSavedWindowState->wp.rcNormalPosition.left,
        mSavedWindowState->wp.rcNormalPosition.top,
        mSavedWindowState->wp.rcNormalPosition.right,
        mSavedWindowState->wp.rcNormalPosition.bottom);
    
    console::printf("=== EnterFullscreen() END ===");
}

void OpenHacksCore::ExitFullscreen()
{
    HWND mainWindow = core_api::get_main_window();
    
    console::printf("=== ExitFullscreen() START ===");
    
    if (mSavedWindowState.has_value())
    {
        console::printf("ExitFullscreen: mSavedWindowState exists");
        console::printf("ExitFullscreen: Saved wp.showCmd=%d, wp.rcNormalPosition: left=%d, top=%d, right=%d, bottom=%d (width=%d, height=%d)",
            mSavedWindowState->wp.showCmd,
            mSavedWindowState->wp.rcNormalPosition.left,
            mSavedWindowState->wp.rcNormalPosition.top,
            mSavedWindowState->wp.rcNormalPosition.right,
            mSavedWindowState->wp.rcNormalPosition.bottom,
            mSavedWindowState->wp.rcNormalPosition.right - mSavedWindowState->wp.rcNormalPosition.left,
            mSavedWindowState->wp.rcNormalPosition.bottom - mSavedWindowState->wp.rcNormalPosition.top);
        
        RECT beforeRect;
        GetWindowRect(mainWindow, &beforeRect);
        console::printf("ExitFullscreen: Before exit - window rect: left=%d, top=%d, right=%d, bottom=%d (width=%d, height=%d)",
            beforeRect.left, beforeRect.top, beforeRect.right, beforeRect.bottom,
            beforeRect.right - beforeRect.left, beforeRect.bottom - beforeRect.top);
        
        Utility::ExitFullscreen(mainWindow, mSavedWindowState.value());
        
        RECT afterRect;
        GetWindowRect(mainWindow, &afterRect);
        console::printf("ExitFullscreen: After Utility::ExitFullscreen - window rect: left=%d, top=%d, right=%d, bottom=%d (width=%d, height=%d)",
            afterRect.left, afterRect.top, afterRect.right, afterRect.bottom,
            afterRect.right - afterRect.left, afterRect.bottom - afterRect.top);
        
        mSavedWindowState.reset();
        console::printf("ExitFullscreen: mSavedWindowState cleared");
        
        OpenHacksVars::SavedWindowState.get_value() = WindowStateData();
    }
    else
    {
        console::printf("ExitFullscreen: mSavedWindowState is empty");
        
        const auto newStyle = static_cast<WindowFrameStyle>((int32_t)OpenHacksVars::MainWindowFrameStyle);
        ApplyMainWindowFrameStyle(newStyle);

        RECT rect = {};
        GetWindowRect(mainWindow, &rect);
        OffsetRect(&rect, 10, 10);
        SetWindowPos(mainWindow, nullptr, rect.left, rect.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
    }
    
    console::printf("=== ExitFullscreen() END ===");
}

void OpenHacksCore::ToggleFullscreen()
{
    HWND mainWindow = core_api::get_main_window();
    if (!Utility::IsFullscreen(mainWindow))
        EnterFullscreen();
    else
        ExitFullscreen();
}
