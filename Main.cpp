
#pragma once
#include <windows.h>

#include <objidl.h>     // Needed for IStream
#include <gdiplus.h>

#include "TrackerWindow.h"

using namespace Gdiplus;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Your GDI+ code here
    TrackerWindow window;
    if (!window.Create(L"Personal Finance Tracker", WS_OVERLAPPEDWINDOW)) {
        return -1;
    }
    window.Show(nCmdShow);
    int result = window.Run();
    GdiplusShutdown(gdiplusToken);
    return result;
}
