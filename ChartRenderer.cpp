#define NOMINMAX  // Prevent Windows from defining min/max macros
#include "ChartRenderer.h"
#include "UserManager.h"
#include "Analytics.h"
#include <windows.h>
#include <commdlg.h>    // For OPENFILENAME, GetSaveFileName, OFN_* constants
#include <gdiplus.h>    // For GDI+ graphics functions
#include <objbase.h>    // For IStream
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <limits>       // For std::numeric_limits

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
void ChartRenderer::RenderHistogram(Gdiplus::Graphics* g, RECT r, const std::vector<ChartSeries>& series, const ChartOptions& opts) {
    // TODO: implement
}

void ChartRenderer::RenderScatterChart(Gdiplus::Graphics* g, RECT r, const std::vector<ChartSeries>& series, const ChartOptions& opts) {
    // TODO: implement
}

void ChartRenderer::RenderAreaChart(Gdiplus::Graphics* g, RECT r, const std::vector<ChartSeries>& series, const ChartOptions& opts) {
    // TODO: implement
}

void ChartRenderer::RenderPieChart(Gdiplus::Graphics* g, RECT r, const std::vector<ChartSeries>& series, const ChartOptions& opts) {
    // TODO: implement
}

void ChartRenderer::RenderBarChart(Gdiplus::Graphics* g, RECT r, const std::vector<ChartSeries>& series, const ChartOptions& opts) {
    // TODO: implement
}

// Static member initialization
HWND ChartRenderer::hChartsDialog = NULL;
HWND ChartRenderer::hTooltip = NULL;
std::map<std::wstring, COLORREF> ChartRenderer::categoryColors;
std::vector<ChartRenderer::ChartAnimation> ChartRenderer::activeAnimations;

// Main chart dialog
void ChartRenderer::ShowChartsDialog(HWND parent) {
    if (hChartsDialog) {
        SetForegroundWindow(hChartsDialog);
        return;
    }

    hChartsDialog = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CHARTS_DIALOG), parent, ChartsDialogProc);
    if (hChartsDialog) {
        ShowWindow(hChartsDialog, SW_SHOW);
    }
}

// Core rendering functions
void ChartRenderer::RenderChart(HDC hdc, RECT rect, ChartType type, const std::vector<ChartSeries>& series, const ChartOptions& options) {
    Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    switch (type) {
    case ChartType::LINE:
        RenderLineChart(&graphics, rect, series, options);
        break;
    case ChartType::BAR:
        RenderBarChart(&graphics, rect, series, options);
        break;
    case ChartType::PIE:
        RenderPieChart(&graphics, rect, series, options);
        break;
    case ChartType::AREA:
        RenderAreaChart(&graphics, rect, series, options);
        break;
    case ChartType::SCATTER:
        RenderScatterChart(&graphics, rect, series, options);
        break;
    case ChartType::HISTOGRAM:
        RenderHistogram(&graphics, rect, series, options);
        break;
    }
}

