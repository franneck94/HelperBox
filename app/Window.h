#pragma once

class Window
{
private:
    static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
    Window();
    Window(const Window &) = delete;
    Window(Window &&) = delete;
    virtual ~Window();

    Window &operator=(const Window &) = delete;
    Window &operator=(Window &&) = delete;

    bool Create();
    bool WaitMessages();
    bool PollMessages(uint32_t TimeoutMs);
    bool ProcessMessages();

    void SignalStop();
    bool ShouldClose();

    void SetWindowName(LPCWSTR lpName);
    void SetWindowPosition(int X, int Y);
    void SetWindowDimension(int Width, int Height);

private:
    virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

protected:
    HWND m_hWnd;
    HINSTANCE m_hInstance;

private:
    HANDLE m_hEvent;
    HICON m_hIcon;

    LPCWSTR m_WindowName;
    int m_X;
    int m_Y;
    int m_Width;
    int m_Height;
};
