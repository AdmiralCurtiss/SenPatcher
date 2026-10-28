#include <cstdint>

#include "gtest/gtest.h"

#include "util/range.h"

using DR = HyoutaUtils::DualRange<uint64_t>;
static DR MakeOne() {
    DR r;
    r.Ranges[0].Offset = 10;
    r.Ranges[0].Length = 9;
    return r;
}
static DR MakeLowHigh() {
    DR r;
    r.Ranges[0].Offset = 10; // 10 - 19
    r.Ranges[0].Length = 9;
    r.Ranges[1].Offset = 40; // 40 - 55
    r.Ranges[1].Length = 15;
    return r;
}
static DR MakeHighLow() {
    DR r;
    r.Ranges[0].Offset = 40; // 40 - 55
    r.Ranges[0].Length = 15;
    r.Ranges[1].Offset = 10; // 10 - 19
    r.Ranges[1].Length = 9;
    return r;
}

TEST(Range, DualRangeInsertRange) {
    // default constructed DualRange is empty
    {
        DR r;
        EXPECT_EQ(0, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }

    // adding a range to an empty DualRange adds it as range 0
    {
        DR r;
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 10);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(10, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }

    // if we add two disjunct ranges the last added one is always range 0
    {
        DR r;
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 10);
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 25, 7);
        EXPECT_EQ(25, r.Ranges[0].Offset);
        EXPECT_EQ(7, r.Ranges[0].Length);
        EXPECT_EQ(5, r.Ranges[1].Offset);
        EXPECT_EQ(10, r.Ranges[1].Length);
    }
    {
        DR r;
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 25, 7);
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 10);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(10, r.Ranges[0].Length);
        EXPECT_EQ(25, r.Ranges[1].Offset);
        EXPECT_EQ(7, r.Ranges[1].Length);
    }

    // adding a third disjunct range discards the one at index 1 at time of adding
    {
        DR r;
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 10);
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 25, 7);
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 45, 9);
        EXPECT_EQ(45, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(25, r.Ranges[1].Offset);
        EXPECT_EQ(7, r.Ranges[1].Length);
    }

    // if the dual range contains a single range and we add a second...
    {
        // starts before existing range, ends before existing range
        // in DualRange:       o--------o
        // inserting:    o--o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 2);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(2, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // starts before existing range, ends on start of existing range
        // in DualRange:       o--------o
        // inserting:    o-----o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 6, 4);
        EXPECT_EQ(6, r.Ranges[0].Offset);
        EXPECT_EQ(19 - 6, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // starts before existing range, ends in the middle of existing range
        // in DualRange:       o--------o
        // inserting:    o---------o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 6, 8);
        EXPECT_EQ(6, r.Ranges[0].Offset);
        EXPECT_EQ(19 - 6, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // starts before existing range, ends on end of existing range
        // in DualRange:       o--------o
        // inserting:    o--------------o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 6, 19 - 6);
        EXPECT_EQ(6, r.Ranges[0].Offset);
        EXPECT_EQ(19 - 6, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // starts before existing range, ends after existing range
        // in DualRange:       o--------o
        // inserting:    o-------------------o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 6, 25);
        EXPECT_EQ(6, r.Ranges[0].Offset);
        EXPECT_EQ(25, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // starts on start of existing range, ends in the middle of existing range
        // in DualRange:       o--------o
        // inserting:          o---o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 4);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // starts on start of existing range, ends on end of existing range
        // in DualRange:       o--------o
        // inserting:          o--------o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 9);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // starts on start of existing range, ends after existing range
        // in DualRange:       o--------o
        // inserting:          o----------o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 11);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(11, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // starts in the middle of existing range, ends in the middle of existing range
        // in DualRange:       o--------o
        // inserting:             o---o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 14, 4);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // starts in the middle of existing range, ends on end of existing range
        // in DualRange:       o--------o
        // inserting:               o---o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 14, 5);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // starts in the middle of existing range, ends after existing range
        // in DualRange:       o--------o
        // inserting:               o-----o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 14, 7);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(11, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // starts on end of existing range, ends after existing range
        // in DualRange:       o--------o
        // inserting:                   o-----o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 19, 7);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(16, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // starts after existing range, ends after existing range
        // in DualRange:       o--------o
        // inserting:                       o-----o
        DR r = MakeOne();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 21, 7);
        EXPECT_EQ(21, r.Ranges[0].Offset);
        EXPECT_EQ(7, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }

    // if the dual range contains two ranges and we add a third...
    {
        // in DualRange:       0--------0        1---------1
        // inserting:    o--o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 2);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(2, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:    o-----o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 6, 4);
        EXPECT_EQ(6, r.Ranges[0].Offset);
        EXPECT_EQ(13, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:    o----------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 7);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(14, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:    o--------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 3, 16);
        EXPECT_EQ(3, r.Ranges[0].Offset);
        EXPECT_EQ(16, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:    o------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 20);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(20, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:    o-----------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 35);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(50, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:    o-----------------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 42);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(50, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:    o---------------------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 50);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(50, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:    o---------------------------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 100);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(100, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:          o---o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 5);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:          o--------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 9);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:          o------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 12);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(12, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:          o-----------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 30);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:          o---------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 33);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:          o---------------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 45);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:          o---------------------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 150);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(150, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:               o-o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 15, 2);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:               o---o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 15, 4);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:               o-------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 15, 6);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(11, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:               o------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 15, 25);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:               o-----------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 15, 30);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:               o----------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 16, 39);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:               o---------------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 16, 44);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(50, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                   o----o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 19, 4);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(13, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                   o--------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 19, 21);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                   o------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 19, 23);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                   o------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 19, 36);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                   o-----------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 19, 56);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(65, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                       o-o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 25, 2);
        EXPECT_EQ(25, r.Ranges[0].Offset);
        EXPECT_EQ(2, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                       o----o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 26, 14);
        EXPECT_EQ(26, r.Ranges[0].Offset);
        EXPECT_EQ(29, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                       o---------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 26, 16);
        EXPECT_EQ(26, r.Ranges[0].Offset);
        EXPECT_EQ(29, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                       o--------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 26, 29);
        EXPECT_EQ(26, r.Ranges[0].Offset);
        EXPECT_EQ(29, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                       o-------------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 26, 99);
        EXPECT_EQ(26, r.Ranges[0].Offset);
        EXPECT_EQ(99, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                            o-----o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 40, 2);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(15, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                            o---------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 40, 15);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(15, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                            o--------------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 40, 20);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(20, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                                o--o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 44, 4);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(15, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                                o-----o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 44, 11);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(15, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                                o----------o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 44, 101);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(105, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                                      o---o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 55, 100);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(115, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       0--------0        1---------1
        // inserting:                                           o---o
        DR r = MakeLowHigh();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 70, 3);
        EXPECT_EQ(70, r.Ranges[0].Offset);
        EXPECT_EQ(3, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:    o--o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 2);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(2, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:    o-----o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 6, 4);
        EXPECT_EQ(6, r.Ranges[0].Offset);
        EXPECT_EQ(13, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:    o----------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 7);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(14, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:    o--------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 3, 16);
        EXPECT_EQ(3, r.Ranges[0].Offset);
        EXPECT_EQ(16, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:    o------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 20);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(20, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:    o-----------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 35);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(50, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:    o-----------------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 42);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(50, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:    o---------------------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 50);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(50, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:    o---------------------------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 5, 100);
        EXPECT_EQ(5, r.Ranges[0].Offset);
        EXPECT_EQ(100, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:          o---o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 5);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:          o--------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 9);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:          o------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 12);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(12, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:          o-----------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 30);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:          o---------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 33);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:          o---------------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 45);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:          o---------------------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 10, 150);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(150, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:               o-o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 15, 2);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:               o---o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 15, 4);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(9, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:               o-------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 15, 6);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(11, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:               o------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 15, 25);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:               o-----------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 15, 30);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:               o----------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 16, 39);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:               o---------------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 16, 44);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(50, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                   o----o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 19, 4);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(13, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                   o--------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 19, 21);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                   o------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 19, 23);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                   o------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 19, 36);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(45, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                   o-----------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 19, 56);
        EXPECT_EQ(10, r.Ranges[0].Offset);
        EXPECT_EQ(65, r.Ranges[0].Length);
        EXPECT_EQ(0, r.Ranges[1].Offset);
        EXPECT_EQ(0, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                       o-o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 25, 2);
        EXPECT_EQ(25, r.Ranges[0].Offset);
        EXPECT_EQ(2, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                       o----o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 26, 14);
        EXPECT_EQ(26, r.Ranges[0].Offset);
        EXPECT_EQ(29, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                       o---------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 26, 16);
        EXPECT_EQ(26, r.Ranges[0].Offset);
        EXPECT_EQ(29, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                       o--------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 26, 29);
        EXPECT_EQ(26, r.Ranges[0].Offset);
        EXPECT_EQ(29, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                       o-------------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 26, 99);
        EXPECT_EQ(26, r.Ranges[0].Offset);
        EXPECT_EQ(99, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                            o-----o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 40, 2);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(15, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                            o---------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 40, 15);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(15, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                            o--------------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 40, 20);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(20, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                                o--o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 44, 4);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(15, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                                o-----o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 44, 11);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(15, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                                o----------o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 44, 101);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(105, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                                      o---o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 55, 100);
        EXPECT_EQ(40, r.Ranges[0].Offset);
        EXPECT_EQ(115, r.Ranges[0].Length);
        EXPECT_EQ(10, r.Ranges[1].Offset);
        EXPECT_EQ(9, r.Ranges[1].Length);
    }
    {
        // in DualRange:       1--------1        0---------0
        // inserting:                                           o---o
        DR r = MakeHighLow();
        HyoutaUtils::DualRangeInsertRange<uint64_t>(r, 70, 3);
        EXPECT_EQ(70, r.Ranges[0].Offset);
        EXPECT_EQ(3, r.Ranges[0].Length);
        EXPECT_EQ(40, r.Ranges[1].Offset);
        EXPECT_EQ(15, r.Ranges[1].Length);
    }
}

