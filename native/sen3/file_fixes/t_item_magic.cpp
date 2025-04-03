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
__declspec(dllexport) char SenPatcherFix_0_magic[] =
    "Fix formatting issues in art/craft descriptions.";
}

namespace SenLib::Sen3::FileFixes::t_item_magic {
static std::string AdjustNewlinesToTwoSpaces(std::string desc) {
    size_t idx = 0;
    std::string s = std::move(desc);
    while (true) {
        size_t nidx = s.find_first_of('\n', idx);
        if (nidx == std::string::npos) {
            break;
        }

        size_t spaces = 0;
        size_t i = nidx + 1;
        while (i < s.size()) {
            if (s[i] == ' ') {
                ++i;
                ++spaces;
            } else {
                break;
            }
        }

        if (spaces != 2) {
            if (spaces < 2) {
                s = HyoutaUtils::TextUtils::InsertSubstring(s, nidx + 1, "  ", 0, 2 - spaces);
            } else {
                s = HyoutaUtils::TextUtils::Remove(s, nidx + 1, spaces - 2);
            }
        }

        idx = nidx + 1;
    }
    return s;
}

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
        auto file_item_en = getCheckedFile(
            "data/text/dat_en/t_item_en.tbl",
            311891,
            HyoutaUtils::Hash::SHA1FromHexString("5deee9b833b2bb93b0a326f586943f3d2e2424b9"));
        if (!file_item_en) {
            return false;
        }

        auto& bin = file->Data;
        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        auto& bin_item_en = file_item_en->Data;
        Tbl tbl_item_en(bin_item_en.data(),
                        bin_item_en.size(),
                        HyoutaUtils::EndianUtils::Endianness::LittleEndian);

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

        // Blessed Arrow: Unclear whether the HP Restore is immediate or over time (it's the
        // former), make this clearer.
        {
            auto& e = tbl_en.Entries[281];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 47, 1, "#0C - #11C");
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

        // Morning Moon: Manually write this out so we can replace the One with Self.
        {
            auto& e = tbl_en.Entries[239];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc =
                "[ #11CSelf#0C - #11C100% chance to evade and counter one attack#0C ]\n" + m.desc;
            e.Data = m.ToBinary();
        }

