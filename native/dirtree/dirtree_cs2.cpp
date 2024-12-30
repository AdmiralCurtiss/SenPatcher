#include "dirtree_cs2.h"

#include "generated/internal_dirtree_cs2.h"

#include "dirtree/entry.h"
#include "dirtree/tree.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen2 {
static constexpr const char* s_version_names[] = {
    "v1.0",
    "v1.1",
    "v1.2",
    "v1.3",
    "v1.3.1",
    "v1.4",
    "v1.4.1",
    "v1.4.2",
};
static constexpr const char* s_dlc_names[] = {
    "All Casual Clothes",
    "All Ride-Alongs",
    "All Accessories",
    "All Glasses",
    "All Arcus Covers",
    "Unspeakable Costumes",
    "Shining Pom Bait Set 1",
    "Shining Pom Bait Set 2",
    "Shining Pom Bait Set 3",
    "Shining Pom Bait Set 4",
    "Shining Pom Bait Set 5",
    "Shining Pom Bait Value Set 1",
    "Shining Pom Bait Value Set 2",
    "Cold Steel Costume Pack",
    "Trails in the Sky Costumes",
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
} // namespace SenLib::Sen2
