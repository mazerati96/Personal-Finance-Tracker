#include <Windows.h>
#include <set>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include "DatabaseManager.h"
#include "DataStructures.h"
#include "Utils.h"
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

json DatabaseManager::RecurringTransactionToJson(const RecurringTransaction& rt) {
    return json(); // TODO: implement
}

RecurringTransaction DatabaseManager::JsonToRecurringTransaction(const json& j) {
    return RecurringTransaction{}; // TODO: implement
}

json DatabaseManager::CategoryToJson(const Category& cat) {
    return json(); // TODO: implement
}

Category DatabaseManager::JsonToCategory(const json& j) {
    return Category{}; // TODO: implement
}

json DatabaseManager::SavingsGoalToJson(const SavingsGoal& sg) {
    return json(); // TODO: implement
}

SavingsGoal DatabaseManager::JsonToSavingsGoal(const json& j) {
    return SavingsGoal{}; // TODO: implement
}

User* DatabaseManager::GetUserByUsername(const std::wstring& username) {
    return nullptr; // TODO: implement
}

std::wstring DatabaseManager::TagsToString(const std::vector<std::wstring>& tags) {
    return L""; // TODO: implement
}

std::wstring DatabaseManager::StringToWString(const std::wstring& str) {
    return str; // likely incorrect signature, adjust if needed
}

std::wstring DatabaseManager::WStringToString(const std::wstring& wstr) {
    return wstr; // likely incorrect signature, adjust if needed
}

// Static member initialization
const std::wstring DatabaseManager::DATA_FILE = L"finance_data.json";
const std::wstring DatabaseManager::BACKUP_DIR = L"backups";
const std::wstring DatabaseManager::EXPORT_DIR = L"exports";
const std::wstring DatabaseManager::CURRENT_VERSION = L"1.0.0";

UINT_PTR DatabaseManager::backupTimerId = 0;
int DatabaseManager::backupInterval = 30;
HANDLE DatabaseManager::fileLock = INVALID_HANDLE_VALUE;

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
// In DatabaseManager.cpp
nlohmann::json DatabaseManager::BudgetToJson(const Budget& budget) {
    nlohmann::json j;
    j["id"] = budget.id;
    j["name"] = WStringToString(budget.name);
    j["amount"] = budget.amount;
    // Add other fields
    return j;
}

Budget DatabaseManager::JsonToBudget(const nlohmann::json& j) {
    Budget budget;
    
    // Completely avoid direct JSON assignment - force string conversion first
    if (j.contains("id")) {
        std::string idStr = j["id"].get<std::string>();
        budget.id = std::wstring(idStr.begin(), idStr.end());
    }
    
    if (j.contains("name")) {
        std::string nameStr = j["name"].get<std::string>();
        budget.name = std::wstring(nameStr.begin(), nameStr.end());
    }
    
    if (j.contains("amount")) {
        budget.amount = j["amount"].get<int>();
    }
    
    return budget;
}


