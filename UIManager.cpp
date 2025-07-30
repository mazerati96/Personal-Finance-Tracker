// Add these includes at the top of UIManager.cpp:
#pragma once
#define NOMINMAX
#include <windows.h>
#include <objidl.h>     // Needed for IStream
#include <gdiplus.h>

#include "UIManager.h"
#include "UserManager.h"
#include "DatabaseManager.h"
#include "FinanceManager.h"
#include "Analytics.h"
#include "ChartRenderer.h"
#include "TrackerWindow.h"  // For the menu IDs and control IDs
#include "resource.h"       // For icon and resource IDs

#include <commctrl.h>

#include <commdlg.h>
#include <string>
#include <vector>

#include <sstream>      // For std::wstringstream
#include <iomanip>      // For std::fixed, std::setprecision
#include <iostream>     // For stream operators

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

extern HWND hMainToolbar, hStatusBar, hMainListView;
extern HWND hUserLabel, hBalanceLabel;

class ExportManager;
class ImportManager;
class TransactionViewer;
class SearchManager;
class RecurringManager;
class BudgetManager;
class GoalsManager;
class CategoryManager;
class CurrencyManager;
class BackupManager;
class ChartRenderer;

// Static member initialization
std::map<std::wstring, COLORREF> UIManager::lightThemeColors;
std::map<std::wstring, COLORREF> UIManager::darkThemeColors;
std::map<std::wstring, COLORREF> UIManager::currentThemeColors;
std::vector<UIManager::AnimationData> UIManager::activeAnimations;

HFONT UIManager::hFontTitle = NULL;
HFONT UIManager::hFontNormal = NULL;
HFONT UIManager::hFontSmall = NULL;
HFONT UIManager::hFontBold = NULL;

// Initialize the static HWND members that were causing errors
HWND UIManager::hMainListView = NULL;
HWND UIManager::hUserLabel = NULL;
HWND UIManager::hBalanceLabel = NULL;
HWND UIManager::hMainToolbar = NULL;

namespace UserDataFilter {
    // These should be implemented in UserManager.cpp, but declare here if needed:
    extern double GetUserBalance(const std::wstring& userId);
    extern double GetUserTotalIncome(const std::wstring& userId);
    extern double GetUserTotalExpenses(const std::wstring& userId);
}

namespace UIManagerUtils {
    std::wstring FormatCurrency(double amount) {
        wchar_t buffer[100];
        swprintf_s(buffer, L"$%.2f", amount);
        return std::wstring(buffer);
    }
}
// Main window components
void UIManager::CreateMenuBar(HWND hwnd) {
    HMENU hMenuBar = CreateMenu();

    // File menu
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_NEW_USER, L"&New User...");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_LOGIN, L"&Login...");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_LOGOUT, L"Log&out");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_BACKUP, L"&Backup Data...");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_RESTORE, L"&Restore Data...");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_EXPORT_CSV, L"Export to &CSV...");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_EXPORT_PDF, L"Export to &PDF...");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_IMPORT, L"&Import Data...");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_EXIT, L"E&xit");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");

    // Transaction menu
    HMENU hTransactionMenu = CreatePopupMenu();
    AppendMenu(hTransactionMenu, MF_STRING, ID_TRANSACTION_ADD_EXPENSE, L"Add &Expense...");
    AppendMenu(hTransactionMenu, MF_STRING, ID_TRANSACTION_ADD_INCOME, L"Add &Income...");
    AppendMenu(hTransactionMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hTransactionMenu, MF_STRING, ID_TRANSACTION_VIEW_ALL, L"&View All Transactions...");
    AppendMenu(hTransactionMenu, MF_STRING, ID_TRANSACTION_SEARCH, L"&Search Transactions...");
    AppendMenu(hTransactionMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hTransactionMenu, MF_STRING, ID_TRANSACTION_RECURRING, L"&Recurring Transactions...");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hTransactionMenu, L"&Transactions");

    // Budget menu
    HMENU hBudgetMenu = CreatePopupMenu();
    AppendMenu(hBudgetMenu, MF_STRING, ID_BUDGET_MANAGE, L"&Manage Budgets...");
    AppendMenu(hBudgetMenu, MF_STRING, ID_BUDGET_VIEW, L"&View Budget Summary...");
    AppendMenu(hBudgetMenu, MF_STRING, ID_BUDGET_ALERTS, L"Budget &Alerts...");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hBudgetMenu, L"&Budget");

    // Goals menu
    HMENU hGoalsMenu = CreatePopupMenu();
    AppendMenu(hGoalsMenu, MF_STRING, ID_GOALS_MANAGE, L"&Manage Goals...");
    AppendMenu(hGoalsMenu, MF_STRING, ID_GOALS_PROGRESS, L"View &Progress...");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hGoalsMenu, L"&Goals");

    // Analytics menu
    HMENU hAnalyticsMenu = CreatePopupMenu();
    AppendMenu(hAnalyticsMenu, MF_STRING, ID_ANALYTICS_SUMMARY, L"Financial &Summary...");
    AppendMenu(hAnalyticsMenu, MF_STRING, ID_ANALYTICS_TRENDS, L"Spending &Trends...");
    AppendMenu(hAnalyticsMenu, MF_STRING, ID_ANALYTICS_CATEGORIES, L"&Category Analysis...");
    AppendMenu(hAnalyticsMenu, MF_STRING, ID_ANALYTICS_CHARTS, L"&Charts && Graphs...");
    AppendMenu(hAnalyticsMenu, MF_STRING, ID_ANALYTICS_REPORTS, L"Generate &Reports...");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hAnalyticsMenu, L"&Analytics");

    // Tools menu
    HMENU hToolsMenu = CreatePopupMenu();
    AppendMenu(hToolsMenu, MF_STRING, ID_TOOLS_SETTINGS, L"User &Settings...");
    AppendMenu(hToolsMenu, MF_STRING, ID_TOOLS_CURRENCY, L"&Currency Settings...");
    AppendMenu(hToolsMenu, MF_STRING, ID_TOOLS_CATEGORIES, L"Manage &Categories...");
    AppendMenu(hToolsMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hToolsMenu, MF_STRING, ID_TOOLS_BACKUP_SETTINGS, L"&Backup Settings...");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hToolsMenu, L"&Tools");

    // Help menu
    HMENU hHelpMenu = CreatePopupMenu();
    AppendMenu(hHelpMenu, MF_STRING, ID_HELP_TUTORIAL, L"&Tutorial...");
    AppendMenu(hHelpMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hHelpMenu, MF_STRING, ID_HELP_ABOUT, L"&About...");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelpMenu, L"&Help");

    SetMenu(hwnd, hMenuBar);
}

