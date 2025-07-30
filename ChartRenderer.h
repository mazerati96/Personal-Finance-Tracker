#pragma once

#define NOMINMAX
#include <windows.h>
#include <objidl.h>     // Needed for IStream
#include <gdiplus.h>
#include <vector>
#include <map>
#include <string>
#include "DataStructures.h"
#include "resource.h"  // For resource IDs
#include <algorithm>

using namespace Gdiplus;
// Add these to your resource.h or constants header file

#ifndef CHART_RENDERER_H
#define CHART_RENDERER_H

// Dialog resource ID
#define IDD_CHARTS_DIALOG           2001

// Control IDs for chart dialog




// Color constants - Add these to your main constants file
#define COLOR_PRIMARY              RGB(52, 152, 219)   // Blue
#define COLOR_SUCCESS              RGB(39, 174, 96)    // Green  
#define COLOR_DANGER               RGB(231, 76, 60)    // Red
#define COLOR_WARNING              RGB(241, 196, 15)   // Yellow
#define COLOR_INFO                 RGB(26, 188, 156)   // Turquoise
#define COLOR_LIGHT                RGB(236, 240, 241)  // Light Gray
#define COLOR_DARK                 RGB(52, 73, 94)     // Dark Blue Gray

#endif // CHART_CONSTANTS_H
// Chart types
enum class ChartType {
    LINE,
    BAR,
    PIE,
    AREA,
    SCATTER,
    HISTOGRAM
};

// Chart data structures
struct ChartDataPoint {
    std::wstring label;
    double value;
    COLORREF color;

    ChartDataPoint() : value(0.0), color(RGB(52, 152, 219)) {}
    ChartDataPoint(const std::wstring& l, double v, COLORREF c = RGB(52, 152, 219))
        : label(l), value(v), color(c) {
    }
};

struct ChartSeries {
    std::wstring name;
    std::vector<ChartDataPoint> data;
    COLORREF color;
    bool visible;

    ChartSeries() : color(RGB(52, 152, 219)), visible(true) {}
    ChartSeries(const std::wstring& n, COLORREF c = RGB(52, 152, 219))
        : name(n), color(c), visible(true) {
    }
};

struct ChartOptions {
    std::wstring title;
    std::wstring xAxisLabel;
    std::wstring yAxisLabel;
    bool showLegend;
    bool showGrid;
    bool showAxes;
    bool showDataLabels;
    bool animate;
    int animationDuration; // milliseconds
    COLORREF backgroundColor;
    COLORREF gridColor;
    COLORREF textColor;
    Font* titleFont;
    Font* labelFont;

    ChartOptions() :
        showLegend(true), showGrid(true), showAxes(true), showDataLabels(false),
        animate(false), animationDuration(1000),
        backgroundColor(Color::White), gridColor(Color::LightGray), textColor(Color::Black),
        titleFont(nullptr), labelFont(nullptr) {
    }
};

class ChartRenderer {
public:
    // Main chart dialog
    static void ShowChartsDialog(HWND parent);

    // Core rendering functions
    static void RenderChart(HDC hdc, RECT rect, ChartType type, const std::vector<ChartSeries>& series, const ChartOptions& options);
    static void RenderLineChart(Graphics* graphics, RECT rect, const std::vector<ChartSeries>& series, const ChartOptions& options);
    static void RenderBarChart(Graphics* graphics, RECT rect, const std::vector<ChartSeries>& series, const ChartOptions& options);
    static void RenderPieChart(Graphics* graphics, RECT rect, const std::vector<ChartSeries>& series, const ChartOptions& options);
    
    static void RenderAreaChart(Graphics* graphics, RECT rect, const std::vector<ChartSeries>& series, const ChartOptions& options);
    static void RenderScatterChart(Graphics* graphics, RECT rect, const std::vector<ChartSeries>& series, const ChartOptions& options);
    static void RenderHistogram(Graphics* graphics, RECT rect, const std::vector<ChartSeries>& series, const ChartOptions& options);
    static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

