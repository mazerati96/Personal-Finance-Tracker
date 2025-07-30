#pragma once
#include "DataStructures.h"
#include <windows.h>
#include <vector>
#include <map>

// Dialog IDs for analytics
#define IDD_ANALYTICS_SUMMARY           700
#define IDD_ANALYTICS_TRENDS            701
#define IDD_ANALYTICS_CATEGORIES        702
#define IDD_ANALYTICS_COMPARISON        703
#define IDD_ANALYTICS_FORECAST          704

// Control IDs
#define IDC_ANALYTICS_CHART             710
#define IDC_ANALYTICS_LIST              711
#define IDC_ANALYTICS_PERIOD_COMBO      712
#define IDC_ANALYTICS_CATEGORY_LIST     713
#define IDC_ANALYTICS_DATE_FROM         714
#define IDC_ANALYTICS_DATE_TO           715
#define IDC_ANALYTICS_INCOME_LABEL      716
#define IDC_ANALYTICS_EXPENSE_LABEL     717
#define IDC_ANALYTICS_BALANCE_LABEL     718
#define IDC_ANALYTICS_EXPORT_BTN        719

// Analytics data structures
struct MonthlyFinancialData {
    std::wstring month;
    double totalIncome;
    double totalExpenses;
    double balance;
    std::map<std::wstring, double> categorySpending;
    int transactionCount;
};

struct FinancialForecast {
    std::wstring period;
    double predictedIncome;
    double predictedExpenses;
    double predictedBalance;
    double confidence; // 0-1
};

struct ComparisonData {
    std::wstring period1;
    std::wstring period2;
    double incomeChange;
    double expenseChange;
    double balanceChange;
    std::map<std::wstring, double> categoryChanges;
};

class Analytics {
public:
    // Dialog functions
    static void ShowSummaryDialog(HWND parent);
    static void ShowTrendsDialog(HWND parent);
    static void ShowCategoryAnalysisDialog(HWND parent);
    static void ShowComparisonDialog(HWND parent);
    static void ShowForecastDialog(HWND parent);

    // Financial summary calculations
    static double GetTotalIncome(const std::wstring& userId, const DateRange& range = DateRange());
    static double GetTotalExpenses(const std::wstring& userId, const DateRange& range = DateRange());
    static double GetBalance(const std::wstring& userId, const DateRange& range = DateRange());
    static double GetAverageMonthlyIncome(const std::wstring& userId, int months = 12);
    static double GetAverageMonthlyExpenses(const std::wstring& userId, int months = 12);

    
    // Trend analysis
    static std::vector<SpendingTrend> GetSpendingTrends(const std::wstring& userId, const std::wstring& period = L"monthly");
    static std::vector<MonthlyFinancialData> GetMonthlyData(const std::wstring& userId, int months = 12);
    static double GetGrowthRate(const std::vector<double>& values);
    static std::wstring GetTrendDirection(const std::vector<double>& values);

    // Category analysis
    static std::vector<CategoryAnalytics> GetCategoryAnalytics(const std::wstring& userId, const DateRange& range = DateRange());
    static std::map<std::wstring, double> GetCategoryTotals(const std::wstring& userId, const DateRange& range = DateRange());
    static std::vector<std::wstring> GetTopCategories(const std::wstring& userId, int count = 5, const DateRange& range = DateRange());
    static double GetCategoryGrowthRate(const std::wstring& userId, const std::wstring& category, int months = 6);

    // Comparison analysis
    static ComparisonData ComparePeriods(const std::wstring& userId, const DateRange& period1, const DateRange& period2);
    static double GetPercentageChange(double oldValue, double newValue);
    static std::wstring GetChangeDescription(double percentageChange);

    // Forecasting
    static std::vector<FinancialForecast> GenerateForecast(const std::wstring& userId, int monthsAhead = 6);
    static double PredictNextMonthExpenses(const std::wstring& userId);
    static double PredictNextMonthIncome(const std::wstring& userId);

    // Pattern analysis
    static std::vector<std::wstring> DetectSpendingPatterns(const std::wstring& userId);
    static std::vector<std::wstring> GetFinancialInsights(const std::wstring& userId);
    static std::vector<std::wstring> GetRecommendations(const std::wstring& userId);

    // Time-based analysis
    static std::map<int, double> GetSpendingByDayOfWeek(const std::wstring& userId, const DateRange& range = DateRange());
    static std::map<int, double> GetSpendingByHour(const std::wstring& userId, const DateRange& range = DateRange());
    static std::map<std::wstring, double> GetSeasonalSpending(const std::wstring& userId);

    // Statistical functions
    static double CalculateStandardDeviation(const std::vector<double>& values);
    static double CalculateMedian(std::vector<double> values);
    static double CalculateAverage(const std::vector<double>& values);
    static std::pair<double, double> GetMinMax(const std::vector<double>& values);

