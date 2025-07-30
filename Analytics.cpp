#define NOMINMAX
#include <Windows.h>
#include "Analytics.h"
#include "UserManager.h"
#include "DatabaseManager.h"
#include "DataStructures.h"
#include "Utils.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <set>
#include <sstream>     // For 'ss', 'stream'
#include <iostream>    // For 'std::cout', 'out'
#include <chrono>      // For 'now' (if using time functions)
#include <vector>      // If not already included
#include <string>      // If not already included


#undef max
#undef min

INT64 Analytics::ForecastDialogProc(HWND hWnd, UINT msg, UINT_PTR wParam, LONG_PTR lParam) {
    return 0; // TODO: handle dialog messages
}

INT64 Analytics::ComparisonDialogProc(HWND hWnd, UINT msg, UINT_PTR wParam, LONG_PTR lParam) {
    return 0;
}

INT64 Analytics::CategoryAnalysisDialogProc(HWND hWnd, UINT msg, UINT_PTR wParam, LONG_PTR lParam) {
    return 0;
}

void Analytics::SetupTrendsDialog(HWND hWnd) {
    // TODO: setup controls
}

std::map<int, double> Analytics::GetSpendingByDayOfWeek(const std::wstring& userId, const DateRange& range) {
    return {}; // TODO: calculate day-of-week stats
}

// Dialog functions
// Add these declarations to make the global functions visible
std::map<std::wstring, double> GetSpendingTrends(const std::wstring& userId, const std::wstring& period) {
    std::map<std::wstring, double> trends;
    trends[L"January"] = 1500.0;
    trends[L"February"] = 1800.0;
    trends[L"March"] = 1200.0;
    return trends;
}
extern std::vector<CategoryAnalytics> GetCategoryAnalytics(const std::wstring& userId, const DateRange& range);
extern std::map<std::wstring, double> GetCategoryTotals(const std::wstring& userId, const DateRange& range);
// Add this in Analytics.cpp before they're used
namespace AdvancedAnalyticsHelpers {
    std::pair<double, double> LinearRegression(const std::vector<double>& x, const std::vector<double>& y) {
        if (x.empty() || y.empty() || x.size() != y.size()) return { 0, 0 };

        const size_t n = x.size();
        double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;

        for (size_t i = 0; i < n; ++i) {
            sumX += x[i];
            sumY += y[i];
            sumXY += x[i] * y[i];
            sumX2 += x[i] * x[i];
        }

        double denominator = n * sumX2 - sumX * sumX;
        if (denominator == 0) return { 0, 0 };

        double slope = (n * sumXY - sumX * sumY) / denominator;
        double intercept = (sumY - slope * sumX) / n;

        return { slope, intercept };
    }

    double PredictValue(const std::pair<double, double>& coeffs, double x) {
        return coeffs.first * x + coeffs.second;
    }
}

// Add these helper functions in Analytics.cpp (before they're used)


void Analytics::ShowSummaryDialog(HWND parent) {
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ANALYTICS_SUMMARY), parent, SummaryDialogProc);
}

void Analytics::ShowTrendsDialog(HWND parent) {
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ANALYTICS_TRENDS), parent, TrendsDialogProc);
}

void Analytics::ShowCategoryAnalysisDialog(HWND parent) {
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ANALYTICS_CATEGORIES), parent, CategoryAnalysisDialogProc);
}

void Analytics::ShowComparisonDialog(HWND parent) {
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ANALYTICS_COMPARISON), parent, ComparisonDialogProc);
}

void Analytics::ShowForecastDialog(HWND parent) {
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ANALYTICS_FORECAST), parent, ForecastDialogProc);
}


double Analytics::GetTotalIncome(const std::wstring& userId, const DateRange& range) {
    double total = 0.0;
    for (const auto& income : incomes) {
        if (income.userId == userId && IsDateInRange(income.date, range)) {
            total += ConvertCurrency(income.amount, income.currency,
                UserManager::GetUserByUsername(userId)->defaultCurrency);
        }
    }
    return total;
}

double Analytics::GetTotalExpenses(const std::wstring& userId, const DateRange& range) {
    double total = 0.0;
    for (const auto& expense : expenses) {
        if (expense.userId == userId && IsDateInRange(expense.date, range)) {
            total += ConvertCurrency(expense.amount, expense.currency,
                UserManager::GetUserByUsername(userId)->defaultCurrency);
        }
    }
    return total;
}

