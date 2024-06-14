#pragma once

/*
 * Создайте менеджер личного бюджета, хранящий информацию о доходах по дням.
 * Программа должна поддерживать запросы трёх типов:
 * 1) Поступление средств на счёт за определённый период (Earn()). Распределяет средства равномерно по дням.
 * 2) Уплата налога 13% (PayTax()). Вычитает 13 процентов из совокупного дохода каждого дня указанного периода,
 * независимо от того, платились ли уже в этот день налоги.
 * 3) Запрос на выведение дохода (ComputeIncome). Печатает в cout чистую прибыль всех дней на данный момент.
 *
 * Событие задаётся диапазоном дат. Если заработано 100 единиц с 2010-01-01 по 2010-01-05, считается, что в каждый из этих пяти дней заработано 20 единиц.
 * Функция для вычисления количества дней на интервале дана в date.h — эта функция использует возможности C,
 * поскольку в C++ календарные функции появились только в стандарте 2020 года и реализованы не во всех основных компиляторах.
 */

#include "date.h"

#include <vector>

struct Day {
    double income = 0;
};

class BudgetManager {
public:
    inline static const Date START_DATE{2000, 1, 1};
    inline static const Date END_DATE{2100, 1, 1};
    BudgetManager();

    void Earn(const Date& begin, const Date& end, double income);
    void PayTax(const Date& begin, const Date& end);
    double ComputeIncome(const Date &begin, const Date &end) const;

private:
    const double after_tax_payment_ = 0.87;
    std::vector<Day> income_by_day_;
};
