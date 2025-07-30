#include "FinanceManager.h"
#include "UserManager.h"
#include "DatabaseManager.h"
#include "Utils.h"
#include <commctrl.h>
#include <commdlg.h>
#include <sstream>
#include <algorithm>
#include <objbase.h>  // For CoCreateGuid
#include <time.h>     // For time()
#include <vector>
#include <string>
#include <functional>  // For std::function

// Static member definitions - CRITICAL: Define static members
std::vector<Expense> FinanceManager::expenses;

std::vector<Category> FinanceManager::categories;

// Static callback definitions
std::function<void()> FinanceManager::OnTransactionAdded = nullptr;
std::function<void()> FinanceManager::OnTransactionUpdated = nullptr;
std::function<void()> FinanceManager::OnTransactionDeleted = nullptr;

// Static constant definitions
const std::vector< std::wstring > FinanceManager::DEFAULT_INCOME_SOURCES = {
    L"Salary",
    L"Freelance",
    L"Investments",
    L"Gifts",
    L"Other"
};


std::wstring FinanceManager::GenerateUniqueId() {
    GUID guid;
    if (CoCreateGuid(&guid) == S_OK) {
        wchar_t guidStr[40];
        StringFromGUID2(guid, guidStr, 40);
        return std::wstring(guidStr);
    }
    // Fallback if GUID generation fails
    static int counter = 0;
    return L"ID-" + std::to_wstring(++counter) +
        L"-" + std::to_wstring(time(nullptr));
}

void FinanceManager::UpdateBudgetSpending(const std::wstring& userId,
    const std::wstring& category,
    double amount) {
    // Find user's budget for this category
    for (auto& budget : budgets) {
        if (budget.userId == userId && budget.category == category) {
            budget.currentSpent += amount;
            break;
        }
    }
}

// Transaction dialogs
void FinanceManager::ShowAddExpenseDialog(HWND parent) {
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADD_EXPENSE_DIALOG), parent, AddExpenseDialogProc);
}

void FinanceManager::ShowAddIncomeDialog(HWND parent) {
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADD_INCOME_DIALOG), parent, AddIncomeDialogProc);
}

void FinanceManager::ShowEditTransactionDialog(HWND parent, const std::wstring& transactionId, bool isExpense) {
    // Store transaction ID for the dialog to use
    SetWindowLongPtr(parent, GWLP_USERDATA, (LONG_PTR)transactionId.c_str());
    SetWindowLongPtr(parent, GWLP_USERDATA, static_cast<LONG_PTR>(isExpense ? 1 : 0));


    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_EDIT_TRANSACTION_DIALOG), parent, EditTransactionDialogProc);
}

// Transaction management
bool FinanceManager::AddExpense(const Expense& expense) {
    if (!ValidateExpense(expense)) {
        return false;
    }

    Expense newExpense = expense;
    if (newExpense.id.empty()) {
        newExpense.id = GenerateUniqueId();
    }

    expenses.push_back(newExpense);

    // Update budget spending
    UpdateBudgetSpending(expense.userId, expense.category, expense.amount);

    // Save data
    DatabaseManager::SaveAllData();

    if (OnTransactionAdded) {
        OnTransactionAdded();
    }

    return true;
}

bool FinanceManager::AddIncome(const Income& income) {
    if (!ValidateIncome(income)) {
        return false;
    }

    Income newIncome = income;
    if (newIncome.id.empty()) {
        newIncome.id = GenerateUniqueId();
    }

    incomes.push_back(newIncome);

    // Save data
    DatabaseManager::SaveAllData();

    if (OnTransactionAdded) {
        OnTransactionAdded();
    }

    return true;
}