// File operations
bool DatabaseManager::SaveAllData() {
    if (!AcquireFileLock()) {
        return false;
    }

    try {
        json root;
        root["version"] = WStringToString(CURRENT_VERSION);
        root["timestamp"] = WStringToString(GetCurrentDateTime());

        // Save users
        json usersArray = json::array();
        for (const auto& user : users) {
            usersArray.push_back(UserToJson(user));
        }
        root["users"] = usersArray;

        // Save expenses
        json expensesArray = json::array();
        for (const auto& expense : expenses) {
            expensesArray.push_back(ExpenseToJson(expense));
        }
        root["expenses"] = expensesArray;

        // Save incomes
        json incomesArray = json::array();
        for (const auto& income : incomes) {
            incomesArray.push_back(IncomeToJson(income));
        }
        root["incomes"] = incomesArray;

        // Save budgets
        json budgetsArray = json::array();
        for (const auto& budget : budgets) {
            budgetsArray.push_back(BudgetToJson(budget));
        }
        root["budgets"] = budgetsArray;

        // Save recurring transactions
        json recurringArray = json::array();
        for (const auto& rt : recurringTransactions) {
            recurringArray.push_back(RecurringTransactionToJson(rt));
        }
        root["recurringTransactions"] = recurringArray;

        // Save savings goals
        json goalsArray = json::array();
        for (const auto& goal : savingsGoals) {
            goalsArray.push_back(SavingsGoalToJson(goal));
        }
        root["savingsGoals"] = goalsArray;

        // Save categories
        json categoriesArray = json::array();
        for (const auto& category : categories) {
            categoriesArray.push_back(CategoryToJson(category));
        }
        root["categories"] = categoriesArray;

        // Write to file
        std::ofstream file(DATA_FILE);
        if (!file.is_open()) {
            ReleaseFileLock();
            return false;
        }

        file << root.dump(4); // Pretty print with 4 spaces
        file.close();

        ReleaseFileLock();
        return true;
    }
    catch (const std::exception&) {
        ReleaseFileLock();
        return false;
    }
}

bool DatabaseManager::LoadAllData() {
    if (!FileExists(DATA_FILE)) {
        // Initialize with default data if file doesn't exist
        InitializeDefaultData();
        return SaveAllData();
    }

    if (!AcquireFileLock()) {
        return false;
    }

    try {
        std::ifstream file(DATA_FILE);
        if (!file.is_open()) {
            ReleaseFileLock();
            return false;
        }

        json root;
        file >> root;
        file.close();

        // Clear existing data
        users.clear();
        expenses.clear();
        incomes.clear();
        budgets.clear();
        recurringTransactions.clear();
        savingsGoals.clear();
        categories.clear();

        // Load users
        if (root.contains("users")) {
            for (const auto& userJson : root["users"]) {
                users.push_back(JsonToUser(userJson));
            }
        }

        // Load expenses
        if (root.contains("expenses")) {
            for (const auto& expenseJson : root["expenses"]) {
                expenses.push_back(JsonToExpense(expenseJson));
            }
        }

        // Load incomes
        if (root.contains("incomes")) {
            for (const auto& incomeJson : root["incomes"]) {
                incomes.push_back(JsonToIncome(incomeJson));
            }
        }

        // Load budgets
        if (root.contains("budgets")) {
            for (const auto& budgetJson : root["budgets"]) {
                budgets.push_back(JsonToBudget(budgetJson));
            }
        }

        // Load recurring transactions
        if (root.contains("recurringTransactions")) {
            for (const auto& rtJson : root["recurringTransactions"]) {
                recurringTransactions.push_back(JsonToRecurringTransaction(rtJson));
            }
        }

        // Load savings goals
        if (root.contains("savingsGoals")) {
            for (const auto& goalJson : root["savingsGoals"]) {
                savingsGoals.push_back(JsonToSavingsGoal(goalJson));
            }
        }

        // Load categories
        if (root.contains("categories")) {
            for (const auto& categoryJson : root["categories"]) {
                categories.push_back(JsonToCategory(categoryJson));
            }
        }
        else {
            // Initialize default categories if none exist
            InitializeDefaultData();
        }

        ReleaseFileLock();
        return true;
    }
    catch (const std::exception&) {
        ReleaseFileLock();
        // On error, initialize with default data
        InitializeDefaultData();
        return false;
    }
}

