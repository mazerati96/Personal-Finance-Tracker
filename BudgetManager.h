#pragma once
#include <windows.h>

class BudgetManager {
public:
    static void CheckBudgetAlerts(HWND hwnd);
    static void ShowBudgetAlertsDialog(HWND hwnd);
    static void ShowBudgetSummaryDialog(HWND hwnd);
    static void ShowBudgetManagerDialog(HWND hwnd);
};
