#include "dirtree_cs4.h"

#include "generated/internal_dirtree_cs4.h"

#include "dirtree/entry.h"
#include "dirtree/tree.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen4 {
static constexpr const char* s_version_names[] = {
    "v1.0.2",
    "v1.1.0",
    "v1.1.1",
    "v1.2.0",
    "v1.2.1",
    "v1.2.2",
};
static constexpr const char* s_dlc_names[] = {
    "Self-Assertion Panels Vol. 2",
    "Swimsuit Bundle",
    "Magical Girl Bundle",
    "Hair Extension Set",
    "Attachment Set",
    "Ride-Along Set",
    "Standard Costume Bundle",
    "Headwear Set",
    "Free Sample Set A",
    "Free Sample Set B",
    "Free Sample Set C",
    "Consumable Starter Set",
    "Consumable Value Set",
    "Cold Steel III Costume Bundle",
    "Lossless Audio Pack",
};

HyoutaUtils::DirTree::Tree GetDirTree() {
    static_assert(s_max_dlc_index == (sizeof(s_dlc_names) / sizeof(s_dlc_names[0])));
    return HyoutaUtils::DirTree::Tree{
        .Entries = reinterpret_cast<const HyoutaUtils::DirTree::Entry*>(&s_raw_dirtree[0]),
        .NumberOfEntries = s_raw_dirtree_entry_count,
        .StringTable =
            &s_raw_dirtree[s_raw_dirtree_entry_count * 16 + s_raw_dirtree_hash_count * 20],
        .StringTableLength = s_raw_dirtree_string_size,
        .HashTable = reinterpret_cast<const HyoutaUtils::Hash::SHA1*>(
            &s_raw_dirtree[s_raw_dirtree_entry_count * 16]),
        .HashTableLength = s_raw_dirtree_hash_count,
        .VersionNames = s_version_names,
        .NumberOfVersions = sizeof(s_version_names) / sizeof(s_version_names[0]),
        .DlcNames = s_dlc_names,
        .NumberOfDlcs = s_max_dlc_index,
    };
}
} // namespace SenLib::Sen4