bool DatabaseManager::BackupData(const std::wstring& backupPath) {
    std::wstring finalPath = backupPath;

    if (finalPath.empty()) {
        // Create automatic backup filename
        CreateDirectoryIfNotExists(BACKUP_DIR);
        finalPath = BACKUP_DIR + L"\\backup_" + GetTimestamp() + L".json";
    }

    // Copy current data file to backup location
    try {
        std::filesystem::copy_file(DATA_FILE, finalPath, std::filesystem::copy_options::overwrite_existing);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool DatabaseManager::RestoreFromBackup(const std::wstring& backupPath) {
    if (!FileExists(backupPath)) {
        return false;
    }

    try {
        // Create backup of current data first
        BackupData(BACKUP_DIR + L"\\pre_restore_backup_" + GetTimestamp() + L".json");

        // Copy backup file to current data file location
        std::filesystem::copy_file(backupPath, DATA_FILE, std::filesystem::copy_options::overwrite_existing);

        // Load the restored data
        return LoadAllData();
    }
    catch (const std::exception&) {
        return false;
    }
}

// Export/Import
bool DatabaseManager::ExportToCSV(const std::wstring& filePath, const std::wstring& userId) {
    try {
        std::wofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }

        // Export expenses
        file << L"EXPENSES" << std::endl;
        file << L"Date,Category,Amount,Note,Tags,Currency,Location" << std::endl;

        for (const auto& expense : expenses) {
            if (!userId.empty() && expense.userId != userId) continue;

            file << EscapeCSVField(expense.date) << L","
                << EscapeCSVField(expense.category) << L","
                << expense.amount << L","
                << EscapeCSVField(expense.note) << L","
                << EscapeCSVField(TagsToString(expense.tags)) << L","
                << EscapeCSVField(CurrencyToString(expense.currency)) << L","
                << EscapeCSVField(expense.location) << std::endl;
        }

        file << std::endl;

        // Export incomes
        file << L"INCOMES" << std::endl;
        file << L"Date,Source,Amount,Note,Tags,Currency,Taxable" << std::endl;

        for (const auto& income : incomes) {
            if (!userId.empty() && income.userId != userId) continue;

            file << EscapeCSVField(income.date) << L","
                << EscapeCSVField(income.source) << L","
                << income.amount << L","
                << EscapeCSVField(income.note) << L","
                << EscapeCSVField(TagsToString(income.tags)) << L","
                << EscapeCSVField(CurrencyToString(income.currency)) << L","
                << (income.isTaxable ? L"Yes" : L"No") << std::endl;
        }

        file << std::endl;

        // Export budgets
        file << L"BUDGETS" << std::endl;
        file << L"Category,Monthly Limit,Current Spent,Warning Threshold,Active" << std::endl;

        for (const auto& budget : budgets) {
            if (!userId.empty() && budget.userId != userId) continue;

            file << EscapeCSVField(budget.category) << L","
                << budget.monthlyLimit << L","
                << budget.currentSpent << L","
                << budget.warningThreshold << L","
                << (budget.isActive ? L"Yes" : L"No") << std::endl;
        }

        file.close();
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool DatabaseManager::ExportToPDF(const std::wstring& filePath, const std::wstring& userId) {
    // PDF export would require a PDF library like PDFlib or libharu
    // For now, we'll create a simple text-based report
    try {
        std::wofstream file(filePath + L".txt"); // Change extension to .txt for now
        if (!file.is_open()) {
            return false;
        }

        file << L"PERSONAL FINANCE REPORT" << std::endl;
        file << L"======================" << std::endl;
        file << L"Generated: " << GetCurrentDateTime() << std::endl << std::endl;

        // Summary
        double totalIncome = 0.0, totalExpenses = 0.0;
        for (const auto& income : incomes) {
            if (userId.empty() || income.userId == userId) {
                totalIncome += income.amount;
            }
        }

        for (const auto& expense : expenses) {
            if (userId.empty() || expense.userId == userId) {
                totalExpenses += expense.amount;
            }
        }

        file << L"FINANCIAL SUMMARY" << std::endl;
        file << L"Total Income: $" << std::fixed << std::setprecision(2) << totalIncome << std::endl;
        file << L"Total Expenses: $" << std::fixed << std::setprecision(2) << totalExpenses << std::endl;
        file << L"Balance: $" << std::fixed << std::setprecision(2) << (totalIncome - totalExpenses) << std::endl;
        file << std::endl;

        // Recent transactions
        file << L"RECENT EXPENSES (Last 10)" << std::endl;
        file << L"Date       Category         Amount    Note" << std::endl;
        file << L"----       --------         ------    ----" << std::endl;

        int count = 0;
        for (auto it = expenses.rbegin(); it != expenses.rend() && count < 10; ++it) {
            if (userId.empty() || it->userId == userId) {
                file << std::left << std::setw(11) << it->date
                    << std::setw(17) << it->category
                    << L"$" << std::right << std::setw(8) << std::fixed << std::setprecision(2) << it->amount
                    << L"  " << it->note << std::endl;
                count++;
            }
        }

        file.close();
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool DatabaseManager::ImportFromCSV(const std::wstring& filePath, const std::wstring& userId) {
    try {
        std::wifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }

        std::wstring line;
        std::wstring section;
        bool skipHeader = true;

        while (std::getline(file, line)) {
            if (line.empty()) {
                skipHeader = true;
                continue;
            }

            // Check for section headers
            if (line == L"EXPENSES" || line == L"INCOMES" || line == L"BUDGETS") {
                section = line;
                skipHeader = true;
                continue;
            }

            // Skip header lines
            if (skipHeader) {
                skipHeader = false;
                continue;
            }

            auto fields = ParseCSVLine(line);

            if (section == L"EXPENSES" && fields.size() >= 4) {
                Expense expense;
                expense.id = GenerateUniqueId();
                expense.userId = userId;
                expense.date = fields[0];
                expense.category = fields[1];
                expense.amount = _wtof(fields[2].c_str());
                expense.note = fields[3];

                //if (fields.size() > 4) expense.tags = ParseTags(fields[4]);//commented out for now
                if (fields.size() > 5) expense.currency = StringToCurrency(fields[5]);
                if (fields.size() > 6) expense.location = fields[6];

                if (ValidateExpense(expense)) {
                    expenses.push_back(expense);
                }
            }
            else if (section == L"INCOMES" && fields.size() >= 4) {
                Income income;
                income.id = GenerateUniqueId();
                income.userId = userId;
                income.date = fields[0];
                income.source = fields[1];
                income.amount = _wtof(fields[2].c_str());
                income.note = fields[3];

				//if (fields.size() > 4) income.tags = ParseTags(fields[4]);//commented out for now
                if (fields.size() > 5) income.currency = StringToCurrency(fields[5]);
                if (fields.size() > 6) income.isTaxable = (fields[6] == L"Yes");

                if (ValidateIncome(income)) {
                    incomes.push_back(income);
                }
            }
        }

        file.close();
        return SaveAllData();
    }
    catch (const std::exception&) {
        return false;
    }
}

bool DatabaseManager::ImportFromJSON(const std::wstring& filePath) {
    try {
        std::ifstream file(WStringToString(filePath));
        if (!file.is_open()) {
            return false;
        }

        json importData;
        file >> importData;
        file.close();

        // Import users (merge, don't replace)
        if (importData.contains("users")) {
            for (const auto& userJson : importData["users"]) {
                User user = JsonToUser(userJson);
                if (!GetUserByUsername(user.username)) {
                    users.push_back(user);
                }
            }
        }

        // Import other data (append)
        if (importData.contains("expenses")) {
            for (const auto& expenseJson : importData["expenses"]) {
                expenses.push_back(JsonToExpense(expenseJson));
            }
        }

        if (importData.contains("incomes")) {
            for (const auto& incomeJson : importData["incomes"]) {
                incomes.push_back(JsonToIncome(incomeJson));
            }
        }

        if (importData.contains("budgets")) {
            for (const auto& budgetJson : importData["budgets"]) {
                budgets.push_back(JsonToBudget(budgetJson));
            }
        }

        return SaveAllData();
    }
    catch (const std::exception&) {
        return false;
    }
}

// Auto-backup
void DatabaseManager::EnableAutoBackup(int intervalMinutes) {
    DisableAutoBackup(); // Stop existing timer

    backupInterval = intervalMinutes;
    backupTimerId = SetTimer(NULL, 0, intervalMinutes * 60 * 1000, BackupTimerProc);
}

void DatabaseManager::DisableAutoBackup() {
    if (backupTimerId != 0) {
        KillTimer(NULL, backupTimerId);
        backupTimerId = 0;
    }
}

bool DatabaseManager::IsAutoBackupEnabled() {
    return backupTimerId != 0;
}

void CALLBACK DatabaseManager::BackupTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    BackupData(); // Create automatic backup
}

// Data validation and repair
bool DatabaseManager::ValidateDatabase() {
    std::vector<std::wstring> issues;

    // Check for duplicate IDs
    std::set<std::wstring> expenseIds, incomeIds, budgetIds, goalIds, recurringIds;

    for (const auto& expense : expenses) {
        if (expenseIds.count(expense.id)) {
            issues.push_back(L"Duplicate expense ID: " + expense.id);
        }
        else {
            expenseIds.insert(expense.id);
        }

        if (!ValidateExpense(expense)) {
            issues.push_back(L"Invalid expense data: " + expense.id);
        }
    }

    for (const auto& income : incomes) {
        if (incomeIds.count(income.id)) {
            issues.push_back(L"Duplicate income ID: " + income.id);
        }
        else {
            incomeIds.insert(income.id);
        }

        if (!ValidateIncome(income)) {
            issues.push_back(L"Invalid income data: " + income.id);
        }
    }

    // Check user references
    std::set<std::wstring> userIds;
    for (const auto& user : users) {
        userIds.insert(user.username);
    }

    for (const auto& expense : expenses) {
        if (!userIds.count(expense.userId)) {
            issues.push_back(L"Expense references non-existent user: " + expense.userId);
        }
    }

    return issues.empty();
}

bool DatabaseManager::RepairDatabase() {
    bool repaired = false;

    // Remove invalid entries
    auto originalExpenseCount = expenses.size();
    expenses.erase(std::remove_if(expenses.begin(), expenses.end(),
        [](const Expense& e) { return !ValidateExpense(e); }), expenses.end());

    if (expenses.size() != originalExpenseCount) {
        repaired = true;
    }

    auto originalIncomeCount = incomes.size();
    incomes.erase(std::remove_if(incomes.begin(), incomes.end(),
        [](const Income& i) { return !ValidateIncome(i); }), incomes.end());

    if (incomes.size() != originalIncomeCount) {
        repaired = true;
    }

    // Fix duplicate IDs
    std::set<std::wstring> usedIds;
    for (auto& expense : expenses) {
        if (usedIds.count(expense.id) || expense.id.empty()) {
            expense.id = GenerateUniqueId();
            repaired = true;
        }
        usedIds.insert(expense.id);
    }

    for (auto& income : incomes) {
        if (usedIds.count(income.id) || income.id.empty()) {
            income.id = GenerateUniqueId();
            repaired = true;
        }
        usedIds.insert(income.id);
    }

    if (repaired) {
        SaveAllData();
    }

    return repaired;
}

std::vector<std::wstring> DatabaseManager::GetDatabaseIssues() {
    std::vector<std::wstring> issues;

    // This would contain the validation logic from ValidateDatabase
    // but return the issues instead of just true/false

    return issues;
}

// Migration and versioning
std::wstring DatabaseManager::GetDatabaseVersion() {
    if (!FileExists(DATA_FILE)) {
        return L"0.0.0";
    }

    try {
        std::ifstream file(DATA_FILE);
        json root;
        file >> root;

        if (root.contains("version")) {
            return StringToWString(root["version"]);
        }
    }
    catch (const std::exception&) {
        // Ignore errors
    }

    return L"0.0.0";
}

bool DatabaseManager::MigrateDatabase(const std::wstring& fromVersion, const std::wstring& toVersion) {
    // Database migration logic would go here
    // For now, just return true as we don't have version-specific migrations
    return true;
}

// File paths
std::wstring DatabaseManager::GetDataFilePath() {
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDir);
    return std::wstring(currentDir) + L"\\" + DATA_FILE;
}

std::wstring DatabaseManager::GetBackupDirectory() {
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDir);
    return std::wstring(currentDir) + L"\\" + BACKUP_DIR;
}

std::wstring DatabaseManager::GetExportDirectory() {
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDir);
    return std::wstring(currentDir) + L"\\" + EXPORT_DIR;
}