void UIManager::CreateMainToolbar(HWND hwnd) {
    hMainToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
        0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Set up toolbar buttons
    TBBUTTON tbb[] = {
        {0, ID_TRANSACTION_ADD_EXPENSE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, (INT_PTR)L"Add Expense"},
        {1, ID_TRANSACTION_ADD_INCOME, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, (INT_PTR)L"Add Income"},
        {0, 0, 0, TBSTYLE_SEP, {0}, 0, 0},
        {2, ID_ANALYTICS_SUMMARY, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, (INT_PTR)L"Summary"},
        {3, ID_ANALYTICS_CHARTS, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, (INT_PTR)L"Charts"},
        {0, 0, 0, TBSTYLE_SEP, {0}, 0, 0},
        {4, ID_FILE_BACKUP, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, (INT_PTR)L"Backup"},
    };

    SendMessage(hMainToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SendMessage(hMainToolbar, TB_ADDBUTTONS, sizeof(tbb) / sizeof(TBBUTTON), (LPARAM)tbb);
}

void UIManager::CreateStatusBar(HWND hwnd) {
    hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Set up status bar parts
    int parts[] = { 200, 400, 600, -1 };
    SendMessage(hStatusBar, SB_SETPARTS, 4, (LPARAM)parts);

    UIManager::UpdateStatusBar(hwnd);
}


void UIManager::UpdateStatusBar(HWND hwnd) {
    if (!hStatusBar) return;

    SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)L"Ready");

    if (UserManager::IsUserLoggedIn()) {
        User* user = UserManager::GetCurrentUser();
        std::wstring userText = L"User: " + user->displayName;
        SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)userText.c_str());

        double balance = UserDataFilter::GetUserBalance(UserManager::GetCurrentUserId());
        wchar_t balanceText[100];
        swprintf_s(balanceText, L"Balance: $%.2f", balance);
        SendMessage(hStatusBar, SB_SETTEXT, 2, (LPARAM)balanceText);

        SendMessage(hStatusBar, SB_SETTEXT, 3, (LPARAM)L"Auto-backup enabled");
    }
    else {
        SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)L"Not logged in");
        SendMessage(hStatusBar, SB_SETTEXT, 2, (LPARAM)L"");
        SendMessage(hStatusBar, SB_SETTEXT, 3, (LPARAM)L"");
    }
}




// Main window components
void UIManager::CreateMainContent(HWND hwnd) {
    InitializeFonts();
    InitializeGraphics();

    // Initialize color schemes
    lightThemeColors[L"background"] = COLOR_BACKGROUND;
    lightThemeColors[L"surface"] = COLOR_SURFACE;
    lightThemeColors[L"primary"] = COLOR_PRIMARY;
    lightThemeColors[L"text"] = COLOR_TEXT_PRIMARY;

    darkThemeColors[L"background"] = RGB(45, 52, 54);
    darkThemeColors[L"surface"] = RGB(55, 63, 71);
    darkThemeColors[L"primary"] = RGB(116, 185, 255);
    darkThemeColors[L"text"] = RGB(223, 230, 233);

    // Set initial theme
    if (UserManager::IsUserLoggedIn() && UserManager::GetCurrentUser()->isDarkMode) {
        currentThemeColors = darkThemeColors;
    }
    else {
        currentThemeColors = lightThemeColors;
    }

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    // Create main list view for dashboard
    hMainListView = CreateStyledListView(hwnd, ID_TRANSACTION_LIST,
        SIDEBAR_WIDTH + 20, TOOLBAR_HEIGHT + 20,
        clientRect.right - SIDEBAR_WIDTH - 40,
        clientRect.bottom - TOOLBAR_HEIGHT - STATUSBAR_HEIGHT - 40);

    // Create sidebar
    SidebarManager::CreateSidebar(hwnd);

    // Create user info labels
    hUserLabel = CreateStyledLabel(hwnd, L"Welcome!",
        SIDEBAR_WIDTH + 20, TOOLBAR_HEIGHT + 5, 200, 20);

    hBalanceLabel = CreateStyledLabel(hwnd, L"Balance: $0.00",
        clientRect.right - 200, TOOLBAR_HEIGHT + 5, 180, 20, COLOR_PRIMARY);
}

