#include <string_view>
#include <unordered_map>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen1/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"

#include "text_dat_us_t_item_tbl_t_magic_tbl.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_item_jp[] =
    "Apply series consistency fixes to Japanese item descriptions.";
}

namespace SenLib::Sen1::FileFixes::text_dat_t_item_tbl {
using namespace HyoutaUtils::TextUtils;

static size_t CountCodepoints(std::string_view sv, size_t codepoints) {
    size_t counter = 0;
    for (size_t i = 0; i < sv.size(); ++i) {
        if (counter == codepoints) {
            return i;
        }
        char c = sv[i];
        ++counter;
        if ((c & 0b1110'0000) == 0b1100'0000) {
            ++i;
        } else if ((c & 0b1111'0000) == 0b1110'0000) {
            ++i;
            ++i;
        }
    }
    return sv.size();
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file_item = getCheckedFile(
            "data/text/dat/t_item.tbl",
            129908,
            HyoutaUtils::Hash::SHA1FromHexString("8a735f256c69b0b2cd2b663820953fb49523723e"));
        auto file_magic = getCheckedFile(
            "data/text/dat/t_magic.tbl",
            24434,
            HyoutaUtils::Hash::SHA1FromHexString("2bb6ead07062528187e75724d828ab0fc8336708"));
        if (!file_item || !file_magic) {
            return false;
        }

        auto& bin_item = file_item->Data;
        auto& bin_magic = file_magic->Data;
        Tbl tbl_item(bin_item.data(),
                     bin_item.size(),
                     HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                     HyoutaUtils::TextUtils::GameTextEncoding::ShiftJIS);
        Tbl tbl_magic(bin_magic.data(),
                      bin_magic.size(),
                      HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                      HyoutaUtils::TextUtils::GameTextEncoding::ShiftJIS);

        {
            std::unordered_map<uint16_t, MagicData> magic;
            for (auto& e : tbl_magic.Entries) {
                MagicData m(e.Data.data(),
                            e.Data.size(),
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                            HyoutaUtils::TextUtils::GameTextEncoding::ShiftJIS);
                magic.emplace(m.Idx, std::move(m));
            }

            for (TblEntry& entry : tbl_item.Entries) {
                ItemData item(entry.Data.data(),
                              entry.Data.size(),
                              HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                              HyoutaUtils::TextUtils::GameTextEncoding::ShiftJIS);
                if (item.ItemType == 0xaa) {
                    bool isSingleArt = [&]() {
                        if (!text_dat_us_t_item_tbl_t_magic_tbl::IsSingleArt(item)) {
                            return false;
                        }
                        auto it = magic.find(item.Action2Value1_Art1);
                        return it != magic.end() && it->second.Name == item.Name;
                    }();

                    // series consistency: add R to rare quartzes
                    if (isSingleArt && item.Action2_Rarity == 1) {
                        item.Name += "\xef\xbc\xb2";
                    }

                    // TODO: series consistency: add magic power class to description

                    // series consistency: quartz that just boost a stat and nothing else should say
                    // Stat Boost
                    if (text_dat_us_t_item_tbl_t_magic_tbl::IsStatusQuartz(item)) {
                        item.Desc = Insert(
                            item.Desc, CountCodepoints(item.Desc, 5), "\xe5\xbc\xb7\xe5\x8c\x96");
                    }

                    // series consistency: quartz that have passive effects should say Special
                    if (text_dat_us_t_item_tbl_t_magic_tbl::IsSpecialQuartz(item)) {
                        if (item.Action2_Rarity == 0) {
                            item.Desc = Insert(item.Desc,
                                               CountCodepoints(item.Desc, 5),
                                               "\xe7\x89\xb9\xe6\xae\x8a");
                        } else {
                            item.Desc = Insert(
                                item.Desc,
                                CountCodepoints(item.Desc, item.Action2_Rarity == 1 ? 7u : 8u),
                                " \xe7\x89\xb9\xe6\xae\x8a");
                        }
                    }

                    entry.Data = item.ToBinary(HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                                               HyoutaUtils::TextUtils::GameTextEncoding::ShiftJIS);
                }
            }
        }

        std::vector<char> bin2;
        HyoutaUtils::Stream::MemoryStream ms(bin2);
        tbl_item.WriteToStream(ms,
                               HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                               HyoutaUtils::TextUtils::GameTextEncoding::ShiftJIS);
        result.emplace_back(
            std::move(bin2), file_item->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::text_dat_t_item_tbl
