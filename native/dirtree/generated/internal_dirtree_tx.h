#pragma once

#include <array>

#include "dirtree/entry.h"
#include "util/hash/sha1.h"

static constexpr auto s_dirtree = std::array<HyoutaUtils::DirTree::Entry, 34>{{
HyoutaUtils::DirTree::Entry::MakeDirectory(0, 0, 31, 0, 1, 18),
HyoutaUtils::DirTree::Entry::MakeFile(118, 10, 31, 0, 6, 877325813), // Asset1.bra
HyoutaUtils::DirTree::Entry::MakeFile(128, 10, 3, 0, 9, 499015689), // Asset2.bra
HyoutaUtils::DirTree::Entry::MakeFile(128, 10, 28, 0, 13, 499000182), // Asset2.bra
HyoutaUtils::DirTree::Entry::MakeFile(138, 10, 31, 0, 11, 1653278165), // Asset3.bra
HyoutaUtils::DirTree::Entry::MakeFile(148, 10, 31, 0, 28, 785690772), // Asset4.bra
HyoutaUtils::DirTree::Entry::MakeFile(178, 9, 31, 0, 24, 716768383), // Audio.bra
HyoutaUtils::DirTree::Entry::MakeFile(158, 10, 16, 0, 16, 4730880), // Galaxy.dll
HyoutaUtils::DirTree::Entry::MakeFile(73, 12, 28, 0, 7, 555182718), // Japanese.bra
HyoutaUtils::DirTree::Entry::MakeDirectory(196, 5, 31, 0, 19, 8), // movie
HyoutaUtils::DirTree::Entry::MakeDirectory(196, 7, 28, 0, 27, 7), // movieJP
HyoutaUtils::DirTree::Entry::MakeFile(60, 13, 15, 0, 14, 225056), // steam_api.dll
HyoutaUtils::DirTree::Entry::MakeFile(168, 10, 1, 0, 3, 69753366), // System.bra
HyoutaUtils::DirTree::Entry::MakeFile(168, 10, 2, 0, 12, 69753391), // System.bra
HyoutaUtils::DirTree::Entry::MakeFile(168, 10, 28, 0, 25, 69753379), // System.bra
HyoutaUtils::DirTree::Entry::MakeFile(17, 15, 3, 0, 26, 7363728), // TokyoXanadu.exe
HyoutaUtils::DirTree::Entry::MakeFile(17, 15, 4, 0, 10, 7373456), // TokyoXanadu.exe
HyoutaUtils::DirTree::Entry::MakeFile(17, 15, 8, 0, 15, 7373456), // TokyoXanadu.exe
HyoutaUtils::DirTree::Entry::MakeFile(17, 15, 16, 0, 19, 7225344), // TokyoXanadu.exe
HyoutaUtils::DirTree::Entry::MakeFile(32, 14, 31, 0, 29, 54043326), // ev06_00_00.wmv
HyoutaUtils::DirTree::Entry::MakeFile(46, 14, 31, 0, 17, 37874313), // ev06_15_01.wmv
HyoutaUtils::DirTree::Entry::MakeFile(85, 11, 31, 0, 4, 65056403), // tx_ed_n.wmv
HyoutaUtils::DirTree::Entry::MakeFile(96, 11, 3, 0, 23, 282630005), // tx_ed_t.wmv
HyoutaUtils::DirTree::Entry::MakeFile(96, 11, 28, 0, 5, 277473440), // tx_ed_t.wmv
HyoutaUtils::DirTree::Entry::MakeFile(107, 11, 31, 0, 27, 1535158), // tx_logo.wmv
HyoutaUtils::DirTree::Entry::MakeFile(0, 17, 31, 0, 1, 521617), // tx_logo_aksys.wmv
HyoutaUtils::DirTree::Entry::MakeFile(187, 9, 31, 0, 21, 116196714), // tx_op.wmv
HyoutaUtils::DirTree::Entry::MakeFile(32, 14, 28, 0, 18, 51503154), // ev06_00_00.wmv
HyoutaUtils::DirTree::Entry::MakeFile(46, 14, 28, 0, 0, 36419522), // ev06_15_01.wmv
HyoutaUtils::DirTree::Entry::MakeFile(85, 11, 28, 0, 2, 49299195), // tx_ed_n.wmv
HyoutaUtils::DirTree::Entry::MakeFile(96, 11, 28, 0, 20, 272285332), // tx_ed_t.wmv
HyoutaUtils::DirTree::Entry::MakeFile(107, 11, 28, 0, 8, 1562559), // tx_logo.wmv
HyoutaUtils::DirTree::Entry::MakeFile(0, 17, 28, 0, 1, 521617), // tx_logo_aksys.wmv
HyoutaUtils::DirTree::Entry::MakeFile(187, 9, 28, 0, 22, 96611962), // tx_op.wmv
}};

