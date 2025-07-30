// Utils.cpp
#include "Utils.h"
#include "DataStructures.h"
#include "UIManager.h"
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <codecvt>
#include <locale>
#include <set>
#include <ctime>
#include <map>

// =============================================================================
// CURRENCY UTILITIES
// =============================================================================
 std::wstring CurrencyToString(CurrencyType currency) {
    switch (currency) {
    case CurrencyType::USD: return L"USD";
    case CurrencyType::EUR: return L"EUR";
    case CurrencyType::GBP: return L"GBP";
    case CurrencyType::JPY: return L"JPY";
    case CurrencyType::CAD: return L"CAD";
    case CurrencyType::AUD: return L"AUD";
    default: return L"USD";
    }
}

CurrencyType StringToCurrency(const std::wstring& currencyString) {
    if (currencyString == L"USD") return CurrencyType::USD;
    if (currencyString == L"EUR") return CurrencyType::EUR;
    if (currencyString == L"GBP") return CurrencyType::GBP;
    if (currencyString == L"JPY") return CurrencyType::JPY;
    if (currencyString == L"CAD") return CurrencyType::CAD;
    if (currencyString == L"AUD") return CurrencyType::AUD;
    return CurrencyType::USD;
}

double ConvertCurrency(double amount, CurrencyType from, CurrencyType to) {
    // Enhanced currency conversion with more rates
    if (from == to) return amount;

    // Stub implementation - you'd normally get exchange rates from an API
    std::map<CurrencyType, double> rates = {
        {CurrencyType::USD, 1.0},
        {CurrencyType::EUR, 0.85},
        {CurrencyType::GBP, 0.73},
        {CurrencyType::JPY, 110.0},
        {CurrencyType::CAD, 1.25},
        {CurrencyType::AUD, 1.35}
    };

    return (amount / rates[from]) * rates[to];
}

// =============================================================================
// AUTH TYPE UTILITIES
// =============================================================================
std::wstring AuthTypeToString(AuthType authType) {
    switch (authType) {
    case AuthType::PASSWORD: return L"Password";
    default: return L"Password";
    }
}

AuthType StringToAuthType(const std::wstring& authTypeString) {
    if (authTypeString == L"Password") return AuthType::PASSWORD;
    return AuthType::PASSWORD;
}

// =============================================================================
// DATE AND TIME UTILITIES
// =============================================================================


std::wstring GetCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::tm tm;
    localtime_s(&tm, &time_t);

    std::wstringstream ss;
    ss << std::put_time(&tm, L"%Y-%m-%d");
    return ss.str();
}

std::wstring GetCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time_t);

    std::wstringstream ss;
    ss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S");
    return ss.str();
}
// =============================================================================
// ID GENERATION
// =============================================================================
std::wstring GenerateUniqueId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::wstring id = L"ID_";
    std::wstring chars = L"0123456789ABCDEF";

    for (int i = 0; i < 8; ++i) {
        id += chars[dis(gen)];
    }

    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    id += L"_" + std::to_wstring(timestamp);

    return id;
}

// =============================================================================
// VALIDATION FUNCTIONS
// =============================================================================
bool ValidateUser(const User& user) {
    if (user.username.empty() || user.username.length() < 3) {
        return false;
    }

    if (user.username.find_first_of(L" \t\n\r") != std::wstring::npos) {
        return false;
    }

    if (user.displayName.empty()) {
        return false;
    }

    return true;
}

bool ValidateExpense(const Expense& expense) {
    return !expense.userId.empty() &&
        !expense.category.empty() &&
        expense.amount > 0 &&
        !expense.date.empty();
}

bool ValidateIncome(const Income& income) {
    return !income.userId.empty() &&
        !income.source.empty() &&
        income.amount > 0 &&
        !income.date.empty();
}