double Analytics::GetBalance(const std::wstring& userId, const DateRange& range) {
    return GetTotalIncome(userId, range) - GetTotalExpenses(userId, range);
}

double Analytics::GetAverageMonthlyIncome(const std::wstring& userId, int months) {
    auto monthlyData = GetMonthlyData(userId, months);
    if (monthlyData.empty()) return 0.0;

    double total = 0.0;
    for (const auto& data : monthlyData) {
        total += data.totalIncome;
    }
    return total / monthlyData.size();
}

double Analytics::GetAverageMonthlyExpenses(const std::wstring& userId, int months) {
    auto monthlyData = GetMonthlyData(userId, months);
    if (monthlyData.empty()) return 0.0;

    double total = 0.0;
    for (const auto& data : monthlyData) {
        total += data.totalExpenses;
    }
    return total / monthlyData.size();
}

// Trend analysis
std::vector<SpendingTrend> Analytics::GetSpendingTrends(const std::wstring& userId, const std::wstring& period) {
    return Analytics::GetSpendingTrends(userId, period); 
}

std::vector<MonthlyFinancialData> Analytics::GetMonthlyData(const std::wstring& userId, int months) {
    std::map<std::wstring, MonthlyFinancialData> monthlyMap;

    // Process expenses
    for (const auto& expense : expenses) {
        if (expense.userId != userId) continue;

        std::wstring monthKey = expense.date.substr(0, 7); // YYYY-MM

        if (monthlyMap.find(monthKey) == monthlyMap.end()) {
            monthlyMap[monthKey] = MonthlyFinancialData();
            monthlyMap[monthKey].month = monthKey;
        }

        double convertedAmount = ConvertCurrency(expense.amount, expense.currency,
            UserManager::GetUserByUsername(userId)->defaultCurrency);

        monthlyMap[monthKey].totalExpenses += convertedAmount;
        monthlyMap[monthKey].categorySpending[expense.category] += convertedAmount;
        monthlyMap[monthKey].transactionCount++;
    }

    // Process incomes
    for (const auto& income : incomes) {
        if (income.userId != userId) continue;

        std::wstring monthKey = income.date.substr(0, 7); // YYYY-MM

        if (monthlyMap.find(monthKey) == monthlyMap.end()) {
            monthlyMap[monthKey] = MonthlyFinancialData();
            monthlyMap[monthKey].month = monthKey;
        }

        double convertedAmount = ConvertCurrency(income.amount, income.currency,
            UserManager::GetUserByUsername(userId)->defaultCurrency);

        monthlyMap[monthKey].totalIncome += convertedAmount;
        monthlyMap[monthKey].transactionCount++;
    }

    // Calculate balances
    for (auto& pair : monthlyMap) {
        pair.second.balance = pair.second.totalIncome - pair.second.totalExpenses;
    }

    // Convert to vector and sort
    std::vector<MonthlyFinancialData> result;
    for (const auto& pair : monthlyMap) {
        result.push_back(pair.second);
    }

    std::sort(result.begin(), result.end(),
        [](const MonthlyFinancialData& a, const MonthlyFinancialData& b) {
            return a.month > b.month; // Most recent first
        });

    // Limit to requested number of months
    if (result.size() > static_cast<size_t>(months)) {
        result.resize(months);
    }

    return result;
}

double Analytics::GetGrowthRate(const std::vector<double>& values) {
    if (values.size() < 2) return 0.0;

    double firstValue = values.front();
    double lastValue = values.back();

    if (firstValue <= 0) return 0.0;

    int periods = static_cast<int>(values.size()) - 1;
    return (std::pow(lastValue / firstValue, 1.0 / periods) - 1.0) * 100.0;
}

std::wstring Analytics::GetTrendDirection(const std::vector<double>& values) {
    if (values.size() < 2) return L"stable";

    // Simple linear regression to determine trend
    double n = static_cast<double>(values.size());
    double sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;

    for (size_t i = 0; i < values.size(); ++i) {
        double x = static_cast<double>(i);
        double y = values[i];
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumXX += x * x;
    }

    double slope = (n * sumXY - sumX * sumY) / (n * sumXX - sumX * sumX);

    if (slope > 0.01) return L"increasing";
    else if (slope < -0.01) return L"decreasing";
    else return L"stable";
}