bool FinanceManager::UpdateExpense(const std::wstring& id, const Expense& expense) {
    auto it = std::find_if(expenses.begin(), expenses.end(),
        [&id](const Expense& e) { return e.id == id; });

    if (it != expenses.end()) {
        // Update budget spending (remove old, add new)
        UpdateBudgetSpending(it->userId, it->category, -it->amount);

        *it = expense;
        it->id = id; // Preserve ID

        UpdateBudgetSpending(expense.userId, expense.category, expense.amount);

        DatabaseManager::SaveAllData();

        if (OnTransactionUpdated) {
            OnTransactionUpdated();
        }

        return true;
    }

    return false;
}

bool FinanceManager::UpdateIncome(const std::wstring& id, const Income& income) {
    auto it = std::find_if(incomes.begin(), incomes.end(),
        [&id](const Income& e) { return e.id == id; });

    if (it != incomes.end()) {
        *it = income;
        it->id = id; // Preserve ID

        DatabaseManager::SaveAllData();

        if (OnTransactionUpdated) {
            OnTransactionUpdated();
        }

        return true;
    }

    return false;
}

bool FinanceManager::DeleteExpense(const std::wstring& id) {
    auto it = std::find_if(expenses.begin(), expenses.end(),
        [&id](const Expense& e) { return e.id == id; });

    if (it != expenses.end()) {
        // Update budget spending
        UpdateBudgetSpending(it->userId, it->category, -it->amount);

        expenses.erase(it);

        DatabaseManager::SaveAllData();

        if (OnTransactionDeleted) {
            OnTransactionDeleted();
        }

        return true;
    }

    return false;
}

bool FinanceManager::DeleteIncome(const std::wstring& id) {
    auto it = std::find_if(incomes.begin(), incomes.end(),
        [&id](const Income& i) { return i.id == id; });

    if (it != incomes.end()) {
        incomes.erase(it);

        DatabaseManager::SaveAllData();

        if (OnTransactionDeleted) {
            OnTransactionDeleted();
        }

        return true;
    }

    return false;
}

// Transaction retrieval
Expense* FinanceManager::GetExpenseById(const std::wstring& id) {
    auto it = std::find_if(expenses.begin(), expenses.end(),
        [&id](const Expense& e) { return e.id == id; });

    return (it != expenses.end()) ? &(*it) : nullptr;
}

Income* FinanceManager::GetIncomeById(const std::wstring& id) {
    auto it = std::find_if(incomes.begin(), incomes.end(),
        [&id](const Income& i) { return i.id == id; });

    return (it != incomes.end()) ? &(*it) : nullptr;
}

std::vector<Expense> FinanceManager::GetUserExpenses(const std::wstring& userId) {
    std::vector<Expense> userExpenses;
    for (const auto& expense : expenses) {
        if (expense.userId == userId) {
            userExpenses.push_back(expense);
        }
    }
    return userExpenses;
}

std::vector<Income> FinanceManager::GetUserIncomes(const std::wstring& userId) {
    std::vector<Income> userIncomes;
    for (const auto& income : incomes) {
        if (income.userId == userId) {
            userIncomes.push_back(income);
        }
    }
    return userIncomes;
}

// Validation
bool FinanceManager::ValidateTransactionData(const std::wstring& category, double amount, const std::wstring& date) {
    if (category.empty()) {
        MessageBox(NULL, L"Category cannot be empty.", L"Validation Error", MB_OK);
        return false;
    }

    if (amount <= 0) {
        MessageBox(NULL, L"Amount must be greater than zero.", L"Validation Error", MB_OK);
        return false;
    }

    if (date.empty()) {
        MessageBox(NULL, L"Date cannot be empty.", L"Validation Error", MB_OK);
        return false;
    }

    return true;
}