void ChartRenderer::RenderLineChart(Graphics* graphics, RECT rect, const std::vector<ChartSeries>& series, const ChartOptions& options) {
    if (series.empty()) return;

    RECT chartArea = CalculateChartArea(rect, options);
    auto dataRange = CalculateDataRange(series);

    // Draw background
    SolidBrush bgBrush(options.backgroundColor);
    graphics->FillRectangle(&bgBrush, (INT)rect.left, (INT)rect.top, (INT)(rect.right - rect.left), (INT)(rect.bottom - rect.top));

    // Draw grid
    if (options.showGrid) {
        Pen gridPen(options.gridColor, 1.0f);

        // Vertical grid lines
        int gridLines = 10;
        for (int i = 0; i <= gridLines; ++i) {
            INT x = static_cast<INT>(chartArea.left + (chartArea.right - chartArea.left) * i / gridLines);
            graphics->DrawLine(&gridPen, x, (INT)chartArea.top, x, (INT)chartArea.bottom);
        }

        // Horizontal grid lines
        for (int i = 0; i <= gridLines; ++i) {
            INT y = static_cast<INT>(chartArea.top + (chartArea.bottom - chartArea.top) * i / gridLines);
            graphics->DrawLine(&gridPen, (INT)chartArea.left, y, (INT)chartArea.right, y);
        }
    }

    // Draw axes
    if (options.showAxes) {
        Pen axisPen(options.textColor, 2.0f);
        graphics->DrawLine(&axisPen, (INT)chartArea.left, (INT)chartArea.bottom, (INT)chartArea.right, (INT)chartArea.bottom); // X-axis
        graphics->DrawLine(&axisPen, (INT)chartArea.left, (INT)chartArea.top, (INT)chartArea.left, (INT)chartArea.bottom);    // Y-axis
    }

    // Draw series
    for (const auto& serie : series) {
        if (!serie.visible || serie.data.empty()) continue;

        auto points = CalculateDataPoints(chartArea, serie, dataRange.first, dataRange.second);

        if (points.size() > 1) {
            Pen linePen(Color(255, GetRValue(serie.color), GetGValue(serie.color), GetBValue(serie.color)), 2.0f);

            // Convert to GDI+ points
            std::vector<PointF> gdipPoints;
            for (const auto& pt : points) {
                gdipPoints.push_back(pt);
            }

            graphics->DrawLines(&linePen, gdipPoints.data(), static_cast<INT>(gdipPoints.size()));
        }

        // Draw data points
        SolidBrush pointBrush(Color(255, GetRValue(serie.color), GetGValue(serie.color), GetBValue(serie.color)));
        for (const auto& pt : points) {
            graphics->FillEllipse(&pointBrush, pt.X - 3.0f, pt.Y - 3.0f, 6.0f, 6.0f);
        }
    }

    // Draw title
    if (!options.title.empty()) {
        Font titleFont(L"Arial", 16, FontStyleBold);
        SolidBrush textBrush(options.textColor);
        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);

        RectF titleRect(static_cast<REAL>(rect.left), static_cast<REAL>(rect.top),
            static_cast<REAL>(rect.right - rect.left), 30.0f);

        graphics->DrawString(options.title.c_str(), -1, &titleFont, titleRect, &format, &textBrush);
    }
}

// Specialized charts for financial data
void ChartRenderer::RenderCashFlowChart(HDC hdc, RECT rect, const std::wstring& userId, int months) {
    auto monthlyData = Analytics::GetMonthlyData(userId, months);
    if (monthlyData.empty()) return;

    std::vector<ChartSeries> series;

    // Income series
    ChartSeries incomeSeries(L"Income", COLOR_SUCCESS);
    // Expense series
    ChartSeries expenseSeries(L"Expenses", COLOR_DANGER);
    // Balance series
    ChartSeries balanceSeries(L"Balance", COLOR_PRIMARY);

    for (auto it = monthlyData.rbegin(); it != monthlyData.rend(); ++it) {
        incomeSeries.data.emplace_back(it->month, it->totalIncome);
        expenseSeries.data.emplace_back(it->month, it->totalExpenses);
        balanceSeries.data.emplace_back(it->month, it->balance);
    }

    series.push_back(incomeSeries);
    series.push_back(expenseSeries);
    series.push_back(balanceSeries);

    ChartOptions options;
    options.title = L"Cash Flow Trend";
    options.showLegend = true;
    options.showGrid = true;

    RenderChart(hdc, rect, ChartType::LINE, series, options);
}

void ChartRenderer::RenderCategoryPieChart(HDC hdc, RECT rect, const std::wstring& userId, const DateRange& range) {
    auto categoryTotals = Analytics::GetCategoryTotals(userId, range);
    if (categoryTotals.empty()) return;

    std::vector<ChartSeries> series;
    ChartSeries categorySeries(L"Categories", COLOR_PRIMARY);

    for (const auto& pair : categoryTotals) {
        COLORREF categoryColor = GetCategoryColor(pair.first);
        categorySeries.data.emplace_back(pair.first, pair.second, categoryColor);
    }

    series.push_back(categorySeries);

    ChartOptions options;
    options.title = L"Spending by Category";
    options.showLegend = true;
    options.showDataLabels = true;

    RenderChart(hdc, rect, ChartType::PIE, series, options);
}

void ChartRenderer::RenderBudgetProgressChart(HDC hdc, RECT rect, const std::wstring& userId) {
    auto userBudgets = UserDataFilter::GetUserBudgets(userId);
    if (userBudgets.empty()) return;

    std::vector<ChartSeries> series;
    ChartSeries budgetSeries(L"Budget Progress", COLOR_PRIMARY);

    for (const auto& budget : userBudgets) {
        if (!budget.isActive) continue;

        double percentage = budget.monthlyLimit > 0 ? (budget.currentSpent / budget.monthlyLimit) * 100 : 0;
        COLORREF barColor = COLOR_SUCCESS;

        if (percentage > 100) barColor = COLOR_DANGER;
        else if (percentage > budget.warningThreshold * 100) barColor = COLOR_WARNING;

        budgetSeries.data.emplace_back(budget.category, percentage, barColor);
    }

    series.push_back(budgetSeries);

    ChartOptions options;
    options.title = L"Budget Progress (%)";
    options.showDataLabels = true;

    RenderChart(hdc, rect, ChartType::BAR, series, options);
}

