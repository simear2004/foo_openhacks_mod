#pragma once
#include <windows.h>
#include <oleauto.h>
#include <comdef.h>
#include <comdefsp.h>
#include <map>

#define TO_VARIANT_BOOL(v) ((v) ? (VARIANT_TRUE) : (VARIANT_FALSE))
#define TO_BOOLEAN(v) ((v) == VARIANT_TRUE ? true : false)

#define RETURN_HR_IF(condition, hr)                                                                                                                            \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        if ((condition))                                                                                                                                       \
            return (hr);                                                                                                                                       \
    }                                                                                                                                                          \
    while (false)

#define RETURN_IF_FAILED(hr)                                                                                                                                   \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        if (FAILED(hr))                                                                                                                                        \
            return (hr);                                                                                                                                       \
    }                                                                                                                                                          \
    while (false)

struct DISPCache
{
public:
    using HashKey = ULONG;

    static HashKey Hash(const OLECHAR* name)
    {
        return LHashValOfName(LANG_NEUTRAL, name);
    }

    bool Find(const HashKey key, DISPID* id) const
    {
        if (id == nullptr)
            return false;

        if (auto iter = mCache.find(key); iter != mCache.end())
        {
            *id = iter->second;
            return true;
        }

        return false;
    }

    void Add(const HashKey key, const DISPID id)
    {
        mCache[key] = id;
    }

private:
    using CacheMap = std::map<HashKey, DISPID>;
    CacheMap mCache;
};

struct TypeInfoCache
{
public:
    bool IsValid() const
    {
        return mTypeInfoPtr != nullptr;
    }

    HRESULT InitFromTypelib(ITypeLib* typeLib, const GUID& guid)
    {
        if (typeLib == nullptr)
            return E_INVALIDARG;
        return typeLib->GetTypeInfoOfGuid(guid, &mTypeInfoPtr);
    }

    // wrap IDispatch
    HRESULT GetTypeInfo(UINT i, LCID lcid, ITypeInfo** ppv)
    {
        RETURN_HR_IF(!IsValid(), E_UNEXPECTED);
        RETURN_HR_IF(i != 0, DISP_E_BADINDEX);

        mTypeInfoPtr->AddRef();
        *ppv = mTypeInfoPtr.GetInterfacePtr();
        return S_OK;
    }

    HRESULT GetIDsOfNames(LPOLESTR* names, UINT cnames, MEMBERID* memid)
    {
        RETURN_HR_IF(!IsValid(), E_UNEXPECTED);
        RETURN_HR_IF(names == nullptr, E_INVALIDARG);

        HRESULT hr = S_OK;
        for (UINT i = 0; i < cnames && SUCCEEDED(hr); ++i)
        {
            const auto hash = DISPCache::Hash(names[i]);
            if (!mCache.Find(hash, &memid[i]))
            {
                hr = mTypeInfoPtr->GetIDsOfNames(&names[i], 1, &memid[i]);
                if (SUCCEEDED(hr))
                    mCache.Add(hash, memid[i]);
            }
        }

        return hr;
    }

    HRESULT Invoke(PVOID ins, MEMBERID memid, WORD flags, DISPPARAMS* params, VARIANT* result, EXCEPINFO* excep_info, UINT* err)
    {
        RETURN_HR_IF(!IsValid(), E_UNEXPECTED);
        return mTypeInfoPtr->Invoke(ins, memid, flags, params, result, excep_info, err);
    }

private:
    ITypeInfoPtr mTypeInfoPtr;
    DISPCache mCache;
};