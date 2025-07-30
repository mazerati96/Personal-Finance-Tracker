#include "SpendingManager.h"
#include "DataStructures.h"
#include "Utils.h"
#include "resource.h"  
#include "UserManager.h"
// For resource IDs
#include <sstream>     // For 'ss', 'stream'
#include <iostream>    // For 'std::cout', 'out'
#include <chrono>      // For 'now' (if using time functions)
#include <vector>      // If not already included
#include <string>      // If not already included
#include <fstream>
#include <iomanip>
#include <sstream>     // For 'ss', 'stream'
#include <string>      // If not already included
#include <ctime>


// Global variables for dialog controls
HWND hExpenseDialog = NULL;
HWND hIncomeDialog = NULL;
HWND hCategoryEdit, hAmountEdit, hNoteEdit;
HWND hSourceEdit, hIncomeAmountEdit, hIncomeNoteEdit;



double GetTotalExpenses() {
    double total = 0.0;
    for (const auto& exp : expenses) {
        total += exp.amount;
    }
    return total;
}

double GetTotalIncome() {
    double total = 0.0;
    for (const auto& inc : incomes) {
        total += inc.amount;
    }
    return total;
}

double GetBalance() {
    return GetTotalIncome() - GetTotalExpenses();
}

// Get next unique expense ID
int GetNextExpenseId() {
    static int nextId = 1;
    return nextId++;
}

// Get next unique income ID  
int GetNextIncomeId() {
    static int nextId = 1;
    return nextId++;
}

LRESULT CALLBACK ExpenseDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BUTTON_OK: {
            wchar_t category[256], note[256], amountStr[50];

            GetWindowText(hCategoryEdit, category, 256);
            GetWindowText(hAmountEdit, amountStr, 50);
            GetWindowText(hNoteEdit, note, 256);

            if (wcslen(category) == 0 || wcslen(amountStr) == 0) {
                MessageBox(hwnd, L"Please fill in category and amount.", L"Error", MB_OK);
                return 0;
            }

            double amount = _wtof(amountStr);
            if (amount <= 0) {
                MessageBox(hwnd, L"Please enter a valid amount greater than 0.", L"Error", MB_OK);
                return 0;
            }

            Expense newExpense;
            newExpense.category = category;
            newExpense.amount = amount;
            newExpense.note = note;
            newExpense.date = GetCurrentDate();

            expenses.push_back(newExpense);

            MessageBox(hwnd, L"Expense added successfully!", L"Success", MB_OK);
            DestroyWindow(hwnd);
            return 0;
        }
        case ID_BUTTON_CANCEL:
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_DESTROY:
        hExpenseDialog = NULL;
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK IncomeDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BUTTON_OK: {
            wchar_t source[256], note[256], amountStr[50];

            GetWindowText(hSourceEdit, source, 256);
            GetWindowText(hIncomeAmountEdit, amountStr, 50);
            GetWindowText(hIncomeNoteEdit, note, 256);

            if (wcslen(source) == 0 || wcslen(amountStr) == 0) {
                MessageBox(hwnd, L"Please fill in source and amount.", L"Error", MB_OK);
                return 0;
            }

            double amount = _wtof(amountStr);
            if (amount <= 0) {
                MessageBox(hwnd, L"Please enter a valid amount greater than 0.", L"Error", MB_OK);
                return 0;
            }

            Income newIncome;
            newIncome.source = source;
            newIncome.amount = amount;
            newIncome.note = note;
            newIncome.date = GetCurrentDate();

            incomes.push_back(newIncome);

            MessageBox(hwnd, L"Income added successfully!", L"Success", MB_OK);
            DestroyWindow(hwnd);
            return 0;
        }
        case ID_BUTTON_CANCEL:
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_DESTROY:
        hIncomeDialog = NULL;
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void AddExpenseDialog(HWND parent) {
    if (hExpenseDialog != NULL) {
        SetForegroundWindow(hExpenseDialog);
        return;
    }

    // Register window class for expense dialog
    WNDCLASS wc = {};
    wc.lpfnWndProc = ExpenseDialogProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"ExpenseDialog";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    // Create dialog window
    hExpenseDialog = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        L"ExpenseDialog",
        L"Add New Expense",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 280,
        parent, NULL, GetModuleHandle(NULL), NULL
    );

    if (!hExpenseDialog) return;

    // Create controls
    CreateWindow(L"STATIC", L"Category:", WS_VISIBLE | WS_CHILD,
        20, 25, 80, 20, hExpenseDialog, NULL, GetModuleHandle(NULL), NULL);
    hCategoryEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        110, 22, 200, 25, hExpenseDialog, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow(L"STATIC", L"Amount ($):", WS_VISIBLE | WS_CHILD,
        20, 65, 80, 20, hExpenseDialog, NULL, GetModuleHandle(NULL), NULL);
    hAmountEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        110, 62, 120, 25, hExpenseDialog, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow(L"STATIC", L"Note (optional):", WS_VISIBLE | WS_CHILD,
        20, 105, 100, 20, hExpenseDialog, NULL, GetModuleHandle(NULL), NULL);
    hNoteEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        110, 102, 200, 80, hExpenseDialog, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow(L"BUTTON", L"Add Expense", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        90, 200, 100, 35, hExpenseDialog, (HMENU)ID_BUTTON_OK, GetModuleHandle(NULL), NULL);
    CreateWindow(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        200, 200, 80, 35, hExpenseDialog, (HMENU)ID_BUTTON_CANCEL, GetModuleHandle(NULL), NULL);

    ShowWindow(hExpenseDialog, SW_SHOW);
    SetFocus(hCategoryEdit);
}