bool FinanceManager::ValidateAmount(const std::wstring& amountStr, double& amount) {
    if (amountStr.empty()) {
        return false;
    }

    try {
        amount = std::stod(amountStr);
        return amount > 0;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool FinanceManager::ValidateDate(const std::wstring& date) {
    // Simple date validation - could be enhanced
    return !date.empty() && date.length() >= 8; // YYYY-MM-DD minimum
}

// Utility functions
std::vector<std::wstring> FinanceManager::ParseTags(const std::wstring& tagString) {
    std::vector<std::wstring> tags;
    std::wstringstream ss(tagString);
    std::wstring tag;

    while (std::getline(ss, tag, L',')) {
        // Trim whitespace
        tag.erase(0, tag.find_first_not_of(L" \t"));
        tag.erase(tag.find_last_not_of(L" \t") + 1);

        if (!tag.empty()) {
            tags.push_back(tag);
        }
    }

    return tags;
}

std::wstring FinanceManager::TagsToString(const std::vector<std::wstring>& tags) {
    std::wstring result;
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) result += L", ";
        result += tags[i];
    }
    return result;
}

std::wstring FinanceManager::SelectReceiptFile(HWND parent) {
    wchar_t fileName[MAX_PATH] = L"";

    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = parent;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Image Files\0*.jpg;*.jpeg;*.png;*.bmp;*.gif\0PDF Files\0*.pdf\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        return std::wstring(fileName);
    }

    return L"";
}

