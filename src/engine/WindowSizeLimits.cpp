#include "WindowSizeLimits.hpp"

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#ifdef _WIN32
LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_GETMINMAXINFO)
    {
        MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);

        mmi->ptMinTrackSize.x = g_minWidth;
        mmi->ptMinTrackSize.y = g_minHeight;
        mmi->ptMaxTrackSize.x = g_maxWidth;
        mmi->ptMaxTrackSize.y = g_maxHeight;

        return 0;
    }

    return CallWindowProc(g_originalWndProc, hwnd, msg, wParam, lParam);
}
#endif

void setWindowSizeLimits(sf::Window& window, int minW, int minH, int maxW, int maxH)
{
    #ifdef _WIN32
        g_minWidth  = minW;
        g_minHeight = minH;
        g_maxWidth  = maxW;
        g_maxHeight = maxH;
        HWND hwnd = window.getSystemHandle();
        g_originalWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)CustomWndProc);
    #elif defined(__linux__)
        Display* display = XOpenDisplay(nullptr);
        if (!display) return;
        Window handle = window.getSystemHandle();
        XSizeHints hints = {};
        hints.flags = PMinSize | PMaxSize;
        hints.min_width = minW;
        hints.min_height = minH;
        hints.max_width = maxW;
        hints.max_height = maxH;
        XSetWMNormalHints(display, handle, &hints);
        XCloseDisplay(display);
    #endif
}