// Category analysis
std::vector<CategoryAnalytics> Analytics::GetCategoryAnalytics(const std::wstring& userId, const DateRange& range) {
    return ::GetCategoryAnalytics(userId, range); // Use global function
}

std::map<std::wstring, double> Analytics::GetCategoryTotals(const std::wstring& userId, const DateRange& range) {
    return ::GetCategoryTotals(userId, range); // Use global function
}

std::vector<std::wstring> Analytics::GetTopCategories(const std::wstring& userId, int count, const DateRange& range) {
    auto categoryTotals = GetCategoryTotals(userId, range);

    // Convert to vector and sort by amount
    std::vector<std::pair<std::wstring, double>> categoryPairs;
    for (const auto& pair : categoryTotals) {
        categoryPairs.emplace_back(pair.first, pair.second);
    }

    std::sort(categoryPairs.begin(), categoryPairs.end(),
        [](const std::pair<std::wstring, double>& a, const std::pair<std::wstring, double>& b) {
            return a.second > b.second;
        });

    std::vector<std::wstring> result;
    for (int i = 0; i < count && i < static_cast<int>(categoryPairs.size()); ++i) {
        result.push_back(categoryPairs[i].first);
    }

    return result;
}

double Analytics::GetCategoryGrowthRate(const std::wstring& userId, const std::wstring& category, int months) {
    auto monthlyData = GetMonthlyData(userId, months);

    std::vector<double> categoryValues;
    for (const auto& data : monthlyData) {
        auto it = data.categorySpending.find(category);
        categoryValues.push_back(it != data.categorySpending.end() ? it->second : 0.0);
    }

    std::reverse(categoryValues.begin(), categoryValues.end()); // Chronological order
    return GetGrowthRate(categoryValues);
}

// Comparison analysis
ComparisonData Analytics::ComparePeriods(const std::wstring& userId, const DateRange& period1, const DateRange& period2) {
    ComparisonData comparison;
    comparison.period1 = std::wstring(period1.startDate) + L" to " + period1.endDate;
    comparison.period2 = std::wstring(period2.startDate) + L" to " + period2.endDate;

    double income1 = GetTotalIncome(userId, period1);
    double income2 = GetTotalIncome(userId, period2);
    double expenses1 = GetTotalExpenses(userId, period1);
    double expenses2 = GetTotalExpenses(userId, period2);

    comparison.incomeChange = GetPercentageChange(income1, income2);
    comparison.expenseChange = GetPercentageChange(expenses1, expenses2);
    comparison.balanceChange = GetPercentageChange(
        income1 - expenses1, income2 - expenses2);

    // Category comparisons
    auto categories1 = GetCategoryTotals(userId, period1);
    auto categories2 = GetCategoryTotals(userId, period2);

    std::set<std::wstring> allCategories;
    for (const auto& pair : categories1) allCategories.insert(pair.first);
    for (const auto& pair : categories2) allCategories.insert(pair.first);

    for (const auto& category : allCategories) {
        double amount1 = categories1.count(category) ? categories1[category] : 0.0;
        double amount2 = categories2.count(category) ? categories2[category] : 0.0;
        comparison.categoryChanges[category] = GetPercentageChange(amount1, amount2);
    }

    return comparison;
}

double Analytics::GetPercentageChange(double oldValue, double newValue) {
    if (oldValue == 0.0) {
        return newValue == 0.0 ? 0.0 : 100.0;
    }
    return ((newValue - oldValue) / oldValue) * 100.0;
}

std::wstring Analytics::GetChangeDescription(double percentageChange) {
    if (percentageChange > 10.0) return L"significant increase";
    else if (percentageChange > 2.0) return L"moderate increase";
    else if (percentageChange > -2.0) return L"stable";
    else if (percentageChange > -10.0) return L"moderate decrease";
    else return L"significant decrease";
}

