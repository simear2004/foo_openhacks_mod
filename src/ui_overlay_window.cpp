#include "pch.h"
#include "ui_overlay_window.h"

void COverlayWindow::Activate(const CRect& rect /*= {}*/)
{
    ShowAbove(rect);
}

void COverlayWindow::Deactivate()
{
    if (m_hWnd != nullptr && IsWindowVisible())
        ShowWindow(SW_HIDE);
}

void COverlayWindow::CleanUp()
{
    if (m_hWnd != nullptr)
        DestroyWindow();
}

LRESULT COverlayWindow::OnCreate(LPCREATESTRUCT)
{
    SetLayeredWindowAttributes(*this, 0, 128, LWA_ALPHA);
    return 0;
}

void COverlayWindow::ShowAbove(const CRect& rect)
{
    if (m_hWnd == nullptr)
        Create(HWND_DESKTOP);

    SetWindowPos(nullptr, rect, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}
