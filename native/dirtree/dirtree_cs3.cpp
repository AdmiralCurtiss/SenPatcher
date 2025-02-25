#include "dirtree_cs3.h"

#include "dirtree/entry.h"
#include "dirtree/tree.h"
#include "util/hash/sha1.h"

#ifdef HAS_COMPRESSED_DIRTREE_CS3
#include "init_dirtree_from_buffer.h"
#include "sen/decompress_helper.h"

static constexpr char CompressedDirtreeData[] = {
#include "embed_compressed_dirtree_cs3.h"
};
static constexpr size_t CompressedDirtreeLength = sizeof(CompressedDirtreeData);
#else
#include "generated/internal_dirtree_cs3.h"
#endif

namespace SenLib::Sen3 {
static constexpr const char* s_version_names[] = {
    "v1.03",
    "v1.04",
    "v1.05",
    "v1.06",
    "v1.07",
};
static constexpr const char* s_dlc_names[] = {
    "Angel Set",
    "Advanced Medicine Set 1",
    "Altina's \"Kitty Noir\" Costume",
    "Advanced Medicine Set 2",
    "Free Sample Set A",
    "Free Sample Set B",
    "Faculty Swimsuit Set",
    "Juna's \"Lloyd Bannings\" Costume",
    "Rean's \"Unspeakable\" Costume",
    "Advanced Medicine Set 3",
    "Dragon Incense Set 1",
    "Dragon Incense Set 2",
    "Droplet Set 1",
    "Droplet Set 2",
    "Droplet Set 3",
    "Droplet Set 4",
    "Droplet Set 5",
    "Monster Ingredients Set 1",
    "Monster Ingredients Set 2",
    "Monster Ingredients Set 3",
    "Sepith Set 1",
    "Sepith Set 2",
    "Shining Pom Droplet Set 1",
    "Shining Pom Droplet Set 2",
    "Shining Pom Droplet Set 3",
    "Shining Pom Droplet Set 4",
    "Shining Pom Droplet Set 5",
    "Shining Pom Droplet Value Set 1",
    "Shining Pom Droplet Value Set 2",
    "Shining Pom Droplet Value Set 3",
    "Shining Pom Droplet Value Set 4",
    "Shining Pom Droplet Value Set 5",
    "Spirit Incense Set 1",
    "Spirit Incense Set 2",
    "Spirit Incense Set 3",
    "U-Material Set 1",
    "U-Material Set 2",
    "U-Material Set 3",
    "Useful Accessories Set",
    "Zeram Capsule Set 1",
    "Zeram Capsule Set 2",
    "Zeram Powder Set 1",
    "Zeram Powder Set 2",
    "Zeram Powder Set 3",
    "Altina's Casual Clothes",
    "ARCUS Cover Set A",
    "ARCUS Cover Set B",
    "ARCUS Cover Set C",
    "ARCUS Cover Set D",
    "Ashen Knight Set",
    "Ash's Casual Clothes",
    "Bunny Set",
    "Cool Hair Extension Set",
    "Cute Hair Extension Set",
    "Devil Set",
    "Hardcore Set",
    "Juna's \"Active Red\" Costume",
    "Juna's \"Crossbell Cheer!\" Costume",
    "Juna's Casual Clothes",
    "Kurt's Casual Clothes",
    "Mascot Headgear Set",
    "Mask Set",
    "Musse's \"Coquettish Blue\" Costume",
    "Musse's Casual Clothes",
    "Raccoon Set",
    "Rainbow Hair Set",
    "Rare Eyewear",
    "Rean's Casual Clothes",
    "Rean's Traveling Outfit (Cold Steel II)",
    "Ride-Along Black Rabbit",
    "Ride-Along Dana",
    "Ride-Along Elie",
    "Ride-Along Ozzie",
    "Ride-Along School Renne",
    "Self-Assertion Panels",
    "Snow Leopard Set",
    "Standard Glasses Set",
    "Stylish Sunglasses Set",
    "Thors Main Campus Uniforms",
};

HyoutaUtils::DirTree::Tree GetDirTree() {
#ifdef HAS_COMPRESSED_DIRTREE_CS3
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
} // namespace SenLib::Sen3
