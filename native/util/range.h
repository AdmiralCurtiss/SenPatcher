#pragma once

#include <array>

namespace HyoutaUtils {
template<typename T>
struct Range {
    T Offset = T();
    T Length = T();
};

template<typename T>
struct DualRange {
    // Represents a set of two disjunct ranges.
    // The intended use case is a valid range tracker for a ringbuffer-like structure that starts
    // being filled anywhere inside the buffer and has a single but arbitrary looping location.
    // If used by the functions below, the following conditions always apply:
    // - This stores up to two ranges, but can also store less. The initial state is zero ranges.
    // - A range is valid if its length is > 0.
    // - If only one range is valid, it is guaranteed to be the one at index 0.
    //   (in other words, if range 0 is invalid, range 1 is never valid)
    // - If both ranges are valid, it is guaranteed that the two ranges do not touch.
    //   If they end up touching they will automatically be merged into a single range.
    // - The most recently updated range is at index 0.
    // - If a third disjunct range is added, the range currently at index 1 is discarded.
    std::array<Range<T>, 2> Ranges;
};

template<typename T>
void DualRangeInsertRange(DualRange<T>& dualRange, const T offset, const T length) {
    if (length == T(0)) {
        return;
    }

    T r0start = dualRange.Ranges[0].Offset;
    T r0length = dualRange.Ranges[0].Length;
    T r1start = dualRange.Ranges[1].Offset;
    T r1length = dualRange.Ranges[1].Length;

    if (offset <= (r0start + r0length) && (offset + length) >= r0start) {
        T newStart = (r0start < offset) ? r0start : offset;
        T newEnd =
            ((r0start + r0length) > (offset + length)) ? (r0start + r0length) : (offset + length);
        r0start = newStart;
        r0length = (newEnd - newStart);

        // merge ranges if they touch now
        if (r1start <= (r0start + r0length) && (r1start + r1length) >= r0start) {
            newStart = (r0start < r1start) ? r0start : r1start;
            newEnd = ((r0start + r0length) > (r1start + r1length)) ? (r0start + r0length)
                                                                   : (r1start + r1length);
            r0start = newStart;
            r0length = (newEnd - newStart);
            dualRange.Ranges[0].Offset = r0start;
            dualRange.Ranges[0].Length = r0length;
            dualRange.Ranges[1].Offset = T(0);
            dualRange.Ranges[1].Length = T(0);
        } else {
            dualRange.Ranges[0].Offset = r0start;
            dualRange.Ranges[0].Length = r0length;
        }
        return;
    }

    if (offset <= (r1start + r1length) && (offset + length) >= r1start) {
        T newStart = (r1start < offset) ? r1start : offset;
        T newEnd =
            ((r1start + r1length) > (offset + length)) ? (r1start + r1length) : (offset + length);
        r1start = newStart;
        r1length = (newEnd - newStart);

        // merge ranges if they touch now. also write the updated range as range0.
        // (range0 should always be the most recently updated)
        if (r0start <= (r1start + r1length) && (r0start + r0length) >= r1start) {
            newStart = (r1start < r0start) ? r1start : r0start;
            newEnd = ((r1start + r1length) > (r0start + r0length)) ? (r1start + r1length)
                                                                   : (r0start + r0length);
            r1start = newStart;
            r1length = (newEnd - newStart);
            dualRange.Ranges[0].Offset = r1start;
            dualRange.Ranges[0].Length = r1length;
            dualRange.Ranges[1].Offset = T(0);
            dualRange.Ranges[1].Length = T(0);
        } else {
            dualRange.Ranges[0].Offset = r1start;
            dualRange.Ranges[0].Length = r1length;
            dualRange.Ranges[1].Offset = r0start;
            dualRange.Ranges[1].Length = r0length;
        }
        return;
    }

    // cannot append to either range, replace range1 instead, write as range0.
    dualRange.Ranges[0].Offset = offset;
    dualRange.Ranges[0].Length = length;
    dualRange.Ranges[1].Offset = r0start;
    dualRange.Ranges[1].Length = r0length;
}

template<typename T>
T DualRangeGetValidRangeLengthAt(const DualRange<T>& dualRange, const T offset) {
    const T r0start = dualRange.Ranges[0].Offset;
    const T r0length = dualRange.Ranges[0].Length;
    if (offset >= r0start && (offset - r0start) < r0length) {
        return r0start + r0length - offset;
    }
    const T r1start = dualRange.Ranges[1].Offset;
    const T r1length = dualRange.Ranges[1].Length;
    if (offset >= r1start && (offset - r1start) < r1length) {
        return r1start + r1length - offset;
    }
    return T(0);
}
} // namespace HyoutaUtils