// Dialog procedures
LRESULT CALLBACK FinanceManager::AddExpenseDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        SetWindowText(hDlg, L"Add New Expense");

        // Create controls
        CreateWindow(L"STATIC", L"Category:", WS_VISIBLE | WS_CHILD,
            20, 20, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hCategoryCombo = CreateWindow(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWN,
            110, 18, 200, 150, hDlg, (HMENU)IDC_EXPENSE_CATEGORY_COMBO, GetModuleHandle(NULL), NULL);
        PopulateCategoryCombo(hCategoryCombo);

        CreateWindow(L"STATIC", L"Amount:", WS_VISIBLE | WS_CHILD,
            20, 60, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            110, 58, 120, 25, hDlg, (HMENU)IDC_EXPENSE_AMOUNT_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Date:", WS_VISIBLE | WS_CHILD,
            20, 100, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hDatePicker = CreateWindow(DATETIMEPICK_CLASS, L"", WS_VISIBLE | WS_CHILD | WS_BORDER | DTS_SHORTDATEFORMAT,
            110, 98, 120, 25, hDlg, (HMENU)IDC_EXPENSE_DATE_PICKER, GetModuleHandle(NULL), NULL);
        SetupDatePicker(hDatePicker);

        CreateWindow(L"STATIC", L"Note:", WS_VISIBLE | WS_CHILD,
            20, 140, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
            110, 138, 200, 60, hDlg, (HMENU)IDC_EXPENSE_NOTE_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Tags:", WS_VISIBLE | WS_CHILD,
            20, 210, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            110, 208, 200, 25, hDlg, (HMENU)IDC_EXPENSE_TAGS_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Currency:", WS_VISIBLE | WS_CHILD,
            20, 250, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hCurrencyCombo = CreateWindow(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,
            110, 248, 100, 100, hDlg, (HMENU)IDC_EXPENSE_CURRENCY_COMBO, GetModuleHandle(NULL), NULL);
        PopulateCurrencyCombo(hCurrencyCombo);

        CreateWindow(L"STATIC", L"Location:", WS_VISIBLE | WS_CHILD,
            20, 290, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            110, 288, 200, 25, hDlg, (HMENU)IDC_EXPENSE_LOCATION_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"Receipt...", WS_VISIBLE | WS_CHILD,
            110, 325, 80, 30, hDlg, (HMENU)IDC_EXPENSE_RECEIPT_BTN, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"Add Expense", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            120, 370, 100, 35, hDlg, (HMENU)IDOK, GetModuleHandle(NULL), NULL);
        CreateWindow(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD,
            230, 370, 80, 35, hDlg, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_EXPENSE_RECEIPT_BTN:
        {
            std::wstring receiptPath = SelectReceiptFile(hDlg);
            if (!receiptPath.empty()) {
                // Store the receipt path (you might want to display it in a label)
                SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)receiptPath.c_str());
            }
            return TRUE;
        }

        case IDOK:
        {
            Expense expense;
            if (GetExpenseFromDialog(hDlg, expense)) {
                if (!UserManager::IsUserLoggedIn()) {
                    MessageBox(hDlg, L"No user logged in.", L"Error", MB_OK);
                    return TRUE;
                }

                expense.userId = UserManager::GetCurrentUserId();

                if (AddExpense(expense)) {
                    MessageBox(hDlg, L"Expense added successfully!", L"Success", MB_OK);
                    EndDialog(hDlg, IDOK);
                }
                else {
                    MessageBox(hDlg, L"Failed to add expense.", L"Error", MB_OK);
                }
            }
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

LRESULT CALLBACK FinanceManager::AddIncomeDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        SetWindowText(hDlg, L"Add New Income");

        // Create controls
        CreateWindow(L"STATIC", L"Source:", WS_VISIBLE | WS_CHILD,
            20, 20, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hSourceCombo = CreateWindow(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWN,
            110, 18, 200, 150, hDlg, (HMENU)IDC_INCOME_SOURCE_COMBO, GetModuleHandle(NULL), NULL);
        PopulateIncomeSourceCombo(hSourceCombo);

        CreateWindow(L"STATIC", L"Amount:", WS_VISIBLE | WS_CHILD,
            20, 60, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            110, 58, 120, 25, hDlg, (HMENU)IDC_INCOME_AMOUNT_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Date:", WS_VISIBLE | WS_CHILD,
            20, 100, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hDatePicker = CreateWindow(DATETIMEPICK_CLASS, L"", WS_VISIBLE | WS_CHILD | WS_BORDER | DTS_SHORTDATEFORMAT,
            110, 98, 120, 25, hDlg, (HMENU)IDC_INCOME_DATE_PICKER, GetModuleHandle(NULL), NULL);
        SetupDatePicker(hDatePicker);

        CreateWindow(L"STATIC", L"Note:", WS_VISIBLE | WS_CHILD,
            20, 140, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
            110, 138, 200, 60, hDlg, (HMENU)IDC_INCOME_NOTE_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Tags:", WS_VISIBLE | WS_CHILD,
            20, 210, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            110, 208, 200, 25, hDlg, (HMENU)IDC_INCOME_TAGS_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Currency:", WS_VISIBLE | WS_CHILD,
            20, 250, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hCurrencyCombo = CreateWindow(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,
            110, 248, 100, 100, hDlg, (HMENU)IDC_INCOME_CURRENCY_COMBO, GetModuleHandle(NULL), NULL);
        PopulateCurrencyCombo(hCurrencyCombo);

        CreateWindow(L"BUTTON", L"Taxable", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            110, 290, 100, 25, hDlg, (HMENU)IDC_INCOME_TAXABLE_CHECK, GetModuleHandle(NULL), NULL);
        CheckDlgButton(hDlg, IDC_INCOME_TAXABLE_CHECK, BST_CHECKED); // Default to taxable

        CreateWindow(L"BUTTON", L"Add Income", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            120, 330, 100, 35, hDlg, (HMENU)IDOK, GetModuleHandle(NULL), NULL);
        CreateWindow(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD,
            230, 330, 80, 35, hDlg, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        {
            Income income;
            if (GetIncomeFromDialog(hDlg, income)) {
                if (!UserManager::IsUserLoggedIn()) {
                    MessageBox(hDlg, L"No user logged in.", L"Error", MB_OK);
                    return TRUE;
                }

                income.userId = UserManager::GetCurrentUserId();

                if (AddIncome(income)) {
                    MessageBox(hDlg, L"Income added successfully!", L"Success", MB_OK);
                    EndDialog(hDlg, IDOK);
                }
                else {
                    MessageBox(hDlg, L"Failed to add income.", L"Error", MB_OK);
                }
            }
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

LRESULT CALLBACK FinanceManager::EditTransactionDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    // Implementation similar to Add dialogs but with pre-populated data
    // This is a simplified version
    switch (message) {
    case WM_INITDIALOG:
    {
        SetWindowText(hDlg, L"Edit Transaction");
        // Set up controls and populate with existing data
        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            // Update transaction logic
            EndDialog(hDlg, IDOK);
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

// Helper functions
void FinanceManager::PopulateCategoryCombo(HWND hCombo) {
    for (const auto& category : categories) {
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)category.name.c_str());
    }
}

void FinanceManager::PopulateIncomeSourceCombo(HWND hCombo) {
    for (const auto& source : DEFAULT_INCOME_SOURCES) {
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)source.c_str());
    }
}

void FinanceManager::PopulateCurrencyCombo(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"USD");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"EUR");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"GBP");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"JPY");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"CAD");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"AUD");

    // Select default currency based on user preference
    if (UserManager::IsUserLoggedIn()) {
        User* user = UserManager::GetCurrentUser();
        if (user) {
            SendMessage(hCombo, CB_SETCURSEL, (int)user->defaultCurrency, 0);
        }
    }
    else {
        SendMessage(hCombo, CB_SETCURSEL, 0, 0); // Default to USD
    }
}

