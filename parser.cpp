#include "date.h"
#include "parser.h"

void BudgetRequestsParser::Earn(BudgetManager& budget, std::string_view params) {
    const Date date_begin = Date::FromString(params.substr(0, 10));
    const Date date_end = Date::FromString(params.substr(11, 10));
    const double income = std::stod(std::string(params.substr(22)));
    budget.Earn(date_begin, date_end, income);
}

void BudgetRequestsParser::PayTax(BudgetManager& budget, std::string_view params) {
    const Date date_begin = Date::FromString(params.substr(0, 10));
    const Date date_end = Date::FromString(params.substr(11, 10));
    budget.PayTax(date_begin, date_end);
}

double BudgetRequestsParser::ComputeIncome(const BudgetManager& budget, std::string_view params) {
    const Date date_begin = Date::FromString(params.substr(0, 10));
    const Date date_end = Date::FromString(params.substr(11, 10));
    return budget.ComputeIncome(date_begin, date_end);
}