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