void AddIncomeDialog(HWND parent) {
    if (hIncomeDialog != NULL) {
        SetForegroundWindow(hIncomeDialog);
        return;
    }

    // Register window class for income dialog
    WNDCLASS wc = {};
    wc.lpfnWndProc = IncomeDialogProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"IncomeDialog";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    // Create dialog window
    hIncomeDialog = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        L"IncomeDialog",
        L"Add New Income",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 280,
        parent, NULL, GetModuleHandle(NULL), NULL
    );

    if (!hIncomeDialog) return;

    // Create controls
    CreateWindow(L"STATIC", L"Source:", WS_VISIBLE | WS_CHILD,
        20, 25, 80, 20, hIncomeDialog, NULL, GetModuleHandle(NULL), NULL);
    hSourceEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        110, 22, 200, 25, hIncomeDialog, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow(L"STATIC", L"Amount ($):", WS_VISIBLE | WS_CHILD,
        20, 65, 80, 20, hIncomeDialog, NULL, GetModuleHandle(NULL), NULL);
    hIncomeAmountEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        110, 62, 120, 25, hIncomeDialog, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow(L"STATIC", L"Note (optional):", WS_VISIBLE | WS_CHILD,
        20, 105, 100, 20, hIncomeDialog, NULL, GetModuleHandle(NULL), NULL);
    hIncomeNoteEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        110, 102, 200, 80, hIncomeDialog, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow(L"BUTTON", L"Add Income", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        90, 200, 100, 35, hIncomeDialog, (HMENU)ID_BUTTON_OK, GetModuleHandle(NULL), NULL);
    CreateWindow(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        200, 200, 80, 35, hIncomeDialog, (HMENU)ID_BUTTON_CANCEL, GetModuleHandle(NULL), NULL);

    ShowWindow(hIncomeDialog, SW_SHOW);
    SetFocus(hSourceEdit);
}

void ShowExpensesDialog(HWND hwnd) {
    std::wstringstream ss;
    ss << L"EXPENSES:\n";
    ss << L"=========\n\n";

    if (expenses.empty()) {
        ss << L"No expenses recorded.\n";
    }
    else {
        for (size_t i = 0; i < expenses.size(); i++) {
            const auto& exp = expenses[i];
            ss << L"#" << (i + 1) << L" - " << exp.date << L"\n";
            ss << L"Category: " << exp.category << L"\n";
            ss << L"Amount: $" << std::fixed << std::setprecision(2) << exp.amount << L"\n";
            if (!exp.note.empty()) {
                ss << L"Note: " << exp.note << L"\n";
            }
            ss << L"\n";
        }
        ss << L"TOTAL EXPENSES: $" << std::fixed << std::setprecision(2) << GetTotalExpenses() << L"\n";
    }

    MessageBox(hwnd, ss.str().c_str(), L"View Expenses", MB_OK);
}

void ShowIncomeDialog(HWND hwnd) {
    std::wstringstream ss;
    ss << L"INCOME:\n";
    ss << L"=======\n\n";

    if (incomes.empty()) {
        ss << L"No income recorded.\n";
    }
    else {
        for (size_t i = 0; i < incomes.size(); i++) {
            const auto& inc = incomes[i];
            ss << L"#" << (i + 1) << L" - " << inc.date << L"\n";
            ss << L"Source: " << inc.source << L"\n";
            ss << L"Amount: $" << std::fixed << std::setprecision(2) << inc.amount << L"\n";
            if (!inc.note.empty()) {
                ss << L"Note: " << inc.note << L"\n";
            }
            ss << L"\n";
        }
        ss << L"TOTAL INCOME: $" << std::fixed << std::setprecision(2) << GetTotalIncome() << L"\n";
    }

    MessageBox(hwnd, ss.str().c_str(), L"View Income", MB_OK);
}