    // Specialized charts for financial data
    static void RenderCashFlowChart(HDC hdc, RECT rect, const std::wstring& userId, int months = 12);
    static void RenderCategoryPieChart(HDC hdc, RECT rect, const std::wstring& userId, const DateRange& range = DateRange());
    static void RenderBudgetProgressChart(HDC hdc, RECT rect, const std::wstring& userId);
    static void RenderSavingsGoalChart(HDC hdc, RECT rect, const std::wstring& userId);
    static void RenderIncomeVsExpensesChart(HDC hdc, RECT rect, const std::wstring& userId, int months = 12);
    static void RenderMonthlyTrendChart(HDC hdc, RECT rect, const std::wstring& userId, int months = 12);

    // Data conversion helpers
    static std::vector<ChartSeries> ConvertSpendingTrends(const std::vector<SpendingTrend>& trends);
    static std::vector<ChartSeries> ConvertCategoryAnalytics(const std::vector<CategoryAnalytics>& analytics);
    static std::vector<ChartSeries> ConvertBudgetData(const std::vector<Budget>& budgets);
    static std::vector<ChartSeries> ConvertGoalData(const std::vector<SavingsGoal>& goals);

    // Export functions
    static bool ExportChartToPNG(const std::wstring& filePath, ChartType type, const std::vector<ChartSeries>& series, const ChartOptions& options, int width = 800, int height = 600);
    static bool ExportChartToSVG(const std::wstring& filePath, ChartType type, const std::vector<ChartSeries>& series, const ChartOptions& options, int width = 800, int height = 600);

    // Color schemes
    static std::vector<COLORREF> GetDefaultColorPalette();
    static std::vector<COLORREF> GetProfessionalColorPalette();
    static std::vector<COLORREF> GetVibrantColorPalette();
    static std::vector<COLORREF> GetMonochromeColorPalette();
    static COLORREF GetCategoryColor(const std::wstring& category);

private:
    // Dialog procedure
    static LRESULT CALLBACK ChartsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // Chart component rendering
    static void RenderTitle(Graphics* graphics, RECT rect, const std::wstring& title, Font* font, Brush* brush);
    static void RenderLegend(Graphics* graphics, RECT rect, const std::vector<ChartSeries>& series, Font* font, Brush* textBrush);
    static void RenderAxes(Graphics* graphics, RECT rect, const ChartOptions& options, Font* font, Brush* textBrush, Pen* axisPen);
    static void RenderGrid(Graphics* graphics, RECT rect, const ChartOptions& options, Pen* gridPen);
    static void RenderDataLabels(Graphics* graphics, const std::vector<PointF>& points, const std::vector<std::wstring>& labels, Font* font, Brush* textBrush);

    // Calculation helpers
    static RECT CalculateChartArea(RECT totalRect, const ChartOptions& options);
    static std::pair<double, double> CalculateDataRange(const std::vector<ChartSeries>& series);
    static std::vector<PointF> CalculateDataPoints(RECT chartArea, const ChartSeries& series, double minValue, double maxValue);
    static RectF CalculateBarRect(RECT chartArea, int index, int totalBars, double value, double maxValue);

    // Color and styling helpers
    static Brush* CreateBrushFromColor(COLORREF color);
    static Pen* CreatePenFromColor(COLORREF color, float width = 1.0f);
    static Font* CreateFont(const std::wstring& family, float size, FontStyle style = FontStyleRegular);
    static StringFormat* CreateStringFormat(StringAlignment horizontal = StringAlignmentCenter, StringAlignment vertical = StringAlignmentCenter);

    // Animation system
    struct ChartAnimation {
        ChartType type;
        HWND hwnd;
        RECT rect;
        std::vector<ChartSeries> series;
        ChartOptions options;
        DWORD startTime;
        int duration;
        bool active;
    };

    static std::vector<ChartAnimation> activeAnimations;
    static void StartChartAnimation(HWND hwnd, RECT rect, ChartType type, const std::vector<ChartSeries>& series, const ChartOptions& options);
    static void UpdateAnimations();
    static void StopAllAnimations();