// JSON conversion functions
json DatabaseManager::UserToJson(const User& user) {
    json j;
    j["username"] = WStringToString(user.username);
    j["displayName"] = WStringToString(user.displayName);
    j["authType"] = WStringToString(AuthTypeToString(user.authType));
    j["authHash"] = WStringToString(user.authHash);
    j["profilePicPath"] = WStringToString(user.profilePicPath);
    j["defaultCurrency"] = WStringToString(CurrencyToString(user.defaultCurrency));
    j["isDarkMode"] = user.isDarkMode;
    j["createdDate"] = WStringToString(user.createdDate);
    return j;
}

User DatabaseManager::JsonToUser(const json& j) {
    User user;
    if (j.contains("username")) user.username = StringToWString(j["username"]);
    if (j.contains("displayName")) user.displayName = StringToWString(j["displayName"]);
    if (j.contains("authType")) user.authType = StringToAuthType(StringToWString(j["authType"]));
    if (j.contains("authHash")) user.authHash = StringToWString(j["authHash"]);
    if (j.contains("profilePicPath")) user.profilePicPath = StringToWString(j["profilePicPath"]);
    if (j.contains("defaultCurrency")) user.defaultCurrency = StringToCurrency(StringToWString(j["defaultCurrency"]));
    if (j.contains("isDarkMode")) user.isDarkMode = j["isDarkMode"];
    if (j.contains("createdDate")) user.createdDate = StringToWString(j["createdDate"]);
    return user;
}

