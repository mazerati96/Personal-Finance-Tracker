#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#pragma once
#include <windows.h>
#include <windowsx.h>

#include <vector>
#include <string>
#include <map>
#include <memory>


// Forward declarations
struct User;
struct Expense;
struct Income;
struct Budget;
struct RecurringTransaction;
struct SavingsGoal;
struct Category;

// Enums
enum class TransactionType { EXPENSE, INCOME };
enum class RecurrenceType { DAILY, WEEKLY, MONTHLY, YEARLY };
enum class AuthType { NONE, PIN, PASSWORD };
enum class CurrencyType { USD, EUR, GBP, JPY, CAD, AUD };

// Core data structures
struct User {
    std::wstring username;
    std::wstring passwordHash;
    CurrencyType defaultCurrency;
    AuthType authType;
    bool isDarkMode;
    std::wstring createdDate;
    std::wstring displayName;
    std::wstring profilePicPath;
	std::wstring authHash;  // For PIN or password authentication
    User() : defaultCurrency(CurrencyType::USD), authType(AuthType::PASSWORD), isDarkMode(false) {}

    User& operator=(const User& other) {
        if (this != &other) {
            username = other.username;
            passwordHash = other.passwordHash;
            defaultCurrency = other.defaultCurrency;
            authType = other.authType;
            isDarkMode = other.isDarkMode;
            createdDate = other.createdDate;
            displayName = other.displayName;
            profilePicPath = other.profilePicPath;
        }
        return *this;
    }
};

struct Category {
    std::wstring name;
    std::wstring color;  // Hex color for UI
    std::wstring icon;   // Icon identifier
    bool isDefault;

    Category() : isDefault(false) {}
    Category(const std::wstring& n, const std::wstring& c, const std::wstring& i, bool def = false)
        : name(n), color(c), icon(i), isDefault(def) {
    }
};

struct Expense {
    std::wstring id;
    std::wstring userId;
    std::wstring category;
    double amount;
    std::wstring note;
    std::wstring date;
    std::vector<std::wstring> tags;
    std::wstring receiptPath;
    CurrencyType currency;
    double exchangeRate;  // To default currency
    std::wstring location;

    Expense() : amount(0.0), currency(CurrencyType::USD), exchangeRate(1.0) {}
};

struct Income {
    std::wstring id;
    std::wstring userId;
    std::wstring source;
    double amount;
    std::wstring note;
    std::wstring date;
    std::vector<std::wstring> tags;
    CurrencyType currency;
    double exchangeRate;
    bool isTaxable;

    Income() : amount(0.0), currency(CurrencyType::USD), exchangeRate(1.0), isTaxable(true) {}
};

struct Budget {
    int amount;
    std::wstring id;
    std::wstring name;
    std::wstring userId;
    std::wstring category;
    double monthlyLimit;
    double currentSpent;
    std::wstring startDate;
    std::wstring endDate;
    bool isActive;
    double warningThreshold;  // Percentage (0.8 = 80%)

    Budget() : monthlyLimit(0.0), currentSpent(0.0), isActive(true), warningThreshold(0.8) {}
};

struct RecurringTransaction {
    std::wstring id;
    std::wstring userId;
    std::wstring description;
    std::wstring category;
    double amount;
    TransactionType type;
    RecurrenceType recurrence;
    int dayOfMonth;  // For monthly
    int dayOfWeek;   // For weekly
    std::wstring startDate;
    std::wstring endDate;
    std::wstring lastProcessed;
    bool isActive;

    RecurringTransaction() : amount(0.0), type(TransactionType::EXPENSE),
        recurrence(RecurrenceType::MONTHLY), dayOfMonth(1), dayOfWeek(1), isActive(true) {
    }
};

struct SavingsGoal {
    std::wstring id;
    std::wstring userId;
    std::wstring name;
    std::wstring description;
    double targetAmount;
    double currentAmount;
    std::wstring targetDate;
    std::wstring createdDate;
    std::wstring category;
    bool isActive;

    SavingsGoal() : targetAmount(0.0), currentAmount(0.0), isActive(true) {}
};

struct DateRange {
    std::wstring startDate;
    std::wstring endDate;

    DateRange() {}
    DateRange(const std::wstring& start, const std::wstring& end)
        : startDate(start), endDate(end) {
    }
};

struct FilterCriteria {
    DateRange dateRange;
    std::vector<std::wstring> categories;
    std::vector<std::wstring> tags;
    double minAmount;
    double maxAmount;
    std::wstring searchText;

    FilterCriteria() : minAmount(0.0), maxAmount(999999.0) {}
};

// Currency conversion rates (simplified - in real app, would fetch from API)
struct CurrencyRate {
    CurrencyType from;
    CurrencyType to;
    double rate;
    std::wstring lastUpdated;
};

// Analytics structures
struct SpendingTrend {
    std::wstring period;  // "2025-07", "2025-W30", etc.
    double totalIncome;
    double totalExpenses;
    double balance;
    std::map<std::wstring, double> categoryBreakdown;
};

struct CategoryAnalytics {
    std::wstring category;
    double totalSpent;
    double budgetLimit;
    double percentageOfTotal;
    int transactionCount;
    double averageTransaction;
    std::wstring trend;  // "increasing", "decreasing", "stable"
};

// Constants
const std::vector<Category> DEFAULT_CATEGORIES = {
    {L"Food & Dining", L"#FF6B6B", L"🍽️", true},
    {L"Transportation", L"#4ECDC4", L"🚗", true},
    {L"Shopping", L"#45B7D1", L"🛍️", true},
    {L"Entertainment", L"#96CEB4", L"🎬", true},
    {L"Bills & Utilities", L"#FFEAA7", L"💡", true},
    {L"Healthcare", L"#DDA0DD", L"🏥", true},
    {L"Education", L"#98D8C8", L"📚", true},
    {L"Travel", L"#F7DC6F", L"✈️", true},
    {L"Miscellaneous", L"#AED6F1", L"📦", true}
};

const std::vector<std::wstring> DEFAULT_INCOME_SOURCES = {
    L"Salary", L"Freelance", L"Investment", L"Gift", L"Bonus", L"Other"
};

// Global data containers
extern std::vector<User> users;
// In DataStructures.h
extern std::vector<Expense> expenses;
extern std::vector<Income> incomes;

extern std::vector<Budget> budgets;
extern std::vector<RecurringTransaction> recurringTransactions;
extern std::vector<SavingsGoal> savingsGoals;
extern std::vector<Category> categories;
extern std::map<CurrencyType, std::map<CurrencyType, double>> exchangeRates;

// Current session
extern std::wstring currentUserId;
extern User* currentUser;


#endif