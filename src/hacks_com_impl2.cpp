#include "pch.h"
#include "hacks_com_impl.h"
#include <minhook.h>

namespace
{
static auto OriginCLSIDFromProgID = CLSIDFromProgID;

STDMETHODIMP HacksCLSIDFromProgID(LPCOLESTR lpszProgID, LPCLSID lpclsid)
{
    if (lpszProgID != nullptr && lpclsid != nullptr && _wcsicmp(lpszProgID, OpenHacksCOM::PROGID_OpenHacks) == 0)
    {
        *lpclsid = __uuidof(OpenHacksCOM);
        return S_OK;
    }

    return OriginCLSIDFromProgID ? OriginCLSIDFromProgID(lpszProgID, lpclsid) : REGDB_E_CLASSNOTREG;
}

} // namespace

OpenHacksCOM::OpenHacksCOM()
{
}

OpenHacksCOM::~OpenHacksCOM()
{
}

HRESULT OpenHacksCOM::Initialise(HMODULE mod)
{
    WCHAR wszModulePath[MAX_PATH] = {};
    DWORD pathLength = GetModuleFileNameW(mod, wszModulePath, ARRAYSIZE(wszModulePath));
    RETURN_HR_IF(pathLength == 0 || pathLength >= MAX_PATH, E_FAIL);
    RETURN_IF_FAILED(LoadTypeLibEx(wszModulePath, REGKIND::REGKIND_NONE, &StaticTypeLibPtr));
    RETURN_IF_FAILED(InitTypeInfo());
    RETURN_IF_FAILED(RegisterOpenHacksObject());
    return S_OK;
}

HRESULT OpenHacksCOM::RegisterOpenHacksObject()
{
    CComPtr<OpenHacksClassFactory> classFactory = new (std::nothrow) OpenHacksClassFactory();
    RETURN_HR_IF(classFactory == nullptr, E_OUTOFMEMORY);

    DWORD dwRegister = 0;
    RETURN_IF_FAILED(CoRegisterClassObject(__uuidof(OpenHacksCOM), classFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &dwRegister));

    if (HRESULT hr = HookCLSIDFromProgID(); FAILED(hr))
    {
        CoRevokeClassObject(dwRegister);
        return hr;
    }

    StaticRegisterCookie = dwRegister;
    return S_OK;
}

HRESULT OpenHacksCOM::UnregisterOpenHacksObject()
{
    if (StaticRegisterCookie != 0)
    {
        HRESULT hr = CoRevokeClassObject(StaticRegisterCookie);
        StaticRegisterCookie = 0;
        return hr;
    }

    return S_OK;
}

HRESULT OpenHacksCOM::Finalise()
{
    return UnregisterOpenHacksObject();
}

HRESULT OpenHacksCOM::HookCLSIDFromProgID()
{
    MH_STATUS status = MH_UNKNOWN;
    if (MH_Initialize() == MH_OK)
    {
        status = MH_CreateHook(&CLSIDFromProgID, &HacksCLSIDFromProgID, reinterpret_cast<void**>(&OriginCLSIDFromProgID));

        if (status == MH_OK)
            status = MH_EnableHook(&CLSIDFromProgID);

        if (status != MH_OK)
            MH_Uninitialize();
    }

    return status == MH_OK ? S_OK : E_FAIL;
}

STDMETHODIMP OpenHacksCOM::QueryInterface(REFIID riid, void** ppvObject)
{
    if (riid == IID_IUnknown || riid == IID_IDispatch || riid == __uuidof(IOpenHacks))
    {
        *ppvObject = static_cast<IDispatch*>(this);
        AddRef();
        return S_OK;
    }

    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) OpenHacksCOM::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}

STDMETHODIMP_(ULONG) OpenHacksCOM::Release()
{
    LONG count = InterlockedDecrement(&mRefCount);
    if (count == 0)
        delete this;
    return count;
}

HRESULT OpenHacksCOM::InitTypeInfo()
{
    if (!StaticTypeInfo.IsValid() && StaticTypeLibPtr != nullptr)
        RETURN_IF_FAILED(StaticTypeInfo.InitFromTypelib(StaticTypeLibPtr, __uuidof(IOpenHacks)));

    return StaticTypeInfo.IsValid() ? S_OK : E_FAIL;
}

STDMETHODIMP OpenHacksCOM::GetTypeInfoCount(UINT* pctinfo)
{
    *pctinfo = (StaticTypeLibPtr != nullptr) ? 1 : 0;
    return S_OK;
}

STDMETHODIMP OpenHacksCOM::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
    return StaticTypeInfo.GetTypeInfo(iTInfo, lcid, ppTInfo);
}

STDMETHODIMP OpenHacksCOM::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
{
    return StaticTypeInfo.GetIDsOfNames(rgszNames, cNames, rgDispId);
}

STDMETHODIMP OpenHacksCOM::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo,
                                  UINT* puArgErr)
{
    return StaticTypeInfo.Invoke(static_cast<IDispatch*>(this), dispIdMember, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}

STDMETHODIMP OpenHacksClassFactory::QueryInterface(REFIID riid, void** ppvObject)
{
    if (riid == IID_IUnknown || riid == IID_IClassFactory)
    {
        *ppvObject = static_cast<IClassFactory*>(this);
        AddRef();
        return S_OK;
    }

    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) OpenHacksClassFactory::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}

STDMETHODIMP_(ULONG) OpenHacksClassFactory::Release()
{
    LONG count = InterlockedDecrement(&mRefCount);
    if (count == 0)
    {
        delete this;
    }
    return count;
}

STDMETHODIMP OpenHacksClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
    RETURN_HR_IF(pUnkOuter != nullptr, CLASS_E_NOAGGREGATION);

    if (CComPtr<OpenHacksCOM> pOpenHacks = new (std::nothrow) OpenHacksCOM())
        return pOpenHacks->QueryInterface(riid, ppvObject);
    return E_OUTOFMEMORY;
}

STDMETHODIMP OpenHacksClassFactory::LockServer(BOOL fLock)
{
    return S_OK;
}
