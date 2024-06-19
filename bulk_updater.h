#pragma once

#include "entities.h"
#include "summing_segment_tree.h"

#include <cstdint>

struct DayState {
public:
    double ComputeIncome() {
        return (earned - spent);
    }
public:
    double earned = 0.;
    double spent = 0.;
};

inline DayState operator+(const DayState& first, const DayState& second) {
    return {first.earned + second.earned, first.spent + second.spent};
}

inline DayState operator*(const DayState& first, double factor) {
    return {first.earned * factor, first.spent};
}

struct BulkMoneyAdder {
    DayState delta = {};
};

struct BulkTaxApplier {
    double factor = 1.0;
};

class BulkLinearUpdater {
public:
    BulkLinearUpdater() = default;

    BulkLinearUpdater(const BulkMoneyAdder& add)
        : add_(add) {
    }

    BulkLinearUpdater(const BulkTaxApplier& tax)
        : tax_(tax) {
    }

    void CombineWith(const BulkLinearUpdater& other) {
        /*tax_.count += other.tax_.count;
        add_.delta = add_.delta * other.tax_.ComputeFactor() + other.add_.delta;*/
        tax_.factor *= other.tax_.factor;
        add_.delta = add_.delta * other.tax_.factor + other.add_.delta;
    }

    DayState Collapse(DayState origin, IndexSegment segment) const {
        //return origin * tax_.ComputeFactor() + add_.delta * static_cast<double>(segment.length());
        return origin * tax_.factor + add_.delta * static_cast<double>(segment.length());
    }

private:
    // apply tax first, then add
    BulkTaxApplier tax_;
    BulkMoneyAdder add_;
};
