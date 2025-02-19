#include "init_dirtree_from_buffer.h"

#include <bit>
#include <cassert>

#include "dirtree/tree.h"

#include "util/memread.h"

namespace HyoutaUtils::DirTree {
HyoutaUtils::DirTree::Tree InitDirTreeFromBuffer(const char* buffer,
                                                 size_t length,
                                                 const char* const* versionNames,
                                                 size_t numberOfVersions,
                                                 const char* const* dlcNames,
                                                 size_t numberOfDlcs) {
    // TODO: In C++23 this can use std::start_lifetime_as() to avoid the UB reinterpret_cast.

    assert((std::bit_cast<size_t>(buffer) % 16) == 0); // must be aligned
    if (length > 16) {
        const size_t raw_dirtree_entry_count = HyoutaUtils::MemRead::ReadUInt32(buffer);
        const size_t raw_dirtree_hash_count = HyoutaUtils::MemRead::ReadUInt32(buffer + 4);
        const size_t raw_dirtree_string_size = HyoutaUtils::MemRead::ReadUInt32(buffer + 8);
        const size_t max_dlc_index = HyoutaUtils::MemRead::ReadUInt32(buffer + 12);
        const char* raw_dirtree = &buffer[16];
        const size_t buffer_min_length = raw_dirtree_entry_count * 16 + raw_dirtree_hash_count * 20
                                         + raw_dirtree_string_size + 16;
        if (length >= buffer_min_length) {
            return HyoutaUtils::DirTree::Tree{
                .Entries = reinterpret_cast<const HyoutaUtils::DirTree::Entry*>(&raw_dirtree[0]),
                .NumberOfEntries = raw_dirtree_entry_count,
                .StringTable =
                    &raw_dirtree[raw_dirtree_entry_count * 16 + raw_dirtree_hash_count * 20],
                .StringTableLength = raw_dirtree_string_size,
                .HashTable = reinterpret_cast<const HyoutaUtils::Hash::SHA1*>(
                    &raw_dirtree[raw_dirtree_entry_count * 16]),
                .HashTableLength = raw_dirtree_hash_count,
                .VersionNames = versionNames,
                .NumberOfVersions = numberOfVersions,
                .DlcNames = dlcNames,
                .NumberOfDlcs = (max_dlc_index < numberOfDlcs ? max_dlc_index : numberOfDlcs),
            };
        }
    }
    return HyoutaUtils::DirTree::Tree{
        .Entries = nullptr,
        .NumberOfEntries = 0,
        .StringTable = nullptr,
        .StringTableLength = 0,
        .HashTable = nullptr,
        .HashTableLength = 0,
        .VersionNames = versionNames,
        .NumberOfVersions = numberOfVersions,
        .DlcNames = dlcNames,
        .NumberOfDlcs = numberOfDlcs,
    };
}
} // namespace HyoutaUtils::DirTree