void FinanceManager::SetupDatePicker(HWND hDatePicker) {
    // Set current date as default
    SYSTEMTIME st;
    GetLocalTime(&st);
    DateTime_SetSystemtime(hDatePicker, GDT_VALID, &st);
}

bool FinanceManager::GetExpenseFromDialog(HWND hDlg, Expense& expense) {
    wchar_t buffer[512];

    // Get category
    GetDlgItemText(hDlg, IDC_EXPENSE_CATEGORY_COMBO, buffer, 512);
    expense.category = buffer;

    // Get amount
    GetDlgItemText(hDlg, IDC_EXPENSE_AMOUNT_EDIT, buffer, 512);
    if (!ValidateAmount(buffer, expense.amount)) {
        MessageBox(hDlg, L"Please enter a valid amount.", L"Validation Error", MB_OK);
        return false;
    }

    // Get date
    SYSTEMTIME st;
    if (DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_EXPENSE_DATE_PICKER), &st) == GDT_VALID) {
        wchar_t dateStr[32];
        swprintf_s(dateStr, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
        expense.date = dateStr;
    }

    // Get note
    GetDlgItemText(hDlg, IDC_EXPENSE_NOTE_EDIT, buffer, 512);
    expense.note = buffer;

    // Get tags
    GetDlgItemText(hDlg, IDC_EXPENSE_TAGS_EDIT, buffer, 512);
    expense.tags = ParseTags(buffer);

    // Get currency
    int currencySelection = (int)SendDlgItemMessage(hDlg, IDC_EXPENSE_CURRENCY_COMBO, CB_GETCURSEL, 0, 0);
    switch (currencySelection) {
    case 0: expense.currency = CurrencyType::USD; break;
    case 1: expense.currency = CurrencyType::EUR; break;
    case 2: expense.currency = CurrencyType::GBP; break;
    case 3: expense.currency = CurrencyType::JPY; break;
    case 4: expense.currency = CurrencyType::CAD; break;
    case 5: expense.currency = CurrencyType::AUD; break;
    default: expense.currency = CurrencyType::USD; break;
    }

    // Get location
    GetDlgItemText(hDlg, IDC_EXPENSE_LOCATION_EDIT, buffer, 512);
    expense.location = buffer;

    // Get receipt path (if any)
    wchar_t* receiptPath = (wchar_t*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    if (receiptPath) {
        expense.receiptPath = receiptPath;
    }

    // Validate required fields
    if (!ValidateTransactionData(expense.category, expense.amount, expense.date)) {
        return false;
    }

    return true;
}

bool FinanceManager::GetIncomeFromDialog(HWND hDlg, Income& income) {
    wchar_t buffer[512];

    // Get source
    GetDlgItemText(hDlg, IDC_INCOME_SOURCE_COMBO, buffer, 512);
    income.source = buffer;

    // Get amount
    GetDlgItemText(hDlg, IDC_INCOME_AMOUNT_EDIT, buffer, 512);
    if (!ValidateAmount(buffer, income.amount)) {
        MessageBox(hDlg, L"Please enter a valid amount.", L"Validation Error", MB_OK);
        return false;
    }

    // Get date
    SYSTEMTIME st;
    if (DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_INCOME_DATE_PICKER), &st) == GDT_VALID) {
        wchar_t dateStr[32];
        swprintf_s(dateStr, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
        income.date = dateStr;
    }

    // Get note
    GetDlgItemText(hDlg, IDC_INCOME_NOTE_EDIT, buffer, 512);
    income.note = buffer;

    // Get tags
    GetDlgItemText(hDlg, IDC_INCOME_TAGS_EDIT, buffer, 512);
    income.tags = ParseTags(buffer);

    // Get currency
    int currencySelection = (int)SendDlgItemMessage(hDlg, IDC_INCOME_CURRENCY_COMBO, CB_GETCURSEL, 0, 0);
    switch (currencySelection) {
    case 0: income.currency = CurrencyType::USD; break;
    case 1: income.currency = CurrencyType::EUR; break;
    case 2: income.currency = CurrencyType::GBP; break;
    case 3: income.currency = CurrencyType::JPY; break;
    case 4: income.currency = CurrencyType::CAD; break;
    case 5: income.currency = CurrencyType::AUD; break;
    default: income.currency = CurrencyType::USD; break;
    }

    // Get taxable status
    income.isTaxable = IsDlgButtonChecked(hDlg, IDC_INCOME_TAXABLE_CHECK) == BST_CHECKED;

    // Validate required fields
    if (!ValidateTransactionData(income.source, income.amount, income.date)) {
        return false;
    }

    return true;
}