void ChartRenderer::RenderMonthlyTrendChart(HDC hdc, RECT rect, const std::wstring& userId, int months) {
    RenderCashFlowChart(hdc, rect, userId, months); // Reuse cash flow chart
}

// Data conversion helpers
std::vector<ChartSeries> ChartRenderer::ConvertSpendingTrends(const std::vector<SpendingTrend>& trends) {
    std::vector<ChartSeries> series;

    ChartSeries incomeSeries(L"Income", COLOR_SUCCESS);
    ChartSeries expenseSeries(L"Expenses", COLOR_DANGER);
    ChartSeries balanceSeries(L"Balance", COLOR_PRIMARY);

    for (const auto& trend : trends) {
        incomeSeries.data.emplace_back(trend.period, trend.totalIncome);
        expenseSeries.data.emplace_back(trend.period, trend.totalExpenses);
        balanceSeries.data.emplace_back(trend.period, trend.balance);
    }

    series.push_back(incomeSeries);
    series.push_back(expenseSeries);
    series.push_back(balanceSeries);

    return series;
}

std::vector<ChartSeries> ChartRenderer::ConvertCategoryAnalytics(const std::vector<CategoryAnalytics>& analytics) {
    std::vector<ChartSeries> series;
    ChartSeries categorySeries(L"Category Spending", COLOR_PRIMARY);

    for (const auto& category : analytics) {
        COLORREF categoryColor = GetCategoryColor(category.category);
        categorySeries.data.emplace_back(category.category, category.totalSpent, categoryColor);
    }

    series.push_back(categorySeries);
    return series;
}

// Color schemes
std::vector<COLORREF> ChartRenderer::GetDefaultColorPalette() {
    return {
        RGB(52, 152, 219),   // Blue
        RGB(231, 76, 60),    // Red
        RGB(39, 174, 96),    // Green
        RGB(241, 196, 15),   // Yellow
        RGB(155, 89, 182),   // Purple
        RGB(230, 126, 34),   // Orange
        RGB(26, 188, 156),   // Turquoise
        RGB(149, 165, 166),  // Gray
        RGB(192, 57, 43),    // Dark Red
        RGB(142, 68, 173)    // Dark Purple
    };
}

std::vector<COLORREF> ChartRenderer::GetProfessionalColorPalette() {
    return {
        RGB(54, 69, 79),     // Dark Blue Gray
        RGB(69, 90, 100),    // Blue Gray
        RGB(84, 110, 122),   // Light Blue Gray
        RGB(149, 165, 166),  // Gray
        RGB(189, 195, 199),  // Light Gray
        RGB(236, 240, 241),  // Very Light Gray
        RGB(52, 73, 94),     // Dark Gray Blue
        RGB(44, 62, 80)      // Very Dark Blue
    };
}

COLORREF ChartRenderer::GetCategoryColor(const std::wstring& category) {
    // Initialize category colors if empty
    if (categoryColors.empty()) {
        auto colors = GetDefaultColorPalette();
        // Note: categories variable needs to be defined or passed as parameter
        // For now, we'll handle individual categories as they come
    }

    auto it = categoryColors.find(category);
    if (it != categoryColors.end()) {
        return it->second;
    }

    // Assign a new color
    auto colors = GetDefaultColorPalette();
    COLORREF newColor = colors[categoryColors.size() % colors.size()];
    categoryColors[category] = newColor;
    return newColor;
}

