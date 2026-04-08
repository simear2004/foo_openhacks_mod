#pragma once
#include <oaidl.h>

// warning C4467: usage of ATL attributes is deprecated
#pragma warning(disable:4467)

#ifdef __INTELLISENSE__
#define COM_PROPGET(name, ...) \
        STDMETHOD(get_##name)(__VA_ARGS__)
#define COM_PROPPUT(name, ...) \
        STDMETHOD(put_##name)(__VA_ARGS__)
#else
#define COM_PROPGET(name, ...) \
        [propget] STDMETHOD(name)(__VA_ARGS__)
#define COM_PROPPUT(name, ...) \
        [propput] STDMETHOD(name)(__VA_ARGS__)
#endif

[module(name = "foo_openhacks")];

[
    object,
    dual,
    pointer_default(unique),
    uuid("5faf8474-cde1-4fd4-8151-6ced18b7039b")
]
__interface IOpenHacks : IDispatch
{
    // Properties
    COM_PROPGET(DPI, [out, retval] LONG* pValue);

    COM_PROPGET(MenuBarVisible, [out, retval] VARIANT_BOOL* pValue);
    COM_PROPPUT(MenuBarVisible, [in] VARIANT_BOOL value);

    COM_PROPGET(StatusBarVisible, [out, retval] VARIANT_BOOL* pValue);
    COM_PROPPUT(StatusBarVisible, [in] VARIANT_BOOL value);

    COM_PROPGET(Fullscreen, [out, retval] VARIANT_BOOL* pValue);
    COM_PROPPUT(Fullscreen, [in] VARIANT_BOOL value);

    COM_PROPGET(WindowState, [out, retval] LONG* pValue);
    COM_PROPPUT(WindowState, [in] LONG value);

    COM_PROPGET(WindowFrameStyle, [out, retval] LONG* pValue);
    COM_PROPPUT(WindowFrameStyle, [in] LONG value);

    COM_PROPGET(EnableWin10Shadow, [out, retval] VARIANT_BOOL* pValue);
    COM_PROPPUT(EnableWin10Shadow, [in] VARIANT_BOOL value);

    COM_PROPGET(DisableResizeWhenMaximized, [out, retval] VARIANT_BOOL* pValue);
    COM_PROPPUT(DisableResizeWhenMaximized, [in] VARIANT_BOOL value);

    COM_PROPGET(DisableResizeWhenFullscreen, [out, retval] VARIANT_BOOL* pValue);
    COM_PROPPUT(DisableResizeWhenFullscreen, [in] VARIANT_BOOL value);

    // PseudoCaptionSettings Properties
    COM_PROPGET(PseudoCaptionLeft, [out, retval] LONG* pValue);
    COM_PROPPUT(PseudoCaptionLeft, [in] LONG value);

    COM_PROPGET(PseudoCaptionTop, [out, retval] LONG* pValue);
    COM_PROPPUT(PseudoCaptionTop, [in] LONG value);

    COM_PROPGET(PseudoCaptionRight, [out, retval] LONG* pValue);
    COM_PROPPUT(PseudoCaptionRight, [in] LONG value);

    COM_PROPGET(PseudoCaptionBottom, [out, retval] LONG* pValue);
    COM_PROPPUT(PseudoCaptionBottom, [in] LONG value);

    COM_PROPGET(PseudoCaptionWidth, [out, retval] LONG* pValue);
    COM_PROPPUT(PseudoCaptionWidth, [in] LONG value);

    COM_PROPGET(PseudoCaptionHeight, [out, retval] LONG* pValue);
    COM_PROPPUT(PseudoCaptionHeight, [in] LONG value);

    COM_PROPGET(PseudoCaptionLeftEnabled, [out, retval] VARIANT_BOOL* pValue);
    COM_PROPPUT(PseudoCaptionLeftEnabled, [in] VARIANT_BOOL value);

    COM_PROPGET(PseudoCaptionTopEnabled, [out, retval] VARIANT_BOOL* pValue);
    COM_PROPPUT(PseudoCaptionTopEnabled, [in] VARIANT_BOOL value);

    COM_PROPGET(PseudoCaptionRightEnabled, [out, retval] VARIANT_BOOL* pValue);
    COM_PROPPUT(PseudoCaptionRightEnabled, [in] VARIANT_BOOL value);

    COM_PROPGET(PseudoCaptionBottomEnabled, [out, retval] VARIANT_BOOL* pValue);
    COM_PROPPUT(PseudoCaptionBottomEnabled, [in] VARIANT_BOOL value);

    // Methods
    STDMETHOD(ToggleMenuBar)();
    STDMETHOD(ToggleStatusBar)();
    STDMETHOD(ToggleFullscreen)();
};
