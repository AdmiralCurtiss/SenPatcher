#pragma once

#include <cstddef>
#include <cstdint>

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

// For iterating over a directory. This blocks out sequential entries that all describe different
// versions of the same file, so it's easy to check a single file in the filesystem against all
// possible variants it could be according to the dirtree.
struct DirectoryIterationState {
    uint32_t Begin = 0;
    uint32_t End = 0;
};
inline bool GetNextEntries(DirectoryIterationState& state,
                           const HyoutaUtils::DirTree::Tree& dirtree,
                           const HyoutaUtils::DirTree::Entry& entry) {
    const uint32_t last = state.End;
    const uint32_t count = entry.GetDirectoryNumberOfEntries();
    if (last == count) {
        return false;
    }

    state.Begin = last;
    const uint32_t start = entry.GetDirectoryFirstEntry();
    for (uint32_t i = last + 1; i < count; ++i) {
        if (dirtree.Entries[start + last].Filename != dirtree.Entries[start + i].Filename) {
            state.End = i;
            return true;
        }
    }
    state.End = count;
    return true;
}
} // namespace HyoutaUtils::DirTree