// Export functions
bool ChartRenderer::ExportChartToPNG(const std::wstring& filePath, ChartType type,
    const std::vector<ChartSeries>& series, const ChartOptions& options,
    int width, int height) {
    try {
        // Create bitmap
        Bitmap bitmap(width, height, PixelFormat32bppARGB);
        Graphics graphics(&bitmap);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);

        RECT rect = { 0, 0, width, height };

        // Get HDC from Graphics (simplified approach)
        HDC hdc = graphics.GetHDC();
        RenderChart(hdc, rect, type, series, options);
        graphics.ReleaseHDC(hdc);

        // Alternative save method using IStream
        // Create a file stream
        HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        IStream* pStream = nullptr;
        if (SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &pStream))) {
            // Get PNG encoder CLSID
            CLSID pngClsid;
            if (GetEncoderClsid(L"image/png", &pngClsid) >= 0) {
                // Save to stream first
                Status status = bitmap.Save(pStream, &pngClsid);

                if (status == Ok) {
                    // Get stream size
                    STATSTG statstg;
                    pStream->Stat(&statstg, STATFLAG_NONAME);
                    ULONG streamSize = statstg.cbSize.LowPart;

                    // Reset stream position
                    LARGE_INTEGER li = { 0 };
                    pStream->Seek(li, STREAM_SEEK_SET, NULL);

                    // Copy stream to file
                    BYTE* buffer = new BYTE[streamSize];
                    ULONG bytesRead;
                    pStream->Read(buffer, streamSize, &bytesRead);

                    DWORD bytesWritten;
                    WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL);

                    delete[] buffer;
                    pStream->Release();
                    CloseHandle(hFile);

                    return (bytesWritten == bytesRead);
                }
            }
            pStream->Release();
        }
        CloseHandle(hFile);
        return false;
    }
    catch (...) {
        return false;
    }
}

// Helper function for GetEncoderClsid
int ChartRenderer::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;          // number of image encoders
    UINT size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

// Dialog procedure
LRESULT CALLBACK ChartRenderer::ChartsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        SetWindowText(hDlg, L"Financial Charts");

        // Create chart type combo
        HWND hChartTypeCombo = CreateWindow(L"COMBOBOX", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,
            20, 20, 150, 100, hDlg, (HMENU)IDC_CHART_TYPE_COMBO, GetModuleHandle(NULL), NULL);

        SendMessage(hChartTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Cash Flow");
        SendMessage(hChartTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Category Pie");
        SendMessage(hChartTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Budget Progress");
        SendMessage(hChartTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Monthly Trends");
        SendMessage(hChartTypeCombo, CB_SETCURSEL, 0, 0);

        // Create chart display area
        CreateWindow(L"STATIC", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_OWNERDRAW,
            20, 60, 560, 400, hDlg, (HMENU)IDC_CHART_DISPLAY, GetModuleHandle(NULL), NULL);

        // Create buttons
        CreateWindow(L"BUTTON", L"Refresh", WS_VISIBLE | WS_CHILD,
            200, 20, 80, 25, hDlg, (HMENU)IDC_REFRESH_CHART, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"Export PNG", WS_VISIBLE | WS_CHILD,
            300, 20, 80, 25, hDlg, (HMENU)IDC_EXPORT_PNG, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"Close", WS_VISIBLE | WS_CHILD,
            500, 20, 80, 25, hDlg, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_REFRESH_CHART:
        {
            HWND hChartDisplay = GetDlgItem(hDlg, IDC_CHART_DISPLAY);
            InvalidateRect(hChartDisplay, NULL, TRUE);
            return TRUE;
        }

        case IDC_EXPORT_PNG:
        {
            if (!UserManager::IsUserLoggedIn()) {
                MessageBox(hDlg, L"Please log in first.", L"Error", MB_OK);
                return TRUE;
            }

            wchar_t fileName[MAX_PATH] = L"chart.png";
            OPENFILENAME ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hDlg;
            ofn.lpstrFile = fileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"PNG Files\0*.png\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrDefExt = L"png";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetSaveFileName(&ofn)) {
                // Get selected chart type and export
                int chartType = (int)SendDlgItemMessage(hDlg, IDC_CHART_TYPE_COMBO, CB_GETCURSEL, 0, 0);

                std::vector<ChartSeries> series;
                ChartOptions options;
                ChartType type = ChartType::LINE;

                std::wstring userId = UserManager::GetCurrentUserId();

                switch (chartType) {
                case 0: // Cash Flow
                    type = ChartType::LINE;
                    options.title = L"Cash Flow Chart";
                    break;
                case 1: // Category Pie
                    type = ChartType::PIE;
                    options.title = L"Category Distribution";
                    break;
                case 2: // Budget Progress
                    type = ChartType::BAR;
                    options.title = L"Budget Progress";
                    break;
                }

                if (ExportChartToPNG(fileName, type, series, options)) {
                    MessageBox(hDlg, L"Chart exported successfully!", L"Export", MB_OK);
                }
                else {
                    MessageBox(hDlg, L"Failed to export chart.", L"Error", MB_OK);
                }
            }
            return TRUE;
        }

        case IDC_CHART_TYPE_COMBO:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                HWND hChartDisplay = GetDlgItem(hDlg, IDC_CHART_DISPLAY);
                InvalidateRect(hChartDisplay, NULL, TRUE);
            }
            return TRUE;

        case IDCANCEL:
            DestroyWindow(hDlg);
            hChartsDialog = NULL;
            return TRUE;
        }
        break;

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
        if (pdis->CtlID == IDC_CHART_DISPLAY) {
            if (!UserManager::IsUserLoggedIn()) {
                // Draw "Please log in" message
                SetBkMode(pdis->hDC, TRANSPARENT);
                SetTextColor(pdis->hDC, RGB(100, 100, 100));

                std::wstring message = L"Please log in to view charts";
                DrawText(pdis->hDC, message.c_str(), -1, &pdis->rcItem,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }

            // Get selected chart type
            int chartType = (int)SendDlgItemMessage(hDlg, IDC_CHART_TYPE_COMBO, CB_GETCURSEL, 0, 0);
            std::wstring userId = UserManager::GetCurrentUserId();

            // Render appropriate chart
            switch (chartType) {
            case 0: // Cash Flow
                RenderCashFlowChart(pdis->hDC, pdis->rcItem, userId, 12);
                break;
            case 1: // Category Pie
            {
                DateRange range; // You'll need to define this properly
                RenderCategoryPieChart(pdis->hDC, pdis->rcItem, userId, range);
            }
            break;
            case 2: // Budget Progress
                RenderBudgetProgressChart(pdis->hDC, pdis->rcItem, userId);
                break;
            case 3: // Monthly Trends
                RenderMonthlyTrendChart(pdis->hDC, pdis->rcItem, userId, 12);
                break;
            }
        }
        return TRUE;
    }

    case WM_DESTROY:
        hChartsDialog = NULL;
        return TRUE;
    }

    return FALSE;
}

