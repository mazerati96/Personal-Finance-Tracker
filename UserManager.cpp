#include "Utils.h"
#include "UserManager.h"
#include "DataStructures.h"
#include <sstream>

#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>

std::wstring UserManager::currentUsername = L""; // TODO: set appropriately

void UserManager::PopulateUserList(HWND hListBox) {
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    for (const auto& user : users) {
        // Add username to listbox
        int index = static_cast<int>(SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)user.username.c_str()));
        // Note: No need to store additional data since we use username directly
    }
}


void UserManager::PopulateCurrencyCombo(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"USD");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"EUR");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"GBP");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"JPY");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"CAD");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"AUD");
    SendMessage(hCombo, CB_SETCURSEL, 0, 0); // Default to USD
}

void UserManager::PopulateAuthTypeCombo(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"None");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"PIN");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Password");
    SendMessage(hCombo, CB_SETCURSEL, 0, 0); // Default to None
}

// Static member initialization
const std::wstring UserManager::SALT = L"PersonalFinanceTracker2025";
std::function<void()> UserManager::OnUserLoggedIn = nullptr;
std::function<void()> UserManager::OnUserLoggedOut = nullptr;


// User authentication
bool UserManager::AuthenticateUser(const std::wstring& username, const std::wstring& auth) {
    auto user = GetUserByUsername(username);
    if (!user) return false;
    if (user->authType == AuthType::NONE) {
        return true; // No authentication required
    }
    std::wstring hashedAuth = HashString(auth);
    return user->authHash == hashedAuth;
}

bool UserManager::CreateUser(const User& user, const std::wstring& rawAuth) {
    // Check if username already exists
    if (GetUserByUsername(user.username)) {
        return false;
    }

    User newUser = user;
    newUser.createdDate = GetCurrentDateTime();

    // Hash the authentication if provided
    if (!rawAuth.empty() && user.authType != AuthType::NONE) {
        newUser.authHash = HashString(rawAuth);
    }

    // Validate user data
    if (!ValidateUser(newUser)) {
        return false;
    }

    users.push_back(newUser);
    return true;
}

bool UserManager::DeleteUser(const std::wstring& username) {
    auto it = std::find_if(users.begin(), users.end(),
        [&username](const User& u) { return u.username == username; });

    if (it != users.end()) {
        // If this is the current user, log out first
        if (UserManager::currentUsername == username) {  // Assuming currentUsername is wstring
            LogoutUser();
        }

        // Remove user data using username (since your structs use userId as wstring)
        expenses.erase(
            std::remove_if(expenses.begin(), expenses.end(),
                [&username](const Expense& e) { return e.userId == username; }),
            expenses.end());

        incomes.erase(
            std::remove_if(incomes.begin(), incomes.end(),
                [&username](const Income& i) { return i.userId == username; }),
            incomes.end());

        budgets.erase(
            std::remove_if(budgets.begin(), budgets.end(),
                [&username](const Budget& b) { return b.userId == username; }),
            budgets.end());

        recurringTransactions.erase(
            std::remove_if(recurringTransactions.begin(), recurringTransactions.end(),
                [&username](const RecurringTransaction& rt) { return rt.userId == username; }),
            recurringTransactions.end());

        savingsGoals.erase(
            std::remove_if(savingsGoals.begin(), savingsGoals.end(),
                [&username](const SavingsGoal& sg) { return sg.userId == username; }),
            savingsGoals.end());

        users.erase(it);
        return true;
    }
    return false;
}

bool UserManager::ChangeUserAuth(const std::wstring& username, const std::wstring& newAuth, AuthType newType) {
    auto user = GetUserByUsername(username);
    if (!user) return false;

    user->authType = newType;
    if (newType != AuthType::NONE && !newAuth.empty()) {
        user->authHash = HashString(newAuth);
    }
    else {
        user->authHash.clear();
    }

    return true;
}

// User management
std::vector<User> UserManager::GetAllUsers() {
    return users;
}

User* UserManager::GetUserByUsername(const std::wstring& username) {
    auto it = std::find_if(users.begin(), users.end(),
        [&username](const User& u) { return u.username == username; });

    return (it != users.end()) ? &(*it) : nullptr;
}

