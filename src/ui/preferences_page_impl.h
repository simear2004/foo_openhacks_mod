#pragma once

// from SDK helpers & libPPUI
#define END_MSG_MAP_HOOK()                                                                                                                                     \
    break;                                                                                                                                                     \
    default:                                                                                                                                                   \
        return __super::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID);                                                                 \
        }                                                                                                                                                      \
        return FALSE;                                                                                                                                          \
        }

static bool window_service_trait_defer_destruction(const service_base*)
{
    return true;
}

template <typename _parentClass>
class CWindowFixSEH : public _parentClass
{
public:
    BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0)
    {
        __try
        {
            return _parentClass::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID);
        }
        __except (uExceptFilterProc(GetExceptionInformation()))
        {
            return FALSE; /* should not get here */
        }
    }
    template <typename... arg_t>
    CWindowFixSEH(arg_t&&... arg) : _parentClass(std::forward<arg_t>(arg)...)
    {
    }
};

//! Special service_impl_t replacement for service classes that also implement ATL/WTL windows.
template <typename _t_base>
class window_service_impl_t : public implement_service_query<CWindowFixSEH<_t_base>>
{
private:
    typedef window_service_impl_t<_t_base> t_self;
    typedef implement_service_query<CWindowFixSEH<_t_base>> t_base;

public:
    BEGIN_MSG_MAP_EX(window_service_impl_t)
        MSG_WM_DESTROY(OnDestroyPassThru)
        CHAIN_MSG_MAP(__super)
    END_MSG_MAP_HOOK()

    int FB2KAPI service_release() throw()
    {
        int ret = --m_counter;
        if (ret == 0)
        {
            if (window_service_trait_defer_destruction(this) && !InterlockedExchange(&m_delayedDestroyInProgress, 1))
            {
                PFC_ASSERT_NO_EXCEPTION(service_impl_helper::release_object_delayed(this););
            }
            else if (this->m_hWnd != NULL)
            {
                if (!InterlockedExchange(&m_destroyWindowInProgress, 1))
                {                                           // don't double-destroy in weird scenarios
                    service_ptr_t<service_base> bump(this); // prevent delete this from occurring in mid-DestroyWindow
                    PFC_ASSERT_NO_EXCEPTION(::DestroyWindow(this->m_hWnd));
                    // We don't know what else happened inside DestroyWindow() due to message queue flush
                    // Safely retry destruction by bump object destructor
                    // m_hWnd doesn't have to be null here - we'll possibly get cleaned up by OnFinalMessage() instead
                }
            }
            else
            { // m_hWnd is NULL
                PFC_ASSERT_NO_EXCEPTION(delete this);
            }
        }
        return ret;
    }
    int FB2KAPI service_add_ref() throw()
    {
        return ++m_counter;
    }

    template <typename... arg_t>
    window_service_impl_t(arg_t&&... arg) : t_base(std::forward<arg_t>(arg)...){};

    ~window_service_impl_t()
    {
        PFC_ASSERT(this->m_hWnd == NULL);
    }

private:
    void OnDestroyPassThru()
    {
        SetMsgHandled(FALSE);
        m_destroyWindowInProgress = 1;
    }
    void OnFinalMessage(HWND p_wnd) override
    {
        t_base::OnFinalMessage(p_wnd);
        service_ptr_t<service_base> bump(this);
    }
    volatile LONG m_destroyWindowInProgress = 0;
    volatile LONG m_delayedDestroyInProgress = 0;
    pfc::refcounter m_counter;
};

namespace fb2k
{
template <typename obj_t, typename... arg_t>
service_ptr_t<obj_t> service_new_window(arg_t&&... arg)
{
    return new window_service_impl_t<obj_t>(std::forward<arg_t>(arg)...);
}
} // namespace fb2k

template <typename TDialog>
class preferences_page_instance_impl : public TDialog
{
public:
    preferences_page_instance_impl(HWND parent, preferences_page_callback::ptr callback) : TDialog(callback)
    {
        WIN32_OP(this->Create(parent) != NULL);

        // complain early if what we created isn't a child window
        PFC_ASSERT((this->GetStyle() & (WS_POPUP | WS_CHILD)) == WS_CHILD);
    }
    HWND get_wnd()
    {
        return this->m_hWnd;
    }
};

static bool window_service_trait_defer_destruction(const preferences_page_instance*)
{
    return false;
}

template <typename TDialog>
class preferences_page_impl : public preferences_page_v3
{
public:
    preferences_page_instance::ptr instantiate(HWND parent, preferences_page_callback::ptr callback)
    {
        return fb2k::service_new_window<preferences_page_instance_impl<TDialog>>(parent, callback);
    }
};

#define DECLARE_PREFERENCES_PAGE(name, impl, sort_priority, guid, parent_guid)                                                                                 \
    namespace                                                                                                                                                  \
    {                                                                                                                                                          \
    class open_hacks_##impl : public preferences_page_impl<impl>                                                                                               \
    {                                                                                                                                                          \
    public:                                                                                                                                                    \
        const char* get_name() override                                                                                                                        \
        {                                                                                                                                                      \
            return name;                                                                                                                                       \
        }                                                                                                                                                      \
        GUID get_guid() override                                                                                                                               \
        {                                                                                                                                                      \
            return guid;                                                                                                                                       \
        }                                                                                                                                                      \
        GUID get_parent_guid() override                                                                                                                        \
        {                                                                                                                                                      \
            return parent_guid;                                                                                                                                \
        }                                                                                                                                                      \
        double get_sort_priority() override                                                                                                                    \
        {                                                                                                                                                      \
            return sort_priority;                                                                                                                              \
        }                                                                                                                                                      \
        bool get_help_url(pfc::string_base& p_out) override                                                                                                    \
        {                                                                                                                                                      \
            p_out = OpenHacksVars::kOpenHacksHelpURL;                                                                                                          \
            return true;                                                                                                                                       \
        }                                                                                                                                                      \
    };                                                                                                                                                         \
    static preferences_page_factory_t<open_hacks_##impl> g_open_hacks_##impl;                                                                                  \
    }