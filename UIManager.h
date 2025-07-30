#pragma once
#include <windows.h>
#include <objidl.h>     // Needed for IStream
#include <gdiplus.h>
#include "DataStructures.h"
#include "TrackerWindow.h"
#include <commctrl.h>

#include <functional>
#include <set>
#include <map>
#include <vector>
#include <string>


using namespace Gdiplus;

// UI Constants
#define MAIN_WINDOW_WIDTH           1200
#define MAIN_WINDOW_HEIGHT          800
#define SIDEBAR_WIDTH               250
#define TOOLBAR_HEIGHT              40
#define STATUSBAR_HEIGHT            25

// Timer IDs
#define TIMER_REFRESH_UI            1001
#define TIMER_CHECK_BUDGETS         1002
#define TIMER_PROCESS_RECURRING     1003
#define TIMER_ANIMATION             1004

// Color scheme
#define COLOR_PRIMARY               RGB(52, 152, 219)
#define COLOR_SECONDARY             RGB(149, 165, 166)
#define COLOR_SUCCESS               RGB(39, 174, 96)
#define COLOR_DANGER                RGB(231, 76, 60)
#ifndef COLOR_BACKGROUND
#define COLOR_BACKGROUND            RGB(236, 240, 241)
#endif
#define COLOR_SURFACE               RGB(255, 255, 255)
#define COLOR_TEXT_PRIMARY          RGB(44, 62, 80)
#define COLOR_TEXT_SECONDARY        RGB(127, 140, 141)

// Dashboard layout constants
#define DASHBOARD_CARD_WIDTH        280
#define DASHBOARD_CARD_HEIGHT       120
#define DASHBOARD_CARD_MARGIN       20
#define DASHBOARD_CHART_HEIGHT      300

enum class NotificationType {
    INFO,
    SUCCESS,
    WARNING,
    ERROR_TYPE  // Changed from ERROR to avoid Windows macro conflict
};


class UIManager {
private:
    static HWND hMainListView;
    static HWND hUserLabel;
    static HWND hBalanceLabel;
    static HWND hMainToolbar;

    // Control IDs
    static const int ID_TRANSACTION_LIST = 1001;
    
public:
    // Main window components
    static void CreateMainContent(HWND hwnd);
    static void ResizeMainContent(HWND hwnd, RECT clientRect);
    static void RefreshMainContent(HWND hwnd);
    static void ClearMainContent(HWND hwnd);

    // User interface state management
    static void UpdateUIForLoggedInUser(HWND hwnd);
    static void UpdateUIForLoggedOutUser(HWND hwnd);

    // Dashboard components
    static void PaintDashboard(HWND hwnd, HDC hdc);
    static void PaintWelcomeScreen(HWND hwnd, HDC hdc);
    static void DrawDashboardCard(HDC hdc, RECT rect, const std::wstring& title, const std::wstring& value, COLORREF color);
    static void DrawQuickStats(HDC hdc, RECT rect);
    static void DrawRecentTransactions(HDC hdc, RECT rect);
    static void DrawBudgetOverview(HDC hdc, RECT rect);

    // ListView helpers
    static void HandleListViewDoubleClick();
    static void HandleListViewColumnClick(LPNMLISTVIEW pnmv);

    // Control creation helpers
    static HWND CreateStyledButton(HWND parent, const std::wstring& text, int id, int x, int y, int width, int height);
    static HWND CreateStyledEdit(HWND parent, int id, int x, int y, int width, int height, DWORD style = 0);
    static HWND CreateStyledCombo(HWND parent, int id, int x, int y, int width, int height);
    static HWND CreateStyledListView(HWND parent, int id, int x, int y, int width, int height);
    static HWND CreateStyledLabel(HWND parent, const std::wstring& text, int x, int y, int width, int height, COLORREF color = COLOR_TEXT_PRIMARY);

    // Common dialogs
    static void ShowAboutDialog(HWND parent);
    static void ShowTutorialDialog(HWND parent);

    // Menu and toolbar creation - ADD THESE NEW DECLARATIONS:
    static void CreateMenuBar(HWND hwnd);
    static void CreateMainToolbar(HWND hwnd);
    static void CreateStatusBar(HWND hwnd);
    static void UpdateStatusBar(HWND hwnd);