// Fix the CustomControls::DrawProgressRing function to avoid GDI+ errors
void CustomControls::DrawProgressRing(HDC hdc, RECT rect, double percentage, COLORREF color, int thickness) {
    int centerX = (rect.left + rect.right) / 2;
    int centerY = (rect.top + rect.bottom) / 2;
    int radius = std::min(rect.right - rect.left, rect.bottom - rect.top) / 2 - thickness;

    // Use standard GDI instead of GDI+ to avoid Graphics object issues
    HPEN backgroundPen = CreatePen(PS_SOLID, thickness, RGB(GetRValue(color) / 4, GetGValue(color) / 4, GetBValue(color) / 4));
    HPEN progressPen = CreatePen(PS_SOLID, thickness, color);

    HPEN oldPen = (HPEN)SelectObject(hdc, backgroundPen);

    // Draw background circle
    Arc(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius,
        centerX, centerY - radius, centerX, centerY - radius);

    if (percentage > 0) {
        SelectObject(hdc, progressPen);
        // Calculate end point for arc based on percentage
        double angle = (percentage * 360.0 - 90.0) * 3.14159 / 180.0; // Convert to radians, start from top
        int endX = centerX + (int)(radius * cos(angle));
        int endY = centerY + (int)(radius * sin(angle));

        // Draw progress arc
        Arc(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius,
            centerX, centerY - radius, endX, endY);
    }

    SelectObject(hdc, oldPen);
    DeleteObject(backgroundPen);
    DeleteObject(progressPen);
}

void UIManager::ResizeMainContent(HWND hwnd, RECT clientRect) {
    // Resize main list view
    if (hMainListView) {
        SetWindowPos(hMainListView, NULL,
            SIDEBAR_WIDTH + 20, TOOLBAR_HEIGHT + 40,
            clientRect.right - SIDEBAR_WIDTH - 40,
            clientRect.bottom - TOOLBAR_HEIGHT - STATUSBAR_HEIGHT - 80,
            SWP_NOZORDER);
    }

    // Resize user labels
    if (hUserLabel) {
        SetWindowPos(hUserLabel, NULL,
            SIDEBAR_WIDTH + 20, TOOLBAR_HEIGHT + 5,
            200, 20, SWP_NOZORDER);
    }

    if (hBalanceLabel) {
        SetWindowPos(hBalanceLabel, NULL,
            clientRect.right - 200, TOOLBAR_HEIGHT + 5,
            180, 20, SWP_NOZORDER);
    }

    // Update sidebar
    SidebarManager::UpdateSidebar(hwnd);
}

void UIManager::RefreshMainContent(HWND hwnd) {
    if (!UserManager::IsUserLoggedIn()) {
        ClearMainContent(hwnd);
        return;
    }

    // Update user info
    User* user = UserManager::GetCurrentUser();
    std::wstring welcomeText = L"Welcome, " + user->displayName + L"!";
    SetWindowText(hUserLabel, welcomeText.c_str());

    // Update balance
    double balance = UserDataFilter::GetUserBalance(user->username);
    std::wstringstream balanceStream;
    balanceStream << L"Balance: $" << std::fixed << std::setprecision(2) << balance;
    SetWindowText(hBalanceLabel, balanceStream.str().c_str());

    // Refresh main content based on current section
    SidebarManager::RefreshSectionContent(hwnd);

    // Force redraw
    InvalidateRect(hwnd, NULL, TRUE);
}

void UIManager::ClearMainContent(HWND hwnd) {
    if (hMainListView) {
        ListView_DeleteAllItems(hMainListView);
    }

    if (hUserLabel) {
        SetWindowText(hUserLabel, L"Not logged in");
    }

    if (hBalanceLabel) {
        SetWindowText(hBalanceLabel, L"Balance: --");
    }

    InvalidateRect(hwnd, NULL, TRUE);
}

// User interface state management
void UIManager::UpdateUIForLoggedInUser(HWND hwnd) {
    User* user = UserManager::GetCurrentUser();

    // Apply user's theme preference
    if (user->isDarkMode) {
        ApplyDarkTheme(hwnd);
    }
    else {
        ApplyLightTheme(hwnd);
    }

    // Enable menu items and toolbar buttons
    HMENU hMenu = GetMenu(hwnd);
    if (hMenu) {
        EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_ENABLED); // Transactions menu
        EnableMenuItem(hMenu, 3, MF_BYPOSITION | MF_ENABLED); // Budget menu
        EnableMenuItem(hMenu, 4, MF_BYPOSITION | MF_ENABLED); // Goals menu
        EnableMenuItem(hMenu, 5, MF_BYPOSITION | MF_ENABLED); // Analytics menu
        DrawMenuBar(hwnd);
    }

    // Update toolbar
    if (hMainToolbar) {
        // Enable toolbar buttons
        SendMessage(hMainToolbar, TB_ENABLEBUTTON, ID_TRANSACTION_ADD_EXPENSE, MAKELONG(TRUE, 0));
        SendMessage(hMainToolbar, TB_ENABLEBUTTON, ID_TRANSACTION_ADD_INCOME, MAKELONG(TRUE, 0));
        SendMessage(hMainToolbar, TB_ENABLEBUTTON, ID_ANALYTICS_SUMMARY, MAKELONG(TRUE, 0));
    }

    RefreshMainContent(hwnd);
}