json DatabaseManager::ExpenseToJson(const Expense& expense) {
    json j;
    j["id"] = WStringToString(expense.id);
    j["userId"] = WStringToString(expense.userId);
    j["category"] = WStringToString(expense.category);
    j["amount"] = expense.amount;
    j["note"] = WStringToString(expense.note);
    j["date"] = WStringToString(expense.date);
    j["receiptPath"] = WStringToString(expense.receiptPath);
    j["currency"] = WStringToString(CurrencyToString(expense.currency));
    j["exchangeRate"] = expense.exchangeRate;
    j["location"] = WStringToString(expense.location);

    json tagsArray = json::array();
    for (const auto& tag : expense.tags) {
        tagsArray.push_back(WStringToString(tag));
    }
    j["tags"] = tagsArray;

    return j;
}

Expense DatabaseManager::JsonToExpense(const json& j) {
    Expense expense;
    if (j.contains("id")) expense.id = StringToWString(j["id"]);
    if (j.contains("userId")) expense.userId = StringToWString(j["userId"]);
    if (j.contains("category")) expense.category = StringToWString(j["category"]);
    if (j.contains("amount")) expense.amount = j["amount"];
    if (j.contains("note")) expense.note = StringToWString(j["note"]);
    if (j.contains("date")) expense.date = StringToWString(j["date"]);
    if (j.contains("receiptPath")) expense.receiptPath = StringToWString(j["receiptPath"]);
    if (j.contains("currency")) expense.currency = StringToCurrency(StringToWString(j["currency"]));
    if (j.contains("exchangeRate")) expense.exchangeRate = j["exchangeRate"];
    if (j.contains("location")) expense.location = StringToWString(j["location"]);

    if (j.contains("tags") && j["tags"].is_array()) {
        for (const auto& tagJson : j["tags"]) {
            expense.tags.push_back(StringToWString(tagJson));
        }
    }

    return expense;
}

