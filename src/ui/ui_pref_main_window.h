#pragma once
#include "ui_dialog_base.h"
#include "ui_overlay_window.h"

class UIPrefMainWindowDialog : public UIPrefDialogBase<UIPrefMainWindowDialog, IDD_PREF_MAINWINDOW>
{
public:
    UIPrefMainWindowDialog(preferences_page_callback::ptr cb) : UIPrefDialogBase(cb)
    {
    }

    BEGIN_MSG_MAP(UIPrefAdvancedDialog)
        CHAIN_MSG_MAP(UIPrefDialogBase)
        MSG_WM_COMMAND(OnCommand)
        MSG_WM_SETFOCUS(OnSetFocus)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

protected:
    void OnInitDialog() override;
    void OnApply() override;
    void OnFinalMessage(HWND wnd) override;

private:
    void OnCommand(UINT code, int id, CWindow ctrl);
    void OnSetFocus(CWindow wndOld);

    void LoadUIState();
    void SaveUIState();
    void UpdateCtrlState();
    void ApplySettings();

    void ShowOrHidePseudoCaptionOverlayAutomatically();

private:
    CComboBox mComboFrameStyle;
    COverlayWindow mPseudoCaptionOverlay;
};
