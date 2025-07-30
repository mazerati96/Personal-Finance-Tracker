// Utils.h
#ifndef UTILS_H
#define UTILS_H

#include "DataStructures.h"
#include "UIManager.h"
#include <string>
#include <vector>
#include <map>

// =============================================================================
// CURRENCY UTILITIES
// =============================================================================
std::wstring CurrencyToString(CurrencyType currency);
CurrencyType StringToCurrency(const std::wstring& currencyString);
double ConvertCurrency(double amount, CurrencyType from, CurrencyType to);

// =============================================================================
// AUTH TYPE UTILITIES
// =============================================================================
std::wstring AuthTypeToString(AuthType authType);
AuthType StringToAuthType(const std::wstring& authTypeStr);

// =============================================================================
// DATE AND TIME UTILITIES
// =============================================================================
 std::wstring GetCurrentDateTime();
 std::wstring GetCurrentDate();
 void InitializeDefaultData();

// =============================================================================
// ID GENERATION
// =============================================================================
std::wstring GenerateUniqueId();

// =============================================================================
// VALIDATION FUNCTIONS
// =============================================================================
bool ValidateUserInput(const std::wstring& username, const std::wstring& displayName, const std::wstring& auth);
bool ValidateExpense(const Expense& expense);
bool ValidateIncome(const Income& income);
bool ValidateUser(const User& user);
// =============================================================================
// DATE RANGE UTILITIES
// =============================================================================
bool IsDateInRange(const std::wstring& date, const DateRange& range);

// =============================================================================
// DATA INITIALIZATION
// =============================================================================

// =============================================================================
// STRING CONVERSION UTILITIES
// =============================================================================
std::wstring TagsToString(const std::vector<std::wstring>& tags);
std::vector<std::wstring> ParseTags(const std::wstring& tagsString);

// =============================================================================
// BUDGET AND SPENDING
// =============================================================================
void UpdateBudgetSpending(const std::wstring& budgetId, double amount);

// =============================================================================
// USER MANAGEMENT
// =============================================================================
User GetUserByUsername(const std::wstring& username);

// =============================================================================
// ANALYTICS HELPERS
// =============================================================================

std::map<std::wstring, double> GetCategoryAnalytics(const std::wstring& userId);
std::map<std::wstring, double> GetCategoryTotals(const std::wstring& userId);

// =============================================================================
// GOAL TRACKING
// =============================================================================
double GetSavingsGoalProgress(const std::wstring& goalId);
int GetDaysUntilGoalDeadline(const std::wstring& goalId);

// =============================================================================
// UI HELPER FUNCTIONS
// =============================================================================
std::wstring FormatCurrencyType(double amount, CurrencyType currency);
std::wstring FormatPercentage(double percentage);
std::wstring FormatDate(const std::wstring& date);

// =============================================================================
// NOTIFICATION HELPERS
// =============================================================================
void ShowNotification(const std::wstring& message, NotificationType type); 

// =============================================================================
// DATA PERSISTENCE HELPERS
// =============================================================================
std::wstring DataToString(const std::map<std::wstring, std::wstring>& data);
std::map<std::wstring, std::wstring> StringToData(const std::wstring& dataString);

// =============================================================================
// MATHEMATICAL UTILITIES
// =============================================================================
double CalculateCompoundInterest(double principal, double rate, int periods);
double CalculateMonthlyPayment(double principal, double rate, int months);

// =============================================================================
// FILE AND PATH UTILITIES
// =============================================================================
bool FileExists(const std::wstring& filePath);
std::wstring GetApplicationPath();
std::wstring GetDataDirectory();

// =============================================================================
// ERROR HANDLING
// =============================================================================
void LogError(const std::wstring& error, const std::wstring& function = L"");
void LogInfo(const std::wstring& info);

// =============================================================================
// STRING UTILITIES
// =============================================================================
std::wstring Trim(const std::wstring& str);
std::vector<std::wstring> Split(const std::wstring& str, wchar_t delimiter);
std::wstring Join(const std::vector<std::wstring>& strings, const std::wstring& delimiter);
std::wstring ToLower(const std::wstring& str);
std::wstring ToUpper(const std::wstring& str);

// =============================================================================
// CONVERSION UTILITIES
// =============================================================================

std::wstring DoubleToWString(double value, int precision = 2);
double WStringToDouble(const std::wstring& str);
int WStringToInt(const std::wstring& str);
std::wstring IntToWString(int value);

#endif // UTILS_H