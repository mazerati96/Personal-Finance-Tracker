// Add these to TrackerWindow.h to make the variables accessible to UIManager.cpp:

#pragma once
#include <windows.h>

// Command identifiers for buttons (existing code...)
#define ID_BUTTON_ADD_EXPENSE        101
#define ID_BUTTON_VIEW_EXPENSES      102
#define ID_BUTTON_ADD_INCOME         103
#define ID_BUTTON_VIEW_INCOME        104
#define ID_BUTTON_SHOW_SUMMARY       105
#define ID_BUTTON_SAVE_DATA          106
#define ID_BUTTON_LOAD_DATA          107
#define ID_BUTTON_EXIT_APP           108

// Dialog resource IDs (existing code...)
#define IDD_ADD_EXPENSE              400
#define IDD_ADD_INCOME               401

// ADD THESE NEW SECTIONS:

// Menu IDs - moved from TrackerWindow.cpp
#define ID_FILE_NEW_USER                1001
#define ID_FILE_LOGIN                   1002
#define ID_FILE_LOGOUT                  1003
#define ID_FILE_BACKUP                  1004
#define ID_FILE_RESTORE                 1005
#define ID_FILE_EXPORT_CSV              1006
#define ID_FILE_EXPORT_PDF              1007
#define ID_FILE_IMPORT                  1008
#define ID_FILE_EXIT                    1009

#define ID_TRANSACTION_ADD_EXPENSE      2001
#define ID_TRANSACTION_ADD_INCOME       2002
#define ID_TRANSACTION_VIEW_ALL         2003
#define ID_TRANSACTION_SEARCH           2004
#define ID_TRANSACTION_RECURRING        2005

#define ID_BUDGET_MANAGE                3001
#define ID_BUDGET_VIEW                  3002
#define ID_BUDGET_ALERTS                3003

#define ID_GOALS_MANAGE                 4001
#define ID_GOALS_PROGRESS               4002

#define ID_ANALYTICS_SUMMARY            5001
#define ID_ANALYTICS_TRENDS             5002
#define ID_ANALYTICS_CATEGORIES         5003
#define ID_ANALYTICS_CHARTS             5004
#define ID_ANALYTICS_REPORTS            5005

#define ID_TOOLS_SETTINGS               6001
#define ID_TOOLS_CURRENCY               6002
#define ID_TOOLS_CATEGORIES             6003
#define ID_TOOLS_BACKUP_SETTINGS        6004

#define ID_HELP_ABOUT                   7001
#define ID_HELP_TUTORIAL                7002

// External variable declarations for UI components
extern HWND hMainToolbar, hStatusBar, hMainListView;
extern HWND hUserLabel, hBalanceLabel;

// Function declaration (existing...)
int InitTrackerWindow(HINSTANCE hInstance, int nCmdShow);

class TrackerWindow {
public:
    TrackerWindow();
    ~TrackerWindow();

    bool Create(const wchar_t* windowTitle, DWORD style, int width = 800, int height = 600);
    void Show(int nCmdShow);
    int Run();

private:
    HWND hwnd;
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};