void ShowSummaryDialog(HWND hwnd) {
    std::wstringstream ss;
    ss << L"FINANCIAL SUMMARY:\n";
    ss << L"==================\n\n";
    ss << L"Total Income:  $" << std::fixed << std::setprecision(2) << GetTotalIncome() << L"\n";
    ss << L"Total Expenses: $" << std::fixed << std::setprecision(2) << GetTotalExpenses() << L"\n";
    ss << L"Balance:       $" << std::fixed << std::setprecision(2) << GetBalance() << L"\n\n";

    if (GetBalance() >= 0) {
        ss << L"Status: You're in the positive! 💰";
    }
    else {
        ss << L"Status: You're overspending! ⚠️";
    }

    MessageBox(hwnd, ss.str().c_str(), L"Financial Summary", MB_OK);
}

void SaveDataToFile() {
    std::wofstream file(L"finance_data.txt");
    if (!file.is_open()) {
        MessageBox(NULL, L"Could not save data to file.", L"Error", MB_OK);
        return;
    }

    // Save expenses
    file << L"EXPENSES:" << std::endl;
    for (const auto& exp : expenses) {
        file << exp.date << L"|" << exp.category << L"|" << exp.amount << L"|" << exp.note << std::endl;
    }

    file << L"INCOME:" << std::endl;
    for (const auto& inc : incomes) {
        file << inc.date << L"|" << inc.source << L"|" << inc.amount << L"|" << inc.note << std::endl;
    }

    file.close();
    MessageBox(NULL, L"Data saved successfully!", L"Save", MB_OK);
}



void LoadDataFromFile() {
    std::wifstream file(L"finance_data.txt");
    if (!file.is_open()) {
        // File doesn't exist yet, which is fine for first run
        return;
    }

    expenses.clear();
    incomes.clear();

    std::wstring line;
    bool readingExpenses = false;
    bool readingIncome = false;
    
    while (getline(file, line)) {
        if (line == L"EXPENSES:") {
            readingExpenses = true;
            readingIncome = false;
            continue;
        }
        else if (line == L"INCOME:") {
            readingExpenses = false;
            readingIncome = true;
            continue;
        }

        if (line.empty()) continue;

        // Parse line: date|category/source|amount|note
        size_t pos1 = line.find(L'|');
        size_t pos2 = line.find(L'|', pos1 + 1);
        size_t pos3 = line.find(L'|', pos2 + 1);

        if (pos1 != std::wstring::npos && pos2 != std::wstring::npos && pos3 != std::wstring::npos) {
            std::wstring date = line.substr(0, pos1);
            std::wstring categoryOrSource = line.substr(pos1 + 1, pos2 - pos1 - 1);
            double amount = _wtof(line.substr(pos2 + 1, pos3 - pos2 - 1).c_str());  // Fixed: _wtof and c_str()
            std::wstring note = line.substr(pos3 + 1);

            if (readingExpenses) {
                Expense newExpense;
                newExpense.id = L"EXP_" + std::to_wstring(GetNextExpenseId());  // Generate unique ID
                newExpense.userId = UserManager::currentUsername;  // Use current username (wstring)
                newExpense.amount = amount;
                newExpense.note = note;           // FIXED: Use 'note', not 'description'
                newExpense.category = categoryOrSource;
                newExpense.date = date;
                newExpense.currency = CurrencyType::USD;  // Set default values
                newExpense.exchangeRate = 1.0;
                expenses.push_back(newExpense);
            }
            else if (readingIncome) {
                Income newIncome;
                newIncome.id = L"INC_" + std::to_wstring(GetNextIncomeId());  // Generate unique ID
                newIncome.userId = UserManager::currentUsername;  // Use current username (wstring)
                newIncome.amount = amount;
                newIncome.note = note;            // FIXED: Use 'note', not 'description'
                newIncome.source = categoryOrSource;
                newIncome.date = date;
                newIncome.currency = CurrencyType::USD;  // Set default values
                newIncome.exchangeRate = 1.0;
                newIncome.isTaxable = true;
                incomes.push_back(newIncome);
            }
        }
    }

    file.close();
}