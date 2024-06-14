#pragma once

/*
 * Программа должна обрабатывать такие запросы:
 * 1) ComputeIncome <дата начала> <дата конца> — вычислить чистую прибыль за данный диапазон дат и вывести результат в cout.
 * 2) Earn <дата начала> <дата конца> <прибыль> — учесть, что в этот период равномерно по дням была заработана указанная сумма.
 * Прибыль — произвольное положительное число double.
 * 3) PayTax <дата начала> <дата конца> — заплатить налог 13% в каждый день указанного диапазона.
 * Это означает простое умножение всей прибыли в диапазоне на 0,87, независимо от того, отдавался ли уже налог за какой-либо из указанных дней.
 * Прибыль за эти дни, которая обнаружится позже, налогами из прошлого не облагается.
 *
 * Обе даты — начальная и конечная — включаются в диапазон.
 * В первой строке записано количество запросов, а затем на отдельных строках сами запросы.
 *
 * Формат выходных данных
 * Каждый запрос типа ComputeIncome выводит на отдельной строке действительное число — чистую прибыль за указанный период.
 *
 * Ограничения на входные данные:
 * - Все даты находятся в диапазоне от 2000-01-01 до 2099-12-31.
 * - Количество запросов невелико.
 * - Количество дней в одном запросе — любое в пределах указанного диапазона.
 * - Дата конца периода не раньше даты начала периода.
 * - Все даты корректны.
 */

#include "budget_manager.h"

#include <string>
#include <string_view>

struct BudgetRequestsParser {
    void static Earn(BudgetManager& budget, std::string_view params);
    void static PayTax(BudgetManager& budget, std::string_view params);
    double static ComputeIncome(const BudgetManager& budget, std::string_view params);
};