// Similar implementations for other JSON conversion functions...
json DatabaseManager::IncomeToJson(const Income& income) {
    json j;
    j["id"] = WStringToString(income.id);
    j["userId"] = WStringToString(income.userId);
    j["source"] = WStringToString(income.source);
    j["amount"] = income.amount;
    j["note"] = WStringToString(income.note);
    j["date"] = WStringToString(income.date);
    j["currency"] = WStringToString(CurrencyToString(income.currency));
    j["exchangeRate"] = income.exchangeRate;
    j["isTaxable"] = income.isTaxable;

    json tagsArray = json::array();
    for (const auto& tag : income.tags) {
        tagsArray.push_back(WStringToString(tag));
    }
    j["tags"] = tagsArray;

    return j;
}

Income DatabaseManager::JsonToIncome(const json& j) {
    Income income;
    if (j.contains("id")) income.id = StringToWString(j["id"]);
    if (j.contains("userId")) income.userId = StringToWString(j["userId"]);
    if (j.contains("source")) income.source = StringToWString(j["source"]);
    if (j.contains("amount")) income.amount = j["amount"];
    if (j.contains("note")) income.note = StringToWString(j["note"]);
    if (j.contains("date")) income.date = StringToWString(j["date"]);
    if (j.contains("currency")) income.currency = StringToCurrency(StringToWString(j["currency"]));
    if (j.contains("exchangeRate")) income.exchangeRate = j["exchangeRate"];
    if (j.contains("isTaxable")) income.isTaxable = j["isTaxable"];

    if (j.contains("tags") && j["tags"].is_array()) {
        for (const auto& tagJson : j["tags"]) {
            income.tags.push_back(StringToWString(tagJson));
        }
    }

    return income;
}