static constexpr auto s_stringtable = std::array<char, 203>{{
116, 120, 95, 108, 111, 103, 111, 95, 97, 107, 115, 121, 115, 46, 119, 109, 118, 84, 111, 107, 121, 111, 88, 97, 110, 97, 100, 117, 46, 101, 120, 101,
101, 118, 48, 54, 95, 48, 48, 95, 48, 48, 46, 119, 109, 118, 101, 118, 48, 54, 95, 49, 53, 95, 48, 49, 46, 119, 109, 118, 115, 116, 101, 97,
109, 95, 97, 112, 105, 46, 100, 108, 108, 74, 97, 112, 97, 110, 101, 115, 101, 46, 98, 114, 97, 116, 120, 95, 101, 100, 95, 110, 46, 119, 109, 118,
116, 120, 95, 101, 100, 95, 116, 46, 119, 109, 118, 116, 120, 95, 108, 111, 103, 111, 46, 119, 109, 118, 65, 115, 115, 101, 116, 49, 46, 98, 114, 97,
65, 115, 115, 101, 116, 50, 46, 98, 114, 97, 65, 115, 115, 101, 116, 51, 46, 98, 114, 97, 65, 115, 115, 101, 116, 52, 46, 98, 114, 97, 71, 97,
108, 97, 120, 121, 46, 100, 108, 108, 83, 121, 115, 116, 101, 109, 46, 98, 114, 97, 65, 117, 100, 105, 111, 46, 98, 114, 97, 116, 120, 95, 111, 112,
46, 119, 109, 118, 109, 111, 118, 105, 101, 74, 80
}};

static constexpr auto s_hashtable = std::array<HyoutaUtils::Hash::SHA1, 30>{{
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{3, -23, 76, -5, 10, -70, 73, 89, 5, -31, 20, -56, -74, -11, -19, -83, 23, 115, 110, 44}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{4, 94, -126, -117, 96, -47, -126, 125, 54, 45, 24, -94, 8, 60, -86, -49, -72, 73, -80, -101}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{13, -5, 28, -127, -4, -33, 86, -104, -83, -16, -89, 106, 108, -98, 63, 59, -112, 50, -21, -115}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{16, -79, 96, 100, 1, -33, -20, -124, 30, -38, 43, -81, -126, -50, -63, -57, 8, -14, 92, -1}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{25, -2, 79, -80, 89, 89, 56, 25, -39, 48, 39, -26, 122, 75, -92, 4, 100, 24, 117, -52}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{31, 20, 105, -82, -106, -37, -77, 119, -62, -64, -43, -95, 1, -86, 81, 51, 106, 58, 40, -110}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{63, 111, -28, 41, 101, -125, 16, 98, -66, 14, -115, 14, -84, -76, 78, -27, -40, 53, -41, 113}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{75, -124, -79, -99, 61, 56, -31, -98, 73, -20, 85, 43, -18, -63, 64, -48, -50, -54, -70, 17}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{78, -62, 115, 95, 113, 100, 35, 49, 119, 82, -34, 5, -25, -57, 47, 18, 52, 36, -45, -29}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{83, 56, 5, 22, 78, -45, 38, -28, 45, 54, 92, 16, 27, 53, -123, 57, 35, 11, 11, 74}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{89, -50, 27, -15, 35, -60, -125, -25, -128, 70, -75, 78, 109, 37, 9, -12, 76, 77, -115, 25}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{105, 67, -25, -58, 86, 66, -98, -89, -118, -69, 68, 37, -41, 20, 49, 57, -110, -18, -83, 104}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{113, 0, 26, 22, 109, -54, -110, -82, -47, -81, -97, -41, -97, 125, 19, 3, -18, -30, -24, 100}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{116, 105, 93, -85, -111, -1, 1, 56, 120, -52, -2, -43, 12, 40, 12, 107, 13, 19, -94, -100}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{122, -70, 6, -44, 116, 23, 91, -48, -41, -10, 114, -31, 1, -80, -96, 81, 4, 88, 11, -79}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{124, -33, -18, 123, -8, 22, -10, 121, 125, -84, -5, -26, -28, -74, 97, -27, -52, -96, 90, 18}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{125, 64, 126, -38, 80, -15, -98, 103, -114, -99, 59, -116, 88, -58, 85, 20, 67, -11, -91, 76}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-126, 38, -98, -104, -109, -68, 24, -71, -105, 17, -87, -91, -15, -68, 118, 67, -1, 15, -83, -85}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-117, 51, 47, 107, 75, -51, 13, 0, 69, 96, 19, 75, 2, -1, -88, -81, -54, 77, 93, 11}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-111, 95, -121, -26, -75, -60, -59, -66, -81, 69, -118, -17, -11, -58, -40, -97, 8, -2, -52, -62}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-103, 47, 12, 106, -11, 87, 122, 45, -122, 92, 68, -12, 42, -39, -110, -110, -105, -49, 60, 4}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-97, -113, -20, 114, 45, 48, 125, -38, 1, -34, -23, 56, -112, -94, -32, 23, -109, -61, 7, 93}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-89, 32, -81, -57, 101, -90, -121, 66, 7, 17, 45, 68, -104, 98, 48, 85, 6, -99, -112, -12}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-82, -104, 98, 68, -96, -62, -72, -69, -112, 12, -65, -77, -71, -71, -100, -93, 12, 5, 67, -76}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-71, -48, -105, -79, -68, 64, 12, -23, 33, 9, 19, 115, -11, 45, -9, -75, -99, -113, -73, 0}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-70, 6, 55, -85, -36, 80, -23, 112, -109, -12, -34, -94, 41, 124, 29, -14, 114, -109, 12, -11}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-69, -42, -7, -118, 123, -27, 11, 25, -110, -119, -23, -50, -108, -26, 114, -54, 60, -99, -7, 6}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-48, 110, -65, 123, -40, -33, -59, 92, 95, -40, 100, -13, -61, 71, -11, -58, 46, -104, 62, -37}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-33, -119, 45, -15, -9, 72, -29, 117, 55, 86, 90, -60, 52, -113, -113, 101, 36, 59, -2, 127}})),
HyoutaUtils::Hash::SHA1(std::array<char, 20>({{-31, -81, -34, 90, 19, 55, 124, 119, -54, -18, 12, 17, -48, 59, -79, 44, -23, 111, -119, 127}})),
}};

