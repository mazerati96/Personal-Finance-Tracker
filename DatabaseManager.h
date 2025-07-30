#pragma once
#include "DataStructures.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class DatabaseManager {
public:
    // File operations
    static bool SaveAllData();
    static bool LoadAllData();
    static bool BackupData(const std::wstring& backupPath = L"");
    static bool RestoreFromBackup(const std::wstring& backupPath);

    // Export/Import
    static bool ExportToCSV(const std::wstring& filePath, const std::wstring& userId = L"");
    static bool ExportToPDF(const std::wstring& filePath, const std::wstring& userId = L"");
    static bool ImportFromCSV(const std::wstring& filePath, const std::wstring& userId);
    static bool ImportFromJSON(const std::wstring& filePath);

    // Auto-backup
    static void EnableAutoBackup(int intervalMinutes = 30);
    static void DisableAutoBackup();
    static bool IsAutoBackupEnabled();

    // Data validation and repair
    static bool ValidateDatabase();
    static bool RepairDatabase();
    static std::vector<std::wstring> GetDatabaseIssues();

    // Migration and versioning
    static std::wstring GetDatabaseVersion();
    static bool MigrateDatabase(const std::wstring& fromVersion, const std::wstring& toVersion);

    // File paths
    static std::wstring GetDataFilePath();
    static std::wstring GetBackupDirectory();
    static std::wstring GetExportDirectory();

private:
    // JSON conversion functions
    static json UserToJson(const User& user);
    static User JsonToUser(const json& j);

    static json ExpenseToJson(const Expense& expense);
    static Expense JsonToExpense(const json& j);

    static json IncomeToJson(const Income& income);
    static Income JsonToIncome(const json& j);

    static json BudgetToJson(const Budget& budget);
    static Budget JsonToBudget(const json& j);

    static json RecurringTransactionToJson(const RecurringTransaction& rt);
    static RecurringTransaction JsonToRecurringTransaction(const json& j);

    static json SavingsGoalToJson(const SavingsGoal& goal);
    static SavingsGoal JsonToSavingsGoal(const json& j);

    static json CategoryToJson(const Category& category);
    static Category JsonToCategory(const json& j);

    // Helper functions
    static std::wstring WStringToString(const std::wstring& wstr);    // Correct: wstring -> string
    static std::wstring StringToWString(const std::wstring& str);
    static bool CreateDirectoryIfNotExists(const std::wstring& path);
    static std::wstring GetTimestamp();
    static bool FileExists(const std::wstring& path);

    // CSV helpers
    static std::wstring EscapeCSVField(const std::wstring& field);
    static std::vector<std::wstring> ParseCSVLine(const std::wstring& line);

    // Auto-backup timer
    static UINT_PTR backupTimerId;
    static int backupInterval;
    static void CALLBACK BackupTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

    // Constants
    static const std::wstring DATA_FILE;
    static const std::wstring BACKUP_DIR;
    static const std::wstring EXPORT_DIR;
    static const std::wstring CURRENT_VERSION;

    // File locking
    static HANDLE fileLock;
    static bool AcquireFileLock();
    static void ReleaseFileLock();
    // Utility functions
  

    // String/Enum conversion
    static std::wstring TagsToString(const std::vector<std::wstring>& tags);
    static std::vector<std::wstring> ParseTags(const std::wstring& tagString);
    
 
    

   
    // User management
    static User* GetUserByUsername(const std::wstring& username);

        // Correct: string -> wstring
    
    
    
};

// Email/Report Generation
class ReportGenerator {
public:
    struct ReportOptions {
        std::wstring userId;
        DateRange dateRange;
        bool includeCharts;
        bool includeAnalytics;
        std::wstring templatePath;
        std::vector<std::wstring> categories;

        ReportOptions() : includeCharts(true), includeAnalytics(true) {}
    };

    static bool GenerateMonthlyReport(const ReportOptions& options, const std::wstring& outputPath);
    static bool GenerateYearlyReport(const ReportOptions& options, const std::wstring& outputPath);
    static bool GenerateCustomReport(const ReportOptions& options, const std::wstring& outputPath);

    static void ShowReportDialog(HWND parent);

    // Email functionality (requires SMTP setup)
    static bool EmailReport(const std::wstring& reportPath, const std::wstring& emailAddress);
    static bool ScheduleEmailReport(const ReportOptions& options, const std::wstring& emailAddress,
        const std::wstring& schedule); // "monthly", "weekly", etc.

private:
    static std::wstring GenerateHTMLReport(const ReportOptions& options);
    static std::wstring GenerateChartHTML(const std::vector<SpendingTrend>& trends);
    static std::wstring GenerateAnalyticsHTML(const std::vector<CategoryAnalytics>& analytics);
    static LRESULT CALLBACK ReportDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};