        // Detector-alpha: Miscolored dash.
        {
            auto& e = tbl_en.Entries[81];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 37, 3, "#0C - #11C");
            e.Data = m.ToBinary();
        }

        // Lost Genesis: Miscolored dash.
        {
            auto& e = tbl_en.Entries[48];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 59, 3, "#0C - #11C");
            e.Data = m.ToBinary();
        }

        // Chrono Break: Miscolored dash.
        {
            auto& e = tbl_en.Entries[50];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 46, 3, "#0C - #11C");
            e.Data = m.ToBinary();
        }

        // Radiant Spin: Miscolored dashes.
        {
            auto& e = tbl_en.Entries[87];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 85, 3, "#0C - #11C");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 75, 3, "#0C - #11C");
            e.Data = m.ToBinary();
        }

        // Chrono Break: Miscolored dash.
        {
            auto& e = tbl_en.Entries[90];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 66, 3, "#0C - #11C");
            e.Data = m.ToBinary();
        }

        // Detector-beta: Miscolored dash.
        {
            auto& e = tbl_en.Entries[95];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 69, 3, "#0C - #11C");
            e.Data = m.ToBinary();
        }

        // Southern Cross: Miscolored dash.
        {
            auto& e = tbl_en.Entries[331];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 71, 3, "#0C - #11C");
            e.Data = m.ToBinary();
        }

        // Unbound Rage: Unclear whether the CP+50 is immediate or over time (it's the latter), make
        // it clearer.
        {
            auto& e = tbl_en.Entries[150];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 39, 1, "-");
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 35, " (2 turns)");
            e.Data = m.ToBinary();
        }

        // Heavenly Gift: Unclear whether the HP Recovery and CP+20 is immediate or over time (both
        // are the latter), make it clearer.
        {
            auto& e = tbl_en.Entries[79];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 72, 1, "-");
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 68, " (3 turns)");
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 53, " (3 turns)");

            // This is actually a bit too long to fit into the textbox, so as an exception to the
            // typical formatting combine the HP and CP terms...
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 54, 19, "& ");

            e.Data = m.ToBinary();
        }

        // Impassion: "Restores XX CP" is inconsistent with the usual phrasing of "CP+XX".
        {
            auto& e = tbl_en.Entries[35];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 36, 14, "CP+30");
            e.Data = m.ToBinary();
        }

        // Heat Up: "Restores XX CP" is inconsistent with the usual phrasing of "CP+XX".
        // Also has "Stats Down" which is wrong, otherwise written as "Stat Down".
        {
            auto& e = tbl_en.Entries[38];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 61, 1);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 36, 14, "CP+20");
            e.Data = m.ToBinary();
        }

        // Remedy Fantasia: Like CS4 the 200 CP effect is formatted very strangely, improve that.
        {
            auto& e = tbl_en.Entries[85];
            MagicData m(e.Data.data(), e.Data.size());
            std::string color = m.desc.substr(20, 10);
            std::string effect = m.desc.substr(70, 27);
            effect = HyoutaUtils::TextUtils::Insert(effect, 25, " ");
            effect = HyoutaUtils::TextUtils::Remove(effect, 14, 5);
            effect = "Restores " + HyoutaUtils::TextUtils::Insert(effect, 3, " HP");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 65, 33, color + effect);
            e.Data = m.ToBinary();
        }

        // Palace of Eregion: Formatting consistency with CS4.
        {
            auto& e = tbl_en.Entries[99];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 73, " ");
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 62, 5);
            e.Data = m.ToBinary();
        }

        // Earth Pulse: "Restores HP (25%)" -> "Restores 25% HP" for consistency.
        {
            auto& e = tbl_en.Entries[17];
            MagicData m(e.Data.data(), e.Data.size());
            std::string effect = m.desc.substr(50, 3);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 46, 8, effect + " HP");
            e.Data = m.ToBinary();
        }

        // Write out all the autogenerated Order descriptions so we can make the formatting
        // consistent with the non-generated ones.
        {
            auto& e = tbl_en.Entries[302];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc = "[ Attack (#11C6 turns#0C/#11CDamage +20%#0C) - #11CCP+10#0C ]\n" + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[304];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc = "[ Magic (#11C6 turns#0C/#11CNo Cast Time#0C) ]\n" + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[308];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc = "[ Attack (#11C4 turns#0C/#11CDamage +60%#0C) ]\n" + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[314];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc = "[ Defense (#11C8 turns#0C/#11C90% damage reduction#0C) ]\n" + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[315];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc = "[ Break (#11C4 turns#0C/#11CBreak damage +300%#0C) ]\n" + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[321];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc = "[ Strike (#11C10 turns#0C/#11CCritical +50%#0C) - #11CCP+80#0C ]\n" + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[326];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc =
                "[ Special (#11C4 turns#0C/#11CAbsolute Reflect#0C) - #11CCP+80#0C ]\n" + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[328];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc =
                "[ Strike (#11C4 turns#0C/#11CCritical +30%#0C) - #11CAccelerate#0C ]\n" + m.desc;
            e.Data = m.ToBinary();
        }

        // Write out the autogenerated Magic descriptions so that the quartz <-> magic sync works.
        {
            auto& e = tbl_en.Entries[13];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc = "[ #32IAttack Art (#11C#240IB#0C): #11COne#0C ]\n" + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[59];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc =
                "[ #38IAttack Art (#11C#240IA+#0C): #11CLine L (Set)#0C - #11CRandom Stat"
                "\xE2\x86\x93"
                "#0C ]\n"
                + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[60];
            MagicData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc =
                "[ #38IAttack Art (#11C#240ISS#0C): #11CAll#0C - #11CRemoves positive effects#0C "
                "]\n"
                + m.desc;
            e.Data = m.ToBinary();
        }

        // Gold Dragon, Crazy Hunt, Golden Aura, Ebon Dragon, Perfect Order:
        // Listed as "Strike" but should be "Critical" (consistency with CS4/Reverie)
        for (int idx : {305, 319, 321, 323, 328}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 2, 6, "Critical");
            e.Data = m.ToBinary();
        }

        // Split unrelated effects like "Restores 20 CP/Cures Stat Down" to two separate ones
        for (int idx : {25, 26, 27, 29, 30, 38, 56, 57, 62, 63, 282, 283}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            auto slash = m.desc.find('/');
            if (slash != std::string::npos) {
                m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, slash, 1, "#0C - #11C");
                e.Data = m.ToBinary();
            }
        }

        // Merge cases like "Poison (50%) Petrify (50%)" to "Poison/Petrify (50%)" like Reverie
        for (int idx : {14, 32, 78, 83, 98, 135, 143, 145, 149, 152, 167}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            auto firstPercentage = m.desc.find("%)");
            if (firstPercentage != std::string::npos) {
                auto next = std::string_view(m.desc).substr(firstPercentage + 2);
                size_t drop = 0;
                if (next.starts_with("#0C - #11C")) {
                    drop = 10;
                } else if (next.starts_with("#0C #11C")) {
                    drop = 8;
                } else if (next.starts_with(" ")) {
                    drop = 1;
                }
                if (drop > 0) {
                    auto openParens =
                        std::string_view(m.desc).substr(0, firstPercentage).find_last_of('(');
                    if (openParens != std::string_view::npos && openParens != 0) {
                        m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(
                            m.desc, openParens - 1, (firstPercentage - openParens) + 3 + drop, "/");
                        e.Data = m.ToBinary();
                    }
                }
            }
        }

        // Split cases like "Impede (100%) Burn (20%)" to "Impede (100%) - Burn (20%)" like Reverie
        for (int idx : {65, 71, 77, 91, 110, 119, 124, 129, 133, 148, 186}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            auto firstPercentage = m.desc.find("%)");
            if (firstPercentage != std::string::npos) {
                auto next = std::string_view(m.desc).substr(firstPercentage + 2);
                size_t drop = 0;
                if (next.starts_with("#0C #11C")) {
                    drop = 8;
                } else if (next.starts_with(" ")) {
                    drop = 1;
                }
                if (drop > 0) {
                    m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(
                        m.desc, firstPercentage + 2, drop, "#0C - #11C");
                    e.Data = m.ToBinary();
                }
            }
        }

        // Add a space for things like "Delay+10" -> "Delay +10"
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "magic") {
                MagicData m(e.Data.data(), e.Data.size());
                if (m.flags.find('Z') != std::string::npos) {
                    auto endpos = m.desc.find("]");
                    if (endpos != std::string::npos && endpos > 0) {
                        bool modified = false;
                        while (true) {
                            auto desc = std::string_view(m.desc).substr(0, endpos);
                            endpos = desc.find_last_of('+');
                            if (endpos == std::string::npos || endpos == 0) {
                                break;
                            }

                            const char c = m.desc[endpos - 1];
                            if (c >= 'a' && c <= 'z') {
                                m.desc = HyoutaUtils::TextUtils::Insert(m.desc, endpos, " ");
                                modified = true;
                            }
                        }
                        if (modified) {
                            e.Data = m.ToBinary();
                        }
                    }
                }
            }
        }

        // Reverie has all the turn counts as "for X turns" instead of as "(X turns)" which is IMO a
        // lot more readable, especially in contexts like "SPD Up (L) (4 turns)". So transform that
        // here.
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "magic") {
                MagicData m(e.Data.data(), e.Data.size());
                if (m.flags.find('Z') != std::string::npos) {
                    auto endpos = m.desc.find("]");
                    if (endpos != std::string::npos && endpos > 0) {
                        bool modified = false;
                        while (true) {
                            auto nextCloseParens = m.desc.find_last_of(')', endpos);
                            if (nextCloseParens == std::string::npos) {
                                break;
                            }
                            auto nextOpenParens = m.desc.find_last_of('(', nextCloseParens);
                            if (nextOpenParens == std::string::npos) {
                                break;
                            }

                            auto parensContent = std::string_view(m.desc).substr(
                                nextOpenParens + 1, nextCloseParens - (nextOpenParens + 1));
                            if (!parensContent.empty() && parensContent[0] >= '0'
                                && parensContent[0] <= '9'
                                && (parensContent.ends_with("turn")
                                    || parensContent.ends_with("turns"))) {
                                std::string f = "for ";
                                f += parensContent;
                                m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(
                                    m.desc,
                                    nextOpenParens,
                                    (nextCloseParens + 1) - nextOpenParens,
                                    f);
                                modified = true;
                            }

                            endpos = nextOpenParens;
                        }
                        if (modified) {
                            e.Data = m.ToBinary();
                        }
                    }
                }
            }
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


        // =============== magic done, item next ===============


        // Sennin Gem
        {
            auto& e = tbl_item_en.Entries[597];
            ItemQData m(e.Data.data(), e.Data.size());
            m.item.desc = ReplaceSubstring(m.item.desc, 36, 3, "ADF", 0, 3);
            e.Data = m.ToBinary();
        }

        // Seraph
        {
            auto& e = tbl_item_en.Entries[763];
            ItemQData m(e.Data.data(), e.Data.size());
            m.item.desc = Remove(m.item.desc, 65, 1);
            e.Data = m.ToBinary();
        }

        // Solar Gyre
        {
            auto& e = tbl_item_en.Entries[764];
            ItemQData m(e.Data.data(), e.Data.size());
            m.item.desc = ReplaceSubstring(m.item.desc, 52, 1, "/", 0, 1);
            e.Data = m.ToBinary();
        }

        // Panzer Goggles
        {
            auto& e = tbl_item_en.Entries[303];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = ReplaceSubstring(m.desc, 28, 1, "10", 0, 2);
            tbl_item_en.Entries[303].Data = m.ToBinary();
        }

        // Orange Corsage
        {
            auto& e = tbl_item_en.Entries[318];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Insert(m.desc, 18, "%");
            e.Data = m.ToBinary();
        }

        // Glory Emblem
        {
            auto& e = tbl_item_en.Entries[391];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Insert(m.desc, 12, "%");
            e.Data = m.ToBinary();
        }

        // Blue Star Lionheart Medal
        {
            auto& e = tbl_item_en.Entries[395];
            auto& e343 = tbl_item_en.Entries[343];
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
            auto& e = tbl_item_en.Entries[34];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = ReplaceSubstring(m.desc, 54, 2, "30", 0, 2);
            e.Data = m.ToBinary();
        }

        // Energy Drink
        {
            auto& e = tbl_item_en.Entries[38];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = ReplaceSubstring(m.desc, 39, 1, "5", 0, 1);
            m.desc = ReplaceSubstring(m.desc, 58, 1, "C", 0, 1);
            e.Data = m.ToBinary();
        }

        // Steel Bell
        {
            auto& e = tbl_item_en.Entries[541];
            auto& e501 = tbl_item_en.Entries[501];
            ItemQData m(e.Data.data(), e.Data.size());
            std::string n = ItemQData(e501.Data.data(), e501.Data.size()).item.name;
            m.item.desc = ReplaceSubstring(m.item.desc, 10, 7, n, 0, n.size());
            e.Data = m.ToBinary();
        }

        // A bunch of items that claim EP when they should be CP
        for (int idx : {
                 51,   52,   53,  54,  61,  62,   63,   64,   65,   66,   67,   1026, 1029, 1034,
                 1040, 1044, 988, 991, 996, 1002, 1006, 1015, 1019, 1022, 1048, 1051, 1057, 1066,
             }) {
            auto& e = tbl_item_en.Entries[static_cast<size_t>(idx)];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Replace(m.desc, "EP", "CP");
            e.Data = m.ToBinary();
        }

        // Extra comma in Sandy-grown stuff
        for (int idx : {68, 69, 70}) {
            auto& e = tbl_item_en.Entries[static_cast<size_t>(idx)];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = Replace(m.desc, ", ", " ");
            e.Data = m.ToBinary();
        }

        // Explosive Snowball, Devil Burger: Prettify by removing the extra (30%), needs to be
        // manually written out.
        {
            auto& e = tbl_item_en.Entries[983];
            ItemData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc =
                "[ Physical (#11C#240IA#0C#11C#242INo#0C#11C#244INo#0C) - #11CArea M (Set)#0C - "
                "#11CConfuse/Freeze (30%)#0C ]\n"
                + m.desc;
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_item_en.Entries[984];
            ItemData m(e.Data.data(), e.Data.size());
            m.flags += 'Z';
            m.desc =
                "[ Physical (#11C#240IS#0C#11C#242INo#0C#11C#244INo#0C) - #11CArea M (Set)#0C - "
                "#11CK.O./Nightmare (30%)#0C ]\n"
                + m.desc;
            e.Data = m.ToBinary();
        }

        // Zeram Powder, Zeram Capsule: Weird order of effects, lists KO cure before CP recovery
        // when usually HP/EP/CP are all at the start. Swap that around.
        for (int idx : {11, 12}) {
            auto& e = tbl_item_en.Entries[static_cast<size_t>(idx)];
            ItemData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::MoveSubstring(m.desc, 45, 72, 11);
            e.Data = m.ToBinary();
        }

        // Item descriptions readability pass. See CS4.
        // TODO: Equipment could use this too
        struct SlashToDash {
            uint16_t Item;
            std::array<uint8_t, 6> Dashes;

            constexpr SlashToDash(uint16_t item,
                                  uint8_t d0,
                                  uint8_t d1 = 0,
                                  uint8_t d2 = 0,
                                  uint8_t d3 = 0,
                                  uint8_t d4 = 0,
                                  uint8_t d5 = 0)
              : Item(item), Dashes{{d0, d1, d2, d3, d4, d5}} {}
        };
        // for (size_t i = 0; i < tbl_item_en.Entries.size(); ++i) {
        //     auto& e = tbl_item_en.Entries[i];
        //     auto a = [&](std::string_view name, std::string_view desc) {
        //         if (desc.find('/') != std::string_view::npos) {
        //             std::string s;
        //             s += "// ";
        //             s += HyoutaUtils::TextUtils::Replace(desc, "\n", "{n}");
        //             s += "\n";
        //             s += "SlashToDash(";
        //             s += std::to_string(i);
        //             size_t j = 1;
        //             while (desc.find('/') != std::string_view::npos) {
        //                 s += ", ";
        //                 s += std::to_string(j);
        //                 ++j;
        //                 desc = desc.substr(desc.find('/') + 1);
        //             }
        //             s += "), // ";
        //             s += name;
        //             printf("%s\n", s.c_str());
        //         }
        //     };
        //     if (e.Name == "item_q") {
        //         ItemQData m(e.Data.data(), e.Data.size());
        //         a(m.item.name, m.item.desc);
        //     } else if (e.Name == "item") {
        //         ItemData m(e.Data.data(), e.Data.size());
        //         a(m.name, m.desc);
        //     }
        // }
        static constexpr std::array slashArray = {
            SlashToDash(8, 1),     // Reviving Balm
            SlashToDash(9, 1),     // Celestial Balm
            SlashToDash(10, 1),    // Celestial Balm EX
            SlashToDash(11, 2),    // Zeram Powder
            SlashToDash(12, 2),    // Zeram Capsule
            SlashToDash(15, 2),    // Spirit Incense
            SlashToDash(16, 2),    // Dragon Incense
            SlashToDash(30, 2, 3), // Power Potion
            SlashToDash(31, 3, 4), // Shield Potion
            SlashToDash(32, 2, 3), // Mind Potion
            SlashToDash(33, 2, 3), // Power Potion II
            SlashToDash(34, 3, 4), // Shield Potion II
            SlashToDash(35, 2, 3), // Mind Potion II
            SlashToDash(37, 1),    // Canned Coffee
            SlashToDash(39, 1),    // Kinako Pecky
            SlashToDash(40, 1),    // Raspberry Pecky
            SlashToDash(41, 1),    // Lemon Pecky
            SlashToDash(42, 1),    // Matcha Pecky
            SlashToDash(43, 1),    // Honey Pecky
            SlashToDash(44, 1),    // Sea Salt Pecky
            SlashToDash(45, 1),    // Red Hot Pecky
            SlashToDash(46, 1),    // Rose Pecky
            SlashToDash(47, 1),    // Energy Pecky
            SlashToDash(48, 1),    // Cocoa Pecky
            SlashToDash(49, 1),    // Tropical Pecky
            SlashToDash(50, 1),    // Wasabi Pecky
            SlashToDash(55, 1),    // Apple Juice
            SlashToDash(56, 1),    // Orange Juice
            SlashToDash(57, 1),    // Grape Juice
            SlashToDash(58, 1),    // Peach Fizz
            SlashToDash(71, 1),    // Freddy Special A
            SlashToDash(72, 1),    // Freddy Special B
            SlashToDash(74, 1),    // Freddy Special D
            SlashToDash(84, 1),    // Repair Stone
            SlashToDash(985, 1),   // Chunky Potato Salad
            SlashToDash(986, 1),   // Piled Onion Rings
            SlashToDash(987, 1),   // Fluffy Chiffon Cake
            SlashToDash(988, 2),   // Southern Punch
            SlashToDash(989, 1),   // Hearty White Stew
            SlashToDash(990, 1),   // Juicy Ham Sandwich
            SlashToDash(991, 2),   // Fresh Tomato Noodles
            SlashToDash(992, 1),   // Dragon Fried Rice
            SlashToDash(993, 2),   // Cafe Macchiato
            SlashToDash(994, 2),   // Honey Bagel
            SlashToDash(995, 1),   // Thick Hamburger Steak
            SlashToDash(996, 2),   // Colorful Bouillabaisse
            SlashToDash(997, 1),   // Fisherman's Paella
            SlashToDash(998, 1),   // Purple Hearts
            SlashToDash(999, 1),   // Heavy Meat Pie
            SlashToDash(1000, 2),  // Aquamarine Ice Cream
            SlashToDash(1001, 1),  // Pasta Pepperoncino
            SlashToDash(1002, 2),  // Tomato Curry
            SlashToDash(1003, 1),  // White Velvet Shortcake
            SlashToDash(1004, 1),  // Croquette Burger
            SlashToDash(1005, 1),  // Septetto Tea
            SlashToDash(1006, 2),  // Hearty Kebab
            SlashToDash(1007, 1),  // Shield Chips
            SlashToDash(1008, 1),  // Hollow Cake
            SlashToDash(1009, 2),  // Orange Liquid
            SlashToDash(1010, 1),  // Cold Stew
            SlashToDash(1011, 2),  // Scorched Noodles
            SlashToDash(1012, 1),  // Brown Liquid
            SlashToDash(1016, 1),  // Abominable Rice
            SlashToDash(1017, 2),  // Purple Liquid
            SlashToDash(1018, 1),  // Salty Vongole
            SlashToDash(1019, 2),  // Ultra Curry
            SlashToDash(1020, 1),  // Hard Cake
            SlashToDash(1021, 1),  // Iridescent Liquid
            SlashToDash(1022, 2),  // Sword Grill
            SlashToDash(1023, 1),  // Pillowy Mashed Potatoes
            SlashToDash(1024, 1),  // Glistening Onion Rings
            SlashToDash(1025, 1),  // Summertime Fruit Cake
            SlashToDash(1026, 2),  // Sunshine Punch
            SlashToDash(1027, 1),  // Silky White Stew
            SlashToDash(1028, 1),  // Nostalgic Sandwich
            SlashToDash(1029, 2),  // Champion Noodles
            SlashToDash(1030, 1),  // Heavenly Rice
            SlashToDash(1031, 2),  // Special Macchiato
            SlashToDash(1032, 2),  // Golden Bagel
            SlashToDash(1033, 1),  // Artisan Hamburger
            SlashToDash(1034, 2),  // Passionate Hotpot
            SlashToDash(1035, 1),  // Seafood Boil
            SlashToDash(1036, 1),  // Violet Cloudbank
            SlashToDash(1037, 1),  // Virtuoso Meat Pie
            SlashToDash(1038, 2),  // Grand Blue Gelato
            SlashToDash(1039, 1),  // Shining Pepperoncino
            SlashToDash(1040, 2),  // Special Tomato Curry
            SlashToDash(1041, 1),  // Supreme Shortcake
            SlashToDash(1042, 1),  // Angel Burger
            SlashToDash(1043, 1),  // Ambrosial Nectar
            SlashToDash(1044, 2),  // Dynast Grill
            SlashToDash(1045, 1),  // Crunchy Potato Croquette
            SlashToDash(1046, 3),  // Onion Trio
            SlashToDash(1047, 1),  // Airy Meringue Cookie
            SlashToDash(1048, 2),  // Yuzu Honey Soda
            SlashToDash(1049, 1),  // Regal Beef Stew
            SlashToDash(1050, 2),  // Croque Vander
            SlashToDash(1051, 2),  // Rainbow Veggie Noodles
            SlashToDash(1052, 1),  // Special Garlic Rice
            SlashToDash(1053, 3),  // Rich Chocolate Parfait
            SlashToDash(1054, 2),  // House of Sweets
            SlashToDash(1055, 1),  // Spicy Loco Moco
            SlashToDash(1056, 1),  // Special Yogurt Chowder
            SlashToDash(1057, 2),  // Spicy Jambalaya
            SlashToDash(1058, 2),  // Esmelas Rain
            SlashToDash(1059, 1),  // Lovely Apple Pie
            SlashToDash(1060, 2),  // Elegant Shaved Ice
            SlashToDash(1061, 1),  // Hunter Carbonara
            SlashToDash(1062, 3),  // Shrimp Curry
            SlashToDash(1064, 1),  // Wild Steak Burger
            SlashToDash(1065, 1),  // Harmonious Earl Grey
            SlashToDash(1066, 2),  // Nord-style Kebab
        };
        for (const auto& slashData : slashArray) {
            auto& e = tbl_item_en.Entries[slashData.Item];
            auto apply = [&](std::string& desc) {
                size_t i = 0;
                size_t slashNum = 0;
                size_t arrayIdx = 0;
                while (i < desc.size()) {
                    if (desc[i] == '/') {
                        ++slashNum;
                        if (slashData.Dashes[arrayIdx] == slashNum) {
                            desc =
                                HyoutaUtils::TextUtils::ReplaceSubstring(desc, i, 1, "#0C - #11C");
                            i += 10;
                            ++arrayIdx;
                            if (arrayIdx >= slashData.Dashes.size()
                                || slashData.Dashes[arrayIdx] == 0) {
                                break;
                            }
                        } else {
                            ++i;
                        }
                    } else {
                        ++i;
                    }
                }
            };
            if (e.Name == "item_q") {
                ItemQData m(e.Data.data(), e.Data.size());
                apply(m.item.desc);
                e.Data = m.ToBinary();
            } else if (e.Name == "item") {
                ItemData m(e.Data.data(), e.Data.size());
                apply(m.desc);
                e.Data = m.ToBinary();
            }
        }


        // sync the magic descriptions onto the base quartzes that give that magic
        struct ItemMagicSync {
            uint16_t Item;
            uint16_t Magic;
        };
        // for (size_t i = 0; i < tbl_item_en.Entries.size(); ++i) {
        //     auto& e = tbl_item_en.Entries[i];
        //     if (e.Name == "item_q") {
        //         ItemQData m(e.Data.data(), e.Data.size());
        //         if (m.arts[0] != 0xffffu && m.arts[1] == 0xffffu && m.arts[2] == 0xffffu
        //             && m.arts[3] == 0xffffu && m.arts[4] == 0xffffu && m.arts[5] == 0xffffu) {
        //             for (size_t j = 0; j < tbl_en.Entries.size(); ++j) {
        //                 auto& e2 = tbl_en.Entries[j];
        //                 if (e2.Name == "magic") {
        //                     MagicData m2(e2.Data.data(), e2.Data.size());
        //                     if (m2.idx == m.arts[0]) {
        //                         printf("ItemMagicSync{.Item = %zu, .Magic = %zu}, // %s / %s\n",
        //                                i,
        //                                j,
        //                                m.item.name.c_str(),
        //                                m2.name.c_str());
        //                         break;
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }
        static constexpr std::array syncArray = {
            ItemMagicSync{.Item = 499, .Magic = 13}, // Needle Shoot / Needle Shoot
            ItemMagicSync{.Item = 500, .Magic = 14}, // Ivy Nail / Ivy Nail
            ItemMagicSync{.Item = 501, .Magic = 15}, // Megalith Fall / Megalith Fall
            ItemMagicSync{.Item = 502, .Magic = 16}, // Gravion Hammer / Gravion Hammer
            ItemMagicSync{.Item = 503, .Magic = 18}, // Crest / Crest
            ItemMagicSync{.Item = 504, .Magic = 19}, // La Crest / La Crest
            ItemMagicSync{.Item = 505, .Magic = 17}, // Earth Pulse / Earth Pulse
            ItemMagicSync{.Item = 506, .Magic = 20}, // Adamantine Shield / Adamantine Shield
            ItemMagicSync{.Item = 512, .Magic = 13}, // Needle Shoot R / Needle Shoot
            ItemMagicSync{.Item = 513, .Magic = 14}, // Ivy Nail R / Ivy Nail
            ItemMagicSync{.Item = 514, .Magic = 15}, // Megalith Fall R / Megalith Fall
            ItemMagicSync{.Item = 515, .Magic = 16}, // Gravion Hammer R / Gravion Hammer
            ItemMagicSync{.Item = 516, .Magic = 18}, // Crest R / Crest
            ItemMagicSync{.Item = 517, .Magic = 19}, // La Crest R / La Crest
            ItemMagicSync{.Item = 518, .Magic = 17}, // Earth Pulse R / Earth Pulse
            ItemMagicSync{.Item = 519, .Magic = 20}, // Adamantine Shield R / Adamantine Shield
            ItemMagicSync{.Item = 528, .Magic = 13}, // Needle Shoot SR / Needle Shoot
            ItemMagicSync{.Item = 529, .Magic = 14}, // Ivy Nail SR / Ivy Nail
            ItemMagicSync{.Item = 530, .Magic = 15}, // Megalith Fall SR / Megalith Fall
            ItemMagicSync{.Item = 531, .Magic = 16}, // Gravion Hammer SR / Gravion Hammer
            ItemMagicSync{.Item = 532, .Magic = 18}, // Crest SR / Crest
            ItemMagicSync{.Item = 533, .Magic = 19}, // La Crest SR / La Crest
            ItemMagicSync{.Item = 534, .Magic = 17}, // Earth Pulse SR / Earth Pulse
            ItemMagicSync{.Item = 535, .Magic = 20}, // Adamantine Shield SR / Adamantine Shield
            ItemMagicSync{.Item = 546, .Magic = 21}, // Aqua Bleed / Aqua Bleed
            ItemMagicSync{.Item = 547, .Magic = 22}, // Crystal Edge / Crystal Edge
            ItemMagicSync{.Item = 548, .Magic = 23}, // Blue Ascension / Blue Ascension
            ItemMagicSync{.Item = 549, .Magic = 24}, // Diamond Nova / Diamond Nova
            ItemMagicSync{.Item = 550, .Magic = 25}, // Tear / Tear
            ItemMagicSync{.Item = 551, .Magic = 26}, // Teara / Teara
            ItemMagicSync{.Item = 552, .Magic = 27}, // Tearal / Tearal
            ItemMagicSync{.Item = 553, .Magic = 28}, // Curia / Curia
            ItemMagicSync{.Item = 554, .Magic = 29}, // Thelas / Thelas
            ItemMagicSync{.Item = 555, .Magic = 30}, // Athelas / Athelas
            ItemMagicSync{.Item = 561, .Magic = 21}, // Aqua Bleed R / Aqua Bleed
            ItemMagicSync{.Item = 562, .Magic = 22}, // Crystal Edge R / Crystal Edge
            ItemMagicSync{.Item = 563, .Magic = 23}, // Blue Ascension R / Blue Ascension
            ItemMagicSync{.Item = 564, .Magic = 24}, // Diamond Nova R / Diamond Nova
            ItemMagicSync{.Item = 565, .Magic = 25}, // Tear R / Tear
            ItemMagicSync{.Item = 566, .Magic = 26}, // Teara R / Teara
            ItemMagicSync{.Item = 567, .Magic = 27}, // Tearal R / Tearal
            ItemMagicSync{.Item = 568, .Magic = 28}, // Curia R / Curia
            ItemMagicSync{.Item = 569, .Magic = 29}, // Thelas R / Thelas
            ItemMagicSync{.Item = 570, .Magic = 30}, // Athelas R / Athelas
            ItemMagicSync{.Item = 579, .Magic = 21}, // Aqua Bleed SR / Aqua Bleed
            ItemMagicSync{.Item = 580, .Magic = 22}, // Crystal Edge SR / Crystal Edge
            ItemMagicSync{.Item = 581, .Magic = 23}, // Blue Ascension SR / Blue Ascension
            ItemMagicSync{.Item = 582, .Magic = 24}, // Diamond Nova SR / Diamond Nova
            ItemMagicSync{.Item = 583, .Magic = 25}, // Tear SR / Tear
            ItemMagicSync{.Item = 584, .Magic = 26}, // Teara SR / Teara
            ItemMagicSync{.Item = 585, .Magic = 27}, // Tearal SR / Tearal
            ItemMagicSync{.Item = 586, .Magic = 28}, // Curia SR / Curia
            ItemMagicSync{.Item = 587, .Magic = 29}, // Thelas SR / Thelas
            ItemMagicSync{.Item = 588, .Magic = 30}, // Athelas SR / Athelas
            ItemMagicSync{.Item = 599, .Magic = 31}, // Fire Bolt / Fire Bolt
            ItemMagicSync{.Item = 600, .Magic = 32}, // Venom Flame / Venom Flame
            ItemMagicSync{.Item = 601, .Magic = 33}, // Flare Butterfly / Flare Butterfly
            ItemMagicSync{.Item = 602, .Magic = 34}, // Zeruel Cannon / Zeruel Cannon
            ItemMagicSync{.Item = 603, .Magic = 36}, // Forte / Forte
            ItemMagicSync{.Item = 604, .Magic = 37}, // La Forte / La Forte
            ItemMagicSync{.Item = 605, .Magic = 38}, // Heat Up / Heat Up
            ItemMagicSync{.Item = 606, .Magic = 35}, // Impassion / Impassion
            ItemMagicSync{.Item = 612, .Magic = 31}, // Fire Bolt R / Fire Bolt
            ItemMagicSync{.Item = 613, .Magic = 32}, // Venom Flame R / Venom Flame
            ItemMagicSync{.Item = 614, .Magic = 33}, // Flare Butterfly R / Flare Butterfly
            ItemMagicSync{.Item = 615, .Magic = 34}, // Zeruel Cannon R / Zeruel Cannon
            ItemMagicSync{.Item = 616, .Magic = 36}, // Forte R / Forte
            ItemMagicSync{.Item = 617, .Magic = 37}, // La Forte R / La Forte
            ItemMagicSync{.Item = 618, .Magic = 38}, // Heat Up R / Heat Up
            ItemMagicSync{.Item = 619, .Magic = 35}, // Impassion R / Impassion
            ItemMagicSync{.Item = 628, .Magic = 31}, // Fire Bolt SR / Fire Bolt
            ItemMagicSync{.Item = 629, .Magic = 32}, // Venom Flame SR / Venom Flame
            ItemMagicSync{.Item = 630, .Magic = 33}, // Flare Butterfly SR / Flare Butterfly
            ItemMagicSync{.Item = 631, .Magic = 34}, // Zeruel Cannon SR / Zeruel Cannon
            ItemMagicSync{.Item = 632, .Magic = 36}, // Forte SR / Forte
            ItemMagicSync{.Item = 633, .Magic = 37}, // La Forte SR / La Forte
            ItemMagicSync{.Item = 634, .Magic = 38}, // Heat Up SR / Heat Up
            ItemMagicSync{.Item = 635, .Magic = 35}, // Impassion SR / Impassion
            ItemMagicSync{.Item = 646, .Magic = 39}, // Air Strike / Air Strike
            ItemMagicSync{.Item = 647, .Magic = 40}, // Nemesis Arrow / Nemesis Arrow
            ItemMagicSync{.Item = 648, .Magic = 41}, // Aerial Dust / Aerial Dust
            ItemMagicSync{.Item = 649, .Magic = 42}, // Ixion Volt / Ixion Volt
            ItemMagicSync{.Item = 650, .Magic = 43}, // Breath / Breath
            ItemMagicSync{.Item = 651, .Magic = 44}, // Holy Breath / Holy Breath
            ItemMagicSync{.Item = 652, .Magic = 45}, // Recuria / Recuria
            ItemMagicSync{.Item = 658, .Magic = 39}, // Air Strike R / Air Strike
            ItemMagicSync{.Item = 659, .Magic = 40}, // Nemesis Arrow R / Nemesis Arrow
            ItemMagicSync{.Item = 660, .Magic = 41}, // Aerial Dust R / Aerial Dust
            ItemMagicSync{.Item = 661, .Magic = 42}, // Ixion Volt R / Ixion Volt
            ItemMagicSync{.Item = 662, .Magic = 43}, // Breath R / Breath
            ItemMagicSync{.Item = 663, .Magic = 44}, // Holy Breath R / Holy Breath
            ItemMagicSync{.Item = 664, .Magic = 45}, // Recuria R / Recuria
            ItemMagicSync{.Item = 673, .Magic = 39}, // Air Strike SR / Air Strike
            ItemMagicSync{.Item = 674, .Magic = 40}, // Nemesis Arrow SR / Nemesis Arrow
            ItemMagicSync{.Item = 675, .Magic = 41}, // Aerial Dust SR / Aerial Dust
            ItemMagicSync{.Item = 676, .Magic = 42}, // Ixion Volt SR / Ixion Volt
            ItemMagicSync{.Item = 677, .Magic = 43}, // Breath SR / Breath
            ItemMagicSync{.Item = 678, .Magic = 44}, // Holy Breath SR / Holy Breath
            ItemMagicSync{.Item = 679, .Magic = 45}, // Recuria SR / Recuria
            ItemMagicSync{.Item = 690, .Magic = 46}, // Soul Blur / Soul Blur
            ItemMagicSync{.Item = 691, .Magic = 47}, // Calvary Edge / Calvary Edge
            ItemMagicSync{.Item = 692, .Magic = 48}, // Lost Genesis / Lost Genesis
            ItemMagicSync{.Item = 693, .Magic = 49}, // Chrono Drive / Chrono Drive
            ItemMagicSync{.Item = 694, .Magic = 50}, // Chrono Break / Chrono Break
            ItemMagicSync{.Item = 695, .Magic = 51}, // Chrono Burst / Chrono Burst
            ItemMagicSync{.Item = 700, .Magic = 46}, // Soul Blur R / Soul Blur
            ItemMagicSync{.Item = 701, .Magic = 47}, // Calvary Edge R / Calvary Edge
            ItemMagicSync{.Item = 702, .Magic = 48}, // Lost Genesis R / Lost Genesis
            ItemMagicSync{.Item = 703, .Magic = 49}, // Chrono Drive R / Chrono Drive
            ItemMagicSync{.Item = 704, .Magic = 50}, // Chrono Break R / Chrono Break
            ItemMagicSync{.Item = 705, .Magic = 51}, // Chrono Burst R / Chrono Burst
            ItemMagicSync{.Item = 713, .Magic = 46}, // Soul Blur SR / Soul Blur
            ItemMagicSync{.Item = 714, .Magic = 47}, // Calvary Edge SR / Calvary Edge
            ItemMagicSync{.Item = 715, .Magic = 48}, // Lost Genesis SR / Lost Genesis
            ItemMagicSync{.Item = 716, .Magic = 49}, // Chrono Drive SR / Chrono Drive
            ItemMagicSync{.Item = 717, .Magic = 50}, // Chrono Break SR / Chrono Break
            ItemMagicSync{.Item = 718, .Magic = 51}, // Chrono Burst SR / Chrono Burst
            ItemMagicSync{.Item = 728, .Magic = 52}, // Golden Sphere / Golden Sphere
            ItemMagicSync{.Item = 729, .Magic = 53}, // Cross Crusade / Cross Crusade
            ItemMagicSync{.Item = 730, .Magic = 54}, // Seventh Caliber / Seventh Caliber
            ItemMagicSync{.Item = 731, .Magic = 55}, // Fortuna / Fortuna
            ItemMagicSync{.Item = 732, .Magic = 56}, // Shining / Shining
            ItemMagicSync{.Item = 733, .Magic = 57}, // Seraphic Ring / Seraphic Ring
            ItemMagicSync{.Item = 738, .Magic = 52}, // Golden Sphere R / Golden Sphere
            ItemMagicSync{.Item = 739, .Magic = 53}, // Cross Crusade R / Cross Crusade
            ItemMagicSync{.Item = 740, .Magic = 54}, // Seventh Caliber R / Seventh Caliber
            ItemMagicSync{.Item = 741, .Magic = 55}, // Fortuna R / Fortuna
            ItemMagicSync{.Item = 742, .Magic = 56}, // Shining R / Shining
            ItemMagicSync{.Item = 743, .Magic = 57}, // Seraphic Ring R / Seraphic Ring
            ItemMagicSync{.Item = 751, .Magic = 52}, // Golden Sphere SR / Golden Sphere
            ItemMagicSync{.Item = 752, .Magic = 53}, // Cross Crusade SR / Cross Crusade
            ItemMagicSync{.Item = 753, .Magic = 54}, // Seventh Caliber SR / Seventh Caliber
            ItemMagicSync{.Item = 754, .Magic = 55}, // Fortuna SR / Fortuna
            ItemMagicSync{.Item = 755, .Magic = 56}, // Shining SR / Shining
            ItemMagicSync{.Item = 756, .Magic = 57}, // Seraphic Ring SR / Seraphic Ring
            ItemMagicSync{.Item = 767, .Magic = 58}, // Silver Thorn / Silver Thorn
            ItemMagicSync{.Item = 768, .Magic = 59}, // Galion Fort / Galion Fort
            ItemMagicSync{.Item = 769, .Magic = 60}, // Albion Wolf / Albion Wolf
            ItemMagicSync{.Item = 770, .Magic = 61}, // Analyze / Analyze
            ItemMagicSync{.Item = 771, .Magic = 62}, // Saintly Force / Saintly Force
            ItemMagicSync{.Item = 772, .Magic = 63}, // Crescent Mirror / Crescent Mirror
            ItemMagicSync{.Item = 777, .Magic = 58}, // Silver Thorn R / Silver Thorn
            ItemMagicSync{.Item = 778, .Magic = 59}, // Galion Fort R / Galion Fort
            ItemMagicSync{.Item = 779, .Magic = 60}, // Albion Wolf R / Albion Wolf
            ItemMagicSync{.Item = 780, .Magic = 61}, // Analyze R / Analyze
            ItemMagicSync{.Item = 781, .Magic = 62}, // Saintly Force R / Saintly Force
            ItemMagicSync{.Item = 782, .Magic = 63}, // Crescent Mirror R / Crescent Mirror
            ItemMagicSync{.Item = 791, .Magic = 58}, // Silver Thorn SR / Silver Thorn
            ItemMagicSync{.Item = 792, .Magic = 59}, // Galion Fort SR / Galion Fort
            ItemMagicSync{.Item = 793, .Magic = 60}, // Albion Wolf SR / Albion Wolf
            ItemMagicSync{.Item = 794, .Magic = 61}, // Analyze SR / Analyze
            ItemMagicSync{.Item = 795, .Magic = 62}, // Saintly Force SR / Saintly Force
            ItemMagicSync{.Item = 796, .Magic = 63}, // Crescent Mirror SR / Crescent Mirror
        };
        for (const ItemMagicSync& info : syncArray) {
            auto& e = tbl_en.Entries[info.Magic];
            auto& e2 = tbl_item_en.Entries[info.Item];
            MagicData m(e.Data.data(), e.Data.size());
            ItemQData m2(e2.Data.data(), e2.Data.size());

            std::string desc = m.desc;
            // not sure what designates rarity in CS3 so just use the name...
            if (m2.item.name.ends_with(" R") || m2.item.name.ends_with(" SR")) {
                auto img = desc.find('I');
                if (img == std::string::npos) {
                    continue;
                }
                desc = HyoutaUtils::TextUtils::Insert(
                    desc, img + 1, m2.item.name.ends_with(" R") ? "(R)" : "(SR)");
            }
            if (m2.item.HP != 0 || m2.item.EP != 0 || m2.item.STR != 0 || m2.item.DEF != 0
                || m2.item.ATS != 0 || m2.item.ADF != 0 || m2.item.SPD != 0 || m2.item.MOV != 0
                || m2.item.ACC != 0 || m2.item.EVA != 0) {
                auto newline = desc.find('\n');
                if (newline == std::string::npos) {
                    continue;
                }
                std::array<int32_t, 10> statArray{m2.item.HP,
                                                  m2.item.EP,
                                                  m2.item.STR,
                                                  m2.item.DEF,
                                                  m2.item.ATS,
                                                  m2.item.ADF,
                                                  m2.item.SPD,
                                                  m2.item.MOV,
                                                  m2.item.ACC,
                                                  m2.item.EVA};
                static constexpr std::array<const char*, 10> statNames{
                    "HP", "EP", "STR", "DEF", "ATS", "ADF", "SPD", "MOV", "ACC", "EVA"};

                bool first = true;
                std::string stats = "(";
                // HP+EP may combine
                for (size_t i = 0; i < 2; ++i) {
                    if (statArray[i] != 0) {
                        if (!first) {
                            stats += "/";
                        }
                        stats += statNames[i];
                        // vanilla CS3 doesn't combine here
                        // for (size_t j = i + 1; j < 2; ++j) {
                        //     if (statArray[i] == statArray[j]) {
                        //         stats += "\xEF\xBD\xA5";
                        //         stats += statNames[j];
                        //         statArray[j] = 0;
                        //     }
                        // }
                        stats += "+";
                        stats += std::to_string(statArray[i]);
                        first = false;
                    }
                }
                // STR+DEF+ATS+ADF+SPD+MOV may combine
                for (size_t i = 2; i < 8; ++i) {
                    if (statArray[i] != 0) {
                        if (!first) {
                            stats += "/";
                        }
                        stats += statNames[i];
                        // vanilla CS3 doesn't combine here
                        // for (size_t j = i + 1; j < 8; ++j) {
                        //     if (statArray[i] == statArray[j]) {
                        //         stats += "\xEF\xBD\xA5";
                        //         stats += statNames[j];
                        //         statArray[j] = 0;
                        //     }
                        // }
                        stats += "+";
                        stats += std::to_string(statArray[i]);
                        first = false;
                    }
                }
                for (size_t i = 8; i < 10; ++i) {
                    if (statArray[i] != 0) {
                        if (!first) {
                            stats += "/";
                        }
                        stats += statNames[i];
                        stats += "+";
                        stats += std::to_string(statArray[i]);
                        stats += "%";
                        first = false;
                    }
                }
                stats += ") ";

                // find best place to insert
                size_t insertPos = newline + 1;
                while (insertPos < desc.size()) {
                    if (desc[insertPos] == ' ') {
                        ++insertPos;
                    } else {
                        break;
                    }
                }
                desc = HyoutaUtils::TextUtils::Insert(desc, insertPos, stats);
            }

            if (m2.item.flags.find('Z') == std::string::npos) {
                m2.item.flags += 'Z';
            }
            m2.item.desc = std::move(desc);
            e2.Data = m2.ToBinary();
        }

        // "(X turns)" -> "for X turns"
        for (size_t i = 0; i < tbl_item_en.Entries.size(); ++i) {
            auto& e = tbl_item_en.Entries[i];
            const auto apply = [&](ItemData& m) -> bool {
                bool modified = false;
                if (m.flags.find('Z') != std::string::npos) {
                    auto endpos = m.desc.find("]");
                    if (endpos != std::string::npos && endpos > 0) {
                        while (true) {
                            auto nextCloseParens = m.desc.find_last_of(')', endpos);
                            if (nextCloseParens == std::string::npos) {
                                break;
                            }
                            auto nextOpenParens = m.desc.find_last_of('(', nextCloseParens);
                            if (nextOpenParens == std::string::npos) {
                                break;
                            }

                            auto parensContent = std::string_view(m.desc).substr(
                                nextOpenParens + 1, nextCloseParens - (nextOpenParens + 1));
                            if (!parensContent.empty() && parensContent[0] >= '0'
                                && parensContent[0] <= '9'
                                && (parensContent.ends_with("turn")
                                    || parensContent.ends_with("turns"))) {
                                std::string f = "for ";
                                f += parensContent;
                                m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(
                                    m.desc,
                                    nextOpenParens,
                                    (nextCloseParens + 1) - nextOpenParens,
                                    f);
                                modified = true;
                            }

                            endpos = nextOpenParens;
                        }
                    }
                }
                return modified;
            };
            if (e.Name == "item_q") {
                ItemQData m(e.Data.data(), e.Data.size());
                if (apply(m.item)) {
                    e.Data = m.ToBinary();
                }
            } else if (e.Name == "item") {
                ItemData m(e.Data.data(), e.Data.size());
                if (apply(m)) {
                    e.Data = m.ToBinary();
                }
            }
        }

        // normalize newlines
        for (size_t i = 0; i < tbl_item_en.Entries.size(); ++i) {
            auto& e = tbl_item_en.Entries[i];
            if (e.Name == "item") {
                ItemData m(e.Data.data(), e.Data.size());
                if (m.desc.starts_with("[") && m.flags.find_first_of('Z') != std::string::npos
                    && m.desc.find_first_of('\n') != std::string::npos) {
                    m.desc = AdjustNewlinesToTwoSpaces(m.desc);
                    e.Data = m.ToBinary();
                }
            } else if (e.Name == "item_q") {
                ItemQData m(e.Data.data(), e.Data.size());
                if (m.item.desc.starts_with("[")
                    && m.item.flags.find_first_of('Z') != std::string::npos
                    && m.item.desc.find_first_of('\n') != std::string::npos) {
                    m.item.desc = AdjustNewlinesToTwoSpaces(m.item.desc);
                    e.Data = m.ToBinary();
                }
            }
        }


        // =============== finalize ===============


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
        // for (size_t i = 0; i < tbl_item_en.Entries.size(); ++i) {
        //     auto& e = tbl_item_en.Entries[i];
        //     if (e.Name == "item") {
        //         ItemData m(e.Data.data(), e.Data.size());
        //         auto pos = m.flags.find('Z');
        //         if (pos != std::string::npos) {
        //             m.flags.erase(m.flags.begin() + pos);
        //         }
        //         e.Data = m.ToBinary();
        //     } else if (e.Name == "item_q") {
        //         ItemQData m(e.Data.data(), e.Data.size());
        //         auto pos = m.item.flags.find('Z');
        //         if (pos != std::string::npos) {
        //             m.item.flags.erase(m.item.flags.begin() + pos);
        //         }
        //         e.Data = m.ToBinary();
        //     }
        // }


        std::vector<char> result_en_vec;
        std::vector<char> result_item_en_vec;
        {
            HyoutaUtils::Stream::MemoryStream result_en(result_en_vec);
            tbl_en.WriteToStream(result_en, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        }
        {
            HyoutaUtils::Stream::MemoryStream result_en(result_item_en_vec);
            tbl_item_en.WriteToStream(result_en,
                                      HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        }


        result.emplace_back(
            std::move(result_en_vec), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        // there's two identical copies in the game files
        result.emplace_back(
            result_item_en_vec, file_item_en->Filename, SenPatcher::P3ACompressionType::LZ4);
        result.emplace_back(std::move(result_item_en_vec),
                            SenPatcher::InitializeP3AFilename("data/text/dat_fr/t_item_en.tbl"),
                            SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t_item_magic
