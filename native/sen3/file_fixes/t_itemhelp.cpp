#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen3/file_fixes.h"
#include "sen3/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"

namespace SenLib::Sen3::FileFixes::t_itemhelp {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_itemhelp.tbl",
            9902,
            HyoutaUtils::Hash::SHA1FromHexString("cb9135407b8264ac813e921329374a844f55036b"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // Add a space for things like "Delay+10" -> "Delay +10"
        for (int idx : {
                 19,  20,  119, 120, 121, 139, 140, 141, 142, 143,
                 149, 153, 154, 155, 161, 169, 170, 171, 172,
             }) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            if (e.Name == "ItemHelpData") {
                ItemHelpData m(e.Data.data(), e.Data.size());
                bool modified = false;
                size_t endpos = m.str.size();
                while (true) {
                    auto desc = std::string_view(m.str).substr(0, endpos);
                    endpos = desc.find_last_of('+');
                    if (endpos == std::string::npos || endpos == 0) {
                        break;
                    }

                    const char c = m.str[endpos - 1];
                    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                        m.str = HyoutaUtils::TextUtils::Insert(m.str, endpos, " ");
                        modified = true;
                    }
                }
                if (modified) {
                    e.Data = m.ToBinary();
                }
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
} // namespace SenLib::Sen3::FileFixes::t_itemhelp
