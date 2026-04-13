#pragma once
#include "resource.h"
#include <libPPUI/GDIUtils.h>
#include "ui_dark_mode.h"

template <typename T, UINT DIALOG_ID>
class UIPrefDialogBase : public CDialogImpl<T>, public preferences_page_instance
{
public:
    enum
    {
        IDD = DIALOG_ID
    };

    BEGIN_MSG_MAP(UIPrefDialogBase)
        MSG_WM_INITDIALOG(OnInitDialog)
    END_MSG_MAP()

public:
    UIPrefDialogBase(preferences_page_callback::ptr cb) : mPageCallback(cb)
    {
    }

    virtual ~UIPrefDialogBase()
    {
    }

    uint32_t get_state() override
    {
        return mPageState;
    }

    void apply() override
    {
        OnApply();
        mPageState = mDefaultPageState;
    }

    void reset() override
    {
        OnReset();
        mPageState = mDefaultPageState;
        NotifyStateChanges(true);
    }

    void OnFinalMessage(HWND wnd) override
    {
    }

protected:
    BOOL OnInitDialog(CWindow focus, LPARAM param)
    {
        mDarkModeHooks.AddDialogWithControls(*this);
        OnInitDialog();
        mDefaultPageState = (IsResetable() ? preferences_state::resettable : 0) | preferences_state::dark_mode_supported;
        mPageState = mDefaultPageState;
        mPageCallback->on_state_changed();
        mDialogInitialized = true;
        return TRUE;
    }

    virtual void OnInitDialog()
    {
    }

    virtual void OnApply()
    {
    }

    virtual void OnReset()
    {
    }

    virtual bool IsResetable() const
    {
        return false;
    }

    void SetHeaderFont(uint32_t id)
    {
        if (mHeaderFont.IsNull())
        {
            CreateScaledFontEx(mHeaderFont, GetFont(), 1.3, FW_BOLD);
        }

        GetDlgItem(id).SetFont(mHeaderFont);
    }

    void NotifyStateChanges(bool changed)
    {
        if (mDialogInitialized)
        {
            mPageState = mDefaultPageState;
            mPageState |= (changed ? preferences_state::changed : 0);
            mPageState |= (mRestartRequired ? preferences_state::needs_restart : 0);
            mPageCallback->on_state_changed();
            mRestartRequired = false;
        }
    }

    FORCEINLINE bool IsDialogInitialized() const
    {
        return mDialogInitialized;
    }

    FORCEINLINE void RequestRestart()
    {
        mRestartRequired = true;
    }

    FORCEINLINE void CancelRequestRestart()
    {
        mRestartRequired = false;
    }

protected:
    bool mDialogInitialized = false;
    uint32_t mPageState = 0;
    uint32_t mDefaultPageState = 0;
    bool mRestartRequired = false;
    preferences_page_callback::ptr mPageCallback;
    CFont mHeaderFont;
    CDarkModeHooks mDarkModeHooks;
};
