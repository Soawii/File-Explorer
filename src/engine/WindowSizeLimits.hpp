#pragma once
#include "../conf.hpp"
#include <SFML/Graphics.hpp>

#ifdef _WIN32
#include <windows.h>
static WNDPROC g_originalWndProc = nullptr;

static int g_minWidth = 0;
static int g_minHeight = 0;
static int g_maxWidth = 9999; 
static int g_maxHeight = 9999;

LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

void setWindowSizeLimits(sf::Window& window, int minW, int minH, int maxW, int maxH);