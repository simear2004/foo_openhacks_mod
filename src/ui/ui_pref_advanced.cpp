#include "pch.h"
#include "ui_pref_advanced.h"
#include "preferences_page_impl.h"
#include "hacks_core.h"
#include "hacks_vars.h"
#include "hacks_guids.h"

DECLARE_PREFERENCES_PAGE("高级", UIPrefAdvancedDialog, 100.0, OpenHacksGuids::kAdvancedPageGuid, OpenHacksGuids::kDUIPageGuid);

void UIPrefAdvancedDialog::OnInitDialog()
{
    SetHeaderFont(IDC_PREF_HEADER1);

    mComboMenuBar.Attach(GetDlgItem(IDC_MENUBAR));
    mComboMenuBar.AddString(TEXT("显示"));
    mComboMenuBar.AddString(TEXT("隐藏"));

    mComboStatusBar.Attach(GetDlgItem(IDC_STATUSBAR));
    mComboStatusBar.AddString(TEXT("显示"));
    mComboStatusBar.AddString(TEXT("隐藏"));

    LoadUIState();
}

void UIPrefAdvancedDialog::OnApply()
{
    SaveUIState();
    ApplySettings();
}

void UIPrefAdvancedDialog::OnCommand(UINT code, int id, CWindow ctrl)
{
    if (code == CBN_SELCHANGE)
    {
        NotifyStateChanges(true);
    }
}

void UIPrefAdvancedDialog::LoadUIState()
{
    mComboMenuBar.SetCurSel(OpenHacksVars::ShowMainMenu ? 0 : 1);
    mComboStatusBar.SetCurSel(OpenHacksVars::ShowStatusBar ? 0 : 1);
}

void UIPrefAdvancedDialog::SaveUIState()
{
    OpenHacksVars::ShowMainMenu = mComboMenuBar.GetCurSel() == 1 ? false : true;
    OpenHacksVars::ShowStatusBar = mComboStatusBar.GetCurSel() == 1 ? false : true;
}

void UIPrefAdvancedDialog::ApplySettings()
{
    auto& api = OpenHacksCore::Get();
    api.ShowOrHideMenuBar(OpenHacksVars::ShowMainMenu);
    api.ShowOrHideStatusBar(OpenHacksVars::ShowStatusBar);
}
