#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen4/tbl.h"
#include "sha1.h"
#include "util/text.h"

namespace SenLib::Sen4::FileFixes::t_item {
std::string_view GetDescription() {
    return "Fix incorrect item descriptions and inconsistent formatting.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    using namespace HyoutaUtils::TextUtils;

    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_item_en.tbl",
            484211,
            SenPatcher::SHA1FromHexString("4e83e0152b272f6e7739c89a07c1b0c5e2499e8a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
        //     auto& e = tbl_en.Entries[i];
        //     if (e.Name == "item" || e.Name == "item_q") {
        //         ItemData m(e.Data.data(), e.Data.size());
        //         m.flags = Replace(m.flags, "Z", "");
        //         m.desc = Replace(m.desc, "\n", " ");
        //         e.Data = m.ToBinary();
        //     }
        // }

        // Tasty Potato Chowder
        {
            auto& e = tbl_en.Entries[1468];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = ReplaceSubstring(m.desc, 39, 2, "15", 0, 2); // 2000 HP -> 1500 HP
            e.Data = m.ToBinary();
        }

        // Seraph
        {
            auto& e = tbl_en.Entries[1211];
            ItemData m(e.Data.data(), e.Data.size());
            // inconsistent formatting
            m.desc = Remove(m.desc, 61, 1);
            m.desc = ReplaceSubstring(m.desc, 112, 2, "/", 0, 1);
            m.desc = Remove(m.desc, 127, 1);
            e.Data = m.ToBinary();
        }

        // Luck
        {
            auto& e = tbl_en.Entries[1198];
            ItemData m(e.Data.data(), e.Data.size());
            // inconsistent formatting
            m.desc = Remove(m.desc, 59, 1);
            e.Data = m.ToBinary();
        }

        // Bluster
        {
            auto& e = tbl_en.Entries[1118];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Remove(m.desc, 97, 1); // 1 more turns -> 1 more turn
            e.Data = m.ToBinary();
        }

        // Aeolus Gem
        {
            auto& e = tbl_en.Entries[1134];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Remove(m.desc, 125, 1); // 1 more turns -> 1 more turn
            e.Data = m.ToBinary();
        }

        // Iron Winged Lion Medal
        {
            auto& e = tbl_en.Entries[677];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Insert(m.desc, 25, " "); // missing space before parens
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
} // namespace SenLib::Sen4::FileFixes::t_item