    // Theme management
    static void ApplyDarkTheme(HWND hwnd);
    static void ApplyLightTheme(HWND hwnd);
    static void RefreshTheme(HWND hwnd);
    static bool IsDarkModeEnabled();

    // Animation system
    static void StartAnimation(HWND hwnd, int duration, std::function<void(double)> callback);
    static void StopAnimation(HWND hwnd);

    // Utility functions
    static void CenterWindow(HWND hwnd, HWND parent = NULL);
    static void SetWindowIcon(HWND hwnd, int iconId);
    static void ShowNotification(HWND parent, const std::wstring& message, const std::wstring& title, DWORD type = MB_OK);

public:
    // Dialog procedures
    static LRESULT CALLBACK AboutDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK TutorialDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // Drawing helpers
    static void DrawGradientBackground(HDC hdc, RECT rect, COLORREF color1, COLORREF color2);
    static void DrawRoundedRectangle(HDC hdc, RECT rect, int radius, COLORREF fillColor, COLORREF borderColor = RGB(0, 0, 0));
    static void DrawProgressBar(HDC hdc, RECT rect, double percentage, COLORREF color);
    static void DrawChart(HDC hdc, RECT rect, const std::vector<double>& data, COLORREF color);
    static void DrawIcon(HDC hdc, int x, int y, int size, const std::wstring& iconType);

    // GDI+ helpers
    static void InitializeGraphics();
    static void CleanupGraphics();
    static Graphics* CreateGraphicsFromHDC(HDC hdc);

    // Animation helpers
    struct AnimationData {
        HWND hwnd;
        int duration;
        DWORD startTime;
        std::function<void(double)> callback;
        bool active;
    };
    static std::vector<AnimationData> activeAnimations;
    static void ProcessAnimations();

    // Theme colors
    static std::map<std::wstring, COLORREF> lightThemeColors;
    static std::map<std::wstring, COLORREF> darkThemeColors;
    static std::map<std::wstring, COLORREF> currentThemeColors;

    // Font management
    static HFONT hFontTitle;
    static HFONT hFontNormal;
    static HFONT hFontSmall;
    static HFONT hFontBold;
    static void InitializeFonts();
    static void CleanupFonts();

    // Control styling
    static void StyleButton(HWND hButton);
    static void StyleEdit(HWND hEdit);
    static void StyleCombo(HWND hCombo);
    static void StyleListView(HWND hListView);

    // Layout helpers
    static RECT CalculateCardPosition(int index, int cardsPerRow, RECT containerRect);
    static int CalculateOptimalCardLayout(RECT containerRect, int* cardsPerRow, int* rows);
};

// Dashboard card data structure
struct DashboardCard {
    std::wstring title;
    std::wstring value;
    std::wstring subtitle;
    COLORREF color;
    std::wstring iconType;
    bool showTrend;
    double trendValue;
    std::function<void()> onClick;
};

// Sidebar navigation
class SidebarManager {
public:
    enum class SidebarSection {
        DASHBOARD,
        TRANSACTIONS,
        BUDGET,
        GOALS,
        ANALYTICS,
        SETTINGS
    };

    static void CreateSidebar(HWND parent);
    static void UpdateSidebar(HWND hwnd);
    static void SelectSection(SidebarSection section);
    static SidebarSection GetCurrentSection();
    static void RefreshSectionContent(HWND hwnd);

private:
    static LRESULT CALLBACK SidebarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void DrawSidebarBackground(HDC hdc, RECT rect);
    static void DrawSidebarItem(HDC hdc, RECT rect, const std::wstring& text, const std::wstring& icon, bool selected);
    static void HandleSidebarClick(int y);

    static HWND hSidebar;
    static SidebarSection currentSection;
    static std::vector<std::pair<std::wstring, std::wstring>> sidebarItems; // text, icon
};

// Custom controls
class CustomControls {
public:
    // Custom button with modern styling
    static HWND CreateModernButton(HWND parent, const std::wstring& text, int id, int x, int y, int width, int height, COLORREF color = COLOR_PRIMARY);

    // Progress ring for goals
    static void DrawProgressRing(HDC hdc, RECT rect, double percentage, COLORREF color, int thickness = 8);

