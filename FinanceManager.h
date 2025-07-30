#pragma once
#include "DataStructures.h"
#include <windows.h>
#include <functional>
#include <vector>
#include "nlohmann/json.hpp"

// Dialog IDs for finance management
#define IDD_ADD_EXPENSE_DIALOG          600
#define IDD_ADD_INCOME_DIALOG           601
#define IDD_EDIT_TRANSACTION_DIALOG     602
#define IDD_TRANSACTION_LIST_DIALOG     603
#define IDD_SEARCH_DIALOG               604
#define IDD_RECURRING_DIALOG            605

// Control IDs for expense dialog
#define IDC_EXPENSE_CATEGORY_COMBO      610
#define IDC_EXPENSE_AMOUNT_EDIT         611
#define IDC_EXPENSE_NOTE_EDIT           612
#define IDC_EXPENSE_DATE_PICKER         613
#define IDC_EXPENSE_TAGS_EDIT           614
#define IDC_EXPENSE_CURRENCY_COMBO      615
#define IDC_EXPENSE_RECEIPT_BTN         616
#define IDC_EXPENSE_LOCATION_EDIT       617

// Control IDs for income dialog
#define IDC_INCOME_SOURCE_COMBO         620
#define IDC_INCOME_AMOUNT_EDIT          621
#define IDC_INCOME_NOTE_EDIT            622
#define IDC_INCOME_DATE_PICKER          623
#define IDC_INCOME_TAGS_EDIT            624
#define IDC_INCOME_CURRENCY_COMBO       625
#define IDC_INCOME_TAXABLE_CHECK        626

// Control IDs for transaction list
#define IDC_TRANSACTION_LIST            630
#define IDC_FILTER_COMBO                631
#define IDC_SEARCH_EDIT                 632
#define IDC_DATE_FROM_PICKER            633
#define IDC_DATE_TO_PICKER              634
#define IDC_CATEGORY_FILTER_COMBO       635

// Control IDs for recurring transactions
#define IDC_RECURRING_LIST              640
#define IDC_RECURRING_DESC_EDIT         641
#define IDC_RECURRING_AMOUNT_EDIT       642
#define IDC_RECURRING_TYPE_COMBO        643
#define IDC_RECURRING_FREQ_COMBO        644
#define IDC_RECURRING_DAY_EDIT          645

class FinanceManager {
private:
    // Static data containers - shared across all instances
    static std::vector<Expense> expenses;
    
    static std::vector<Category> categories;

    static const std::vector<std::wstring> DEFAULT_INCOME_SOURCES;

public:
    // Validation functions - ADD THESE MISSING DECLARATION

    // Utility functions - ADD THESE MISSING DECLARATIONS
    static std::wstring GenerateUniqueId();
    static void UpdateBudgetSpending(const std::wstring& userId, const std::wstring& category, double amount);

    // Transaction dialogs
    static void ShowAddExpenseDialog(HWND parent);
    static void ShowAddIncomeDialog(HWND parent);
    static void ShowEditTransactionDialog(HWND parent, const std::wstring& transactionId, bool isExpense);

    // Transaction management
    static bool AddExpense(const Expense& expense);
    static bool AddIncome(const Income& income);
    static bool UpdateExpense(const std::wstring& id, const Expense& expense);
    static bool UpdateIncome(const std::wstring& id, const Income& income);
    static bool DeleteExpense(const std::wstring& id);
    static bool DeleteIncome(const std::wstring& id);

    // Transaction retrieval
    static Expense* GetExpenseById(const std::wstring& id);
    static Income* GetIncomeById(const std::wstring& id);
    static std::vector<Expense> GetUserExpenses(const std::wstring& userId);
    static std::vector<Income> GetUserIncomes(const std::wstring& userId);

    // Validation
    static bool ValidateTransactionData(const std::wstring& category, double amount, const std::wstring& date);
    static bool ValidateAmount(const std::wstring& amountStr, double& amount);
    static bool ValidateDate(const std::wstring& date);

    // Utility functions
    static std::vector<std::wstring> ParseTags(const std::wstring& tagString);
    static std::wstring TagsToString(const std::vector<std::wstring>& tags);
    static std::wstring SelectReceiptFile(HWND parent);

    // Data access functions - ADD THESE FOR EXTERNAL ACCESS
    static std::vector<Expense>& GetExpenses() { return expenses; }
    static std::vector<Income>& GetIncomes() { return incomes; }
    static std::vector<Budget>& GetBudgets() { return budgets; }
    static std::vector<Category>& GetCategories() { return categories; }

    // Callbacks
    static std::function<void()> OnTransactionAdded;
    static std::function<void()> OnTransactionUpdated;
    static std::function<void()> OnTransactionDeleted;

private:
    static LRESULT CALLBACK AddExpenseDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK AddIncomeDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK EditTransactionDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // Helper functions
    static void PopulateCategoryCombo(HWND hCombo);
    static void PopulateIncomeSourceCombo(HWND hCombo);
    static void PopulateCurrencyCombo(HWND hCombo);
    static void SetupDatePicker(HWND hDatePicker);
    static bool GetExpenseFromDialog(HWND hDlg, Expense& expense);
    static bool GetIncomeFromDialog(HWND hDlg, Income& income);
    static void SetExpenseToDialog(HWND hDlg, const Expense& expense);
    static void SetIncomeToDialog(HWND hDlg, const Income& income);
};

