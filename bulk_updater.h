#pragma once

#include "entities.h"
#include "summing_segment_tree.h"

#include <cmath>
#include <cstdint>

struct Day {
    double income = 0.;
    double spend = 0.;
};

inline Day operator+(const Day& left, const Day& right) {
    return {left.income + right.income, left.spend + right.spend};
}

inline Day operator*(const Day& left, double val) {
    return {left.income * val, left.spend};
}

struct BulkMoneyAdder {
    Day delta = {};
};

struct BulkTaxApplier {
    double factor = 1.0;
};

class BulkLinearUpdater {
public:
    BulkLinearUpdater() = default;

    BulkLinearUpdater(const BulkMoneyAdder &add)
        : add_(add) {
    }

    BulkLinearUpdater(const BulkTaxApplier &tax)
        : tax_(tax) {
    }
    /*CombineWith меняет поле BulkOperation, то есть данные внутри самой операции (BulkMoneyAdder и BulkTaxApplier),
    реализуя механизм «проталкивания» вниз*/
    void CombineWith(const BulkLinearUpdater &other) {
        tax_.factor *= other.tax_.factor;
        add_.delta = add_.delta * tax_.factor + other.add_.delta;
    }
    /*Collapse меняет поле Data путем использования подготовленных данных внутри операции.*/
    Day Collapse(Day origin, IndexSegment segment) const {
        return origin * tax_.factor + add_.delta * static_cast<double>(segment.length());
    }

private:
    // apply tax first, then add
    BulkTaxApplier tax_;
    BulkMoneyAdder add_;
};