bool DatabaseManager::CreateDirectoryIfNotExists(const std::wstring& path) {
    return CreateDirectory(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

std::wstring DatabaseManager::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::wstringstream ss;
    ss << std::put_time(std::localtime(&time_t), L"%Y%m%d_%H%M%S");
    return ss.str();
}

bool DatabaseManager::FileExists(const std::wstring& path) {
    DWORD attributes = GetFileAttributes(path.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

std::wstring DatabaseManager::EscapeCSVField(const std::wstring& field) {
    if (field.find(L',') != std::wstring::npos ||
        field.find(L'"') != std::wstring::npos ||
        field.find(L'\n') != std::wstring::npos) {

        std::wstring escaped = L"\"";
        for (wchar_t c : field) {
            if (c == L'"') {
                escaped += L"\"\"";
            }
            else {
                escaped += c;
            }
        }
        escaped += L"\"";
        return escaped;
    }
    return field;
}

std::vector<std::wstring> DatabaseManager::ParseCSVLine(const std::wstring& line) {
    std::vector<std::wstring> result;
    std::wstring current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        wchar_t c = line[i];

        if (c == L'"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == L'"') {
                current += L'"';
                ++i; // Skip next quote
            }
            else {
                inQuotes = !inQuotes;
            }
        }
        else if (c == L',' && !inQuotes) {
            result.push_back(current);
            current.clear();
        }
        else {
            current += c;
        }
    }

    result.push_back(current);
    return result;
}

// File locking
bool DatabaseManager::AcquireFileLock() {
    if (fileLock != INVALID_HANDLE_VALUE) {
        return true; // Already locked
    }

    std::wstring lockFile = DATA_FILE + L".lock";
    fileLock = CreateFile(lockFile.c_str(), GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);

    return fileLock != INVALID_HANDLE_VALUE;
}

void DatabaseManager::ReleaseFileLock() {
    if (fileLock != INVALID_HANDLE_VALUE) {
        CloseHandle(fileLock);
        fileLock = INVALID_HANDLE_VALUE;
    }
}
