#include <string>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen5/tbl.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen5::FileFixes::t_text {
std::string_view GetDescription() {
    return "Fix a few system messages.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_text.tbl",
            67378,
            SenPatcher::SHA1FromHexString("6c9e20fc893472c7f6df9d1a4779e54dbc65b2fe"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        std::vector<TextTableData> tmp;
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            tmp.emplace_back(e.Data.data(), e.Data.size());
        }

        // the 1.1.4 update improves these lines to "%s is currently using this main/sub master
        // quartz", which is *better* than the previous line because it's no longer nonsense but
        // it's still wrong; it's the *slot* that's main/sub, not the quartz. so just apply our
        // standard fix...
        for (size_t i = 0; i < 2; ++i) {
            auto& e = tbl_en.Entries.at(191 + i);
            TextTableData m(e.Data.data(), e.Data.size());
            m.str = "#1C%s has this master quartz equipped in the #3C";
            m.str += (i == 0 ? "main" : "sub");
            m.str += "#1C slot.\nExchange?";
            e.Data = m.ToBinary();
        }

        // S-Break setting lines, still using the bizarre phrasing from CS3
        {
            auto& e = tbl_en.Entries.at(160);
            TextTableData m(e.Data.data(), e.Data.size());
            m.str = " will be set as S-Break.";
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries.at(161);
            TextTableData m(e.Data.data(), e.Data.size());
            m.str = " is already set as S-Break.";
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries.at(236);
            TextTableData m(e.Data.data(), e.Data.size());
            std::string s;
            s += std::string_view(m.str).substr(0, 9);
            s += "Set as S-Break";
            s += std::string_view(m.str).substr(m.str.size() - 5);
            m.str = std::move(s);
            e.Data = m.ToBinary();
        }

        // This line is incorrectly a duplicate 'can now be used.' when it's supposed to be the
        // 'newly learned S-Craft is now set as S-Break' message.
        {
            auto& e = tbl_en.Entries.at(893);
            TextTableData m(e.Data.data(), e.Data.size());
            m.str = (m.str.substr(0, 12) + " was registered as S-Break.");
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
} // namespace SenLib::Sen5::FileFixes::t_text
