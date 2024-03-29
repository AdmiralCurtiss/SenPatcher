#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen3/file_fixes.h"
#include "sen3/tbl.h"
#include "sha1.h"
#include "util/text.h"

namespace SenLib::Sen3::FileFixes::t_mons {
std::string_view GetDescription() {
    return "Fix text issues in enemy descriptions.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    using namespace HyoutaUtils::TextUtils;

    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_mons.tbl",
            116969,
            SenPatcher::SHA1FromHexString("e1f521da8d1a98a4e9945859287da9cf2b0cdd38"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
        //     auto& e = tbl_en.Entries[i];
        //     if (e.Name == "status") {
        //         MonsterData m(e.Data.data(), e.Data.size());
        //         e.Data = m.ToBinary();
        //     }
        // }

        // Aion Type-Gamma II incorrectly claims that it wiped out Calvard's army. JP leaves it
        // vague so do that too.
        {
            auto& e = tbl_en.Entries[308];
            MonsterData m(e.Data.data(), e.Data.size());
            m.desc = Remove(m.desc, 51, 5);
            m.desc = Insert(m.desc, 51, "S");
            m.desc = ReplaceSubstring(m.desc, 60, 1, "\n", 0, 1);
            m.desc = Insert(m.desc, 90, "n entire");
            m.desc = InsertSubstring(m.desc, 98, m.desc, 120, 5);
            m.desc = Remove(m.desc, 112, 18);
            e.Data = m.ToBinary();
        }


        std::vector<char> result_en_vec;
        MemoryStream result_en(result_en_vec);
        tbl_en.WriteToStream(result_en, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(
            std::move(result_en_vec), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t_mons
