#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen4/tbl.h"
#include "sha1.h"

namespace SenLib::Sen4::FileFixes::t_text {
std::string_view GetDescription() {
    return "Fix a few system messages.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              bool allowSwitchToNightmare) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_text.tbl",
            41747,
            SenPatcher::SHA1FromHexString("ed242395ee9b5aa15f11bf83138b161bb116d20a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // std::vector<TextTableData> tmp;
        // for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
        //     auto& e = tbl_en.Entries[i];
        //     tmp.emplace_back(e.Data.data(), e.Data.size());
        // }

        if (allowSwitchToNightmare) {
            auto& e = tbl_en.Entries.at(110);
            TextTableData m(e.Data.data(), e.Data.size());
            size_t newlinepos = m.str.find_first_of('\n');
            if (newlinepos != std::string::npos) {
                m.str = m.str.substr(0, newlinepos);
            }
            e.Data = m.ToBinary();
        }

        // this description is gibberish because someone assumed %s is the MQ name instead of the
        // character name, rewrite it so it makes sense
        for (size_t i = 0; i < 2; ++i) {
            auto& e = tbl_en.Entries.at(144 + i);
            TextTableData m(e.Data.data(), e.Data.size());
            m.str = "#1C%s has this master quartz equipped in the #3C";
            m.str += (i == 0 ? "main" : "sub");
            m.str += "#1C slot.\nExchange?";
            e.Data = m.ToBinary();
        }

        // S-Break setting lines, these make no sense and were probably not noticed because CS3
        // doesn't have settable S-Breaks (but still has these lines)
        {
            auto& e = tbl_en.Entries.at(123);
            TextTableData m(e.Data.data(), e.Data.size());
            m.str = " will be set as S-Break.";
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries.at(124);
            TextTableData m(e.Data.data(), e.Data.size());
            m.str = " is already set as S-Break.";
            e.Data = m.ToBinary();
        }

        // This line is incorrectly a duplicate 'can now be used.' when it's supposed to be the
        // 'newly learned S-Craft is now set as S-Break' message.
        {
            auto& e = tbl_en.Entries.at(709);
            TextTableData m(e.Data.data(), e.Data.size());
            m.str = " was registered as S-Break.";
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
} // namespace SenLib::Sen4::FileFixes::t_text
