#pragma once

#include "dirtree/entry.h"
#include "util/hash/sha1.h"

namespace HyoutaUtils::DirTree {
struct Tree {
    const HyoutaUtils::DirTree::Entry* Entries = nullptr;
    size_t NumberOfEntries = 0;
    const char* StringTable = nullptr;
    size_t StringTableLength = 0;
    const HyoutaUtils::Hash::SHA1* HashTable = nullptr;
    size_t HashTableLength = 0;
    const char* const* VersionNames = nullptr;
    size_t NumberOfVersions = 0;
    const char* const* DlcNames = nullptr;
    size_t NumberOfDlcs = 0;
};
} // namespace HyoutaUtils::DirTree
