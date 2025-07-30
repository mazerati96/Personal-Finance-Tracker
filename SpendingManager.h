#pragma once
#include <windows.h>
#include <vector>
#include <string>


// Dialog control IDs
#define ID_BUTTON_OK            204
#define ID_BUTTON_CANCEL        205

// Function declarations
void AddExpenseDialog(HWND hwnd);
void ShowExpensesDialog(HWND hwnd);
void AddIncomeDialog(HWND hwnd);
void ShowIncomeDialog(HWND hwnd);
void ShowSummaryDialog(HWND hwnd);
void SaveDataToFile();
void LoadDataFromFile();

// Utility functions
std::wstring GetCurrentDate();
double GetTotalExpenses();
double GetTotalIncome();
double GetBalance();
int GetNextExpenseId();
int GetNextIncomeId();