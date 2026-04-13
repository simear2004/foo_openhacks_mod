#pragma once

typedef CWinTraits<WS_POPUP, WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW> COverlayWindowTraits;

class COverlayWindow : public CWindowImpl<COverlayWindow, CWindow, COverlayWindowTraits>
{
public:
    DECLARE_WND_CLASS_EX(TEXT("9193F38D-560A-4D26-BE90-D733DA0EE5AA"), 0, COLOR_HIGHLIGHT)

    BEGIN_MSG_MAP_EX(COverlayWindow)
        MSG_WM_CREATE(OnCreate)
    END_MSG_MAP()

public:
    void Activate(const CRect& rect);
    void Deactivate();
    void CleanUp();

private:
    LRESULT OnCreate(LPCREATESTRUCT);
    void ShowAbove(const CRect& rect);
};