bool UserManager::UpdateUserSettings(const User& updatedUser) {
    auto user = GetUserByUsername(updatedUser.username);
    if (!user) return false;

    // Preserve authentication data
    std::wstring oldAuthHash = user->authHash;
    AuthType oldAuthType = user->authType;

    *user = updatedUser;

    // Restore authentication if not explicitly changed
    if (updatedUser.authHash.empty()) {
        user->authHash = oldAuthHash;
        user->authType = oldAuthType;
    }

    // Update current user pointer if this is the logged-in user
    if (currentUserId == updatedUser.username) {
        currentUser = user;
    }

    return true;
}

// Session management
bool UserManager::LoginUser(const std::wstring& username) {
    auto user = GetUserByUsername(username);
    if (!user) return false;

    currentUserId = username;
    currentUser = user;

    if (OnUserLoggedIn) {
        OnUserLoggedIn();
    }

    return true;
}

void UserManager::LogoutUser() {
    currentUserId.clear();
    currentUser = nullptr;

    if (OnUserLoggedOut) {
        OnUserLoggedOut();
    }
}

bool UserManager::IsUserLoggedIn() {
    return !currentUserId.empty() && currentUser != nullptr;
}

std::wstring UserManager::GetCurrentUserId() {
    return currentUserId;
}

User* UserManager::GetCurrentUser() {
    return currentUser;
}

// Security
std::wstring UserManager::HashString(const std::wstring& input) {
    return SimpleHash(input + SALT);
}

bool UserManager::VerifyHash(const std::wstring& input, const std::wstring& hash) {
    return HashString(input) == hash;
}

std::wstring UserManager::GenerateUserId() {
    return GenerateUniqueId();
}

// Dialog functions
void UserManager::ShowLoginDialog(HWND parent) {
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LOGIN_DIALOG), parent, LoginDialogProc);
}

void UserManager::ShowCreateUserDialog(HWND parent) {
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CREATE_USER_DIALOG), parent, CreateUserDialogProc);
}

void UserManager::ShowUserSettingsDialog(HWND parent) {
    if (!IsUserLoggedIn()) return;
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_USER_SETTINGS_DIALOG), parent, UserSettingsDialogProc);
}

void UserManager::ShowUserManagementDialog(HWND parent) {
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_USER_SETTINGS_DIALOG), parent, UserManagementDialogProc);
}

