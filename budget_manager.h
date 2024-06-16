#pragma once
/*
 * Создайте менеджер личного бюджета, хранящий информацию о доходах по дням.
 * Программа должна поддерживать запросы трёх типов:
 * 1) Поступление средств на счёт за определённый период (Earn()). Распределяет средства равномерно по дням.
 * 2) Уплата налога 13% (PayTax()). Вычитает 13 процентов из совокупного дохода каждого дня указанного периода,
 * независимо от того, платились ли уже в этот день налоги.
 * 3) Запрос на выведение дохода (ComputeIncome). Печатает в cout чистую прибыль всех дней на данный момент.
 * 4) Трата средств (Spend). Разбивает указанную сумму на все дни. Теперь чистая прибыль вычисляется как разница заработанного и потраченного.
 * Траты не влияют на налогообложение. Таким образом, чистая прибыль за день может стать отрицательной.
 *
 * Кроме того, добавьте в запрос PayTax возможность указывать процентную ставку налога.
 *
 * Событие задаётся диапазоном дат. Если заработано 100 единиц с 2010-01-01 по 2010-01-05, считается, что в каждый из этих пяти дней заработано 20 единиц.
 * Функция для вычисления количества дней на интервале дана в date.h — эта функция использует возможности C,
 * поскольку в C++ календарные функции появились только в стандарте 2020 года и реализованы не во всех основных компиляторах.
 */

#include "bulk_updater.h"
#include "date.h"
#include "entities.h"

#include <vector>

class BudgetManager {
public:
    inline static const Date START_DATE{2000, 1, 1};
    inline static const Date END_DATE{2100, 1, 1};

    static size_t GetDayIndex(Date day) {
        return static_cast<size_t>(Date::ComputeDistance(START_DATE, day));
    }

    static IndexSegment MakeDateSegment(Date from, Date to) {
        return {GetDayIndex(from), GetDayIndex(to) + 1};
    }

    double ComputeSum(Date from, Date to) const {
        auto temp = MakeDateSegment(from, to);
        double in = tree_.ComputeSum(temp).income;
        double sp = tree_.ComputeSum(temp).spend;
        return in - sp;
    }

    void AddBulkOperation(Date from, Date to, const BulkLinearUpdater &operation) {
        tree_.AddBulkOperation(MakeDateSegment(from, to), operation);
    }

private:
    SummingSegmentTree<Day, BulkLinearUpdater> tree_{GetDayIndex(END_DATE)};
};