TEST(Range, DualRangeGetValidRangeLengthAt) {
    {
        DR r = MakeOne();
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 0));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 9));
        EXPECT_EQ(9, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 10));
        EXPECT_EQ(8, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 11));
        EXPECT_EQ(7, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 12));
        EXPECT_EQ(2, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 17));
        EXPECT_EQ(1, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 18));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 19));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 20));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 21));
    }
    {
        DR r = MakeLowHigh();
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 0));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 9));
        EXPECT_EQ(9, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 10));
        EXPECT_EQ(8, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 11));
        EXPECT_EQ(7, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 12));
        EXPECT_EQ(2, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 17));
        EXPECT_EQ(1, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 18));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 19));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 20));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 21));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 30));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 39));
        EXPECT_EQ(15, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 40));
        EXPECT_EQ(14, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 41));
        EXPECT_EQ(2, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 53));
        EXPECT_EQ(1, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 54));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 55));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 56));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 100));
    }
    {
        DR r = MakeHighLow();
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 0));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 9));
        EXPECT_EQ(9, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 10));
        EXPECT_EQ(8, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 11));
        EXPECT_EQ(7, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 12));
        EXPECT_EQ(2, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 17));
        EXPECT_EQ(1, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 18));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 19));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 20));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 21));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 30));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 39));
        EXPECT_EQ(15, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 40));
        EXPECT_EQ(14, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 41));
        EXPECT_EQ(2, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 53));
        EXPECT_EQ(1, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 54));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 55));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 56));
        EXPECT_EQ(0, HyoutaUtils::DualRangeGetValidRangeLengthAt<uint64_t>(r, 100));
    }
}
