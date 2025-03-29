#include <string>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen5/tbl.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_item[] = "Improve formatting of item descriptions.";
}

namespace SenLib::Sen5::FileFixes::t_item {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_item.tbl",
            882731,
            HyoutaUtils::Hash::SHA1FromHexString("c268753e9fcf0eacb56d5627b6c2f099918e1de3"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);


        // replace the separator dot between STR/DEF etc. with a slightly smaller one that's used by
        // the autogenerator, which looks a bit better and is consistent with the autogenerator.
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "item") {
                ItemData m(e.Data.data(), e.Data.size());
                m.desc = HyoutaUtils::TextUtils::Replace(m.desc, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                e.Data = m.ToBinary();
            } else if (e.Name == "item_e") {
                ItemEData m(e.Data.data(), e.Data.size());
                m.item.desc =
                    HyoutaUtils::TextUtils::Replace(m.item.desc, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                e.Data = m.ToBinary();
            } else if (e.Name == "item_q") {
                ItemQData m(e.Data.data(), e.Data.size());
                m.item.desc =
                    HyoutaUtils::TextUtils::Replace(m.item.desc, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                e.Data = m.ToBinary();
            }
        }


        std::vector<char> result_en_vec;
        HyoutaUtils::Stream::MemoryStream result_en(result_en_vec);
        tbl_en.WriteToStream(result_en, HyoutaUtils::EndianUtils::Endianness::LittleEndian);


        result.emplace_back(
            std::move(result_en_vec), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen5::FileFixes::t_item