bool ValidateUserInput(const std::wstring& username, const std::wstring& displayName, const std::wstring& auth) {
    if (username.empty()) {
        MessageBox(NULL, L"Username cannot be empty.", L"Validation Error", MB_OK);
        return false;
    }

    if (displayName.empty()) {
        MessageBox(NULL, L"Display name cannot be empty.", L"Validation Error", MB_OK);
        return false;
    }

    if (username.find(L' ') != std::wstring::npos) {
        MessageBox(NULL, L"Username cannot contain spaces.", L"Validation Error", MB_OK);
        return false;
    }

    /*if (GetUserByUsername(username)) {
        MessageBox(NULL, L"Username already exists.", L"Validation Error", MB_OK);
        return false;
    }
    */
    return true;
}
// =============================================================================
// DATE RANGE UTILITIES
// =============================================================================
bool IsDateInRange(const std::wstring& date, const DateRange& range) {
    if (range.startDate.empty() && range.endDate.empty()) return true;
    if (range.startDate.empty()) return date <= range.endDate;
    if (range.endDate.empty()) return date >= range.startDate;
    return date >= range.startDate && date <= range.endDate;
}

// =============================================================================
// DATA INITIALIZATION
// =============================================================================
void InitializeDefaultData() {
    // Initialize default categories
    categories.clear();
    for (const auto& defaultCat : DEFAULT_CATEGORIES) {
        categories.push_back(defaultCat);
    }

    // Initialize exchange rates (simplified - in real app, fetch from API)
    exchangeRates[CurrencyType::USD][CurrencyType::EUR] = 0.85;
    exchangeRates[CurrencyType::USD][CurrencyType::GBP] = 0.73;
    exchangeRates[CurrencyType::USD][CurrencyType::JPY] = 110.0;
    exchangeRates[CurrencyType::USD][CurrencyType::CAD] = 1.25;
    exchangeRates[CurrencyType::USD][CurrencyType::AUD] = 1.35;

    // Reverse rates
    for (auto& fromCurrency : exchangeRates) {
        for (auto& toCurrency : fromCurrency.second) {
            exchangeRates[toCurrency.first][fromCurrency.first] = 1.0 / toCurrency.second;
        }
    }
}

// =============================================================================
// STRING CONVERSION UTILITIES
// =============================================================================
std::wstring TagsToString(const std::vector<std::wstring>& tags) {
    return Join(tags, L",");
}

std::vector<std::wstring> ParseTags(const std::wstring& tagsString) {
    if (tagsString.empty()) return {};
    return Split(tagsString, L',');
}

// =============================================================================
// BUDGET AND SPENDING
// =============================================================================
void UpdateBudgetSpending(const std::wstring& budgetId, double amount) {
    LogInfo(L"Updating budget " + budgetId + L" with amount: " + DoubleToWString(amount));
}

// =============================================================================
// USER MANAGEMENT
// =============================================================================
User GetUserByUsername(const std::wstring& username) {
    User user;
    user.username = username;
    user.displayName = username;
    user.defaultCurrency = CurrencyType::USD;
    user.authType = AuthType::PASSWORD;
    user.isDarkMode = false;
    user.createdDate = GetCurrentDateTime();
    user.profilePicPath = L"";
    return user;
}

// =============================================================================
// ANALYTICS HELPERS
// =============================================================================


std::map<std::wstring, double> GetCategoryAnalytics(const std::wstring& userId) {
    std::map<std::wstring, double> analytics;
    analytics[L"Food"] = 500.0;
    analytics[L"Transport"] = 200.0;
    analytics[L"Entertainment"] = 150.0;
    return analytics;
}

std::map<std::wstring, double> GetCategoryTotals(const std::wstring& userId) {
    return GetCategoryAnalytics(userId);
}

// =============================================================================
// GOAL TRACKING
// =============================================================================
double GetSavingsGoalProgress(const std::wstring& goalId) {
    return 0.0;
}

int GetDaysUntilGoalDeadline(const std::wstring& goalId) {
    return 30;
}

// =============================================================================
// UI HELPER FUNCTIONS
// =============================================================================
std::wstring FormatCurrencyType(double amount, CurrencyType currency) {
    std::wstring currencySymbol = L"$"; // Default to USD

    std::wstringstream ss;
    ss << std::fixed << std::setprecision(2) << amount;
    return currencySymbol + ss.str();
}