void UIManager::UpdateUIForLoggedOutUser(HWND hwnd) {
    // Apply light theme by default
    ApplyLightTheme(hwnd);

    // Disable menu items and toolbar buttons
    HMENU hMenu = GetMenu(hwnd);
    if (hMenu) {
        EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_GRAYED); // Transactions menu
        EnableMenuItem(hMenu, 3, MF_BYPOSITION | MF_GRAYED); // Budget menu
        EnableMenuItem(hMenu, 4, MF_BYPOSITION | MF_GRAYED); // Goals menu
        EnableMenuItem(hMenu, 5, MF_BYPOSITION | MF_GRAYED); // Analytics menu
        DrawMenuBar(hwnd);
    }

    // Update toolbar
    if (hMainToolbar) {
        SendMessage(hMainToolbar, TB_ENABLEBUTTON, ID_TRANSACTION_ADD_EXPENSE, MAKELONG(FALSE, 0));
        SendMessage(hMainToolbar, TB_ENABLEBUTTON, ID_TRANSACTION_ADD_INCOME, MAKELONG(FALSE, 0));
        SendMessage(hMainToolbar, TB_ENABLEBUTTON, ID_ANALYTICS_SUMMARY, MAKELONG(FALSE, 0));
    }

    ClearMainContent(hwnd);
}

// Dashboard components
void UIManager::PaintDashboard(HWND hwnd, HDC hdc) {
    if (!UserManager::IsUserLoggedIn()) {
        PaintWelcomeScreen(hwnd, hdc);
        return;
    }

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    // Calculate dashboard area (excluding sidebar)
    RECT dashboardRect = {
        SIDEBAR_WIDTH + 10,
        TOOLBAR_HEIGHT + 40,
        clientRect.right - 10,
        clientRect.bottom - STATUSBAR_HEIGHT - 10
    };

    // Draw background gradient
    DrawGradientBackground(hdc, dashboardRect,
        currentThemeColors[L"background"], currentThemeColors[L"surface"]);

    // Draw dashboard cards
    std::wstring userId = UserManager::GetCurrentUserId();

    // Quick stats cards
    std::vector<DashboardCard> cards = {
        {L"Total Income", Analytics::FormatCurrency(Analytics::GetTotalIncome(userId)),
         L"This month", COLOR_SUCCESS, L"💰", false, 0.0, nullptr},
        {L"Total Expenses", Analytics::FormatCurrency(Analytics::GetTotalExpenses(userId)),
         L"This month", COLOR_DANGER, L"💸", false, 0.0, nullptr},
        {L"Balance", Analytics::FormatCurrency(Analytics::GetBalance(userId)),
         L"Current", COLOR_PRIMARY, L"🏦", false, 0.0, nullptr},
        {L"Savings Rate", Analytics::FormatPercentage(
             Analytics::GetTotalIncome(userId) > 0 ?
             ((Analytics::GetTotalIncome(userId) - Analytics::GetTotalExpenses(userId)) /
              Analytics::GetTotalIncome(userId)) * 100 : 0),
         L"This month", COLOR_SUCCESS, L"📈", false, 0.0, nullptr}
    };

    int cardsPerRow = 4;
    int cardWidth = (dashboardRect.right - dashboardRect.left - (cardsPerRow + 1) * DASHBOARD_CARD_MARGIN) / cardsPerRow;
    int cardHeight = DASHBOARD_CARD_HEIGHT;
    for (size_t i = 0; i < cards.size(); ++i) {
        int col = static_cast<int>(i) % cardsPerRow;  // Fixed: explicit cast
        int row = static_cast<int>(i) / cardsPerRow;  // Fixed: explicit cast

        RECT cardRect = {
            dashboardRect.left + DASHBOARD_CARD_MARGIN + col * (cardWidth + DASHBOARD_CARD_MARGIN),
            dashboardRect.top + DASHBOARD_CARD_MARGIN + row * (cardHeight + DASHBOARD_CARD_MARGIN),
            dashboardRect.left + DASHBOARD_CARD_MARGIN + col * (cardWidth + DASHBOARD_CARD_MARGIN) + cardWidth,
            dashboardRect.top + DASHBOARD_CARD_MARGIN + row * (cardHeight + DASHBOARD_CARD_MARGIN) + cardHeight
        };

        CustomControls::DrawInfoCard(hdc, cardRect, cards[i]);
    }

    // Draw recent transactions section
    RECT transactionsRect = {
        dashboardRect.left + DASHBOARD_CARD_MARGIN,
        dashboardRect.top + DASHBOARD_CARD_MARGIN * 2 + cardHeight,
        dashboardRect.right / 2 - DASHBOARD_CARD_MARGIN / 2,
        dashboardRect.bottom - DASHBOARD_CARD_MARGIN
    };

    DrawRecentTransactions(hdc, transactionsRect);

    // Draw budget overview section
    RECT budgetRect = {
        dashboardRect.right / 2 + DASHBOARD_CARD_MARGIN / 2,
        dashboardRect.top + DASHBOARD_CARD_MARGIN * 2 + cardHeight,
        dashboardRect.right - DASHBOARD_CARD_MARGIN,
        dashboardRect.bottom - DASHBOARD_CARD_MARGIN
    };

    DrawBudgetOverview(hdc, budgetRect);
}

void UIManager::PaintWelcomeScreen(HWND hwnd, HDC hdc) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    // Draw gradient background
    DrawGradientBackground(hdc, clientRect, RGB(236, 240, 241), RGB(189, 195, 199));

    // Draw welcome message
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_TEXT_PRIMARY);

    HFONT oldFont = (HFONT)SelectObject(hdc, hFontTitle);

    std::wstring welcomeText = L"Personal Finance Tracker";
    RECT textRect = clientRect;
    textRect.top = clientRect.bottom / 2 - 100;
    DrawText(hdc, welcomeText.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hFontNormal);

    std::wstring subtitleText = L"Please log in or create a new user account to get started";
    textRect.top += 60;
    DrawText(hdc, subtitleText.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
}