    // Chart controls
    static void DrawLineChart(HDC hdc, RECT rect, const std::vector<std::pair<std::wstring, double>>& data, COLORREF color);
    static void DrawBarChart(HDC hdc, RECT rect, const std::vector<std::pair<std::wstring, double>>& data, COLORREF color);
    static void DrawPieChart(HDC hdc, RECT rect, const std::vector<std::pair<std::wstring, double>>& data);
    

    // Card-based layouts
    static void DrawInfoCard(HDC hdc, RECT rect, const DashboardCard& card);
    static void DrawTransactionCard(HDC hdc, RECT rect, const Expense& expense);
    static void DrawTransactionCard(HDC hdc, RECT rect, const Income& income);
    static void DrawBudgetCard(HDC hdc, RECT rect, const Budget& budget);
    static void DrawGoalCard(HDC hdc, RECT rect, const SavingsGoal& goal);

    // Input validation helpers
    static bool ValidateNumericInput(HWND hEdit, double& value, bool allowNegative = false);
    static bool ValidateDateInput(HWND hEdit, std::wstring& date);
    static bool ValidateEmailInput(HWND hEdit, std::wstring& email);

private:
    static LRESULT CALLBACK ModernButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static WNDPROC originalButtonProc;
};

// Notification system
class NotificationManager {
public:
 
    struct Notification {
        std::wstring message;
        std::wstring title;
        NotificationType type;
        DWORD timestamp;
        bool autoHide;
        int duration; // seconds
    };

    static void ShowNotification(const std::wstring& message, const std::wstring& title, NotificationType type, int duration = 5);
    static void ShowBudgetAlert(const std::wstring& category, double spent, double limit);
    static void ShowGoalAchieved(const std::wstring& goalName);
    static void ShowRecurringProcessed(int count);
    static void ClearAllNotifications();

    static void CreateNotificationArea(HWND parent);
    static void UpdateNotificationArea(HWND hwnd);

private:
    static std::vector<Notification> activeNotifications;
    static HWND hNotificationArea;
    static void DrawNotification(HDC hdc, RECT rect, const Notification& notification);
    static COLORREF GetNotificationColor(NotificationType type);
    static std::wstring GetNotificationIcon(NotificationType type);
    static void RemoveExpiredNotifications();
};

// Layout management
class LayoutManager {
public:
    struct LayoutConstraint {
        int x, y, width, height;
        bool relativeX, relativeY, relativeWidth, relativeHeight;
        double xPercent, yPercent, widthPercent, heightPercent;
        int marginLeft, marginTop, marginRight, marginBottom;
    };

    static void RegisterControl(HWND control, const LayoutConstraint& constraint);
    static void UnregisterControl(HWND control);
    static void ApplyLayout(HWND parent, RECT clientRect);
    static void ClearLayout();

    // Predefined layouts
    static void ApplyDashboardLayout(HWND parent, RECT clientRect);
    static void ApplyTransactionListLayout(HWND parent, RECT clientRect);
    static void ApplyAnalyticsLayout(HWND parent, RECT clientRect);

private:
    static std::map<HWND, LayoutConstraint> controlConstraints;
    static RECT CalculateControlRect(const LayoutConstraint& constraint, RECT parentRect);
};

// Resource management
class ResourceManager {
public:
    static bool LoadIcons();
    static bool LoadImages();
    static bool LoadStrings();
    static void CleanupResources();

    static HICON GetIcon(const std::wstring& name);
    static HBITMAP GetImage(const std::wstring& name);
    static std::wstring GetString(const std::wstring& key);

    // Built-in icons (using Unicode symbols)
    static const std::wstring ICON_DASHBOARD;
    static const std::wstring ICON_TRANSACTIONS;
    static const std::wstring ICON_BUDGET;
    static const std::wstring ICON_GOALS;
    static const std::wstring ICON_ANALYTICS;
    static const std::wstring ICON_SETTINGS;
    static const std::wstring ICON_INCOME;
    static const std::wstring ICON_EXPENSE;
    static const std::wstring ICON_BALANCE;

private:
    static std::map<std::wstring, HICON> loadedIcons;
    static std::map<std::wstring, HBITMAP> loadedImages;
    static std::map<std::wstring, std::wstring> loadedStrings;
};