#include <algorithm>
#include <cstring>
#include <memory>

#include "gtest/gtest.h"

#include "modload/loaded_mods.h"
#include "p3a/p3a.h"
#include "p3a/structs.h"
#include "util/xorshift.h"

TEST(P3A, BinarySearchLookup) {
    using namespace SenLib::ModLoad;

    static constexpr size_t maxFileCount = 32;
    P3AData p3a;
    LoadedModsData mods;
    p3a.Archive.FileInfo = std::make_unique<SenPatcher::P3AFileInfo[]>(maxFileCount);
    p3a.Archive.FileCount = maxFileCount;
    mods.CombinedFileInfos = std::make_unique<P3AFileRef[]>(maxFileCount);

    HyoutaUtils::RNG::xorshift rng;
    for (size_t i = 0; i < maxFileCount; ++i) {
        mods.CombinedFileInfos[i].ArchiveData = &p3a;
        mods.CombinedFileInfos[i].FileInfo = &p3a.Archive.FileInfo[i];

        p3a.Archive.FileInfo[i].Filename.fill('\0');
        p3a.Archive.FileInfo[i].CompressionType = SenPatcher::P3ACompressionType::None;
        p3a.Archive.FileInfo[i].CompressedSize = i;
        p3a.Archive.FileInfo[i].UncompressedSize = i;
        p3a.Archive.FileInfo[i].Offset = i;
        p3a.Archive.FileInfo[i].Hash = 0;

        for (size_t j = 0; j < 16; ++j) {
            p3a.Archive.FileInfo[i].Filename[j] = static_cast<char>((rng() % 26) + 'a');
        }
    }

    std::array<char, 0x100> otherFilename;
    otherFilename.fill(0);
    for (size_t j = 0; j < 16; ++j) {
        otherFilename[j] = static_cast<char>((rng() % 26) + 'a');
    }

    // precondition for search
    std::stable_sort(mods.CombinedFileInfos.get(),
                     mods.CombinedFileInfos.get() + maxFileCount,
                     [](const P3AFileRef& lhs, const P3AFileRef& rhs) {
                         return strncmp(lhs.FileInfo->Filename.data(),
                                        rhs.FileInfo->Filename.data(),
                                        lhs.FileInfo->Filename.size())
                                < 0;
                     });


    // search in 0 files should not crash or anything like that
    mods.CombinedFileInfoCount = 0;
    EXPECT_EQ(nullptr, FindP3AFileRef(mods, otherFilename));
    EXPECT_EQ(nullptr, FindP3AFileRef(mods, mods.CombinedFileInfos[0].FileInfo[0].Filename));

    // should always find a file that exists, and not a file that does not exist
    for (size_t i = 1; i < maxFileCount - 1; ++i) {
        mods.CombinedFileInfoCount = i;

        EXPECT_EQ(nullptr, FindP3AFileRef(mods, otherFilename));
        for (size_t j = 0; j < i; ++j) {
            EXPECT_EQ(&mods.CombinedFileInfos[j],
                      FindP3AFileRef(mods, mods.CombinedFileInfos[j].FileInfo->Filename));
        }
        EXPECT_EQ(nullptr, FindP3AFileRef(mods, mods.CombinedFileInfos[i + 1].FileInfo->Filename));
    }

    return;
}