void UIManager::DrawDashboardCard(HDC hdc, RECT rect, const std::wstring& title, const std::wstring& value, COLORREF color) {
    // Draw card background
    DrawRoundedRectangle(hdc, rect, 8, currentThemeColors[L"surface"], RGB(220, 220, 220));

    // Draw colored left border
    RECT borderRect = { rect.left, rect.top, rect.left + 4, rect.bottom };
    HBRUSH colorBrush = CreateSolidBrush(color);
    FillRect(hdc, &borderRect, colorBrush);
    DeleteObject(colorBrush);

    // Draw title
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, currentThemeColors[L"text"]);

    HFONT oldFont = (HFONT)SelectObject(hdc, hFontSmall);

    RECT titleRect = { rect.left + 15, rect.top + 15, rect.right - 15, rect.top + 35 };
    DrawText(hdc, title.c_str(), -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Draw value
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, color);

    RECT valueRect = { rect.left + 15, rect.top + 40, rect.right - 15, rect.bottom - 15 };
    DrawText(hdc, value.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
}

void UIManager::DrawRecentTransactions(HDC hdc, RECT rect) {
    // Draw section background
    DrawRoundedRectangle(hdc, rect, 8, currentThemeColors[L"surface"], RGB(220, 220, 220));

    // Draw section title
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, currentThemeColors[L"text"]);

    HFONT oldFont = (HFONT)SelectObject(hdc, hFontBold);

    RECT titleRect = { rect.left + 15, rect.top + 15, rect.right - 15, rect.top + 40 };
    DrawText(hdc, L"Recent Transactions", -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hFontNormal);

    if (!UserManager::IsUserLoggedIn()) {
        SelectObject(hdc, oldFont);
        return;
    }

    // Get recent transactions
    std::wstring userId = UserManager::GetCurrentUserId();
    auto userExpenses = UserDataFilter::GetUserExpenses(userId);
    auto userIncomes = UserDataFilter::GetUserIncomes(userId);

    // Sort by date (most recent first)
    std::sort(userExpenses.begin(), userExpenses.end(),
        [](const Expense& a, const Expense& b) { return a.date > b.date; });
    std::sort(userIncomes.begin(), userIncomes.end(),
        [](const Income& a, const Income& b) { return a.date > b.date; });

    int yPos = rect.top + 50;
    int itemHeight = 30;
    int maxItems = (rect.bottom - rect.top - 60) / itemHeight;
    int itemCount = 0;

    // Merge and display recent transactions
    auto expenseIt = userExpenses.begin();
    auto incomeIt = userIncomes.begin();

    while ((expenseIt != userExpenses.end() || incomeIt != userIncomes.end()) && itemCount < maxItems) {
        bool useExpense = true;

        if (expenseIt == userExpenses.end()) {
            useExpense = false;
        }
        else if (incomeIt != userIncomes.end() && incomeIt->date > expenseIt->date) {
            useExpense = false;
        }

        RECT itemRect = { rect.left + 15, yPos, rect.right - 15, yPos + itemHeight };

        if (useExpense) {
            // Draw expense
            SetTextColor(hdc, COLOR_DANGER);
            std::wstring expenseText = expenseIt->category + L" - $" +
                std::to_wstring((int)expenseIt->amount) + L".00";
            DrawText(hdc, expenseText.c_str(), -1, &itemRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            ++expenseIt;
        }
        else {
            // Draw income
            SetTextColor(hdc, COLOR_SUCCESS);
            std::wstring incomeText = incomeIt->source + L" + $" +
                std::to_wstring((int)incomeIt->amount) + L".00";
            DrawText(hdc, incomeText.c_str(), -1, &itemRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            ++incomeIt;
        }

        yPos += itemHeight;
        itemCount++;
    }

    SelectObject(hdc, oldFont);
}

void UIManager::DrawBudgetOverview(HDC hdc, RECT rect) {
    // Draw section background
    DrawRoundedRectangle(hdc, rect, 8, currentThemeColors[L"surface"], RGB(220, 220, 220));

    // Draw section title
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, currentThemeColors[L"text"]);

    HFONT oldFont = (HFONT)SelectObject(hdc, hFontBold);

    RECT titleRect = { rect.left + 15, rect.top + 15, rect.right - 15, rect.top + 40 };
    DrawText(hdc, L"Budget Overview", -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hFontNormal);

    if (!UserManager::IsUserLoggedIn()) {
        SelectObject(hdc, oldFont);
        return;
    }

    // Get user budgets
    std::wstring userId = UserManager::GetCurrentUserId();
    auto userBudgets = UserDataFilter::GetUserBudgets(userId);

    int yPos = rect.top + 50;
    int itemHeight = 40;
    int maxItems = (rect.bottom - rect.top - 60) / itemHeight;

    for (size_t i = 0; i < userBudgets.size() && i < static_cast<size_t>(maxItems); ++i) {
        const auto& budget = userBudgets[i];
        if (!budget.isActive) continue;

        RECT itemRect = { rect.left + 15, yPos, rect.right - 15, yPos + itemHeight };

        // Draw budget category
        SetTextColor(hdc, currentThemeColors[L"text"]);
        RECT categoryRect = { itemRect.left, itemRect.top, itemRect.right, itemRect.top + 20 };
        DrawText(hdc, budget.category.c_str(), -1, &categoryRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // Draw progress bar
        RECT progressRect = { itemRect.left, itemRect.top + 22, itemRect.right, itemRect.top + 35 };
        double percentage = budget.monthlyLimit > 0 ? (budget.currentSpent / budget.monthlyLimit) : 0.0;

        COLORREF progressColor = COLOR_SUCCESS;
        if (percentage > 1.0) progressColor = COLOR_DANGER;
        else if (percentage > budget.warningThreshold) progressColor = COLOR_WARNING;

        DrawProgressBar(hdc, progressRect, std::min(percentage, 1.0), progressColor);

        yPos += itemHeight;
    }

    SelectObject(hdc, oldFont);
}

// Control creation helpers
HWND UIManager::CreateStyledButton(HWND parent, const std::wstring& text, int id, int x, int y, int width, int height) {
    HWND hButton = CreateWindow(L"BUTTON", text.c_str(),
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW,
        x, y, width, height, parent, (HMENU)(UINT_PTR)id, GetModuleHandle(NULL), NULL);

    StyleButton(hButton);
    return hButton;
}

HWND UIManager::CreateStyledEdit(HWND parent, int id, int x, int y, int width, int height, DWORD style) {
    HWND hEdit = CreateWindow(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | style,
        x, y, width, height, parent, (HMENU)(UINT_PTR)id, GetModuleHandle(NULL), NULL);

    StyleEdit(hEdit);
    return hEdit;
}

HWND UIManager::CreateStyledCombo(HWND parent, int id, int x, int y, int width, int height) {
    HWND hCombo = CreateWindow(L"COMBOBOX", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,
        x, y, width, height, parent, (HMENU)(UINT_PTR)id, GetModuleHandle(NULL), NULL);

    StyleCombo(hCombo);
    return hCombo;
}

HWND UIManager::CreateStyledListView(HWND parent, int id, int x, int y, int width, int height) {
    HWND hListView = CreateWindow(WC_LISTVIEW, L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
        x, y, width, height, parent, (HMENU)(UINT_PTR)id, GetModuleHandle(NULL), NULL);

    StyleListView(hListView);
    return hListView;
}

HWND UIManager::CreateStyledLabel(HWND parent, const std::wstring& text, int x, int y, int width, int height, COLORREF color) {
    HWND hLabel = CreateWindow(L"STATIC", text.c_str(),
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        x, y, width, height, parent, NULL, GetModuleHandle(NULL), NULL);

    SendMessage(hLabel, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
    return hLabel;
}

// Theme management
void UIManager::ApplyDarkTheme(HWND hwnd) {
    currentThemeColors = darkThemeColors;
    RefreshTheme(hwnd);
}

void UIManager::ApplyLightTheme(HWND hwnd) {
    currentThemeColors = lightThemeColors;
    RefreshTheme(hwnd);
}

void UIManager::RefreshTheme(HWND hwnd) {
    // Update window background
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND,
        (LONG_PTR)CreateSolidBrush(currentThemeColors[L"background"]));

    InvalidateRect(hwnd, NULL, TRUE);
}

bool UIManager::IsDarkModeEnabled() {
    return currentThemeColors == darkThemeColors;
}

// Utility functions
void UIManager::CenterWindow(HWND hwnd, HWND parent) {
    RECT rcParent, rcWindow;

    if (parent) {
        GetWindowRect(parent, &rcParent);
    }
    else {
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcParent, 0);
    }

    GetWindowRect(hwnd, &rcWindow);

    int width = rcWindow.right - rcWindow.left;
    int height = rcWindow.bottom - rcWindow.top;

    int x = rcParent.left + (rcParent.right - rcParent.left - width) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - height) / 2;

    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void UIManager::ShowNotification(HWND parent, const std::wstring& message, const std::wstring& title, DWORD type) {
    MessageBox(parent, message.c_str(), title.c_str(), type);
}

