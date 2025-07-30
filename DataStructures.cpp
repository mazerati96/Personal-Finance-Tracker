#include "DataStructures.h"
#include "Utils.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

std::vector<User> users;
std::vector<Expense> expenses;
std::vector<Income> incomes;
std::vector<Budget> budgets;
std::vector<SavingsGoal> savingsGoals;
std::vector<Category> categories;
std::vector<RecurringTransaction> recurringTransactions;
// Global data containers

std::map<CurrencyType, std::map<CurrencyType, double>> exchangeRates;

// Current session
std::wstring currentUserId;
User* currentUser = nullptr;



// Category management functions
void AddCategory(const Category& category) {
    categories.push_back(category);
}

void RemoveCategory(const std::wstring& categoryName) {
    categories.erase(
        std::remove_if(categories.begin(), categories.end(),
            [&categoryName](const Category& cat) { return cat.name == categoryName; }),
        categories.end());
}

Category* FindCategory(const std::wstring& categoryName) {
    auto it = std::find_if(categories.begin(), categories.end(),
        [&categoryName](const Category& cat) { return cat.name == categoryName; });
    return (it != categories.end()) ? &(*it) : nullptr;
}

std::vector<std::wstring> GetCategoryNames() {
    std::vector<std::wstring> names;
    for (const auto& cat : categories) {
        names.push_back(cat.name);
    }
    return names;
}


bool ValidateBudget(const Budget& budget) {
    if (budget.category.empty()) return false;
    if (budget.monthlyLimit <= 0) return false;
    if (budget.userId.empty()) return false;
    return true;
}


// Search and filter functions
std::vector<Expense> FilterExpenses(const FilterCriteria& criteria, const std::wstring& userId) {
    std::vector<Expense> result;
    
    for (const auto& expense : expenses) {
        if (!userId.empty() && expense.userId != userId) continue;
        
        // Date range filter
        if (!IsDateInRange(expense.date, criteria.dateRange)) continue;
        
        // Category filter
        if (!criteria.categories.empty()) {
            bool categoryMatch = std::find(criteria.categories.begin(), criteria.categories.end(), 
                expense.category) != criteria.categories.end();
            if (!categoryMatch) continue;
        }
        
        // Amount range filter
        if (expense.amount < criteria.minAmount || expense.amount > criteria.maxAmount) continue;
        
        // Tags filter
        if (!criteria.tags.empty()) {
            bool tagMatch = false;
            for (const auto& tag : criteria.tags) {
                if (std::find(expense.tags.begin(), expense.tags.end(), tag) != expense.tags.end()) {
                    tagMatch = true;
                    break;
                }
            }
            if (!tagMatch) continue;
        }
        
        // Search text filter
        if (!criteria.searchText.empty()) {
            std::wstring searchLower = criteria.searchText;
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::towlower);
            
            std::wstring categoryLower = expense.category;
            std::transform(categoryLower.begin(), categoryLower.end(), categoryLower.begin(), ::towlower);
            
            std::wstring noteLower = expense.note;
            std::transform(noteLower.begin(), noteLower.end(), noteLower.begin(), ::towlower);
            
            bool textMatch = categoryLower.find(searchLower) != std::wstring::npos ||
                           noteLower.find(searchLower) != std::wstring::npos;
            
            for (const auto& tag : expense.tags) {
                std::wstring tagLower = tag;
                std::transform(tagLower.begin(), tagLower.end(), tagLower.begin(), ::towlower);
                if (tagLower.find(searchLower) != std::wstring::npos) {
                    textMatch = true;
                    break;
                }
            }
            
            if (!textMatch) continue;
        }
        
        result.push_back(expense);
    }
    
    return result;
}

std::vector<Income> FilterIncomes(const FilterCriteria& criteria, const std::wstring& userId) {
    std::vector<Income> result;
    
    for (const auto& income : incomes) {
        if (!userId.empty() && income.userId != userId) continue;
        
        // Date range filter
        if (!IsDateInRange(income.date, criteria.dateRange)) continue;
        
        // Amount range filter
        if (income.amount < criteria.minAmount || income.amount > criteria.maxAmount) continue;
        
        // Tags filter
        if (!criteria.tags.empty()) {
            bool tagMatch = false;
            for (const auto& tag : criteria.tags) {
                if (std::find(income.tags.begin(), income.tags.end(), tag) != income.tags.end()) {
                    tagMatch = true;
                    break;
                }
            }
            if (!tagMatch) continue;
        }
        
        // Search text filter
        if (!criteria.searchText.empty()) {
            std::wstring searchLower = criteria.searchText;
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::towlower);
            
            std::wstring sourceLower = income.source;
            std::transform(sourceLower.begin(), sourceLower.end(), sourceLower.begin(), ::towlower);
            
            std::wstring noteLower = income.note;
            std::transform(noteLower.begin(), noteLower.end(), noteLower.begin(), ::towlower);
            
            bool textMatch = sourceLower.find(searchLower) != std::wstring::npos ||
                           noteLower.find(searchLower) != std::wstring::npos;
            
            for (const auto& tag : income.tags) {
                std::wstring tagLower = tag;
                std::transform(tagLower.begin(), tagLower.end(), tagLower.begin(), ::towlower);
                if (tagLower.find(searchLower) != std::wstring::npos) {
                    textMatch = true;
                    break;
                }
            }
            
            if (!textMatch) continue;
        }
        
        result.push_back(income);
    }
    
    return result;
}

