#include "budget_manager.h"
#include "parser.h"

#include <iostream>
#include <string_view>

using namespace std::literals;

void ParseAndProcessQuery(BudgetManager &manager, std::string_view line) {
    const std::string compute_income_cmd = "ComputeIncome";
    const int cic_len = 13;
    const std::string earn_cmd = "Earn";
    const int e_len = 4;
    const std::string pay_tax_cmd = "PayTax";
    const int pt_len = 6;

    if ((line[0] == 'C') && (line.substr(0, cic_len).compare(compute_income_cmd) == 0)) {
        std::cout << BudgetRequestsParser::ComputeIncome(manager, line.substr(cic_len + 1)) << std::endl;
    } else if ((line[0] == 'E') && (line.substr(0, e_len).compare(earn_cmd) == 0)) {
        BudgetRequestsParser::Earn(manager, line.substr(e_len + 1));
    } else if ((line[0] == 'P') && (line.substr(0, pt_len).compare(pay_tax_cmd) == 0)) {
        BudgetRequestsParser::PayTax(manager, line.substr(pt_len + 1));
    }
}

int ReadNumberOnLine(std::istream &input) {
    std::string line;
    std::getline(input, line);
    return std::stoi(line);
}

int main() {
    BudgetManager manager;

    const int query_count = ReadNumberOnLine(std::cin);

    for (int i = 0; i < query_count; ++i) {
        std::string line;
        std::getline(std::cin, line);
        ParseAndProcessQuery(manager, line);
    }
}