#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sen/file_getter.h"
#include "sen3/file_fixes.h"
#include "sen3/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_item[] = "Fix incorrect item descriptions.";
}

namespace SenLib::Sen3::FileFixes::t_item {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    using namespace HyoutaUtils::TextUtils;

    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_item_en.tbl",
            311891,
            HyoutaUtils::Hash::SHA1FromHexString("5deee9b833b2bb93b0a326f586943f3d2e2424b9"));
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

        // Sennin Gem
        {
            auto& e = tbl_en.Entries[597];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = ReplaceSubstring(m.desc, 36, 3, "ADF", 0, 3);
            e.Data = m.ToBinary();
        }

        // Seraph
        {
            auto& e = tbl_en.Entries[763];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Remove(m.desc, 65, 1);
            e.Data = m.ToBinary();
        }

        // Solar Gyre
        {
            auto& e = tbl_en.Entries[764];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = ReplaceSubstring(m.desc, 52, 1, "/", 0, 1);
            e.Data = m.ToBinary();
        }

        // Impassion R
        {
            auto& e = tbl_en.Entries[619];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = ReplaceSubstring(m.desc, 75, 1, "0", 0, 1);
            m.desc = ReplaceSubstring(m.desc, 81, 1, "4", 0, 1);
            e.Data = m.ToBinary();
        }

        // Panzer Goggles
        {
            auto& e = tbl_en.Entries[303];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = ReplaceSubstring(m.desc, 28, 1, "10", 0, 2);
            tbl_en.Entries[303].Data = m.ToBinary();
        }

        // Orange Corsage
        {
            auto& e = tbl_en.Entries[318];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Insert(m.desc, 18, "%");
            e.Data = m.ToBinary();
        }

        // Glory Emblem
        {
            auto& e = tbl_en.Entries[391];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Insert(m.desc, 12, "%");
            e.Data = m.ToBinary();
        }

        // Blue Star Lionheart Medal
        {
            auto& e = tbl_en.Entries[395];
            auto& e343 = tbl_en.Entries[343];
            ItemData m(e.Data.data(), e.Data.size());
            std::string basestr = ItemData(e343.Data.data(), e343.Data.size()).desc;
            size_t newlinepos = basestr.find_first_of('\n');
            if (newlinepos != std::string::npos) {
                basestr = basestr.substr(0, newlinepos);
            }
            m.flags = m.flags + "Z";
            m.desc = InsertSubstring(basestr, 6, "STR+100/DEF+100/ATS+100/ADF+100/", 0, 32) + "\n "
                     + m.desc;
            e.Data = m.ToBinary();
        }

        // Shield Potion II
        {
            auto& e = tbl_en.Entries[34];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = ReplaceSubstring(m.desc, 54, 2, "30", 0, 2);
            e.Data = m.ToBinary();
        }

        // Energy Drink
        {
            auto& e = tbl_en.Entries[38];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = ReplaceSubstring(m.desc, 39, 1, "5", 0, 1);
            m.desc = ReplaceSubstring(m.desc, 58, 1, "C", 0, 1);
            e.Data = m.ToBinary();
        }

        // Steel Bell
        {
            auto& e = tbl_en.Entries[541];
            auto& e501 = tbl_en.Entries[501];
            ItemData m(e.Data.data(), e.Data.size());
            std::string n = ItemData(e501.Data.data(), e501.Data.size()).name;
            m.desc = ReplaceSubstring(m.desc, 10, 7, n, 0, n.size());
            e.Data = m.ToBinary();
        }

        // A bunch of items that claim EP when they should be CP
        for (int idx : {
                 51,   52,   53,  54,  61,  62,   63,   64,   65,   66,   67,   1026, 1029, 1034,
                 1040, 1044, 988, 991, 996, 1002, 1006, 1015, 1019, 1022, 1048, 1051, 1057, 1066,
             }) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Replace(m.desc, "EP", "CP");
            e.Data = m.ToBinary();
        }

        // Extra comma in Sandy-grown stuff
        for (int idx : {68, 69, 70}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Replace(m.desc, ", ", " ");
            e.Data = m.ToBinary();
        }

        // normalize newlines
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "item" || e.Name == "item_q") {
                ItemData m(e.Data.data(), e.Data.size());
                if (m.desc.starts_with("[") && m.flags.find_first_of('Z') != std::string::npos
                    && m.desc.find_first_of('\n') != std::string::npos) {
                    m.desc = AdjustNewlinesToTwoSpaces(m.desc);
                    e.Data = m.ToBinary();
                }
            }
        }


        std::vector<char> result_en_vec;
        HyoutaUtils::Stream::MemoryStream result_en(result_en_vec);
        tbl_en.WriteToStream(result_en, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // there's two identical copies in the game files
        result.emplace_back(result_en_vec, file->Filename, SenPatcher::P3ACompressionType::LZ4);
        result.emplace_back(std::move(result_en_vec),
                            SenPatcher::InitializeP3AFilename("data/text/dat_fr/t_item_en.tbl"),
                            SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t_item
