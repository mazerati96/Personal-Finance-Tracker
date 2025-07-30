#pragma once
#include "DataStructures.h"
#include <functional>
#include <sstream>     // For 'ss', 'stream'
#include <iostream>    // For 'std::cout', 'out'
#include <chrono>      // For 'now' (if using time functions)
#include <vector>      // If not already included
#include <string>      // If not already included


// Dialog IDs for user management
#define IDD_LOGIN_DIALOG         500
#define IDD_CREATE_USER_DIALOG   501
#define IDD_USER_SETTINGS_DIALOG 502

// Control IDs
#define IDC_USERNAME_EDIT        510
#define IDC_PASSWORD_EDIT        511
#define IDC_CONFIRM_PASSWORD_EDIT 512
#define IDC_DISPLAY_NAME_EDIT    513
#define IDC_AUTH_TYPE_COMBO      514
#define IDC_CURRENCY_COMBO       515
#define IDC_DARK_MODE_CHECK      516
#define IDC_USER_LIST            517
#define IDC_CREATE_USER_BTN      518
#define IDC_DELETE_USER_BTN      519
#define IDC_LOGIN_BTN            520
#define IDC_SETTINGS_BTN         521

class UserManager {
public:
    // User authentication
    static bool AuthenticateUser(const std::wstring& username, const std::wstring& auth);
    static bool CreateUser(const User& user, const std::wstring& rawAuth = L"");
    static bool DeleteUser(const std::wstring& username);
    static bool ChangeUserAuth(const std::wstring& username, const std::wstring& newAuth, AuthType newType);

    // User management
    static std::vector<User> GetAllUsers();
    static User* GetUserByUsername(const std::wstring& username);
    static bool UpdateUserSettings(const User& updatedUser);

    // Session management
    static bool LoginUser(const std::wstring& username);
    static void LogoutUser();
    static bool IsUserLoggedIn();
    static std::wstring GetCurrentUserId();
    static std::wstring currentUsername;
    static User* GetCurrentUser();

    // Security
    static std::wstring HashString(const std::wstring& input);
    static bool VerifyHash(const std::wstring& input, const std::wstring& hash);
    static std::wstring GenerateUserId();

    // Dialog functions
    static void ShowLoginDialog(HWND parent);
    static void ShowCreateUserDialog(HWND parent);
    static void ShowUserSettingsDialog(HWND parent);
    static void ShowUserManagementDialog(HWND parent);

    // Callbacks
    static std::function<void()> OnUserLoggedIn;
    static std::function<void()> OnUserLoggedOut;

private:
    static LRESULT CALLBACK LoginDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK CreateUserDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK UserSettingsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK UserManagementDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // Helper functions
    static void PopulateUserList(HWND hListBox);
    static void PopulateCurrencyCombo(HWND hCombo);
    static void PopulateAuthTypeCombo(HWND hCombo);
    

    // Security helpers
    static const std::wstring SALT;
    static std::wstring SimpleHash(const std::wstring& input);
};

// Utility functions for user data filtering
namespace UserDataFilter {
    std::vector<Expense> GetUserExpenses(const std::wstring& userId);
    std::vector<Income> GetUserIncomes(const std::wstring& userId);
    std::vector<Budget> GetUserBudgets(const std::wstring& userId);
    std::vector<RecurringTransaction> GetUserRecurringTransactions(const std::wstring& userId);
    std::vector<SavingsGoal> GetUserSavingsGoals(const std::wstring& userId);

    double GetUserTotalExpenses(const std::wstring& userId);
    double GetUserTotalIncome(const std::wstring& userId);
    double GetUserBalance(const std::wstring& userId);
}