// Drawing helpers
void UIManager::DrawGradientBackground(HDC hdc, RECT rect, COLORREF color1, COLORREF color2) {
    int height = rect.bottom - rect.top;

    for (int i = 0; i < height; ++i) {
        double ratio = static_cast<double>(i) / height;

        int r = GetRValue(color1) + static_cast<int>((GetRValue(color2) - GetRValue(color1)) * ratio);
        int g = GetGValue(color1) + static_cast<int>((GetGValue(color2) - GetGValue(color1)) * ratio);
        int b = GetBValue(color1) + static_cast<int>((GetBValue(color2) - GetBValue(color1)) * ratio);

        COLORREF lineColor = RGB(r, g, b);
        HPEN pen = CreatePen(PS_SOLID, 1, lineColor);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);

        MoveToEx(hdc, rect.left, rect.top + i, NULL);
        LineTo(hdc, rect.right, rect.top + i);

        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }
}

void UIManager::DrawRoundedRectangle(HDC hdc, RECT rect, int radius, COLORREF fillColor, COLORREF borderColor) {
    HBRUSH fillBrush = CreateSolidBrush(fillColor);
    HPEN borderPen = CreatePen(PS_SOLID, 1, borderColor);

    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);

    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(fillBrush);
    DeleteObject(borderPen);
}

void UIManager::DrawProgressBar(HDC hdc, RECT rect, double percentage, COLORREF color) {
    // Draw background
    HBRUSH bgBrush = CreateSolidBrush(RGB(230, 230, 230));
    FillRect(hdc, &rect, bgBrush);
    DeleteObject(bgBrush);

    // Draw progress
    if (percentage > 0) {
        RECT progressRect = rect;
        progressRect.right = rect.left + static_cast<int>((rect.right - rect.left) * percentage);

        HBRUSH progressBrush = CreateSolidBrush(color);
        FillRect(hdc, &progressRect, progressBrush);
        DeleteObject(progressBrush);
    }

    // Draw border
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);

    MoveToEx(hdc, rect.left, rect.top, NULL);
    LineTo(hdc, rect.right, rect.top);
    LineTo(hdc, rect.right, rect.bottom);
    LineTo(hdc, rect.left, rect.bottom);
    LineTo(hdc, rect.left, rect.top);

    SelectObject(hdc, oldPen);
    DeleteObject(borderPen);
}