// Forecasting
std::vector<FinancialForecast> Analytics::GenerateForecast(const std::wstring& userId, int monthsAhead) {
    std::vector<FinancialForecast> forecasts;
    auto historicalData = GetMonthlyData(userId, 12); // Use 12 months of history

    if (historicalData.size() < 3) {
        // Not enough data for meaningful forecast
        return forecasts;
    }

    // Prepare historical data for trend analysis
    std::vector<double> incomeHistory, expenseHistory;
    for (auto it = historicalData.rbegin(); it != historicalData.rend(); ++it) {
        incomeHistory.push_back(it->totalIncome);
        expenseHistory.push_back(it->totalExpenses);
    }

    // Calculate trends using simple linear regression
    auto incomeCoeffs = AdvancedAnalyticsHelpers::LinearRegression(
        std::vector<double>(incomeHistory.size()), incomeHistory);
    auto expenseCoeffs = AdvancedAnalyticsHelpers::LinearRegression(
        std::vector<double>(expenseHistory.size()), expenseHistory);

    // Generate forecasts
    for (int i = 1; i <= monthsAhead; ++i) {
        FinancialForecast forecast;

        // Generate period string (simplified)
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        tm.tm_mon += i;
        if (tm.tm_mon > 11) {
            tm.tm_year++;
            tm.tm_mon -= 12;
        }

        wchar_t periodStr[32];
        swprintf_s(periodStr, L"%04d-%02d", tm.tm_year + 1900, tm.tm_mon + 1);
        forecast.period = periodStr;

        double x = static_cast<double>(incomeHistory.size() + i);
        forecast.predictedIncome = AdvancedAnalyticsHelpers::PredictValue(incomeCoeffs, x);
        forecast.predictedExpenses = AdvancedAnalyticsHelpers::PredictValue(expenseCoeffs, x);
        forecast.predictedBalance = forecast.predictedIncome - forecast.predictedExpenses;

        // Calculate confidence based on data consistency
        double incomeVolatility = CalculateStandardDeviation(incomeHistory);
        double expenseVolatility = CalculateStandardDeviation(expenseHistory);
        double avgIncome = CalculateAverage(incomeHistory);
        double avgExpenses = CalculateAverage(expenseHistory);

        double incomeCV = avgIncome > 0 ? incomeVolatility / avgIncome : 1.0;
        double expenseCV = avgExpenses > 0 ? expenseVolatility / avgExpenses : 1.0;

        // Lower coefficient of variation means higher confidence
        forecast.confidence = std::max(0.1, 1.0 - (incomeCV + expenseCV) / 2.0);
        forecast.confidence = std::min(0.95, forecast.confidence); // Cap at 95%

        forecasts.push_back(forecast);
    }

    return forecasts;
}

double Analytics::PredictNextMonthExpenses(const std::wstring& userId) {
    auto forecasts = GenerateForecast(userId, 1);
    return forecasts.empty() ? 0.0 : forecasts[0].predictedExpenses;
}

double Analytics::PredictNextMonthIncome(const std::wstring& userId) {
    auto forecasts = GenerateForecast(userId, 1);
    return forecasts.empty() ? 0.0 : forecasts[0].predictedIncome;
}

// Pattern analysis
std::vector<std::wstring> Analytics::DetectSpendingPatterns(const std::wstring& userId) {
    std::vector<std::wstring> patterns;

    // Analyze spending by day of week
    auto daySpending = GetSpendingByDayOfWeek(userId);

    // Find peak spending day
    auto maxDay = std::max_element(daySpending.begin(), daySpending.end(),
        [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
            return a.second < b.second;
        });

    if (maxDay != daySpending.end()) {
        std::wstring dayNames[] = { L"Sunday", L"Monday", L"Tuesday", L"Wednesday",
                                   L"Thursday", L"Friday", L"Saturday" };
        patterns.push_back(L"Highest spending on " + dayNames[maxDay->first]);
    }

    // Analyze category concentration
    auto categoryTotals = GetCategoryTotals(userId);
    if (!categoryTotals.empty()) {
        double totalSpending = 0;
        for (const auto& pair : categoryTotals) {
            totalSpending += pair.second;
        }

        auto maxCategory = std::max_element(categoryTotals.begin(), categoryTotals.end(),
            [](const std::pair<std::wstring, double>& a, const std::pair<std::wstring, double>& b) {
                return a.second < b.second;
            });

        double percentage = (maxCategory->second / totalSpending) * 100.0;
        if (percentage > 40.0) {
            patterns.push_back(FormatCurrency(maxCategory->second) + L" (" +
                FormatPercentage(percentage) + L") spent on " + maxCategory->first);
        }
    }

    // Analyze spending volatility
    auto monthlyData = GetMonthlyData(userId, 6);
    if (monthlyData.size() >= 3) {
        std::vector<double> monthlyExpenses;
        for (const auto& data : monthlyData) {
            monthlyExpenses.push_back(data.totalExpenses);
        }

        double stdDev = CalculateStandardDeviation(monthlyExpenses);
        double mean = CalculateAverage(monthlyExpenses);
        double cv = mean > 0 ? stdDev / mean : 0;

        if (cv > 0.3) {
            patterns.push_back(L"Highly variable spending patterns (CV: " +
                FormatPercentage(cv * 100) + L")");
        }
    }

    return patterns;
}