std::wstring FormatPercentage(double percentage) {
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(1) << percentage << L"%";
    return ss.str();
}

std::wstring FormatDate(const std::wstring& date) {
    return date;
}

// =============================================================================
// NOTIFICATION HELPERS
// =============================================================================
void ShowNotification(const std::wstring& message, NotificationType type) {
    LogInfo(L"Notification: " + message);
}

// =============================================================================
// DATA PERSISTENCE HELPERS
// =============================================================================
std::wstring DataToString(const std::map<std::wstring, std::wstring>& data) {
    std::wstring result;
    for (const auto& pair : data) {
        if (!result.empty()) result += L";";
        result += pair.first + L"=" + pair.second;
    }
    return result;
}

std::map<std::wstring, std::wstring> StringToData(const std::wstring& dataString) {
    std::map<std::wstring, std::wstring> data;
    auto pairs = Split(dataString, L';');

    for (const auto& pair : pairs) {
        auto keyValue = Split(pair, L'=');
        if (keyValue.size() == 2) {
            data[keyValue[0]] = keyValue[1];
        }
    }

    return data;
}

// =============================================================================
// MATHEMATICAL UTILITIES
// =============================================================================
double CalculateCompoundInterest(double principal, double rate, int periods) {
    return principal * std::pow(1.0 + rate, periods);
}

double CalculateMonthlyPayment(double principal, double rate, int months) {
    if (rate == 0) return principal / months;
    double monthlyRate = rate / 12.0;
    return principal * (monthlyRate * std::pow(1 + monthlyRate, months)) /
        (std::pow(1 + monthlyRate, months) - 1);
}

// =============================================================================
// FILE AND PATH UTILITIES
// =============================================================================
bool FileExists(const std::wstring& filePath) {
    return std::filesystem::exists(filePath);
}

std::wstring GetApplicationPath() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring path(buffer);
    return path.substr(0, path.find_last_of(L"\\/"));
}

std::wstring GetDataDirectory() {
    return GetApplicationPath() + L"\\Data";
}

// =============================================================================
// ERROR HANDLING
// =============================================================================
void LogError(const std::wstring& error, const std::wstring& function) {
    std::wstring logMessage = L"[ERROR] " + function + L": " + error;
    OutputDebugString(logMessage.c_str());
}

void LogInfo(const std::wstring& info) {
    std::wstring logMessage = L"[INFO] " + info;
    OutputDebugString(logMessage.c_str());
}

// =============================================================================
// STRING UTILITIES
// =============================================================================
std::wstring Trim(const std::wstring& str) {
    size_t start = str.find_first_not_of(L" \t\r\n");
    if (start == std::wstring::npos) return L"";

    size_t end = str.find_last_not_of(L" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::vector<std::wstring> Split(const std::wstring& str, wchar_t delimiter) {
    std::vector<std::wstring> result;
    std::wstring current;

    for (wchar_t ch : str) {
        if (ch == delimiter) {
            if (!current.empty()) {
                result.push_back(Trim(current));
                current.clear();
            }
        }
        else {
            current += ch;
        }
    }

    if (!current.empty()) {
        result.push_back(Trim(current));
    }

    return result;
}

std::wstring Join(const std::vector<std::wstring>& strings, const std::wstring& delimiter) {
    if (strings.empty()) return L"";

    std::wstring result = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
        result += delimiter + strings[i];
    }

    return result;
}

std::wstring ToLower(const std::wstring& str) {
    std::wstring result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::towlower);
    return result;
}

std::wstring ToUpper(const std::wstring& str) {
    std::wstring result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::towupper);
    return result;
}

// =============================================================================
// CONVERSION UTILITIES
// =============================================================================


std::wstring DoubleToWString(double value, int precision) {
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

double WStringToDouble(const std::wstring& str) {
    try {
        return std::stod(str);
    }
    catch (...) {
        return 0.0;
    }
}

int WStringToInt(const std::wstring& str) {
    try {
        return std::stoi(str);
    }
    catch (...) {
        return 0;
    }
}

std::wstring IntToWString(int value) {
    return std::to_wstring(value);
}