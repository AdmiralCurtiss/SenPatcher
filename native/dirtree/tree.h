#pragma once

#include "dirtree/entry.h"
#include "util/hash/sha1.h"

namespace HyoutaUtils::DirTree {
struct Tree {
    const HyoutaUtils::DirTree::Entry* Entries = nullptr;
    size_t NumberOfEntries = 0;
    const char* StringTable = nullptr;
    const HyoutaUtils::Hash::SHA1* HashTable = nullptr;
};
} // namespace HyoutaUtils::DirTree