std::vector<std::wstring> Analytics::GetFinancialInsights(const std::wstring& userId) {
    std::vector<std::wstring> insights;

    auto monthlyData = GetMonthlyData(userId, 6);
    if (monthlyData.size() < 2) return insights;

    // Income vs Expenses trend
    double avgIncome = 0, avgExpenses = 0;
    for (const auto& data : monthlyData) {
        avgIncome += data.totalIncome;
        avgExpenses += data.totalExpenses;
    }
    avgIncome /= monthlyData.size();
    avgExpenses /= monthlyData.size();

    double savingsRate = avgIncome > 0 ? ((avgIncome - avgExpenses) / avgIncome) * 100 : 0;

    if (savingsRate > 20) {
        insights.push_back(L"Excellent savings rate of " + FormatPercentage(savingsRate) +
            L" - you're building wealth effectively!");
    }
    else if (savingsRate > 10) {
        insights.push_back(L"Good savings rate of " + FormatPercentage(savingsRate) +
            L" - consider increasing to 20% if possible.");
    }
    else if (savingsRate > 0) {
        insights.push_back(L"Low savings rate of " + FormatPercentage(savingsRate) +
            L" - look for opportunities to reduce expenses.");
    }
    else {
        insights.push_back(L"Negative savings rate - expenses exceed income. Consider budgeting and expense reduction.");
    }

    // Budget performance
    auto userBudgets = UserDataFilter::GetUserBudgets(userId);
    int overBudgetCount = 0;
    for (const auto& budget : userBudgets) {
        if (budget.isActive && budget.currentSpent > budget.monthlyLimit) {
            overBudgetCount++;
        }
    }

    if (overBudgetCount > 0) {
        insights.push_back(std::to_wstring(overBudgetCount) + L" budget(s) exceeded this month.");
    }

    return insights;
}



std::vector<std::wstring> Analytics::GetRecommendations(const std::wstring& userId) {
    std::vector<std::wstring> recommendations;

    // Analyze top spending categories
    auto topCategories = GetTopCategories(userId, 3);
    auto categoryTotals = GetCategoryTotals(userId);

    for (const auto& category : topCategories) {
        double categorySpending = categoryTotals[category];
        double growthRate = GetCategoryGrowthRate(userId, category, 6);

        if (growthRate > 20) {
            recommendations.push_back(L"Review " + category + L" spending - increased " +
                FormatPercentage(growthRate) + L" recently");
        }
    }

    // Budget recommendations
    auto userBudgets = UserDataFilter::GetUserBudgets(userId);
    for (const auto& budget : userBudgets) {
        if (budget.isActive) {
            double utilizationRate = budget.monthlyLimit > 0 ?
                (budget.currentSpent / budget.monthlyLimit) * 100 : 0;

            if (utilizationRate < 50) {
                recommendations.push_back(L"Consider reducing " + budget.category +
                    L" budget - only " + FormatPercentage(utilizationRate) +
                    L" utilized");
            }
        }
    }


	
    auto userGoals = UserDataFilter::GetUserSavingsGoals(userId);
    for (const auto& goal : userGoals) {
        if (goal.isActive) {
            double progress = GetSavingsGoalProgress(goal.name);
            int daysToDeadline = GetDaysUntilGoalDeadline(goal.name);
            if (progress < 25 && daysToDeadline < 90) {
                recommendations.push_back(L"Increase savings for '" + goal.name +
                    L"' goal - " + std::to_wstring(daysToDeadline) +
                    L" days remaining");
            }
        }
    }
    return recommendations;

}

