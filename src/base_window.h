#pragma once

class BaseWindow
{
public:
    BaseWindow()
    {
    }

    virtual ~BaseWindow()
    {
        Cleanup();
    }

    FORCEINLINE HWND GetHWND() const
    {
        return mWindow;
    }

    FORCEINLINE void Destroy()
    {
        if (mWindow)
        {
            DestroyWindow(mWindow);
            mWindow = nullptr;
        }

        Cleanup();
    }

    bool Setup(HINSTANCE inst, wstring_view_t className);

    HWND Create(HWND parent, HINSTANCE inst = nullptr, wstring_view_t className = {});
    HWND CreateEx(HWND parent, DWORD style, DWORD exStyle, int x, int y, int width, int height, HINSTANCE inst = nullptr, wstring_view_t className = {});
    void Cleanup();

    static BaseWindow* GetInstance(HWND wnd);

protected:
    virtual LRESULT OnMessage(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) = 0;

private:
    static LRESULT CALLBACK StaticMessageProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

protected:
    HINSTANCE mInstance = nullptr;
    HWND mWindow = nullptr;
    ATOM mClassAtom = INVALID_ATOM;
};
