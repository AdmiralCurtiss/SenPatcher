#include "dirtree_cs1.h"

#include "dirtree/entry.h"
#include "dirtree/tree.h"
#include "util/hash/sha1.h"

#ifdef HAS_COMPRESSED_DIRTREE_CS1
#include "init_dirtree_from_buffer.h"
#include "sen/decompress_helper.h"

static constexpr char CompressedDirtreeData[] = {
#include "embed_compressed_dirtree_cs1.h"
};
static constexpr size_t CompressedDirtreeLength = sizeof(CompressedDirtreeData);
#else
#include "generated/internal_dirtree_cs1.h"
#endif

namespace SenLib::Sen1 {
static constexpr const char* s_version_names[] = {
    "v1.0",
    "v1.1",
    "v1.2.1",
    "v1.3",
    "v1.3.5",
    "v1.4",
    "v1.5",
    "v1.6",
};
static constexpr const char* s_dlc_names[] = {
    "Costume Pack",          // GOG name; on Humble this is listed as "Class VII Casuals Set"
    "Special Item Pack",     // GOG name; on Humble this is listed as "Special Items Bundle"
    "Shining Pom Bait Pack", // GOG name; on Humble this is listed as "Shining Pom Baits"
};

HyoutaUtils::DirTree::Tree GetDirTree() {
#ifdef HAS_COMPRESSED_DIRTREE_CS1
    static auto s_decompressed =
        SenLib::AlignedDecompressFromBuffer(CompressedDirtreeData, CompressedDirtreeLength, 16);
    return HyoutaUtils::DirTree::InitDirTreeFromBuffer(
        s_decompressed ? s_decompressed->AlignedData : nullptr,
        s_decompressed ? s_decompressed->Length : 0,
        s_version_names,
        sizeof(s_version_names) / sizeof(s_version_names[0]),
        s_dlc_names,
        sizeof(s_dlc_names) / sizeof(s_dlc_names[0]));
#else
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
#endif
}
} // namespace SenLib::Sen1