// Statistical functions
double Analytics::CalculateStandardDeviation(const std::vector<double>& values) {
    if (values.size() < 2) return 0.0;

    double mean = CalculateAverage(values);
    double sumSquaredDiffs = 0.0;

    for (double value : values) {
        double diff = value - mean;
        sumSquaredDiffs += diff * diff;
    }

    return std::sqrt(sumSquaredDiffs / (values.size() - 1));
}

double Analytics::CalculateMedian(std::vector<double> values) {
    if (values.empty()) return 0.0;

    std::sort(values.begin(), values.end());
    size_t size = values.size();

    if (size % 2 == 0) {
        return (values[size / 2 - 1] + values[size / 2]) / 2.0;
    }
    else {
        return values[size / 2];
    }
}

double Analytics::CalculateAverage(const std::vector<double>& values) {
    if (values.empty()) return 0.0;

    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

std::pair<double, double> Analytics::GetMinMax(const std::vector<double>& values) {
    if (values.empty()) return { 0.0, 0.0 };

    auto minMax = std::minmax_element(values.begin(), values.end());
    return { *minMax.first, *minMax.second };
}

// Utility functions
std::wstring Analytics::FormatCurrency(double amount) {
    std::wstringstream ss;
    ss << L"$" << std::fixed << std::setprecision(2) << amount;
    return ss.str();
}

std::wstring Analytics::FormatPercentage(double percentage) {
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(1) << percentage << L"%";
    return ss.str();
}

/*
std::wstring Analytics::FormatTrend(double change) {
    std::wstring symbol = change > 0 ? L"↑" : (change < 0 ? L"↓" : L"→");
    return symbol + L" " + FormatPercentage(std::abs(change));
}

COLORREF Analytics::GetTrendColor(double change) {
    if (change > 5.0) return RGB(39, 174, 96);   // Green for positive
    else if (change < -5.0) return RGB(231, 76, 60); // Red for negative
    else return RGB(149, 165, 166);              // Gray for neutral
}

// Date range helpers
DateRange Analytics::GetCurrentMonth() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);

    wchar_t startDate[32], endDate[32];
    swprintf_s(startDate, L"%04d-%02d-01", tm.tm_year + 1900, tm.tm_mon + 1);

    // Last day of month
    tm.tm_mon++;
    if (tm.tm_mon > 11) {
        tm.tm_year++;
        tm.tm_mon = 0;
    }
    tm.tm_mday = 0; // This gives us the last day of previous month
    mktime(&tm);

    swprintf_s(endDate, L"%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon, tm.tm_mday);

    return DateRange(startDate, endDate);
}

DateRange Analytics::GetPreviousMonth() {
    auto current = GetCurrentMonth();
    // Simplified - would need proper date arithmetic
    return current;
}

DateRange Analytics::GetCurrentYear() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);

    wchar_t startDate[32], endDate[32];
    swprintf_s(startDate, L"%04d-01-01", tm.tm_year + 1900);
    swprintf_s(endDate, L"%04d-12-31", tm.tm_year + 1900);

    return DateRange(startDate, endDate);
}*/

// Dialog procedures (simplified implementations)
LRESULT CALLBACK Analytics::SummaryDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        SetWindowText(hDlg, L"Financial Summary");
        SetupSummaryDialog(hDlg);
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

LRESULT CALLBACK Analytics::TrendsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        SetWindowText(hDlg, L"Spending Trends");
        SetupTrendsDialog(hDlg);
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

void Analytics::SetupSummaryDialog(HWND hDlg) {
    if (!UserManager::IsUserLoggedIn()) return;

    std::wstring userId = UserManager::GetCurrentUserId();
    PopulateSummaryData(hDlg, userId);
}

void Analytics::PopulateSummaryData(HWND hDlg, const std::wstring& userId) {
    double totalIncome = GetTotalIncome(userId);
    double totalExpenses = GetTotalExpenses(userId);
    double balance = GetBalance(userId);

    SetDlgItemText(hDlg, IDC_ANALYTICS_INCOME_LABEL, FormatCurrency(totalIncome).c_str());
    SetDlgItemText(hDlg, IDC_ANALYTICS_EXPENSE_LABEL, FormatCurrency(totalExpenses).c_str());
    SetDlgItemText(hDlg, IDC_ANALYTICS_BALANCE_LABEL, FormatCurrency(balance).c_str());

   

}
    