// Dialog procedures
LRESULT CALLBACK UserManager::LoginDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        // Set dialog title
        SetWindowText(hDlg, L"Login");

        // Create controls
        CreateWindow(L"STATIC", L"Username:", WS_VISIBLE | WS_CHILD,
            20, 20, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            110, 18, 200, 25, hDlg, (HMENU)IDC_USERNAME_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Password/PIN:", WS_VISIBLE | WS_CHILD,
            20, 60, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL,
            110, 58, 200, 25, hDlg, (HMENU)IDC_PASSWORD_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"Login", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            90, 100, 80, 30, hDlg, (HMENU)IDC_LOGIN_BTN, GetModuleHandle(NULL), NULL);
        CreateWindow(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD,
            180, 100, 80, 30, hDlg, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);

        // Populate user list (you might want a combo box for this)
        PopulateUserList(GetDlgItem(hDlg, IDC_USERNAME_EDIT));

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_LOGIN_BTN:
        case IDOK:
        {
            wchar_t username[256], password[256];
            GetDlgItemText(hDlg, IDC_USERNAME_EDIT, username, 256);
            GetDlgItemText(hDlg, IDC_PASSWORD_EDIT, password, 256);

            if (wcslen(username) == 0) {
                MessageBox(hDlg, L"Please enter a username.", L"Error", MB_OK);
                return TRUE;
            }

            auto user = GetUserByUsername(username);
            if (!user) {
                MessageBox(hDlg, L"User not found.", L"Error", MB_OK);
                return TRUE;
            }

            if (user->authType != AuthType::NONE && !AuthenticateUser(username, password)) {
                MessageBox(hDlg, L"Invalid password/PIN.", L"Error", MB_OK);
                return TRUE;
            }

            LoginUser(username);
            EndDialog(hDlg, IDOK);
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

LRESULT CALLBACK UserManager::CreateUserDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        SetWindowText(hDlg, L"Create New User");

        // Create controls
        CreateWindow(L"STATIC", L"Username:", WS_VISIBLE | WS_CHILD,
            20, 20, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            110, 18, 200, 25, hDlg, (HMENU)IDC_USERNAME_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Display Name:", WS_VISIBLE | WS_CHILD,
            20, 60, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            110, 58, 200, 25, hDlg, (HMENU)IDC_DISPLAY_NAME_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Authentication:", WS_VISIBLE | WS_CHILD,
            20, 100, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hAuthCombo = CreateWindow(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,
            110, 98, 120, 100, hDlg, (HMENU)IDC_AUTH_TYPE_COMBO, GetModuleHandle(NULL), NULL);
        PopulateAuthTypeCombo(hAuthCombo);

        CreateWindow(L"STATIC", L"Password/PIN:", WS_VISIBLE | WS_CHILD,
            20, 140, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL,
            110, 138, 200, 25, hDlg, (HMENU)IDC_PASSWORD_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Confirm:", WS_VISIBLE | WS_CHILD,
            20, 180, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL,
            110, 178, 200, 25, hDlg, (HMENU)IDC_CONFIRM_PASSWORD_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Currency:", WS_VISIBLE | WS_CHILD,
            20, 220, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hCurrencyCombo = CreateWindow(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,
            110, 218, 120, 100, hDlg, (HMENU)IDC_CURRENCY_COMBO, GetModuleHandle(NULL), NULL);
        PopulateCurrencyCombo(hCurrencyCombo);

        CreateWindow(L"BUTTON", L"Dark Mode", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            110, 260, 100, 25, hDlg, (HMENU)IDC_DARK_MODE_CHECK, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"Create User", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            90, 300, 100, 35, hDlg, (HMENU)IDC_CREATE_USER_BTN, GetModuleHandle(NULL), NULL);
        CreateWindow(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD,
            200, 300, 80, 35, hDlg, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_CREATE_USER_BTN:
        case IDOK:
        {
            wchar_t username[256], displayName[256], password[256], confirmPassword[256];
            GetDlgItemText(hDlg, IDC_USERNAME_EDIT, username, 256);
            GetDlgItemText(hDlg, IDC_DISPLAY_NAME_EDIT, displayName, 256);
            GetDlgItemText(hDlg, IDC_PASSWORD_EDIT, password, 256);
            GetDlgItemText(hDlg, IDC_CONFIRM_PASSWORD_EDIT, confirmPassword, 256);

            // Validation
            if (!ValidateUserInput(username, displayName, password)) {
                return TRUE;
            }

            if (wcscmp(password, confirmPassword) != 0) {
                MessageBox(hDlg, L"Passwords do not match.", L"Error", MB_OK);
                return TRUE;
            }

            // Create user
            User newUser;
            newUser.username = username;
            newUser.displayName = displayName;

            int authSelection = (int)SendDlgItemMessage(hDlg, IDC_AUTH_TYPE_COMBO, CB_GETCURSEL, 0, 0);
            switch (authSelection) {
            case 0: newUser.authType = AuthType::NONE; break;
            case 1: newUser.authType = AuthType::PIN; break;
            case 2: newUser.authType = AuthType::PASSWORD; break;
            default: newUser.authType = AuthType::NONE; break;
            }

            int currencySelection = (int)SendDlgItemMessage(hDlg, IDC_CURRENCY_COMBO, CB_GETCURSEL, 0, 0);
            switch (currencySelection) {
            case 0: newUser.defaultCurrency = CurrencyType::USD; break;
            case 1: newUser.defaultCurrency = CurrencyType::EUR; break;
            case 2: newUser.defaultCurrency = CurrencyType::GBP; break;
            case 3: newUser.defaultCurrency = CurrencyType::JPY; break;
            case 4: newUser.defaultCurrency = CurrencyType::CAD; break;
            case 5: newUser.defaultCurrency = CurrencyType::AUD; break;
            default: newUser.defaultCurrency = CurrencyType::USD; break;
            }

            newUser.isDarkMode = IsDlgButtonChecked(hDlg, IDC_DARK_MODE_CHECK) == BST_CHECKED;

            if (CreateUser(newUser, password)) {
                MessageBox(hDlg, L"User created successfully!", L"Success", MB_OK);
                EndDialog(hDlg, IDOK);
            }
            else {
                MessageBox(hDlg, L"Failed to create user. Username may already exist.", L"Error", MB_OK);
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

LRESULT CALLBACK UserManager::UserSettingsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        if (!IsUserLoggedIn()) {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }

        SetWindowText(hDlg, L"User Settings");

        User* user = GetCurrentUser();

        // Create and populate controls with current user data
        CreateWindow(L"STATIC", L"Display Name:", WS_VISIBLE | WS_CHILD,
            20, 20, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hDisplayName = CreateWindow(L"EDIT", user->displayName.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            110, 18, 200, 25, hDlg, (HMENU)IDC_DISPLAY_NAME_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Currency:", WS_VISIBLE | WS_CHILD,
            20, 60, 80, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hCurrencyCombo = CreateWindow(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,
            110, 58, 120, 100, hDlg, (HMENU)IDC_CURRENCY_COMBO, GetModuleHandle(NULL), NULL);
        PopulateCurrencyCombo(hCurrencyCombo);
        SendMessage(hCurrencyCombo, CB_SETCURSEL, (int)user->defaultCurrency, 0);

        HWND hDarkMode = CreateWindow(L"BUTTON", L"Dark Mode", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            110, 100, 100, 25, hDlg, (HMENU)IDC_DARK_MODE_CHECK, GetModuleHandle(NULL), NULL);
        CheckDlgButton(hDlg, IDC_DARK_MODE_CHECK, user->isDarkMode ? BST_CHECKED : BST_UNCHECKED);

        CreateWindow(L"BUTTON", L"Save Settings", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            90, 150, 100, 35, hDlg, (HMENU)IDC_SETTINGS_BTN, GetModuleHandle(NULL), NULL);
        CreateWindow(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD,
            200, 150, 80, 35, hDlg, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_SETTINGS_BTN:
        case IDOK:
        {
            if (!IsUserLoggedIn()) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }

            User updatedUser = *GetCurrentUser();

            wchar_t displayName[256];
            GetDlgItemText(hDlg, IDC_DISPLAY_NAME_EDIT, displayName, 256);
            updatedUser.displayName = displayName;

            int currencySelection = (int)SendDlgItemMessage(hDlg, IDC_CURRENCY_COMBO, CB_GETCURSEL, 0, 0);
            switch (currencySelection) {
            case 0: updatedUser.defaultCurrency = CurrencyType::USD; break;
            case 1: updatedUser.defaultCurrency = CurrencyType::EUR; break;
            case 2: updatedUser.defaultCurrency = CurrencyType::GBP; break;
            case 3: updatedUser.defaultCurrency = CurrencyType::JPY; break;
            case 4: updatedUser.defaultCurrency = CurrencyType::CAD; break;
            case 5: updatedUser.defaultCurrency = CurrencyType::AUD; break;
            }

            updatedUser.isDarkMode = IsDlgButtonChecked(hDlg, IDC_DARK_MODE_CHECK) == BST_CHECKED;

            if (UpdateUserSettings(updatedUser)) {
                MessageBox(hDlg, L"Settings saved successfully!", L"Success", MB_OK);
                EndDialog(hDlg, IDOK);
            }
            else {
                MessageBox(hDlg, L"Failed to save settings.", L"Error", MB_OK);
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

LRESULT CALLBACK UserManager::UserManagementDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        SetWindowText(hDlg, L"User Management");

        // Create user list
        HWND hUserList = CreateWindow(L"LISTBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL,
            20, 20, 200, 200, hDlg, (HMENU)IDC_USER_LIST, GetModuleHandle(NULL), NULL);
        PopulateUserList(hUserList);

        CreateWindow(L"BUTTON", L"Create User", WS_VISIBLE | WS_CHILD,
            240, 20, 100, 30, hDlg, (HMENU)IDC_CREATE_USER_BTN, GetModuleHandle(NULL), NULL);
        CreateWindow(L"BUTTON", L"Delete User", WS_VISIBLE | WS_CHILD,
            240, 60, 100, 30, hDlg, (HMENU)IDC_DELETE_USER_BTN, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"Close", WS_VISIBLE | WS_CHILD,
            240, 190, 80, 30, hDlg, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_CREATE_USER_BTN:
            ShowCreateUserDialog(hDlg);
            PopulateUserList(GetDlgItem(hDlg, IDC_USER_LIST));
            return TRUE;

        case IDC_DELETE_USER_BTN:
        {
            HWND hUserList = GetDlgItem(hDlg, IDC_USER_LIST);
            int selection = (int)SendMessage(hUserList, LB_GETCURSEL, 0, 0);
            if (selection != LB_ERR) {
                wchar_t username[256];
                SendMessage(hUserList, LB_GETTEXT, selection, (LPARAM)username);

                int result = MessageBox(hDlg,
                    (L"Are you sure you want to delete user '" + std::wstring(username) + L"'?\nAll associated data will be deleted.").c_str(),
                    L"Confirm Delete", MB_YESNO);

                if (result == IDYES) {
                    if (DeleteUser(std::wstring(username))) {  // Pass wstring username, not int
                        MessageBox(hDlg, L"User deleted successfully.", L"Success", MB_OK);
                        PopulateUserList(hUserList);
                    }
                    else {
                        MessageBox(hDlg, L"Failed to delete user.", L"Error", MB_OK);
                    }
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




// Security helpers
std::wstring UserManager::SimpleHash(const std::wstring& input) {
    // Simple hash function - in production, use a proper cryptographic hash
    std::hash<std::wstring> hasher;
    size_t hashValue = hasher(input);

    std::wstringstream ss;
    ss << std::hex << hashValue;
    return ss.str();
}

// UserDataFilter namespace implementation
namespace UserDataFilter {
    std::vector<Expense> GetUserExpenses(const std::wstring& userId) {
        std::vector<Expense> userExpenses;
        for (const auto& expense : expenses) {
            if (expense.userId == userId) {
                userExpenses.push_back(expense);
            }
        }
        return userExpenses;
    }

    std::vector<Income> GetUserIncomes(const std::wstring& userId) {
        std::vector<Income> userIncomes;
        for (const auto& income : incomes) {
            if (income.userId == userId) {
                userIncomes.push_back(income);
            }
        }
        return userIncomes;
    }

    std::vector<Budget> GetUserBudgets(const std::wstring& userId) {
        std::vector<Budget> userBudgets;
        for (const auto& budget : budgets) {
            if (budget.userId == userId) {
                userBudgets.push_back(budget);
            }
        }
        return userBudgets;
    }

    std::vector<RecurringTransaction> GetUserRecurringTransactions(const std::wstring& userId) {
        std::vector<RecurringTransaction> userRecurring;
        for (const auto& rt : recurringTransactions) {
            if (rt.userId == userId) {
                userRecurring.push_back(rt);
            }
        }
        return userRecurring;
    }

    std::vector<SavingsGoal> GetUserSavingsGoals(const std::wstring& userId) {
        std::vector<SavingsGoal> userGoals;
        for (const auto& goal : savingsGoals) {
            if (goal.userId == userId) {
                userGoals.push_back(goal);
            }
        }
        return userGoals;
    }

    double GetUserTotalExpenses(const std::wstring& userId) {
        double total = 0.0;
        for (const auto& expense : expenses) {
            if (expense.userId == userId) {
                total += expense.amount;
            }
        }
        return total;
    }

    double GetUserTotalIncome(const std::wstring& userId) {
        double total = 0.0;
        for (const auto& income : incomes) {
            if (income.userId == userId) {
                total += income.amount;
            }
        }
        return total;
    }

    double GetUserBalance(const std::wstring& userId) {
        return GetUserTotalIncome(userId) - GetUserTotalExpenses(userId);
    }
}