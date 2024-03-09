#include <filesystem>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen3/file_fixes.h"
#include "sen3/tbl.h"
#include "sha1.h"
#include "util/text.h"

namespace SenLib::Sen3::FileFixes::t_name {
std::string_view GetDescription() {
    return "Fix incorrect NPC names.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    using namespace HyoutaUtils::TextUtils;

    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_name.tbl",
            150070,
            SenPatcher::SHA1FromHexString("4ddf1af1f31eeb6c9311804d88b3f779519df457"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // Tally -> Tallys (Zero/Azure/CS4 consistency)
        {
            auto& e = tbl_en.Entries[595];
            NameData m(e.Data.data(), e.Data.size());
            m.name += 's';
            e.Data = m.ToBinary();
        }

        // Second Lieutenant Midrun -> 2nd Lieutenant Midrun (CS4 consistency)
        {
            auto& e = tbl_en.Entries[622];
            NameData m(e.Data.data(), e.Data.size());
            m.name = ReplaceSubstring(m.name, 0, 4, "2", 0, 1);
            e.Data = m.ToBinary();
        }

        // Second Lieutenant Sark -> 2nd Lieutenant Sark (CS4 consistency)
        {
            auto& e = tbl_en.Entries[623];
            NameData m(e.Data.data(), e.Data.size());
            m.name = ReplaceSubstring(m.name, 0, 4, "2", 0, 1);
            e.Data = m.ToBinary();
        }


        std::vector<char> result_en_vec;
        MemoryStream result_en(result_en_vec);
        tbl_en.WriteToStream(result_en, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(result_en_vec), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t_name
