#include "dirtree_tx.h"

#include "dirtree/entry.h"
#include "dirtree/tree.h"
#include "util/hash/sha1.h"

#ifdef HAS_COMPRESSED_DIRTREE_TX
#include "init_dirtree_from_buffer.h"
#include "sen/decompress_helper.h"

static constexpr char CompressedDirtreeData[] = {
#include "embed_compressed_dirtree_tx.h"
};
static constexpr size_t CompressedDirtreeLength = sizeof(CompressedDirtreeData);
#else
#include "generated/internal_dirtree_tx.h"
#endif

namespace SenLib::TX {
static constexpr const char* s_version_names[] = {
    "Steam v1.00 (initial build)",
    "Steam v1.00 (NG+ fix)",
    "Steam v1.07",
    "Steam v1.08",
    "GOG v1.08",
};

HyoutaUtils::DirTree::Tree GetDirTree() {
#ifdef HAS_COMPRESSED_DIRTREE_TX
    static auto s_decompressed =
        SenLib::AlignedDecompressFromBuffer(CompressedDirtreeData, CompressedDirtreeLength, 16);
    return HyoutaUtils::DirTree::InitDirTreeFromBuffer(
        s_decompressed ? s_decompressed->AlignedData : nullptr,
        s_decompressed ? s_decompressed->Length : 0,
        s_version_names,
        sizeof(s_version_names) / sizeof(s_version_names[0]),
        nullptr,
        0);
#else
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
        .DlcNames = nullptr,
        .NumberOfDlcs = s_max_dlc_index,
    };
#endif
}
} // namespace SenLib::TX
