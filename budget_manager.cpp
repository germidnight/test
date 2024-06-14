#include "budget_manager.h"

BudgetManager::BudgetManager() {
    income_by_day_.resize(static_cast<size_t>(Date::ComputeDistance(START_DATE, END_DATE) + 1), {0});
}

void BudgetManager::Earn(const Date &begin, const Date &end, double income) {
    const double income_per_day = static_cast<double>(income) / static_cast<double>(Date::ComputeDistance(begin, end) + 1);

    for (size_t i = static_cast<size_t>(Date::ComputeDistance(START_DATE, begin));
                i <= static_cast<size_t>(Date::ComputeDistance(START_DATE, end)); ++i) {
        income_by_day_[i].income += income_per_day;
    }
}

void BudgetManager::PayTax(const Date &begin, const Date &end) {
    for (size_t i = static_cast<size_t>(Date::ComputeDistance(START_DATE, begin));
                i <= static_cast<size_t>(Date::ComputeDistance(START_DATE, end)); ++i) {
        income_by_day_[i].income *= after_tax_payment_;
    }
}

double BudgetManager::ComputeIncome(const Date &begin, const Date &end) const {
    double total_income = 0;
    for (size_t i = static_cast<size_t>(Date::ComputeDistance(START_DATE, begin));
                i <= static_cast<size_t>(Date::ComputeDistance(START_DATE, end)); ++i) {
        total_income += income_by_day_[i].income;
    }
    return total_income;
}