// Font management
void UIManager::InitializeFonts() {
    hFontTitle = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    hFontNormal = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    hFontSmall = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    hFontBold = CreateFont(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
}

void UIManager::CleanupFonts() {
    if (hFontTitle) DeleteObject(hFontTitle);
    if (hFontNormal) DeleteObject(hFontNormal);
    if (hFontSmall) DeleteObject(hFontSmall);
    if (hFontBold) DeleteObject(hFontBold);
}

// Control styling
void UIManager::StyleButton(HWND hButton) {
    SendMessage(hButton, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
}

void UIManager::StyleEdit(HWND hEdit) {
    SendMessage(hEdit, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
}

void UIManager::StyleCombo(HWND hCombo) {
    SendMessage(hCombo, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
}

void UIManager::StyleListView(HWND hListView) {
    SendMessage(hListView, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
    ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
}

void UIManager::InitializeGraphics() {
    // Initialize GDI+ if needed
}

void UIManager::CleanupGraphics() {
    CleanupFonts();
}

// SidebarManager implementation
HWND SidebarManager::hSidebar = NULL;
SidebarManager::SidebarSection SidebarManager::currentSection = SidebarSection::DASHBOARD;
std::vector<std::pair<std::wstring, std::wstring>> SidebarManager::sidebarItems = {
    {L"Dashboard", L"🏠"},
    {L"Transactions", L"💰"},
    {L"Budget", L"📊"},
    {L"Goals", L"🎯"},
    {L"Analytics", L"📈"},
    {L"Settings", L"⚙️"}
};

void SidebarManager::CreateSidebar(HWND parent) {
    RECT parentRect;
    GetClientRect(parent, &parentRect);

    hSidebar = CreateWindow(L"STATIC", L"",
        WS_VISIBLE | WS_CHILD,
        0, TOOLBAR_HEIGHT,
        SIDEBAR_WIDTH, parentRect.bottom - TOOLBAR_HEIGHT - STATUSBAR_HEIGHT,
        parent, NULL, GetModuleHandle(NULL), NULL);

    SetWindowLongPtr(hSidebar, GWLP_WNDPROC, (LONG_PTR)SidebarProc);
}

void SidebarManager::UpdateSidebar(HWND hwnd) {
    if (hSidebar) {
        InvalidateRect(hSidebar, NULL, TRUE);
    }
}

void SidebarManager::SelectSection(SidebarSection section) {
    currentSection = section;
    UpdateSidebar(GetParent(hSidebar));
}

SidebarManager::SidebarSection SidebarManager::GetCurrentSection() {
    return currentSection;
}

void SidebarManager::RefreshSectionContent(HWND hwnd) {
    // Update main content based on selected section
    switch (currentSection) {
    case SidebarSection::DASHBOARD:
        // Dashboard is handled in PaintDashboard
        break;
    case SidebarSection::TRANSACTIONS:
        // Populate transaction list
        break;
    case SidebarSection::BUDGET:
        // Show budget overview
        break;
    case SidebarSection::GOALS:
        // Show savings goals
        break;
    case SidebarSection::ANALYTICS:
        // Show analytics summary
        break;
    case SidebarSection::SETTINGS:
        // Show settings
        break;
    }
}

LRESULT CALLBACK SidebarManager::SidebarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);

        DrawSidebarBackground(hdc, rect);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        HandleSidebarClick(pt.y);
        return 0;
    }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void SidebarManager::DrawSidebarBackground(HDC hdc, RECT rect) {
    // Draw sidebar background
    HBRUSH bgBrush = CreateSolidBrush(UIManager::currentThemeColors[L"surface"]);
    FillRect(hdc, &rect, bgBrush);
    DeleteObject(bgBrush);

    // Draw sidebar items
    int itemHeight = 50;
    int yPos = 20;

    SetBkMode(hdc, TRANSPARENT);

    for (size_t i = 0; i < sidebarItems.size(); ++i) {
        RECT itemRect = { 10, yPos, rect.right - 10, yPos + itemHeight };

        bool selected = (static_cast<SidebarSection>(i) == currentSection);
        DrawSidebarItem(hdc, itemRect, sidebarItems[i].first, sidebarItems[i].second, selected);

        yPos += itemHeight + 5;
    }
}

void SidebarManager::DrawSidebarItem(HDC hdc, RECT rect, const std::wstring& text, const std::wstring& icon, bool selected) {
    if (selected) {
        // Draw selection background
        HBRUSH selBrush = CreateSolidBrush(UIManager::currentThemeColors[L"primary"]);
        UIManager::DrawRoundedRectangle(hdc, rect, 5, UIManager::currentThemeColors[L"primary"], UIManager::currentThemeColors[L"primary"]);
        DeleteObject(selBrush);
        SetTextColor(hdc, RGB(255, 255, 255));
    }
    else {
        SetTextColor(hdc, UIManager::currentThemeColors[L"text"]);
    }

    // Draw icon
    RECT iconRect = { rect.left + 10, rect.top + 10, rect.left + 40, rect.top + 40 };
    DrawText(hdc, icon.c_str(), -1, &iconRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Draw text
    HFONT oldFont = (HFONT)SelectObject(hdc, UIManager::hFontNormal);
    RECT textRect = { rect.left + 45, rect.top, rect.right - 10, rect.bottom };
    DrawText(hdc, text.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
}

void SidebarManager::HandleSidebarClick(int y) {
    int itemHeight = 50;
    int spacing = 5;
    int startY = 20;

    int itemIndex = (y - startY) / (itemHeight + spacing);

    if (itemIndex >= 0 && itemIndex < static_cast<int>(sidebarItems.size())) {
        SelectSection(static_cast<SidebarSection>(itemIndex));
        RefreshSectionContent(GetParent(hSidebar));
    }
}

// CustomControls implementation
void CustomControls::DrawInfoCard(HDC hdc, RECT rect, const DashboardCard& card) {
    // Draw card background
    UIManager::DrawRoundedRectangle(hdc, rect, 8, UIManager::currentThemeColors[L"surface"], RGB(220, 220, 220));

    // Draw colored accent
    RECT accentRect = { rect.left, rect.top, rect.left + 4, rect.bottom };
    HBRUSH accentBrush = CreateSolidBrush(card.color);
    FillRect(hdc, &accentRect, accentBrush);
    DeleteObject(accentBrush);

    SetBkMode(hdc, TRANSPARENT);

    // Draw icon
    HFONT oldFont = (HFONT)SelectObject(hdc, UIManager::hFontTitle);
    SetTextColor(hdc, card.color);

    RECT iconRect = { rect.right - 50, rect.top + 10, rect.right - 10, rect.top + 50 };
    DrawText(hdc, card.iconType.c_str(), -1, &iconRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Draw title
    SelectObject(hdc, UIManager::hFontSmall);
    SetTextColor(hdc, UIManager::currentThemeColors[L"text"]);

    RECT titleRect = { rect.left + 15, rect.top + 15, rect.right - 60, rect.top + 35 };
    DrawText(hdc, card.title.c_str(), -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Draw value
    SelectObject(hdc, UIManager::hFontBold);
    SetTextColor(hdc, card.color);

    RECT valueRect = { rect.left + 15, rect.top + 40, rect.right - 60, rect.top + 70 };
    DrawText(hdc, card.value.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Draw subtitle
    SelectObject(hdc, UIManager::hFontSmall);
    SetTextColor(hdc, RGB(150, 150, 150));

    RECT subtitleRect = { rect.left + 15, rect.top + 75, rect.right - 60, rect.bottom - 10 };
    DrawText(hdc, card.subtitle.c_str(), -1, &subtitleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
}

// NotificationManager implementation
std::vector<NotificationManager::Notification> NotificationManager::activeNotifications;
HWND NotificationManager::hNotificationArea = NULL;

void NotificationManager::ShowNotification(const std::wstring& message, const std::wstring& title, NotificationType type, int duration) {
    Notification notification;
    notification.message = message;
    notification.title = title;
    notification.type = type;
    notification.timestamp = GetTickCount();
    notification.autoHide = true;
    notification.duration = duration;

    activeNotifications.push_back(notification);

    if (hNotificationArea) {
        InvalidateRect(hNotificationArea, NULL, TRUE);
    }
}

void NotificationManager::ShowBudgetAlert(const std::wstring& category, double spent, double limit) {
    std::wstringstream message;
    message << L"Budget exceeded for " << category << L": $" << std::fixed << std::setprecision(2) << spent
        << L" of $" << limit;

    ShowNotification(message.str(), L"Budget Alert", NotificationType::WARNING);
}

void NotificationManager::ShowGoalAchieved(const std::wstring& goalName) {
    ShowNotification(L"Congratulations! You've achieved your goal: " + goalName,
        L"Goal Achieved", NotificationType::SUCCESS);
}

COLORREF NotificationManager::GetNotificationColor(NotificationType type) {
    switch (type) {
    case NotificationType::SUCCESS: return COLOR_SUCCESS;
    case NotificationType::WARNING: return COLOR_WARNING;
    case NotificationType::ERROR_TYPE: return COLOR_DANGER;
    default: return COLOR_PRIMARY;
    }
}

// ResourceManager implementation
std::map<std::wstring, HICON> ResourceManager::loadedIcons;
std::map<std::wstring, HBITMAP> ResourceManager::loadedImages;
std::map<std::wstring, std::wstring> ResourceManager::loadedStrings;

const std::wstring ResourceManager::ICON_DASHBOARD = L"🏠";
const std::wstring ResourceManager::ICON_TRANSACTIONS = L"💰";
const std::wstring ResourceManager::ICON_BUDGET = L"📊";
const std::wstring ResourceManager::ICON_GOALS = L"🎯";
const std::wstring ResourceManager::ICON_ANALYTICS = L"📈";
const std::wstring ResourceManager::ICON_SETTINGS = L"⚙️";
const std::wstring ResourceManager::ICON_INCOME = L"💵";
const std::wstring ResourceManager::ICON_EXPENSE = L"💸";
const std::wstring ResourceManager::ICON_BALANCE = L"🏦";

bool ResourceManager::LoadIcons() {
    // In a real implementation, you would load actual icon resources
    return true;
}

bool ResourceManager::LoadImages() {
    // Load bitmap resources
    return true;
}

bool ResourceManager::LoadStrings() {
    // Load string resources for internationalization
    loadedStrings[L"welcome"] = L"Welcome to Personal Finance Tracker";
    loadedStrings[L"login_required"] = L"Please log in to continue";
    loadedStrings[L"expense_added"] = L"Expense added successfully";
    loadedStrings[L"income_added"] = L"Income added successfully";

    return true;
}

void ResourceManager::CleanupResources() {
    for (auto& pair : loadedIcons) {
        if (pair.second) DestroyIcon(pair.second);
    }

    for (auto& pair : loadedImages) {
        if (pair.second) DeleteObject(pair.second);
    }

    loadedIcons.clear();
    loadedImages.clear();
    loadedStrings.clear();
}

std::wstring ResourceManager::GetString(const std::wstring& key) {
    auto it = loadedStrings.find(key);
    return (it != loadedStrings.end()) ? it->second : key;
}