#include "TrackerWindow.h"
#include "UIManager.h"
#include "UserManager.h"
#include "DatabaseManager.h"
#include "FinanceManager.h"
#include "Analytics.h"
#include "ChartRenderer.h"
#include "resource.h"
#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

const wchar_t CLASS_NAME[] = L"PersonalTrackerMainWindow";

// Global handles
HWND hMainToolbar, hStatusBar, hMainListView;
HWND hUserLabel, hBalanceLabel;

// GDI+ token
ULONG_PTR gdiplusToken;

TrackerWindow::TrackerWindow() : hwnd(nullptr) {}

TrackerWindow::~TrackerWindow() {}

bool TrackerWindow::Create(const wchar_t* windowTitle, DWORD style, int width, int height) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASS wc = {};
    wc.lpfnWndProc = TrackerWindow::WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));

    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        windowTitle,
        style,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!hwnd) return false;

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    SetTimer(hwnd, 1, 60000, NULL);
    SetTimer(hwnd, 2, 300000, NULL);
    SetTimer(hwnd, 3, 3600000, NULL);

    return true;
}

void TrackerWindow::Show(int nCmdShow) {
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}

int TrackerWindow::Run() {
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK TrackerWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
    {
        INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES };
        InitCommonControlsEx(&icex);

        UIManager::CreateMenuBar(hwnd);           // Create the menu
        UIManager::CreateMainToolbar(hwnd);       // Create the toolbar
        UIManager::CreateStatusBar(hwnd);         // Create the status bar
        UIManager::CreateMainContent(hwnd);  // Create main content area

        DatabaseManager::LoadAllData();



        if (UserManager::GetAllUsers().empty()) {
            MessageBox(hwnd, L"Welcome! Let's create your first user account.", L"First Time Setup", MB_OK);
            UserManager::ShowCreateUserDialog(hwnd);
        } else {
            UserManager::ShowLoginDialog(hwnd);
        }

        UserManager::OnUserLoggedIn = [hwnd]() {
            UIManager::UpdateUIForLoggedInUser(hwnd);
            UIManager::RefreshMainContent(hwnd);
        };

        UserManager::OnUserLoggedOut = [hwnd]() {
            UIManager::UpdateUIForLoggedOutUser(hwnd);
            UIManager::ClearMainContent(hwnd);
        };

        DatabaseManager::EnableAutoBackup(30);
    }
    return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_FILE_NEW_USER: UserManager::ShowCreateUserDialog(hwnd); break;
        case ID_FILE_LOGIN:
            if (UserManager::IsUserLoggedIn()) UserManager::LogoutUser();
            UserManager::ShowLoginDialog(hwnd); break;
        case ID_FILE_LOGOUT:
            if (UserManager::IsUserLoggedIn()) UserManager::LogoutUser(); break;
        case ID_FILE_BACKUP:
        {
            wchar_t filePath[MAX_PATH] = L"";
            OPENFILENAME ofn = { sizeof(ofn) };
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = filePath;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"JSON Files\0*.json\0All Files\0*.*\0";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
            if (GetSaveFileName(&ofn) && DatabaseManager::BackupData(filePath)) {
                MessageBox(hwnd, L"Backup created successfully!", L"Backup", MB_OK);
            } else {
                MessageBox(hwnd, L"Failed to create backup.", L"Error", MB_OK);
            }
        }
        break;
        case ID_FILE_RESTORE:
        {
            wchar_t filePath[MAX_PATH] = L"";
            OPENFILENAME ofn = { sizeof(ofn) };
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = filePath;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"JSON Files\0*.json\0All Files\0*.*\0";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (GetOpenFileName(&ofn) &&
                MessageBox(hwnd, L"This will replace all current data. Continue?", L"Restore Backup", MB_YESNO) == IDYES &&
                DatabaseManager::RestoreFromBackup(filePath)) {
                MessageBox(hwnd, L"Data restored successfully! Please restart the application.", L"Restore", MB_OK);
                PostQuitMessage(0);
            }
        }
        break;
        case ID_FILE_EXIT:
            PostQuitMessage(0); break;
        }
        return 0;

    case WM_SIZE:
    {
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        if (hMainToolbar) SendMessage(hMainToolbar, TB_AUTOSIZE, 0, 0);
        if (hStatusBar) SendMessage(hStatusBar, WM_SIZE, 0, 0);
        UIManager::ResizeMainContent(hwnd, rcClient);
    }
    return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (UserManager::IsUserLoggedIn()) {
            UIManager::PaintDashboard(hwnd, hdc);
        } else {
            UIManager::PaintWelcomeScreen(hwnd, hdc);
        }
        EndPaint(hwnd, &ps);
    }
    return 0;

    case WM_TIMER:
        switch (wParam) {
        case 1: UIManager::RefreshMainContent(hwnd); UIManager::UpdateStatusBar(hwnd); break;
        case 2: if (UserManager::IsUserLoggedIn()) BudgetManager::CheckBudgetAlerts(hwnd); break;
        case 3: if (UserManager::IsUserLoggedIn()) RecurringManager::ProcessRecurringTransactions(); break;
        }
        return 0;

    case WM_CLOSE:
        if (UserManager::IsUserLoggedIn()) {
            int result = MessageBox(hwnd, L"Save data before exiting?", L"Save Data", MB_YESNOCANCEL);
            if (result == IDCANCEL) return 0;
            else if (result == IDYES) DatabaseManager::SaveAllData();
        }
        DatabaseManager::DisableAutoBackup();
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        GdiplusShutdown(gdiplusToken);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
