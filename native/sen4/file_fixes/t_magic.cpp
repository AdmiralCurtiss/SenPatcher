#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen4/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_magic[] = "Fix errors in craft descriptions.";
}

namespace SenLib::Sen4::FileFixes::t_magic {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_magic.tbl",
            162190,
            HyoutaUtils::Hash::SHA1FromHexString("2c71852245a5d7a10c5c7e687e4a095ac1f54b60"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // Divine Knight stuff first

        // Spirit: Claims 25 CP but actually restores 30. Two copies (mech and partner)
        {
            auto& e = tbl_en.Entries[413];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 83, 2, "30");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[414];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 83, 2, "30");
            e.Data = m.ToBinary();
        }

        // Tempete Rouge: Uses the autogenerated description, which is formatted weirdly. We can
        // copy and adapt the description from Moulin Rouge.
        {
            auto& e = tbl_en.Entries[438];
            auto& e2 = tbl_en.Entries[439];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 108) + m.desc;
            m.desc[48] = '3';
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 33, 2, "D");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 20, 1, "A");
            e.Data = m.ToBinary();
        }

        // Void Breaker/2: Listed as ADF/DEF down but usually DEF is listed first.
        {
            auto& e = tbl_en.Entries[444];
            MagicData m(e.Data.data(), e.Data.size());
            std::swap(m.desc[99], m.desc[103]);
            std::swap(m.desc[100], m.desc[104]);
            std::swap(m.desc[101], m.desc[105]);
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[445];
            MagicData m(e.Data.data(), e.Data.size());
            std::swap(m.desc[98], m.desc[102]);
            std::swap(m.desc[99], m.desc[103]);
            std::swap(m.desc[100], m.desc[104]);
            e.Data = m.ToBinary();
        }

        // Shock Breaker: Autogenerated, copy from Void Breaker
        {
            auto& e = tbl_en.Entries[443];
            auto& e2 = tbl_en.Entries[444];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 129) + m.desc;
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 124, " - #11CDelay +10#0C");
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 70, 23);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 35, 1, "A+");
            e.Data = m.ToBinary();
        }

        // Power Smash: Autogenerated, copy from Chaos Saber
        {
            auto& e = tbl_en.Entries[446];
            auto& e2 = tbl_en.Entries[449];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 96) + m.desc;
            m.desc[48] = '7';
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 35, "+");
            m.desc[21] = 'B';
            e.Data = m.ToBinary();
        }

        // Salamander: Autogenerated, copy from Chaos Saber
        {
            auto& e = tbl_en.Entries[447];
            auto& e2 = tbl_en.Entries[449];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 96) + m.desc;
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 91, " - #11CDelay +10#0C");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 75, 6, "Burn");
            m.desc[48] = '1';
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 34, 1, "A+");
            m.desc[21] = 'S';
            e.Data = m.ToBinary();
        }

        // Resounding Beat: Autogenerated, copy from Resonant Beat
        {
            auto& e = tbl_en.Entries[480];
            auto& e2 = tbl_en.Entries[481];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 92) + m.desc;
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 20, 3, "Self");
            e.Data = m.ToBinary();
        }

        // Wild Rage: Autogenerated, copy from Unbound Rage
        {
            auto& e = tbl_en.Entries[487];
            auto& e2 = tbl_en.Entries[495];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 93) + m.desc;
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 29, 32);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 35, 1, "12");
            e.Data = m.ToBinary();
        }

        // All of these say 'One' as the target but they can only target yourself, so it'd be nicer
        // to say 'Self'.
        for (int idx : {415, 416, 482, 483, 486, 487, 488, 493, 494, 495, 496, 498}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            auto pos = m.desc.find("One");
            if (pos != std::string::npos) {
                m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, pos, 3, "Self");
            }
            e.Data = m.ToBinary();
        }

        // A lot of the Divine Knight attacks have the 'Unblockable' trait that is completely
        // omitted in the English descriptions. Fix that.
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "magic") {
                MagicData m(e.Data.data(), e.Data.size());
                if (m.idx >= 5050 && m.idx <= 5545 && m.effects[1].idx == 192
                    && m.flags.find('Z') != std::string::npos) {
                    auto endpos = m.desc.find("]");
                    if (endpos != std::string::npos && endpos > 0) {
                        m.desc = HyoutaUtils::TextUtils::Insert(
                            m.desc, endpos - 1, " - #11CUnblockable#0C");
                        e.Data = m.ToBinary();
                    }
                }
            }
        }

        // remove Z flag to compare with autogenerated descriptions
        // for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
        //     auto& e = tbl_en.Entries[i];
        //     if (e.Name == "magic") {
        //         MagicData m(e.Data.data(), e.Data.size());
        //         auto pos = m.flags.find('Z');
        //         if (pos != std::string::npos) {
        //             m.flags.erase(m.flags.begin() + pos);
        //         }
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
} // namespace SenLib::Sen4::FileFixes::t_magic