// Analytics helper functions
std::map<std::wstring, double> GetCategoryTotals(const std::wstring& userId, const DateRange& dateRange) {
    std::map<std::wstring, double> totals;
    
    FilterCriteria criteria;
    criteria.dateRange = dateRange;
    
    auto filteredExpenses = FilterExpenses(criteria, userId);
    
    for (const auto& expense : filteredExpenses) {
        totals[expense.category] += expense.amount;
    }
    
    return totals;
}


std::vector<SpendingTrend> GetSpendingTrends(const std::wstring& userId, const std::wstring& period) {
    std::vector<SpendingTrend> trends;
    
    // Implementation depends on period type (monthly, weekly, etc.)
    // This is a simplified version for monthly trends
    
    std::map<std::wstring, SpendingTrend> monthlyTrends;
    
    // Process expenses
    for (const auto& expense : expenses) {
        if (expense.userId != userId) continue;                      
        
        // Extract year-month from date (YYYY-MM-DD -> YYYY-MM)
        std::wstring monthKey = expense.date.substr(0, 7);
        
        if (monthlyTrends.find(monthKey) == monthlyTrends.end()) {
            monthlyTrends[monthKey] = SpendingTrend();
            monthlyTrends[monthKey].period = monthKey;
        }
        
        monthlyTrends[monthKey].totalExpenses += expense.amount;
        monthlyTrends[monthKey].categoryBreakdown[expense.category] += expense.amount;
    }
    
    // Process incomes
    for (const auto& income : incomes) {
        if (income.userId != userId) continue;
        
        std::wstring monthKey = income.date.substr(0, 7);
        
        if (monthlyTrends.find(monthKey) == monthlyTrends.end()) {
            monthlyTrends[monthKey] = SpendingTrend();
            monthlyTrends[monthKey].period = monthKey;
        }
        
        monthlyTrends[monthKey].totalIncome += income.amount;
    }
    
    // Calculate balances and convert to vector
    for (auto& trend : monthlyTrends) {
        trend.second.balance = trend.second.totalIncome - trend.second.totalExpenses;
        trends.push_back(trend.second);
    }
    
    // Sort by period
    std::sort(trends.begin(), trends.end(), 
        [](const SpendingTrend& a, const SpendingTrend& b) {
            return a.period < b.period;
        });
    
    return trends;
}

std::vector<CategoryAnalytics> GetCategoryAnalytics(const std::wstring& userId, const DateRange& dateRange) {
    std::vector<CategoryAnalytics> analytics;
    
    auto categoryTotals = GetCategoryTotals(userId, dateRange);
    double totalSpent = 0.0;
    
    for (const auto& pair : categoryTotals) {
        totalSpent += pair.second;
    }
    
    for (const auto& pair : categoryTotals) {
        CategoryAnalytics cat;
        cat.category = pair.first;
        cat.totalSpent = pair.second;
        cat.percentageOfTotal = totalSpent > 0 ? (pair.second / totalSpent) * 100.0 : 0.0;
        
        // Find budget for this category
        auto budgetIt = std::find_if(budgets.begin(), budgets.end(),
            [&](const Budget& b) { return b.userId == userId && b.category == pair.first && b.isActive; });
        
        if (budgetIt != budgets.end()) {
            cat.budgetLimit = budgetIt->monthlyLimit;
        }
        
        // Count transactions
        FilterCriteria criteria;
        criteria.dateRange = dateRange;
        criteria.categories = {pair.first};
        auto categoryExpenses = FilterExpenses(criteria, userId);
        
        cat.transactionCount = static_cast<int>(categoryExpenses.size());
        cat.averageTransaction = cat.transactionCount > 0 ? cat.totalSpent / cat.transactionCount : 0.0;
        
        // Simple trend analysis (compare with previous period)
        cat.trend = L"stable"; // Simplified - would need more complex logic
        
        analytics.push_back(cat);
    }
    
    // Sort by total spent (descending)
    std::sort(analytics.begin(), analytics.end(),
        [](const CategoryAnalytics& a, const CategoryAnalytics& b) {
            return a.totalSpent > b.totalSpent;
        });
    
    return analytics;
}