    // Export functions
    static bool ExportAnalyticsToCSV(const std::wstring& filePath, const std::wstring& userId, const std::wstring& analysisType);
    static bool ExportAnalyticsToPDF(const std::wstring& filePath, const std::wstring& userId, const std::wstring& analysisType);

public:
    // Dialog procedures
    static LRESULT CALLBACK SummaryDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK TrendsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK CategoryAnalysisDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ComparisonDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ForecastDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // Helper functions for dialog setup
    static void SetupSummaryDialog(HWND hDlg);
    static void SetupTrendsDialog(HWND hDlg);
    static void SetupCategoryAnalysisDialog(HWND hDlg);
    static void SetupComparisonDialog(HWND hDlg);
    static void SetupForecastDialog(HWND hDlg);

    // Data population functions
    static void PopulateSummaryData(HWND hDlg, const std::wstring& userId);
    static void PopulateTrendsData(HWND hDlg, const std::wstring& userId, const std::wstring& period);
    static void PopulateCategoryData(HWND hDlg, const std::wstring& userId, const DateRange& range);
    static void PopulateComparisonData(HWND hDlg, const ComparisonData& data);
    static void PopulateForecastData(HWND hDlg, const std::vector<FinancialForecast>& forecasts);

    // Utility functions
    static std::wstring FormatCurrency(double amount);
    static std::wstring FormatPercentage(double percentage);
    /*static std::wstring FormatTrend(double change);
    static COLORREF GetTrendColor(double change);
    static std::wstring GetMonthName(int month);
    static int GetMonthFromDate(const std::wstring& date);
    static int GetYearFromDate(const std::wstring& date);

    // Date range helpers
    static DateRange GetCurrentMonth();
    static DateRange GetPreviousMonth();
    static DateRange GetCurrentYear();
    static DateRange GetPreviousYear();
    static DateRange GetLastNMonths(int months);
    */
};

// Advanced analytics for business intelligence
class AdvancedAnalytics {
public:
    // Cash flow analysis
    static std::vector<double> GetCashFlowHistory(const std::wstring& userId, int months = 12);
    static double GetCashFlowVolatility(const std::wstring& userId, int months = 12);
    static std::wstring GetCashFlowHealthStatus(const std::wstring& userId);

    // Expense analysis
    static std::map<std::wstring, double> GetExpenseDistribution(const std::wstring& userId, const DateRange& range = DateRange());
    static std::vector<std::wstring> GetExpenseAnomalies(const std::wstring& userId, double threshold = 2.0);
    static double GetExpenseVolatility(const std::wstring& userId, const std::wstring& category);

    // Income analysis
    static std::vector<std::wstring> GetIncomeSourceAnalysis(const std::wstring& userId);
    static double GetIncomeStability(const std::wstring& userId);
    static std::vector<std::wstring> GetIncomeDiversificationSuggestions(const std::wstring& userId);

    // Budget analysis
    static std::map<std::wstring, double> GetBudgetPerformance(const std::wstring& userId);
    static std::vector<std::wstring> GetBudgetOptimizationSuggestions(const std::wstring& userId);
    static double GetBudgetAccuracy(const std::wstring& userId, int months = 6);

    // Savings analysis
    static double GetSavingsRate(const std::wstring& userId, const DateRange& range = DateRange());
    static std::vector<std::wstring> GetSavingsOpportunities(const std::wstring& userId);
    static double GetSavingsGoalFeasibility(const SavingsGoal& goal, const std::wstring& userId);

    // Risk analysis
    static double GetFinancialRiskScore(const std::wstring& userId);
    static std::vector<std::wstring> GetRiskFactors(const std::wstring& userId);
    static std::vector<std::wstring> GetRiskMitigationSuggestions(const std::wstring& userId);

    // Behavioral analysis
    static std::map<std::wstring, int> GetSpendingHabits(const std::wstring& userId);
    static std::vector<std::wstring> GetBehavioralInsights(const std::wstring& userId);
    static double GetImpulseSpendingScore(const std::wstring& userId);

public:
    // Statistical analysis helpers
    static double CalculateCoefficiientOfVariation(const std::vector<double>& values);
    static double CalculateZScore(double value, const std::vector<double>& dataset);
    static std::vector<double> CalculateMovingAverage(const std::vector<double>& values, int window);
    static double CalculateCorrelation(const std::vector<double>& x, const std::vector<double>& y);

    // Machine learning helpers (simplified)
    static std::vector<double> LinearRegression(const std::vector<double>& x, const std::vector<double>& y);
    static double PredictValue(const std::vector<double>& coefficients, double x);
    static std::vector<double> DetectOutliers(const std::vector<double>& values, double threshold = 2.0);

    
};