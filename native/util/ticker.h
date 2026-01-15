#pragma once

#include <cstddef>
#include <cstdint>

#include "number.h"

namespace HyoutaUtils {
// Helper class for implementing a ticker or timer that ticks every N ticks from an external source.
// The intended use case is something like the following: You have an external high-resolution
// integer timing source such as QueryPerformanceCounter() and you want to convert it into a lower-
// resolution but still fully accurate ticker that eg. increments by 1 every 1/60'th of a second.
struct Ticker {
    uint64_t AccumulatedTicks; // 'unspent' external ticks to the next increment
    uint64_t TicksPerAdvancement; // how many external ticks must pass for one local tick increment
    uint64_t Multiplicator; // multiplicator to be applied to external ticks
};

inline void InitializeTicker(Ticker& ticker, uint64_t numerator, uint64_t denominator) {
    uint64_t a = NumberUtils::FindGreatestCommonDivisor(numerator, denominator);
    if (a != 0) {
        ticker.TicksPerAdvancement = numerator / a;
        ticker.Multiplicator = denominator / a;
    } else {
        ticker.TicksPerAdvancement = numerator;
        ticker.Multiplicator = denominator;
    }
    ticker.AccumulatedTicks = 0;
}

inline uint64_t AdvanceTicker(Ticker& ticker, uint64_t ticks) {
    const uint64_t t = ticks * ticker.Multiplicator;
    const uint64_t accumulated = ticker.AccumulatedTicks + t;
    const uint64_t divisor = ticker.TicksPerAdvancement;
    const uint64_t localTicks = accumulated / divisor;
    const uint64_t rest = accumulated % divisor;
    ticker.AccumulatedTicks = rest;
    return localTicks;
}
} // namespace HyoutaUtils