void FinanceManager::SetExpenseToDialog(HWND hDlg, const Expense& expense) {
    SetDlgItemText(hDlg, IDC_EXPENSE_CATEGORY_COMBO, expense.category.c_str());

    wchar_t amountStr[32];
    swprintf_s(amountStr, L"%.2f", expense.amount);
    SetDlgItemText(hDlg, IDC_EXPENSE_AMOUNT_EDIT, amountStr);

    SetDlgItemText(hDlg, IDC_EXPENSE_NOTE_EDIT, expense.note.c_str());
    SetDlgItemText(hDlg, IDC_EXPENSE_TAGS_EDIT, TagsToString(expense.tags).c_str());
    SetDlgItemText(hDlg, IDC_EXPENSE_LOCATION_EDIT, expense.location.c_str());

    // Set currency combo
    SendDlgItemMessage(hDlg, IDC_EXPENSE_CURRENCY_COMBO, CB_SETCURSEL, (int)expense.currency, 0);

    // Set date picker (would need date parsing)
    // ... date picker setup code ...
}

void FinanceManager::SetIncomeToDialog(HWND hDlg, const Income& income) {
    SetDlgItemText(hDlg, IDC_INCOME_SOURCE_COMBO, income.source.c_str());

    wchar_t amountStr[32];
    swprintf_s(amountStr, L"%.2f", income.amount);
    SetDlgItemText(hDlg, IDC_INCOME_AMOUNT_EDIT, amountStr);

    SetDlgItemText(hDlg, IDC_INCOME_NOTE_EDIT, income.note.c_str());
    SetDlgItemText(hDlg, IDC_INCOME_TAGS_EDIT, TagsToString(income.tags).c_str());

    // Set currency combo
    SendDlgItemMessage(hDlg, IDC_INCOME_CURRENCY_COMBO, CB_SETCURSEL, (int)income.currency, 0);

    // Set taxable checkbox
    CheckDlgButton(hDlg, IDC_INCOME_TAXABLE_CHECK, income.isTaxable ? BST_CHECKED : BST_UNCHECKED);

    // Set date picker (would need date parsing)
    // ... date picker setup code ...
}