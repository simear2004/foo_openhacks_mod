#pragma once
#include <guiddef.h>
#include "hacks_com.h"
#include "com_utils.h"

enum WindowStateEnum : LONG
{
    WindowStateNormal = 0,
    WindowStateMinimized = 1,
    WindowStateMaximized = 2,
    WindowStateFullscreen = 3,
};

[
    appobject,
    coclass,
    library_block,
    default(IOpenHacks),
    uuid("7c87ccb2-aceb-4698-b31e-30855a6787ac")
]
class OpenHacksCOM : public IOpenHacks
{
public:
    OpenHacksCOM();
    ~OpenHacksCOM();

    static inline LPCOLESTR PROGID_OpenHacks = OLESTR("OpenHacks");

    static HRESULT Initialise(HMODULE mod);
    static HRESULT RegisterOpenHacksObject();
    static HRESULT UnregisterOpenHacksObject();
    static HRESULT Finalise();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // IDispatch
    static HRESULT InitTypeInfo();
    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) override;
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override;
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo,
                        UINT* puArgErr) override;

    STDMETHOD(get_DPI)(LONG* pValue);

    // Properties
    STDMETHOD(get_MenuBarVisible)(VARIANT_BOOL* pValue);
    STDMETHOD(put_MenuBarVisible)(VARIANT_BOOL value);

    STDMETHOD(get_StatusBarVisible)(VARIANT_BOOL* pValue);
    STDMETHOD(put_StatusBarVisible)(VARIANT_BOOL value);

    STDMETHOD(get_Fullscreen)(VARIANT_BOOL* pValue);
    STDMETHOD(put_Fullscreen)(VARIANT_BOOL value);

    STDMETHOD(get_WindowState)(LONG* pValue);
    STDMETHOD(put_WindowState)(LONG value);

    STDMETHOD(get_WindowFrameStyle)(LONG* pValue);
    STDMETHOD(put_WindowFrameStyle)(LONG value);

    STDMETHOD(get_EnableWin10Shadow)(VARIANT_BOOL* pValue);
    STDMETHOD(put_EnableWin10Shadow)(VARIANT_BOOL value);

    STDMETHOD(get_DisableResizeWhenMaximized)(VARIANT_BOOL* pValue);
    STDMETHOD(put_DisableResizeWhenMaximized)(VARIANT_BOOL value);

    STDMETHOD(get_DisableResizeWhenFullscreen)(VARIANT_BOOL* pValue);
    STDMETHOD(put_DisableResizeWhenFullscreen)(VARIANT_BOOL value);

    // PseudoCaptionSettings Properties
    STDMETHOD(get_PseudoCaptionLeft)(LONG* pValue);
    STDMETHOD(put_PseudoCaptionLeft)(LONG value);

    STDMETHOD(get_PseudoCaptionTop)(LONG* pValue);
    STDMETHOD(put_PseudoCaptionTop)(LONG value);

    STDMETHOD(get_PseudoCaptionRight)(LONG* pValue);
    STDMETHOD(put_PseudoCaptionRight)(LONG value);

    STDMETHOD(get_PseudoCaptionBottom)(LONG* pValue);
    STDMETHOD(put_PseudoCaptionBottom)(LONG value);

    STDMETHOD(get_PseudoCaptionWidth)(LONG* pValue);
    STDMETHOD(put_PseudoCaptionWidth)(LONG value);

    STDMETHOD(get_PseudoCaptionHeight)(LONG* pValue);
    STDMETHOD(put_PseudoCaptionHeight)(LONG value);

    STDMETHOD(get_PseudoCaptionLeftEnabled)(VARIANT_BOOL* pValue);
    STDMETHOD(put_PseudoCaptionLeftEnabled)(VARIANT_BOOL value);

    STDMETHOD(get_PseudoCaptionTopEnabled)(VARIANT_BOOL* pValue);
    STDMETHOD(put_PseudoCaptionTopEnabled)(VARIANT_BOOL value);

    STDMETHOD(get_PseudoCaptionRightEnabled)(VARIANT_BOOL* pValue);
    STDMETHOD(put_PseudoCaptionRightEnabled)(VARIANT_BOOL value);

    STDMETHOD(get_PseudoCaptionBottomEnabled)(VARIANT_BOOL* pValue);
    STDMETHOD(put_PseudoCaptionBottomEnabled)(VARIANT_BOOL value);

    // Methods
    STDMETHOD(ToggleMenuBar)();
    STDMETHOD(ToggleStatusBar)();
    STDMETHOD(ToggleFullscreen)();

private:
    static HRESULT HookCLSIDFromProgID();

private:
    LONG mRefCount = 0;
    inline static DWORD StaticRegisterCookie = 0;
    inline static ITypeLibPtr StaticTypeLibPtr;
    inline static TypeInfoCache StaticTypeInfo;
};

class OpenHacksClassFactory : public IClassFactory
{
public:
    OpenHacksClassFactory() = default;
    ~OpenHacksClassFactory() = default;

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
    STDMETHODIMP LockServer(BOOL fLock) override;

private:
    LONG mRefCount = 0;
};
