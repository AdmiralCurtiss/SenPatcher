#include <filesystem>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen3/file_fixes.h"
#include "sen3/tbl.h"
#include "sha1.h"
#include "util/text.h"

namespace SenLib::Sen3::FileFixes::t_magic {
std::string_view GetDescription() {
    return "Fix formatting issues in art/craft descriptions.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    using namespace HyoutaUtils::TextUtils;

    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_magic.tbl",
            85506,
            SenPatcher::SHA1FromHexString("c0b07b04d56268a7c42471d6671dc5cb6243286f"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
        //     auto& e = tbl_en.Entries[i];
        //     if (e.Name == "magic") {
        //         MagicData m(e.Data.data(), e.Data.size());
        //         m.flags = Replace(m.flags, "Z", "");
        //         m.name = Replace(m.name, "\n", " ");
        //         e.Data = m.ToBinary();
        //     }
        // }

        // a bunch of missing or extra spaces
        {
            auto& e = tbl_en.Entries[64];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = InsertSubstring(m.desc, 64, " ", 0, 1);
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[126];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = Remove(m.desc, 83, 1);
            e.Data = m.ToBinary();
        }
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "magic") {
                MagicData m(e.Data.data(), e.Data.size());
                if (m.desc.find(" -#") != std::string::npos) {
                    m.desc = Replace(m.desc, " -#", " - #");
                    e.Data = m.ToBinary();
                }
                if (m.desc.find(" #0C ") != std::string::npos) {
                    m.desc = Replace(m.desc, " #0C ", "#0C ");
                    e.Data = m.ToBinary();
                }
            }
        }

        // Radiant Wings seems to have its description incorrectly copied from CS2
        // PS4 patch 1.03 changes it to "Stokes the fighting spirit, increasing one's strength.",
        // use that
        {
            const auto get_name_substr =
                [&](size_t idx, size_t offset, size_t length) -> std::string {
                auto& e = tbl_en.Entries[idx];
                return MagicData(e.Data.data(), e.Data.size()).name.substr(offset, length);
            };
            const auto get_desc_substr =
                [&](size_t idx, size_t offset, size_t length) -> std::string {
                auto& e = tbl_en.Entries[idx];
                return MagicData(e.Data.data(), e.Data.size()).desc.substr(offset, length);
            };

            auto& e = tbl_en.Entries[89];
            MagicData m(e.Data.data(), e.Data.size());
            std::string s = "";
            s += get_name_substr(109, 5, 3);
            s += get_desc_substr(152, 147, 8);
            s += get_desc_substr(35, 86, 15);
            s += get_desc_substr(24, 115, 4);
            s += m.desc.substr(87, 5);
            s += get_desc_substr(168, 37, 6);
            s += get_desc_substr(231, 30, 4);
            s += m.desc.substr(95, 8);
            s += m.desc.back();
            m.desc = ReplaceSubstring(m.desc, 85, 56, s, 0, s.size());
            e.Data = m.ToBinary();
        }

        // normalize newlines
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "magic") {
                MagicData m(e.Data.data(), e.Data.size());
                if (m.desc.starts_with("[") && m.flags.find_first_of('Z') != std::string::npos
                    && m.desc.find_first_of('\n') != std::string::npos) {
                    m.desc = AdjustNewlinesToTwoSpaces(std::move(m.desc));
                    e.Data = m.ToBinary();
                }
            }
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
} // namespace SenLib::Sen3::FileFixes::t_magic