// Helper functions
RECT ChartRenderer::CalculateChartArea(RECT totalRect, const ChartOptions& options) {
    RECT chartArea = totalRect;

    // Reserve space for title
    if (!options.title.empty()) {
        chartArea.top += 40;
    }

    // Reserve space for legend
    if (options.showLegend) {
        chartArea.right -= 150;
    }

    // Reserve margins
    chartArea.left += 60;   // Left margin for Y-axis labels
    chartArea.right -= 20;  // Right margin
    chartArea.top += 20;    // Top margin
    chartArea.bottom -= 60; // Bottom margin for X-axis labels

    return chartArea;
}

std::pair<double, double> ChartRenderer::CalculateDataRange(const std::vector<ChartSeries>& series) {
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();

    for (const auto& serie : series) {
        for (const auto& point : serie.data) {
            minValue = (std::min)(minValue, point.value);
            maxValue = (std::max)(maxValue, point.value);
        }
    }

    // Add some padding
    double range = maxValue - minValue;
    double padding = range * 0.1;

    return { minValue - padding, maxValue + padding };
}

std::vector<PointF> ChartRenderer::CalculateDataPoints(RECT chartArea, const ChartSeries& series, double minValue, double maxValue) {
    std::vector<PointF> points;

    if (series.data.empty()) return points;

    double valueRange = maxValue - minValue;
    if (valueRange == 0) valueRange = 1;

    for (size_t i = 0; i < series.data.size(); ++i) {
        double normalizedX = static_cast<double>(i) / (series.data.size() - 1);
        double normalizedY = (series.data[i].value - minValue) / valueRange;

        float x = chartArea.left + static_cast<float>(normalizedX * (chartArea.right - chartArea.left));
        float y = chartArea.bottom - static_cast<float>(normalizedY * (chartArea.bottom - chartArea.top));

        points.emplace_back(x, y);
    }

    return points;
}

// Utility functions
std::wstring ChartRenderer::FormatValue(double value, const std::wstring& format) {
    if (format.empty()) return FormatCurrency(value);

    std::wstringstream ss;
    ss << std::fixed << std::setprecision(2) << value;
    return ss.str();
}

std::wstring ChartRenderer::FormatCurrency(double value) {
    std::wstringstream ss;
    ss << L"$" << std::fixed << std::setprecision(2) << value;
    return ss.str();
}

std::wstring ChartRenderer::FormatPercentage(double value) {
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(1) << value << L"%";
    return ss.str();
}