// Budget helper functions
void UpdateBudgetSpending(const std::wstring& userId, const std::wstring& category, double amount) {
    auto budgetIt = std::find_if(budgets.begin(), budgets.end(),
        [&](Budget& b) { return b.userId == userId && b.category == category && b.isActive; });
    
    if (budgetIt != budgets.end()) {
        budgetIt->currentSpent += amount;
    }
}

std::vector<Budget> GetOverBudgetCategories(const std::wstring& userId) {
    std::vector<Budget> overBudget;
    
    for (const auto& budget : budgets) {
        if (budget.userId == userId && budget.isActive && budget.currentSpent > budget.monthlyLimit) {
            overBudget.push_back(budget);
        }
    }
    
    return overBudget;
}

std::vector<Budget> GetNearBudgetLimitCategories(const std::wstring& userId) {
    std::vector<Budget> nearLimit;
    
    for (const auto& budget : budgets) {
        if (budget.userId == userId && budget.isActive) {
            double percentage = budget.monthlyLimit > 0 ? budget.currentSpent / budget.monthlyLimit : 0.0;
            if (percentage >= budget.warningThreshold && percentage < 1.0) {
                nearLimit.push_back(budget);
            }
        }
    }
    
    return nearLimit;
}

// Recurring transaction helpers  
bool ShouldProcessRecurring(const RecurringTransaction& rt) {
    if (!rt.isActive) return false;
    
    std::wstring today = GetCurrentDate();
    
    if (rt.lastProcessed == today) return false; // Already processed today
    
    // Simple logic - would need more sophisticated date handling
    switch (rt.recurrence) {
    case RecurrenceType::DAILY:
        return rt.lastProcessed != today;
    case RecurrenceType::WEEKLY:
        // Check if it's the right day of week
        return true; // Simplified
    case RecurrenceType::MONTHLY:
        // Check if it's the right day of month
        return today.substr(8, 2) == std::to_wstring(rt.dayOfMonth);
    case RecurrenceType::YEARLY:
        // Check if it's the right day and month
        return true; // Simplified
    }
    
    return false;
}

void ProcessRecurringTransaction(RecurringTransaction& rt) {
    if (rt.type == TransactionType::EXPENSE) {
        Expense expense;
        expense.id = GenerateUniqueId();
        expense.userId = rt.userId;
        expense.category = rt.category;
        expense.amount = rt.amount;
        expense.note = rt.description + L" (Recurring)";
        expense.date = GetCurrentDate();
        
        if (ValidateExpense(expense)) {
            expenses.push_back(expense);
            UpdateBudgetSpending(rt.userId, rt.category, rt.amount);
        }
    } else {
        Income income;
        income.id = GenerateUniqueId();
        income.userId = rt.userId;
        income.source = rt.category;
        income.amount = rt.amount;
        income.note = rt.description + L" (Recurring)";
        income.date = GetCurrentDate();
        
        if (ValidateIncome(income)) {
            incomes.push_back(income);
        }
    }
    
    rt.lastProcessed = GetCurrentDate();
}

// Savings goal helpers
double GetSavingsGoalProgress(const SavingsGoal& goal) {
    if (goal.targetAmount <= 0) return 0.0;
    return (goal.currentAmount / goal.targetAmount) * 100.0;
}

std::wstring GetSavingsGoalStatus(const SavingsGoal& goal) {
    double progress = GetSavingsGoalProgress(goal);
    
    if (progress >= 100.0) return L"Completed";
    if (progress >= 75.0) return L"Nearly There";
    if (progress >= 50.0) return L"Halfway";
    if (progress >= 25.0) return L"Making Progress";
    return L"Just Started";
}

int GetDaysUntilGoalDeadline(const SavingsGoal& goal) {
    // Parse goal.deadline and compare with current date
    // Simplified implementation:
    return 30; // TODO: Implement actual date difference calculation
}

double GetRequiredMonthlySavings(const SavingsGoal& goal) {
    double remaining = goal.targetAmount - goal.currentAmount;
    int monthsRemaining = GetDaysUntilGoalDeadline(goal) / 30; // Simplified
    
    return monthsRemaining > 0 ? remaining / monthsRemaining : remaining;
}

