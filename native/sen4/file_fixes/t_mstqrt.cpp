#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen4/tbl.h"
#include "sha1.h"
#include "util/text.h"

namespace SenLib::Sen4::FileFixes::t_mstqrt {
std::string_view GetDescription() {
    return "Fix one incorrect Master Quartz effect description.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    using namespace HyoutaUtils::TextUtils;

    try {
        auto file_en = getCheckedFile(
            "data/text/dat_en/t_mstqrt.tbl",
            40683,
            SenPatcher::SHA1FromHexString("40ae4f525021b550b77e045a00841a42bc460a77"));
        if (!file_en) {
            return false;
        }

        auto& bin = file_en->Data;

        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // Katze says attack/craft instead of magic for its second effect
        {
            auto& e = tbl_en.Entries[323];
            const auto& e2 = tbl_en.Entries[325];
            MasterQuartzMemo m(e.Data.data(), e.Data.size());
            m.str = ReplaceSubstring(
                m.str, 1, 14, MasterQuartzMemo(e2.Data.data(), e2.Data.size()).str, 1, 14);
            e.Data = m.ToBinary();
        }

        std::vector<char> result_en_vec;
        MemoryStream result_en(result_en_vec);
        tbl_en.WriteToStream(result_en, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(
            std::move(result_en_vec), file_en->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen4::FileFixes::t_mstqrt
