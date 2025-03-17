#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen3/file_fixes.h"
#include "sen3/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_magic[] =
    "Fix formatting issues in art/craft descriptions.";
}

namespace SenLib::Sen3::FileFixes::t_magic {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    using namespace HyoutaUtils::TextUtils;

    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_magic.tbl",
            85506,
            HyoutaUtils::Hash::SHA1FromHexString("c0b07b04d56268a7c42471d6671dc5cb6243286f"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

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

        // fix Juna's Gunner Mode craft descriptions using inconsistent formatting
        {
            auto& e = tbl_en.Entries[129];
            auto& e2 = tbl_en.Entries[124];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.desc = m2.desc;
            m.flags += 'Z';
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[130];
            auto& e2 = tbl_en.Entries[125];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.desc = m2.desc;
            m.flags += 'Z';
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[131];
            auto& e2 = tbl_en.Entries[126];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.desc = m2.desc;
            m.flags += 'Z';
            e.Data = m.ToBinary();
        }

        // Spirit and Charge actions in Divine Knight battles are missing detailed descriptions.
        {
            auto& e = tbl_en.Entries[231];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc =
                "[ Recovery - #11CSelf#0C - #11CCures ailments#0C - #11CRestores 50% HP#0C - "
                "#11CCP+30#0C ]\n"
                + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[232];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc =
                "[ Recovery - #11CSelf#0C - #11CCures ailments#0C - #11CRestores 50% HP#0C - "
                "#11CCP+30#0C ]\n"
                + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[233];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = "[ Recovery - #11CSelf#0C - #11CRestores 500EP#0C ]\n" + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[234];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = "[ Recovery - #11CSelf#0C - #11CRestores 500EP#0C ]\n" + m.desc;
            e.Data = m.ToBinary();
        }

        // Defend action uses the autogenerated description but we'd like this to say 'Self'.
        {
            auto& e = tbl_en.Entries[230];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc = "[ Support - #11CSelf#0C - #11CDamage taken -80%#0C ]\n" + m.desc;
            e.Data = m.ToBinary();
        }

        // Tempete Rouge, Moulin Rouge: This says "Impede/Burn (100%)" but Impede should be listed
        // separately from ailments as it's kind of a different thing.
        {
            auto& e = tbl_en.Entries[252];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 80, 1, m.desc, 85, 17);
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[253];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 81, 1, m.desc, 86, 17);
            e.Data = m.ToBinary();
        }

        // Resounding Beat: Autogenerated, copy from Resonant Beat
        {
            auto& e = tbl_en.Entries[282];
            auto& e2 = tbl_en.Entries[283];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 85) + m.desc;
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 20, 3, "Self");
            e.Data = m.ToBinary();
        }

        // Unbound Rage: Target indicator is missing.
        {
            auto& e = tbl_en.Entries[297];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 15, "#11CSelf#0C - ");
            e.Data = m.ToBinary();
        }

        // Wild Rage: Autogenerated, copy from Unbound Rage
        {
            auto& e = tbl_en.Entries[289];
            auto& e2 = tbl_en.Entries[297];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 94) + m.desc;
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 29, 32);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 36, 1, "12");
            e.Data = m.ToBinary();
        }

        // All of these say 'One' as the target but they can only target yourself, so it'd be nicer
        // to say 'Self'.
        for (int idx : {284, 285, 288, 290, 295, 296, 298, 300}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            auto pos = m.desc.find("One");
            if (pos != std::string::npos) {
                m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, pos, 3, "Self");
            }
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


        // remove Z flag to compare with autogenerated descriptions
        // for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
        //     auto& e = tbl_en.Entries[i];
        //     if (e.Name == "magic") {
        //         MagicData m(e.Data.data(), e.Data.size());
        //         m.flags = Replace(m.flags, "Z", "");
        //         m.name = Replace(m.name, "\n", " ");
        //         e.Data = m.ToBinary();
        //     }
        // }


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
} // namespace SenLib::Sen3::FileFixes::t_magic
