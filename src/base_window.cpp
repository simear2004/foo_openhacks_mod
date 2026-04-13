#include "pch.h"
#include "base_window.h"
#include "win32_utils.h"
#include "str.h"

bool BaseWindow::Setup(HINSTANCE inst, wstring_view_t className)
{
    mInstance = inst;
    if (INVALID_ATOM == mClassAtom)
    {
        WNDCLASSEXW wcex{};
        wcex.cbSize = sizeof(wcex);
        wcex.lpszClassName = className.data();
        wcex.hInstance = inst;
        wcex.lpfnWndProc = StaticMessageProc;
        mClassAtom = ::RegisterClassExW(&wcex);
    }

    return 0 != mClassAtom;
}

HWND BaseWindow::Create(HWND parent, HINSTANCE inst /*= nullptr*/, wstring_view_t className/* = {}*/)
{
    if (INVALID_ATOM == mClassAtom)
    {
        if (inst == nullptr || className.empty())
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return nullptr;
        }

        Setup(inst, className);
    }

    return ::CreateWindowExW(0, MAKEINTATOM(mClassAtom), L"", 0, 0, 0, 0, 0, parent, nullptr, inst, this);
}

void BaseWindow::Cleanup()
{
    if (INVALID_ATOM != mClassAtom)
    {
        UnregisterClass(MAKEINTATOM(mClassAtom), mInstance);
        mClassAtom = INVALID_ATOM;
    }

    mInstance = nullptr;
}

BaseWindow* BaseWindow::GetInstance(HWND wnd)
{
    return reinterpret_cast<BaseWindow*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
}

LRESULT CALLBACK BaseWindow::StaticMessageProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    BaseWindow* instance = nullptr;
    if (msg == WM_NCCREATE)
    {
        auto lpcs = reinterpret_cast<LPCREATESTRUCTW>(lp);
        if (instance = reinterpret_cast<BaseWindow*>(lpcs->lpCreateParams))
        {
            instance->mWindow = wnd;
            SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)instance);
        }
    }
    else
    {
        instance = GetInstance(wnd);
    }

    if (msg == WM_NCDESTROY)
    {
        if (instance)
        {
            instance->mWindow = nullptr;
            SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)NULL);
        }
    }

    return instance ? instance->OnMessage(wnd, msg, wp, lp) : ::DefWindowProc(wnd, msg, wp, lp);
}

HWND BaseWindow::CreateEx(HWND parent, DWORD style, DWORD exStyle, int x, int y, int width, int height, HINSTANCE inst /*= nullptr*/,
                          wstring_view_t className /* = {}*/)
{
    if (INVALID_ATOM == mClassAtom)
    {
        if (inst == nullptr || className.empty())
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return nullptr;
        }

        Setup(inst, className);
    }

    return ::CreateWindowExW(exStyle, MAKEINTATOM(mClassAtom), L"", style, x, y, width, height, parent, nullptr, inst, this);
}