// Transaction viewer and search functionality
class TransactionViewer {
public:
    static void ShowTransactionListDialog(HWND parent);
    static void RefreshTransactionList(HWND hListView, const FilterCriteria& criteria = FilterCriteria());

private:
    static LRESULT CALLBACK TransactionListDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static void SetupTransactionListView(HWND hListView);
    static void PopulateTransactionList(HWND hListView, const std::vector<Expense>& expenses, const std::vector<Income>& incomes);
    static void HandleTransactionDoubleClick(HWND hListView);
    static void HandleTransactionDelete(HWND hListView);
};

// Search functionality
class SearchManager {
public:
    static void ShowSearchDialog(HWND parent);
    static std::vector<Expense> SearchExpenses(const FilterCriteria& criteria);
    static std::vector<Income> SearchIncomes(const FilterCriteria& criteria);

private:
    static LRESULT CALLBACK SearchDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static void SetupSearchControls(HWND hDlg);
    static FilterCriteria GetSearchCriteriaFromDialog(HWND hDlg);
    static void DisplaySearchResults(HWND hDlg, const std::vector<Expense>& expenses, const std::vector<Income>& incomes);
};

// Recurring transactions management
class RecurringManager {
public:
    static void ShowRecurringTransactionsDialog(HWND parent);
    static bool AddRecurringTransaction(const RecurringTransaction& rt);
    static bool UpdateRecurringTransaction(const std::wstring& id, const RecurringTransaction& rt);
    static bool DeleteRecurringTransaction(const std::wstring& id);
    static void ProcessRecurringTransactions();
    static std::vector<RecurringTransaction> GetDueRecurringTransactions();

private:
    static LRESULT CALLBACK RecurringDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static void SetupRecurringListView(HWND hListView);
    static void PopulateRecurringList(HWND hListView);
    static void HandleRecurringAdd(HWND hDlg);
    static void HandleRecurringEdit(HWND hDlg);
    static void HandleRecurringDelete(HWND hDlg);
    static bool GetRecurringFromDialog(HWND hDlg, RecurringTransaction& rt);
};

// Budget management
class BudgetManager {
public:
    static void ShowBudgetManagerDialog(HWND parent);
    static void ShowBudgetSummaryDialog(HWND parent);
    static void ShowBudgetAlertsDialog(HWND parent);

    static bool AddBudget(const Budget& budget);
    static bool UpdateBudget(const std::wstring& id, const Budget& budget);
    static bool DeleteBudget(const std::wstring& id);

    static void CheckBudgetAlerts(HWND parent);
    static std::vector<Budget> GetOverBudgetCategories(const std::wstring& userId);
    static std::vector<Budget> GetNearBudgetLimitCategories(const std::wstring& userId);
    static void RecalculateBudgetSpending(const std::wstring& userId);

private:
    static LRESULT CALLBACK BudgetManagerDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK BudgetSummaryDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK BudgetAlertsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

// Savings goals management
class GoalsManager {
public:
    static void ShowGoalsManagerDialog(HWND parent);
    static void ShowProgressDialog(HWND parent);

    static bool AddSavingsGoal(const SavingsGoal& goal);
    static bool UpdateSavingsGoal(const std::wstring& id, const SavingsGoal& goal);
    static bool DeleteSavingsGoal(const std::wstring& id);
    static bool UpdateGoalProgress(const std::wstring& id, double amount);

    static std::vector<SavingsGoal> GetUserSavingsGoals(const std::wstring& userId);
    static double CalculateGoalProgress(const SavingsGoal& goal);
    static int GetDaysToDeadline(const SavingsGoal& goal);

private:
    static LRESULT CALLBACK GoalsManagerDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ProgressDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

// Category management
class CategoryManager {
public:
    static void ShowCategoryManagerDialog(HWND parent);
    static bool AddCategory(const Category& category);
    static bool UpdateCategory(const std::wstring& name, const Category& category);
    static bool DeleteCategory(const std::wstring& name);
    static bool CanDeleteCategory(const std::wstring& name);

private:
    static LRESULT CALLBACK CategoryManagerDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

// Currency management
class CurrencyManager {
public:
    static void ShowCurrencyDialog(HWND parent);
    static bool UpdateExchangeRates();
    static double ConvertAmount(double amount, CurrencyType from, CurrencyType to);

private:
    static LRESULT CALLBACK CurrencyDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

// Export/Import management
class ExportManager {
public:
    enum class ExportType { CSV, PDF, JSON };  // This is already defined correctly

    static void ShowExportDialog(HWND parent, ExportType type);
    static bool ExportTransactions(const std::wstring& filePath, ExportType type, const std::wstring& userId = L"");

private:
    static LRESULT CALLBACK ExportDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

class ImportManager {
public:
    static void ShowImportDialog(HWND parent);
    static bool ImportFromCSV(const std::wstring& filePath, const std::wstring& userId);
    static bool ImportFromJSON(const std::wstring& filePath);

private:
    static LRESULT CALLBACK ImportDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

// Backup management
class BackupManager {
public:
    static void ShowBackupSettingsDialog(HWND parent);

private:
    static LRESULT CALLBACK BackupSettingsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};