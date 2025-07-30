#pragma once
#include <windows.h>

class ExportManager {
public:
    enum ExportType {
        CSV,
        PDF,
        EXCEL
    };

    static void ShowExportDialog(HWND hwnd, ExportType type);
};