    // Interaction handling
    static void HandleChartClick(HWND hwnd, POINT pt, ChartType type, const std::vector<ChartSeries>& series, RECT chartRect);
    static void HandleChartHover(HWND hwnd, POINT pt, ChartType type, const std::vector<ChartSeries>& series, RECT chartRect);
    static void ShowTooltip(HWND hwnd, POINT pt, const std::wstring& text);
    static void HideTooltip();

    // Utility functions
    static std::wstring FormatValue(double value, const std::wstring& format = L"");
    static std::wstring FormatCurrency(double value);
    static std::wstring FormatPercentage(double value);
    static double InterpolateValue(double start, double end, double progress);
    static COLORREF InterpolateColor(COLORREF start, COLORREF end, double progress);

    // Static members
    static HWND hChartsDialog;
    static HWND hTooltip;
    static std::map<std::wstring, COLORREF> categoryColors;
};

// Chart builder for easy chart creation
class ChartBuilder {
public:
    ChartBuilder(ChartType type) : chartType(type) {}

    ChartBuilder& SetTitle(const std::wstring& title);
    ChartBuilder& SetXAxisLabel(const std::wstring& label);
    ChartBuilder& SetYAxisLabel(const std::wstring& label);
    ChartBuilder& ShowLegend(bool show = true);
    ChartBuilder& ShowGrid(bool show = true);
    ChartBuilder& ShowDataLabels(bool show = true);
    ChartBuilder& SetBackgroundColor(COLORREF color);
    ChartBuilder& AddSeries(const ChartSeries& series);
    ChartBuilder& AddDataPoint(const std::wstring& seriesName, const std::wstring& label, double value, COLORREF color = RGB(52, 152, 219));

    // Build methods
    void RenderTo(HDC hdc, RECT rect);
    bool ExportToPNG(const std::wstring& filePath, int width = 800, int height = 600);

    // Quick builders for common financial charts
    static ChartBuilder CreateExpenseCategoryChart(const std::wstring& userId, const DateRange& range = DateRange());
    static ChartBuilder CreateMonthlyTrendChart(const std::wstring& userId, int months = 12);
    static ChartBuilder CreateBudgetComparisonChart(const std::wstring& userId);
    static ChartBuilder CreateIncomeSourceChart(const std::wstring& userId, const DateRange& range = DateRange());
    static ChartBuilder CreateSavingsProgressChart(const std::wstring& userId);

private:
    ChartType chartType;
    ChartOptions options;
    std::vector<ChartSeries> series;
    std::map<std::wstring, int> seriesMap; // series name -> index
};

// Real-time chart updates
class LiveChartManager {
public:
    static void StartLiveUpdates(HWND hwnd, ChartType type, const std::wstring& userId, int updateInterval = 5000);
    static void StopLiveUpdates(HWND hwnd);
    static void UpdateChart(HWND hwnd);

private:
    struct LiveChart {
        HWND hwnd;
        ChartType type;
        std::wstring userId;
        int updateInterval;
        DWORD lastUpdate;
        bool active;
    };

    static std::vector<LiveChart> liveCharts;
    static void ProcessLiveUpdates();
};

// Chart templates
class ChartTemplates {
public:
    static ChartOptions GetDefaultOptions();
    static ChartOptions GetMinimalOptions();
    static ChartOptions GetProfessionalOptions();
    static ChartOptions GetPresentationOptions();

    // Pre-configured chart templates
    static void RenderDashboardSummaryChart(HDC hdc, RECT rect, const std::wstring& userId);
    static void RenderQuickStatsChart(HDC hdc, RECT rect, const std::wstring& userId);
    static void RenderWeeklySpendingChart(HDC hdc, RECT rect, const std::wstring& userId);
    static void RenderTopCategoriesChart(HDC hdc, RECT rect, const std::wstring& userId);

private:
    static Font* GetDefaultTitleFont();
    static Font* GetDefaultLabelFont();
    static Brush* GetDefaultTextBrush();
};