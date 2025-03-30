#include <algorithm>
#include <charconv>
#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen5/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_item[] = "Improve formatting of item descriptions.";
__declspec(dllexport) char SenPatcherFix_0_magic[] = "Fix errors in craft descriptions.";
}

namespace SenLib::Sen5::FileFixes::t_item_magic {
std::optional<size_t> TryParseLengthCode(std::string_view s, size_t& idx) {
    size_t start = idx;
    while (idx < s.size() && s[idx] >= '0' && s[idx] <= '9') {
        ++idx;
    }
    if (idx < s.size() && s[idx] == 'L') {
        // is a length tag
        size_t intval = 0;
        const auto [_, ec] = std::from_chars(&s[start], &s[idx], intval);
        if (ec == std::errc()) {
            ++idx;
            return intval;
        }
    }

    // not a length tag
    idx = start;
    return std::nullopt;
}

static bool AdjustSpacingForBracketed(std::string& s) {
    // need to parse this byte by byte so we can handle the #nL tags...
    size_t idx = 0;

    // does this start with a '['?
    size_t textsize = 0;
    bool shouldAdjust = false;
    while (idx < s.size()) {
        if (s[idx] == '[') {
            shouldAdjust = true;
            break;
        } else if (s[idx] == '#') {
            ++idx;
            auto v = TryParseLengthCode(s, idx);
            if (!v) {
                --idx;
                break;
            }
            textsize = *v;
        } else {
            break;
        }
    }
    if (!shouldAdjust) {
        return false;
    }

    std::string out;

    // set text size if needed
    if (textsize != 0) {
        out += '#';
        out += std::to_string(textsize);
        out += 'L';
    }

    while (idx < s.size()) {
        if (s[idx] == '\n') {
            // revert text size if needed
            if (textsize != 0) {
                out += "#0L";
            }

            // go to next line
            out += "\n  ";

            ++idx;
            while (idx < s.size()) {
                if (s[idx] == ' ') {
                    ++idx;
                } else if (s[idx] == '#') {
                    ++idx;
                    auto v = TryParseLengthCode(s, idx);
                    if (!v) {
                        --idx;
                        break;
                    }
                    textsize = *v;
                } else {
                    break;
                }
            }

            // set text size if needed
            if (textsize != 0) {
                out += '#';
                out += std::to_string(textsize);
                out += 'L';
            }

            // resume standard text handling
        } else if (s[idx] == '#') {
            ++idx;
            auto v = TryParseLengthCode(s, idx);
            if (!v) {
                out += '#';
                continue;
            }
            if (textsize != *v) {
                out += '#';
                out += std::to_string(textsize);
                out += 'L';
                textsize = *v;
            }
        } else {
            out += s[idx];
            ++idx;
        }
    }

    if (out == s) {
        return false;
    }

    s = std::move(out);
    return true;
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_magic.tbl",
            234399,
            HyoutaUtils::Hash::SHA1FromHexString("8573735671f71f0bab97decb0a6ddffe5dffbb68"));
        if (!file) {
            return false;
        }
        auto file_item = getCheckedFile(
            "data/text/dat_en/t_item.tbl",
            882731,
            HyoutaUtils::Hash::SHA1FromHexString("c268753e9fcf0eacb56d5627b6c2f099918e1de3"));
        if (!file_item) {
            return false;
        }

        auto& bin = file->Data;
        auto& bin_item = file_item->Data;
        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        Tbl tbl_item(
            bin_item.data(), bin_item.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // Albion Wolf: Power is SSS, not SS+
        {
            auto& e = tbl_en.Entries[60];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc[29] = 'S';
            e.Data = m.ToBinary();
        }

        // Rosetta Arrow: Burn & Crit is 25%, not 20%
        {
            auto& e = tbl_en.Entries[97];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc[116] = '5';
            m.desc[145] = '5';
            e.Data = m.ToBinary();
        }

        // Remedy Fantasia 2/3: The 'gradual HP regen' is phrased inconsistently here, it's usually
        // 'Restores x% HP for n turns' but here it's listed as 'HP+x% for n turns'.
        // Also change "Restores All HP" -> "Restores all HP"
        {
            auto& e = tbl_en.Entries[113];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 70, 6, "Restores 50% HP");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 43, 1, "a");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[114];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 70, 6, "Restores 50% HP");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 43, 1, "a");
            e.Data = m.ToBinary();
        }

        // Luminary Force: Also gives ADF, not just ATS
        {
            auto& e = tbl_en.Entries[720];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc,
                                                    16,
                                                    "\xE3\x83\xBB"
                                                    "ADF");
            e.Data = m.ToBinary();
        }

        // Caladbolg M: Uses (Set) targeting but not listed as such
        {
            auto& e = tbl_en.Entries[172];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 69, " (Set)");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[173];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 69, " (Set)");
            e.Data = m.ToBinary();
        }

        // Valkyrie Smash (Striker variant): Listed as Line M but is actually Line M+
        {
            auto& e = tbl_en.Entries[216];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 64, 1, "M+");
            e.Data = m.ToBinary();
        }

        // War Hammer: Listed as +250% break but actually gives +300%
        {
            auto& e = tbl_en.Entries[726];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 28, 2, "30");
            e.Data = m.ToBinary();
        }

        // Galaxia Volley 2: Listed as 100% chance of random abormality but is actually 120%
        {
            auto& e = tbl_en.Entries[258];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc[105] = '2';
            e.Data = m.ToBinary();
        }

        // Supreme Lightning Charge: Inconsistent order of stat ups. Usually SPD is listed after ATS
        // but here it's before.
        {
            auto& e = tbl_en.Entries[185];
            MagicData m(e.Data.data(), e.Data.size());
            std::swap(m.desc[65], m.desc[65 + 6]);
            std::swap(m.desc[66], m.desc[66 + 6]);
            std::swap(m.desc[67], m.desc[67 + 6]);
            e.Data = m.ToBinary();
        }

        // Heavy Accel/2: Lists Unbalance as +30% which is inconsisted how this info is presented in
        // this game otherwise (CS3/4 lists this as Unbalance No/Yes/+x% but Reverie assigns a
        // letter grade)
        // Heavy Accel 2 also has 50% Faint but is actually 100%
        {
            auto& e = tbl_en.Entries[323];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 49, 4, "A");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[327];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 49, 4, "A");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 108, 1, "10");
            e.Data = m.ToBinary();
        }

        // True/Umbral Butterflies:
        // - The ailments on not-true are 40%, not 50%
        // - The stat down is missing its turn count
        // - Missing color tags on one of the dashes
        {
            auto& e = tbl_en.Entries[287];
            MagicData m(e.Data.data(), e.Data.size());
            std::string color = m.desc.substr(71, 10);
            m.desc[97] = '4';
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 119, " for 3 turns");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 101, 3, color);
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[290];
            MagicData m(e.Data.data(), e.Data.size());
            std::string color = m.desc.substr(71, 10);
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 119, " for 4 turns");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 101, 3, color);
            e.Data = m.ToBinary();
        }

        // Concentration: Missing turn count for the Insight
        {
            auto& e = tbl_en.Entries[280];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 37, " for 3 turns");
            e.Data = m.ToBinary();
        }

        // Invoke Grianos: Claims to be 'Area M (Set)' but is actually 'All'
        {
            auto& e = tbl_en.Entries[368];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 58, 12, "All");
            e.Data = m.ToBinary();
        }

        // Supreme Dynast Sunder: Missing turn count for the stat down
        {
            auto& e = tbl_en.Entries[293];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 96, " for 4 turns");
            e.Data = m.ToBinary();
        }

        // Burning Heart 3: The gradual HP recovery is 3 turns, not 2
        {
            auto& e = tbl_en.Entries[401];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc[131] = '3';
            e.Data = m.ToBinary();
        }

        // Raging Hammer: Inconsistent formatting with other orders. Usually the immediate effect is
        // in parenthesis and the over-time one is after the dash, this is the other way around.
        // Also other formatting inconsistencies.
        {
            auto& e = tbl_en.Entries[751];
            MagicData m(e.Data.data(), e.Data.size());
            std::string time = m.desc.substr(13, 10);
            std::string effect1 = m.desc.substr(24, 16);
            std::string effect2 = m.desc.substr(40, 5);
            std::string immediate = m.desc.substr(52, 12);
            effect1[10] = 'd';
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(
                m.desc, 52, 12, std::format("{} {} for {}", effect1, effect2, time));
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 9, 39, immediate);
            e.Data = m.ToBinary();
        }

        // Celestia Rain: The 'at 200 CP' effect is listed inconsistently with other instances
        {
            auto& e = tbl_en.Entries[411];
            MagicData m(e.Data.data(), e.Data.size());
            std::string color = m.desc.substr(45, 10);
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 91, 1);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 64, 2, color);
            e.Data = m.ToBinary();
        }

        // Supreme Radiant Spin: CS3/4 style Unbalance bonus and wrong Faint percentage
        {
            auto& e = tbl_en.Entries[463];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 109, 2, "8");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 48, 4, "C");
            e.Data = m.ToBinary();
        }

        // Analysis Complete!: see Raging Hammer. Also the 'abilities' should be 'stats'.
        {
            auto& e = tbl_en.Entries[765];
            MagicData m(e.Data.data(), e.Data.size());
            std::string effect = m.desc.substr(9, 37);
            std::string immediate = m.desc.substr(50, 38);
            immediate = HyoutaUtils::TextUtils::ReplaceSubstring(immediate, 8, 9, "stats");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 50, 38, effect);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 9, 37, immediate);
            e.Data = m.ToBinary();
        }

        // Electromagnetic Net: MOV down, not DEF
        {
            auto& e = tbl_en.Entries[422];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 104, 3, "MOV");
            e.Data = m.ToBinary();
        }

        // Revelation: Also inflicts Seal (40%)
        {
            auto& e = tbl_en.Entries[496];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 65, " - #11CSeal (40%)#0C");
            e.Data = m.ToBinary();
        }

        // Rainbow Shot/2: 'All (Set)' makes no sense, it's just 'All'
        {
            auto& e = tbl_en.Entries[344];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 63, 6);
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[345];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 64, 6);
            e.Data = m.ToBinary();
        }

        // Sacred Circle/2: 'for 2 turns' on the Magic Reflect is wrong, it is just 1 hit and lasts
        // until you get hit
        {
            auto& e = tbl_en.Entries[358];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 73, 12);
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[359];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 73, 12);
            e.Data = m.ToBinary();
        }

        // Blue Oratorio/2: Missing a space
        {
            auto& e = tbl_en.Entries[108];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 54, " ");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[111];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 56, " ");
            e.Data = m.ToBinary();
        }

        // Spirit Unification/Enlightened Spirit Unification/Chained Spirit Unification:
        // 'Unleash' needs to be separated by a dash from the stat boost. Also remove the space
        // before the up arrow, inconsistent with all other stat boosts.
        {
            auto& e = tbl_en.Entries[74];
            MagicData m(e.Data.data(), e.Data.size());
            std::string dash = m.desc.substr(20, 10);
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 53, 1);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 37, 1, dash);
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[79];
            MagicData m(e.Data.data(), e.Data.size());
            std::string dash = m.desc.substr(20, 10);
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 59, 1);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 37, 1, dash);
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[80];
            MagicData m(e.Data.data(), e.Data.size());
            std::string dash = m.desc.substr(20, 10);
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 59, 1);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 37, 1, dash);
            e.Data = m.ToBinary();
        }


        // Divine Knight stuff below

        // Crescent Flash, Waning Crescent Slash: Claims One as the target, but hits All.
        // Note that this distinction is rather pointless in Reverie... You never fight more than
        // one enemy at the same time in this game. But for consistency with the older games it's
        // good to write it like this anyway, and if someone hacks in a multi-enemy battle it would
        // probably behave correctly.
        {
            auto& e = tbl_en.Entries[632];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 62, 3, "All");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[633];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 61, 3, "All");
            e.Data = m.ToBinary();
        }

        // Divine Arc: Claims One as the target, but hits All.
        {
            auto& e = tbl_en.Entries[662];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 60, 3, "All");
            e.Data = m.ToBinary();
        }

        // Gemini Blast/2: Claims One as the target, but hits All. Dash is missing color tag in 2.
        {
            auto& e = tbl_en.Entries[638];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 61, 3, "All");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[639];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 62, 3, "All");
            std::string color = m.desc.substr(65, 3);
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 88, color);
            e.Data = m.ToBinary();
        }

        // Moulin Rouge/2: Claims One as the target, but hits All.
        {
            auto& e = tbl_en.Entries[647];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 62, 3, "All");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[648];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 61, 3, "All");
            e.Data = m.ToBinary();
        }

        // Tempete Rouge: Uses the autogenerated description, which is formatted weirdly. We can
        // copy and adapt the description from Moulin Rouge.
        {
            auto& e = tbl_en.Entries[646];
            auto& e2 = tbl_en.Entries[647];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 118) + m.desc;
            m.desc[48] = '3';
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 34, 2, "D");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 20, 2, "B+");
            e.Data = m.ToBinary();
        }

        // Rumbling Smash: Autogenerated, copy from Rumbling Smash 2
        {
            auto& e = tbl_en.Entries[649];
            auto& e2 = tbl_en.Entries[650];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 90) + m.desc;
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 35, 1);
            e.Data = m.ToBinary();
        }

        // Void Breaker 2: Listed as ADF-DEF down but usually DEF is listed first.
        {
            auto& e = tbl_en.Entries[653];
            MagicData m(e.Data.data(), e.Data.size());
            std::swap(m.desc[98], m.desc[104]);
            std::swap(m.desc[99], m.desc[105]);
            std::swap(m.desc[100], m.desc[106]);
            e.Data = m.ToBinary();
        }

        // Void Breaker: Autogenerated, copy from Void Breaker 2
        {
            auto& e = tbl_en.Entries[652];
            auto& e2 = tbl_en.Entries[653];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 131) + m.desc;
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 22, 1);
            e.Data = m.ToBinary();
        }

        // Shock Breaker: Autogenerated, copy from Void Breaker
        {
            auto& e = tbl_en.Entries[651];
            auto& e2 = tbl_en.Entries[652];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 130) + m.desc;
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 126, " - #11CDelay +10#0C");
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 70, 23);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 34, 1, "A+");
            e.Data = m.ToBinary();
        }

        // Power Smash: Autogenerated, copy from Oracle Saber
        {
            auto& e = tbl_en.Entries[654];
            auto& e2 = tbl_en.Entries[661];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 96) + m.desc;
            m.desc[47] = '7';
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 35, "+");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 21, 1, "C+");
            e.Data = m.ToBinary();
        }

        // Salamander: Autogenerated, copy from Oracle Saber
        {
            auto& e = tbl_en.Entries[655];
            auto& e2 = tbl_en.Entries[661];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 96) + m.desc;
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 90, " - #11CDelay +10#0C");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 74, 6, "Burn");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 34, 1, "A+");
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 22, "+");
            e.Data = m.ToBinary();
        }

        // Criminal Edge/2: Claims One as the target, but hits All.
        {
            auto& e = tbl_en.Entries[658];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 62, 3, "All");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[659];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 61, 3, "All");
            e.Data = m.ToBinary();
        }

        // All the elemental arts are missing their element color icon.
        {
            auto& e = tbl_en.Entries[687];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 2, "#32I");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[688];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 2, "#33I");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[689];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 2, "#34I");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[690];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 2, "#35I");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[702];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 2, "#36I");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[703];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 2, "#37I");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[704];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 2, "#38I");
            e.Data = m.ToBinary();
        }

        // Resonant Beat: This is just wrong in multiple ways. It uses the weird 'HP Absorb'
        // phrasing for gradual recovery and *also* lists the gradual recovery separately but with
        // the wrong percentage. What happened here? Thankfully not too hard to fix by just
        // shuffling this around...
        {
            auto& e = tbl_en.Entries[693];
            MagicData m(e.Data.data(), e.Data.size());
            std::string recovery = m.desc.substr(58, 15);
            m.desc[68] = '0';
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 33, 15, recovery);
            e.Data = m.ToBinary();
        }

        // Resounding Beat: Autogenerated, copy from Resonant Beat
        {
            auto& e = tbl_en.Entries[692];
            auto& e2 = tbl_en.Entries[693];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 92) + m.desc;
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 20, 3, "Self");
            e.Data = m.ToBinary();
        }

        // Wild Rage: Autogenerated, copy from Unbound Rage
        {
            auto& e = tbl_en.Entries[699];
            auto& e2 = tbl_en.Entries[707];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 86) + m.desc;
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 42, 25);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 52, 1, "12");
            e.Data = m.ToBinary();
        }

        // Quick Star: Dash is missing color tag
        {
            auto& e = tbl_en.Entries[706];
            MagicData m(e.Data.data(), e.Data.size());
            std::string color = m.desc.substr(23, 3);
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 43, color);
            e.Data = m.ToBinary();
        }

        // Unbound Rage: Stat boost is missing duration
        {
            auto& e = tbl_en.Entries[707];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 64, " for 1 turn");
            e.Data = m.ToBinary();
        }

        // Blade Dance: Also gives SPD
        {
            auto& e = tbl_en.Entries[708];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc,
                                                    42,
                                                    "\xE3\x83\xBB"
                                                    "SPD");
            e.Data = m.ToBinary();
        }

        // Eye of Balor: Autogenerated, copy from Silvery Runes
        {
            auto& e = tbl_en.Entries[711];
            auto& e2 = tbl_en.Entries[697];
            MagicData m(e.Data.data(), e.Data.size());
            MagicData m2(e2.Data.data(), e2.Data.size());
            m.flags += 'Z';
            m.desc = m2.desc.substr(0, 81) + m.desc;
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 48, 26, "Delay +6");
            e.Data = m.ToBinary();
        }

        // A lot of the Divine Knight attacks have the 'Unblockable' trait that is completely
        // omitted in the English descriptions. Fix that.
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "magic") {
                MagicData m(e.Data.data(), e.Data.size());
                if (m.idx >= 15050 && m.idx <= 15545 && m.effects[1].idx == 192
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

        // True Radiant Wings (Victor): The HP recovery is instant, not gradual. Note that this
        // is different from Laura's True Radiant Wings, where the recovery *is* gradual. Very
        // confusing.
        {
            auto& e = tbl_en.Entries[461];
            MagicData m(e.Data.data(), e.Data.size());
            std::string separator = m.desc.substr(20, 10);
            std::string recovery = m.desc.substr(70, 15);
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 69, 16);
            m.desc = HyoutaUtils::TextUtils::Insert(m.desc, 81, separator + recovery);
            e.Data = m.ToBinary();
        }

        // Aura Rain, Celestia Rain: Description formatting is inconsistent between these two
        // evolutions of the same craft, make this consistent. Also change "Restores All HP" ->
        // "Restores all HP" and "Cures All" -> "Cures K.O., ailments, & Stat Down".
        {
            auto& e = tbl_en.Entries[410];
            MagicData m(e.Data.data(), e.Data.size());
            std::string recovery = m.desc.substr(80, 25);
            std::string separator = m.desc.substr(34, 10);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 78, 28, separator + recovery);
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 0, 4);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 49, 1, "a");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(
                m.desc, 71, 3, "K.O., ailments, & Stat Down");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[411];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 39, 1, "a");
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(
                m.desc, 61, 3, "K.O., ailments, & Stat Down");
            e.Data = m.ToBinary();
        }

        // Pep Boost!/!!: "Cures All" -> "Cures K.O., ailments, & Stat Down"
        {
            auto& e = tbl_en.Entries[504];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(
                m.desc, 70, 3, "K.O., ailments, & Stat Down");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[507];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(
                m.desc, 70, 3, "K.O., ailments, & Stat Down");
            e.Data = m.ToBinary();
        }

        // Impassion: Dash instead of colon (consistency with other arts)
        {
            auto& e = tbl_en.Entries[35];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 17, 2, ":");
            e.Data = m.ToBinary();
        }

        // Recuria: Extra space
        {
            auto& e = tbl_en.Entries[45];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 55, 1);
            e.Data = m.ToBinary();
        }

        // Galion Fort: "(4 turns)" instead of "for 4 turns" like all the other Reverie descriptions
        {
            auto& e = tbl_en.Entries[59];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::Remove(m.desc, 88, 1);
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 80, 1, "for ");
            e.Data = m.ToBinary();
        }

        // Heat Up: "Restores XX CP" is inconsistent with the usual phrasing of "CP+XX".
        {
            auto& e = tbl_en.Entries[38];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 36, 14, "CP+20");
            e.Data = m.ToBinary();
        }

        // Military Might: Miscolored dash
        {
            auto& e = tbl_en.Entries[295];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 50, 3, "#0C - #11C");
            e.Data = m.ToBinary();
        }

        // Supreme Military Might: Miscolored dash
        {
            auto& e = tbl_en.Entries[297];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 50, 3, "#0C - #11C");
            e.Data = m.ToBinary();
        }

        // "Ailments" is capitalized in these, which is inconsistent with some other entries as well
        // as all of CS3 and CS4. One could argue that it should be capitalized everywhere instead
        // since Stat Down is always capitalized too, I suppose...
        for (int idx : {
                 28,  45,  107, 110, 324, 333, 335, 347, 350, 393,
                 397, 401, 479, 482, 541, 543, 621, 622, 623,
             }) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            auto pos = m.desc.find("Ailments");
            if (pos != std::string::npos) {
                m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, pos, 1, "a");
            }
            e.Data = m.ToBinary();
        }

        // Ebon Dragon: Listed as "Strike" but should be "Critical"
        {
            auto& e = tbl_en.Entries[734];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 2, 6, "Critical");
            e.Data = m.ToBinary();
        }

        // Draupnir Guard, Dark Requiem, Fanciful Spell, Testament Cross:
        // Listed as "Arts" but should be "Magic"
        for (int idx : {757, 761, 764, 767}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 2, 4, "Magic");
            e.Data = m.ToBinary();
        }

        // Seraphic Ring: "Restores All HP" -> "Restores all HP" for consistency
        {
            auto& e = tbl_en.Entries[57];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 46, 1, "a");
            e.Data = m.ToBinary();
        }

        // Leanan's Kiss: The "for 3 turns" belongs to the stat up, not the charm.
        {
            auto& e = tbl_en.Entries[65];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::MoveSubstring(m.desc, 101, 88, 12);
            e.Data = m.ToBinary();
        }

        const auto find_for_x_turns =
            [](std::string_view s, size_t offset, size_t& out_pos, size_t& out_len) -> bool {
            for (size_t i = offset; i < s.size(); ++i) {
                if (!s.substr(i).starts_with("for ")) {
                    continue;
                }
                auto p = s.find(" ", i + 4);
                if (p == std::string_view::npos || p == (i + 4)) {
                    continue;
                }
                std::string_view turncount = s.substr(i + 4, p - (i + 4));
                if (turncount == "1") {
                    if (!s.substr(p).starts_with(" turn")) {
                        continue;
                    }
                    out_pos = i;
                    out_len = (p + 5) - i;
                    return true;
                } else {
                    if (!std::all_of(turncount.begin(), turncount.end(), [](char c) {
                            return c >= '0' && c <= '9';
                        })) {
                        continue;
                    }
                    if (!s.substr(p).starts_with(" turns")) {
                        continue;
                    }
                    out_pos = i;
                    out_len = (p + 6) - i;
                    return true;
                }
            }
            return false;
        };

        // Split unrelated effects like "Restores 20 CP/Cures Stat Down" to two separate ones
        for (int idx :
             {25, 26, 27, 29, 30, 38, 57, 62, 63, 113, 114, 302, 304, 350, 738, 742, 749}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            auto slash = m.desc.find('/');
            if (slash != std::string::npos) {
                m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, slash, 1, "#0C - #11C");
                e.Data = m.ToBinary();
            }
        }
        for (int idx : {99, 99, 102, 102, 117, 144, 222}) { // duplicates intentional
            // these also need to copy the next "for X turns"
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            auto slash = m.desc.find('/');
            size_t turnpos;
            size_t turnlen;
            if (slash != std::string::npos && find_for_x_turns(m.desc, slash, turnpos, turnlen)) {
                std::string turncount = m.desc.substr(turnpos, turnlen);
                m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(
                    m.desc, slash, 1, " " + turncount + "#0C - #11C");
                e.Data = m.ToBinary();
            }
        }
        for (int idx : {64, 65, 66}) {
            // these need the first slash after a colon instead of just the first slash
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            auto colon = m.desc.find(':');
            if (colon != std::string::npos) {
                auto slash = m.desc.find('/', colon);
                if (slash != std::string::npos) {
                    m.desc =
                        HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, slash, 1, "#0C - #11C");
                    e.Data = m.ToBinary();
                }
            }
        }

        // Make the orders consistent with how they're written in CS3/4, which IMO makes more sense.
        // Lead with the "N turns/permanent effect" in parenthesis and list the immediate effects
        // afterwards with dashes.
        for (int idx = 714; idx <= 771; ++idx) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            MagicData m(e.Data.data(), e.Data.size());
            if (m.flags.find('Z') == std::string::npos) {
                continue;
            }
            auto end = m.desc.find(" ]");
            if (end == std::string::npos || end == 0) {
                continue;
            }
            auto start = m.desc.rfind(" - ", end);
            if (start == std::string::npos || start == 0 || (start + 3) >= end) {
                continue;
            }

            size_t permStart = start + 3;
            size_t permLen = end - (start + 3);
            size_t immStart = 0;
            size_t immLen = 0;
            bool hasImm = false;
            if (m.desc[start - 1] == ')') {
                // has an immediate effect
                auto tmp = m.desc.find('(');
                if (tmp == std::string::npos || tmp >= (start - 1)) {
                    continue;
                }

                hasImm = true;
                immStart = tmp + 1;
                immLen = (start - 1) - (tmp + 1);
            }

            std::string permanent = m.desc.substr(permStart, permLen);
            size_t turnsStart = 0;
            size_t turnsLen = 0;
            if (!find_for_x_turns(permanent, 1, turnsStart, turnsLen)) {
                continue;
            }

            std::string turnCount = permanent.substr(turnsStart + 4, turnsLen - 4);
            permanent = HyoutaUtils::TextUtils::Remove(permanent, turnsStart - 1, turnsLen + 1);
            permanent = "#11C" + turnCount + "#0C/" + permanent;

            if (hasImm) {
                std::string immediate = m.desc.substr(immStart, immLen);
                m.desc =
                    HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, permStart, permLen, immediate);
                m.desc =
                    HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, immStart, immLen, permanent);
            } else {
                m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(
                    m.desc, permStart - 3, permLen + 3, " (" + permanent + ")");
            }
            e.Data = m.ToBinary();
        }

        // replace the separator dot between STR/DEF etc. with a slightly smaller one that's used by
        // the autogenerator, which looks a bit better and is consistent with the autogenerator.
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "magic") {
                MagicData m(e.Data.data(), e.Data.size());
                m.desc = HyoutaUtils::TextUtils::Replace(m.desc, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                e.Data = m.ToBinary();
            }
        }

        // normalize newlines
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "magic") {
                MagicData m(e.Data.data(), e.Data.size());
                if (AdjustSpacingForBracketed(m.desc)) {
                    e.Data = m.ToBinary();
                }
            }
        }

        // =============== magic done, item next ===============

        // replace the separator dot between STR/DEF etc. with a slightly smaller one that's used by
        // the autogenerator, which looks a bit better and is consistent with the autogenerator.
        for (size_t i = 0; i < tbl_item.Entries.size(); ++i) {
            auto& e = tbl_item.Entries[i];
            if (e.Name == "item") {
                ItemData m(e.Data.data(), e.Data.size());
                m.desc = HyoutaUtils::TextUtils::Replace(m.desc, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                e.Data = m.ToBinary();
            } else if (e.Name == "item_e") {
                ItemEData m(e.Data.data(), e.Data.size());
                m.item.desc =
                    HyoutaUtils::TextUtils::Replace(m.item.desc, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                e.Data = m.ToBinary();
            } else if (e.Name == "item_q") {
                ItemQData m(e.Data.data(), e.Data.size());
                m.item.desc =
                    HyoutaUtils::TextUtils::Replace(m.item.desc, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                e.Data = m.ToBinary();
            }
        }

        // sync the magic descriptions onto the base quartzes that give that magic
        struct ItemMagicSync {
            uint16_t Item;
            uint16_t Magic;
        };
        // for (size_t i = 0; i < tbl_item.Entries.size(); ++i) {
        //     auto& e = tbl_item.Entries[i];
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
            ItemMagicSync{.Item = 2445, .Magic = 13}, // Needle Shoot / Needle Shoot
            ItemMagicSync{.Item = 2446, .Magic = 13}, // Needle Shoot R / Needle Shoot
            ItemMagicSync{.Item = 2447, .Magic = 13}, // Needle Shoot SR / Needle Shoot
            ItemMagicSync{.Item = 2448, .Magic = 13}, // Needle Shoot UR / Needle Shoot
            ItemMagicSync{.Item = 2449, .Magic = 14}, // Ivy Nail / Ivy Nail
            ItemMagicSync{.Item = 2450, .Magic = 14}, // Ivy Nail R / Ivy Nail
            ItemMagicSync{.Item = 2451, .Magic = 14}, // Ivy Nail SR / Ivy Nail
            ItemMagicSync{.Item = 2452, .Magic = 14}, // Ivy Nail UR / Ivy Nail
            ItemMagicSync{.Item = 2453, .Magic = 15}, // Megalith Fall / Megalith Fall
            ItemMagicSync{.Item = 2454, .Magic = 15}, // Megalith Fall R / Megalith Fall
            ItemMagicSync{.Item = 2455, .Magic = 15}, // Megalith Fall SR / Megalith Fall
            ItemMagicSync{.Item = 2456, .Magic = 15}, // Megalith Fall UR / Megalith Fall
            ItemMagicSync{.Item = 2457, .Magic = 16}, // Gravion Hammer / Gravion Hammer
            ItemMagicSync{.Item = 2458, .Magic = 16}, // Gravion Hammer R / Gravion Hammer
            ItemMagicSync{.Item = 2459, .Magic = 16}, // Gravion Hammer SR / Gravion Hammer
            ItemMagicSync{.Item = 2460, .Magic = 16}, // Gravion Hammer UR / Gravion Hammer
            ItemMagicSync{.Item = 2461, .Magic = 18}, // Crest / Crest
            ItemMagicSync{.Item = 2462, .Magic = 18}, // Crest R / Crest
            ItemMagicSync{.Item = 2463, .Magic = 18}, // Crest SR / Crest
            ItemMagicSync{.Item = 2464, .Magic = 18}, // Crest UR / Crest
            ItemMagicSync{.Item = 2465, .Magic = 19}, // La Crest / La Crest
            ItemMagicSync{.Item = 2466, .Magic = 19}, // La Crest R / La Crest
            ItemMagicSync{.Item = 2467, .Magic = 19}, // La Crest SR / La Crest
            ItemMagicSync{.Item = 2468, .Magic = 19}, // La Crest UR / La Crest
            ItemMagicSync{.Item = 2469, .Magic = 17}, // Earth Pulse / Earth Pulse
            ItemMagicSync{.Item = 2470, .Magic = 17}, // Earth Pulse R / Earth Pulse
            ItemMagicSync{.Item = 2471, .Magic = 17}, // Earth Pulse SR / Earth Pulse
            ItemMagicSync{.Item = 2472, .Magic = 17}, // Earth Pulse UR / Earth Pulse
            ItemMagicSync{.Item = 2473, .Magic = 20}, // Adamantine Shield / Adamantine Shield
            ItemMagicSync{.Item = 2474, .Magic = 20}, // Adamantine Shield R / Adamantine Shield
            ItemMagicSync{.Item = 2475, .Magic = 20}, // Adamantine Shield SR / Adamantine Shield
            ItemMagicSync{.Item = 2476, .Magic = 20}, // Adamantine Shield UR / Adamantine Shield
            ItemMagicSync{.Item = 2511, .Magic = 21}, // Aqua Bleed / Aqua Bleed
            ItemMagicSync{.Item = 2512, .Magic = 21}, // Aqua Bleed R / Aqua Bleed
            ItemMagicSync{.Item = 2513, .Magic = 21}, // Aqua Bleed SR / Aqua Bleed
            ItemMagicSync{.Item = 2514, .Magic = 21}, // Aqua Bleed UR / Aqua Bleed
            ItemMagicSync{.Item = 2515, .Magic = 22}, // Crystal Edge / Crystal Edge
            ItemMagicSync{.Item = 2516, .Magic = 22}, // Crystal Edge R / Crystal Edge
            ItemMagicSync{.Item = 2517, .Magic = 22}, // Crystal Edge SR / Crystal Edge
            ItemMagicSync{.Item = 2518, .Magic = 22}, // Crystal Edge UR / Crystal Edge
            ItemMagicSync{.Item = 2519, .Magic = 23}, // Blue Ascension / Blue Ascension
            ItemMagicSync{.Item = 2520, .Magic = 23}, // Blue Ascension R / Blue Ascension
            ItemMagicSync{.Item = 2521, .Magic = 23}, // Blue Ascension SR / Blue Ascension
            ItemMagicSync{.Item = 2522, .Magic = 23}, // Blue Ascension UR / Blue Ascension
            ItemMagicSync{.Item = 2523, .Magic = 24}, // Diamond Nova / Diamond Nova
            ItemMagicSync{.Item = 2524, .Magic = 24}, // Diamond Nova R / Diamond Nova
            ItemMagicSync{.Item = 2525, .Magic = 24}, // Diamond Nova SR / Diamond Nova
            ItemMagicSync{.Item = 2526, .Magic = 24}, // Diamond Nova UR / Diamond Nova
            ItemMagicSync{.Item = 2527, .Magic = 25}, // Tear / Tear
            ItemMagicSync{.Item = 2528, .Magic = 25}, // Tear R / Tear
            ItemMagicSync{.Item = 2529, .Magic = 25}, // Tear SR / Tear
            ItemMagicSync{.Item = 2530, .Magic = 25}, // Tear UR / Tear
            ItemMagicSync{.Item = 2531, .Magic = 26}, // Teara / Teara
            ItemMagicSync{.Item = 2532, .Magic = 26}, // Teara R / Teara
            ItemMagicSync{.Item = 2533, .Magic = 26}, // Teara SR / Teara
            ItemMagicSync{.Item = 2534, .Magic = 26}, // Teara UR / Teara
            ItemMagicSync{.Item = 2535, .Magic = 27}, // Tearal / Tearal
            ItemMagicSync{.Item = 2536, .Magic = 27}, // Tearal R / Tearal
            ItemMagicSync{.Item = 2537, .Magic = 27}, // Tearal SR / Tearal
            ItemMagicSync{.Item = 2538, .Magic = 27}, // Tearal UR / Tearal
            ItemMagicSync{.Item = 2539, .Magic = 28}, // Curia / Curia
            ItemMagicSync{.Item = 2540, .Magic = 28}, // Curia R / Curia
            ItemMagicSync{.Item = 2541, .Magic = 28}, // Curia SR / Curia
            ItemMagicSync{.Item = 2542, .Magic = 28}, // Curia UR / Curia
            ItemMagicSync{.Item = 2543, .Magic = 29}, // Thelas / Thelas
            ItemMagicSync{.Item = 2544, .Magic = 29}, // Thelas R / Thelas
            ItemMagicSync{.Item = 2545, .Magic = 29}, // Thelas SR / Thelas
            ItemMagicSync{.Item = 2546, .Magic = 29}, // Thelas UR / Thelas
            ItemMagicSync{.Item = 2547, .Magic = 30}, // Athelas / Athelas
            ItemMagicSync{.Item = 2548, .Magic = 30}, // Athelas R / Athelas
            ItemMagicSync{.Item = 2549, .Magic = 30}, // Athelas SR / Athelas
            ItemMagicSync{.Item = 2550, .Magic = 30}, // Athelas UR / Athelas
            ItemMagicSync{.Item = 2584, .Magic = 31}, // Fire Bolt / Fire Bolt
            ItemMagicSync{.Item = 2585, .Magic = 31}, // Fire Bolt R / Fire Bolt
            ItemMagicSync{.Item = 2586, .Magic = 31}, // Fire Bolt SR / Fire Bolt
            ItemMagicSync{.Item = 2587, .Magic = 31}, // Fire Bolt UR / Fire Bolt
            ItemMagicSync{.Item = 2588, .Magic = 32}, // Venom Flame / Venom Flame
            ItemMagicSync{.Item = 2589, .Magic = 32}, // Venom Flame R / Venom Flame
            ItemMagicSync{.Item = 2590, .Magic = 32}, // Venom Flame SR / Venom Flame
            ItemMagicSync{.Item = 2591, .Magic = 32}, // Venom Flame UR / Venom Flame
            ItemMagicSync{.Item = 2592, .Magic = 33}, // Flare Butterfly / Flare Butterfly
            ItemMagicSync{.Item = 2593, .Magic = 33}, // Flare Butterfly R / Flare Butterfly
            ItemMagicSync{.Item = 2594, .Magic = 33}, // Flare Butterfly SR / Flare Butterfly
            ItemMagicSync{.Item = 2595, .Magic = 33}, // Flare Butterfly UR / Flare Butterfly
            ItemMagicSync{.Item = 2596, .Magic = 34}, // Zeruel Cannon / Zeruel Cannon
            ItemMagicSync{.Item = 2597, .Magic = 34}, // Zeruel Cannon R / Zeruel Cannon
            ItemMagicSync{.Item = 2598, .Magic = 34}, // Zeruel Cannon SR / Zeruel Cannon
            ItemMagicSync{.Item = 2599, .Magic = 34}, // Zeruel Cannon UR / Zeruel Cannon
            ItemMagicSync{.Item = 2600, .Magic = 36}, // Forte / Forte
            ItemMagicSync{.Item = 2601, .Magic = 36}, // Forte R / Forte
            ItemMagicSync{.Item = 2602, .Magic = 36}, // Forte SR / Forte
            ItemMagicSync{.Item = 2603, .Magic = 36}, // Forte UR / Forte
            ItemMagicSync{.Item = 2604, .Magic = 37}, // La Forte / La Forte
            ItemMagicSync{.Item = 2605, .Magic = 37}, // La Forte R / La Forte
            ItemMagicSync{.Item = 2606, .Magic = 37}, // La Forte SR / La Forte
            ItemMagicSync{.Item = 2607, .Magic = 37}, // La Forte UR / La Forte
            ItemMagicSync{.Item = 2608, .Magic = 38}, // Heat Up / Heat Up
            ItemMagicSync{.Item = 2609, .Magic = 38}, // Heat Up R / Heat Up
            ItemMagicSync{.Item = 2610, .Magic = 38}, // Heat Up SR / Heat Up
            ItemMagicSync{.Item = 2611, .Magic = 38}, // Heat Up UR / Heat Up
            ItemMagicSync{.Item = 2612, .Magic = 35}, // Impassion / Impassion
            ItemMagicSync{.Item = 2613, .Magic = 35}, // Impassion R / Impassion
            ItemMagicSync{.Item = 2614, .Magic = 35}, // Impassion SR / Impassion
            ItemMagicSync{.Item = 2615, .Magic = 35}, // Impassion UR / Impassion
            ItemMagicSync{.Item = 2650, .Magic = 39}, // Air Strike / Air Strike
            ItemMagicSync{.Item = 2651, .Magic = 39}, // Air Strike R / Air Strike
            ItemMagicSync{.Item = 2652, .Magic = 39}, // Air Strike SR / Air Strike
            ItemMagicSync{.Item = 2653, .Magic = 39}, // Air Strike UR / Air Strike
            ItemMagicSync{.Item = 2654, .Magic = 40}, // Nemesis Arrow / Nemesis Arrow
            ItemMagicSync{.Item = 2655, .Magic = 40}, // Nemesis Arrow R / Nemesis Arrow
            ItemMagicSync{.Item = 2656, .Magic = 40}, // Nemesis Arrow SR / Nemesis Arrow
            ItemMagicSync{.Item = 2657, .Magic = 40}, // Nemesis Arrow UR / Nemesis Arrow
            ItemMagicSync{.Item = 2658, .Magic = 41}, // Aerial Dust / Aerial Dust
            ItemMagicSync{.Item = 2659, .Magic = 41}, // Aerial Dust R / Aerial Dust
            ItemMagicSync{.Item = 2660, .Magic = 41}, // Aerial Dust SR / Aerial Dust
            ItemMagicSync{.Item = 2661, .Magic = 41}, // Aerial Dust UR / Aerial Dust
            ItemMagicSync{.Item = 2662, .Magic = 42}, // Ixion Volt / Ixion Volt
            ItemMagicSync{.Item = 2663, .Magic = 42}, // Ixion Volt R / Ixion Volt
            ItemMagicSync{.Item = 2664, .Magic = 42}, // Ixion Volt SR / Ixion Volt
            ItemMagicSync{.Item = 2665, .Magic = 42}, // Ixion Volt UR / Ixion Volt
            ItemMagicSync{.Item = 2666, .Magic = 43}, // Breath / Breath
            ItemMagicSync{.Item = 2667, .Magic = 43}, // Breath R / Breath
            ItemMagicSync{.Item = 2668, .Magic = 43}, // Breath SR / Breath
            ItemMagicSync{.Item = 2669, .Magic = 43}, // Breath UR / Breath
            ItemMagicSync{.Item = 2670, .Magic = 44}, // Holy Breath / Holy Breath
            ItemMagicSync{.Item = 2671, .Magic = 44}, // Holy Breath R / Holy Breath
            ItemMagicSync{.Item = 2672, .Magic = 44}, // Holy Breath SR / Holy Breath
            ItemMagicSync{.Item = 2673, .Magic = 44}, // Holy Breath UR / Holy Breath
            ItemMagicSync{.Item = 2674, .Magic = 45}, // Recuria / Recuria
            ItemMagicSync{.Item = 2675, .Magic = 45}, // Recuria R / Recuria
            ItemMagicSync{.Item = 2676, .Magic = 45}, // Recuria SR / Recuria
            ItemMagicSync{.Item = 2677, .Magic = 45}, // Recuria UR / Recuria
            ItemMagicSync{.Item = 2713, .Magic = 46}, // Soul Blur / Soul Blur
            ItemMagicSync{.Item = 2714, .Magic = 46}, // Soul Blur R / Soul Blur
            ItemMagicSync{.Item = 2715, .Magic = 46}, // Soul Blur SR / Soul Blur
            ItemMagicSync{.Item = 2716, .Magic = 46}, // Soul Blur UR / Soul Blur
            ItemMagicSync{.Item = 2717, .Magic = 47}, // Calvary Edge / Calvary Edge
            ItemMagicSync{.Item = 2718, .Magic = 47}, // Calvary Edge R / Calvary Edge
            ItemMagicSync{.Item = 2719, .Magic = 47}, // Calvary Edge SR / Calvary Edge
            ItemMagicSync{.Item = 2720, .Magic = 47}, // Calvary Edge UR / Calvary Edge
            ItemMagicSync{.Item = 2721, .Magic = 48}, // Lost Genesis / Lost Genesis
            ItemMagicSync{.Item = 2722, .Magic = 48}, // Lost Genesis R / Lost Genesis
            ItemMagicSync{.Item = 2723, .Magic = 48}, // Lost Genesis SR / Lost Genesis
            ItemMagicSync{.Item = 2724, .Magic = 48}, // Lost Genesis UR / Lost Genesis
            ItemMagicSync{.Item = 2725, .Magic = 49}, // Chrono Drive / Chrono Drive
            ItemMagicSync{.Item = 2726, .Magic = 49}, // Chrono Drive R / Chrono Drive
            ItemMagicSync{.Item = 2727, .Magic = 49}, // Chrono Drive SR / Chrono Drive
            ItemMagicSync{.Item = 2728, .Magic = 49}, // Chrono Drive UR / Chrono Drive
            ItemMagicSync{.Item = 2729, .Magic = 50}, // Chrono Break / Chrono Break
            ItemMagicSync{.Item = 2730, .Magic = 50}, // Chrono Break R / Chrono Break
            ItemMagicSync{.Item = 2731, .Magic = 50}, // Chrono Break SR / Chrono Break
            ItemMagicSync{.Item = 2732, .Magic = 50}, // Chrono Break UR / Chrono Break
            ItemMagicSync{.Item = 2733, .Magic = 51}, // Chrono Burst / Chrono Burst
            ItemMagicSync{.Item = 2734, .Magic = 51}, // Chrono Burst R / Chrono Burst
            ItemMagicSync{.Item = 2735, .Magic = 51}, // Chrono Burst SR / Chrono Burst
            ItemMagicSync{.Item = 2736, .Magic = 51}, // Chrono Burst UR / Chrono Burst
            ItemMagicSync{.Item = 2767, .Magic = 52}, // Golden Sphere / Golden Sphere
            ItemMagicSync{.Item = 2768, .Magic = 52}, // Golden Sphere R / Golden Sphere
            ItemMagicSync{.Item = 2769, .Magic = 52}, // Golden Sphere SR / Golden Sphere
            ItemMagicSync{.Item = 2770, .Magic = 52}, // Golden Sphere UR / Golden Sphere
            ItemMagicSync{.Item = 2771, .Magic = 53}, // Cross Crusade / Cross Crusade
            ItemMagicSync{.Item = 2772, .Magic = 53}, // Cross Crusade R / Cross Crusade
            ItemMagicSync{.Item = 2773, .Magic = 53}, // Cross Crusade SR / Cross Crusade
            ItemMagicSync{.Item = 2774, .Magic = 53}, // Cross Crusade UR / Cross Crusade
            ItemMagicSync{.Item = 2775, .Magic = 54}, // Seventh Caliber / Seventh Caliber
            ItemMagicSync{.Item = 2776, .Magic = 54}, // Seventh Caliber R / Seventh Caliber
            ItemMagicSync{.Item = 2777, .Magic = 54}, // Seventh Caliber SR / Seventh Caliber
            ItemMagicSync{.Item = 2778, .Magic = 54}, // Seventh Caliber UR / Seventh Caliber
            ItemMagicSync{.Item = 2779, .Magic = 55}, // Fortuna / Fortuna
            ItemMagicSync{.Item = 2780, .Magic = 55}, // Fortuna R / Fortuna
            ItemMagicSync{.Item = 2781, .Magic = 55}, // Fortuna SR / Fortuna
            ItemMagicSync{.Item = 2782, .Magic = 55}, // Fortuna UR / Fortuna
            ItemMagicSync{.Item = 2783, .Magic = 56}, // Shining / Shining
            ItemMagicSync{.Item = 2784, .Magic = 56}, // Shining R / Shining
            ItemMagicSync{.Item = 2785, .Magic = 56}, // Shining SR / Shining
            ItemMagicSync{.Item = 2786, .Magic = 56}, // Shining UR / Shining
            ItemMagicSync{.Item = 2787, .Magic = 57}, // Seraphic Ring / Seraphic Ring
            ItemMagicSync{.Item = 2788, .Magic = 57}, // Seraphic Ring R / Seraphic Ring
            ItemMagicSync{.Item = 2789, .Magic = 57}, // Seraphic Ring SR / Seraphic Ring
            ItemMagicSync{.Item = 2790, .Magic = 57}, // Seraphic Ring UR / Seraphic Ring
            ItemMagicSync{.Item = 2822, .Magic = 58}, // Silver Thorn / Silver Thorn
            ItemMagicSync{.Item = 2823, .Magic = 58}, // Silver Thorn R / Silver Thorn
            ItemMagicSync{.Item = 2824, .Magic = 58}, // Silver Thorn SR / Silver Thorn
            ItemMagicSync{.Item = 2825, .Magic = 58}, // Silver Thorn UR / Silver Thorn
            ItemMagicSync{.Item = 2826, .Magic = 59}, // Galion Fort / Galion Fort
            ItemMagicSync{.Item = 2827, .Magic = 59}, // Galion Fort R / Galion Fort
            ItemMagicSync{.Item = 2828, .Magic = 59}, // Galion Fort SR / Galion Fort
            ItemMagicSync{.Item = 2829, .Magic = 59}, // Galion Fort UR / Galion Fort
            ItemMagicSync{.Item = 2830, .Magic = 60}, // Albion Wolf / Albion Wolf
            ItemMagicSync{.Item = 2831, .Magic = 60}, // Albion Wolf R / Albion Wolf
            ItemMagicSync{.Item = 2832, .Magic = 60}, // Albion Wolf SR / Albion Wolf
            ItemMagicSync{.Item = 2833, .Magic = 60}, // Albion Wolf UR / Albion Wolf
            ItemMagicSync{.Item = 2834, .Magic = 61}, // Analyze / Analyze
            ItemMagicSync{.Item = 2835, .Magic = 61}, // Analyze R / Analyze
            ItemMagicSync{.Item = 2836, .Magic = 61}, // Analyze SR / Analyze
            ItemMagicSync{.Item = 2837, .Magic = 61}, // Analyze UR / Analyze
            ItemMagicSync{.Item = 2838, .Magic = 62}, // Saintly Force / Saintly Force
            ItemMagicSync{.Item = 2839, .Magic = 62}, // Saintly Force R / Saintly Force
            ItemMagicSync{.Item = 2840, .Magic = 62}, // Saintly Force SR / Saintly Force
            ItemMagicSync{.Item = 2841, .Magic = 62}, // Saintly Force UR / Saintly Force
            ItemMagicSync{.Item = 2842, .Magic = 63}, // Crescent Mirror / Crescent Mirror
            ItemMagicSync{.Item = 2843, .Magic = 63}, // Crescent Mirror R / Crescent Mirror
            ItemMagicSync{.Item = 2844, .Magic = 63}, // Crescent Mirror SR / Crescent Mirror
            ItemMagicSync{.Item = 2845, .Magic = 63}, // Crescent Mirror UR / Crescent Mirror
            ItemMagicSync{.Item = 2878, .Magic = 64}, // Prominence Nova / Prominence Nova
            ItemMagicSync{.Item = 2879, .Magic = 65}, // Leanan's Kiss / Leanan's Kiss
            ItemMagicSync{.Item = 2880, .Magic = 66}, // Grail Thelas / Grail Thelas
            ItemMagicSync{.Item = 2881, .Magic = 67}, // Frozen Epoch / Frozen Epoch
            ItemMagicSync{.Item = 2882, .Magic = 68}, // Aeonian Emperor / Aeonian Emperor
            ItemMagicSync{.Item = 2883, .Magic = 69}, // Melville Ray / Melville Ray
        };
        for (const ItemMagicSync& info : syncArray) {
            auto& e = tbl_en.Entries[info.Magic];
            auto& e2 = tbl_item.Entries[info.Item];
            MagicData m(e.Data.data(), e.Data.size());
            ItemQData m2(e2.Data.data(), e2.Data.size());

            std::string desc = m.desc;
            if (m2.item.rarity != 0) {
                auto img = desc.find('I');
                if (img == std::string::npos) {
                    continue;
                }
                desc = HyoutaUtils::TextUtils::Insert(desc,
                                                      img + 1,
                                                      m2.item.rarity == 1   ? "(R)"
                                                      : m2.item.rarity == 2 ? "(SR)"
                                                      : m2.item.rarity == 3 ? "(UR)"
                                                                            : "?");
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
                        for (size_t j = i + 1; j < 2; ++j) {
                            if (statArray[i] == statArray[j]) {
                                stats += "\xEF\xBD\xA5";
                                stats += statNames[j];
                                statArray[j] = 0;
                            }
                        }
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
                        for (size_t j = i + 1; j < 8; ++j) {
                            if (statArray[i] == statArray[j]) {
                                stats += "\xEF\xBD\xA5";
                                stats += statNames[j];
                                statArray[j] = 0;
                            }
                        }
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
                    } else if (desc[insertPos] == '#') {
                        ++insertPos;
                        auto v = TryParseLengthCode(desc, insertPos);
                        if (!v) {
                            --insertPos;
                            break;
                        }
                    } else {
                        break;
                    }
                }
                desc = HyoutaUtils::TextUtils::Insert(desc, insertPos, stats);
            }

            m2.item.desc = std::move(desc);
            e2.Data = m2.ToBinary();
        }

        // post-sync fix: apply #L codes to the very long stat boosts of the lost arts quartz so
        // their description fits on screen
        for (int idx : {2879, 2881, 2882, 2883}) {
            auto& e = tbl_item.Entries[static_cast<size_t>(idx)];
            ItemQData m(e.Data.data(), e.Data.size());
            auto newline = m.item.desc.find('\n');
            if (newline != std::string::npos) {
                m.item.desc = HyoutaUtils::TextUtils::Insert(m.item.desc, newline + 3, "#80L");
                e.Data = m.ToBinary();
            }
        }

        // normalize newlines
        for (size_t i = 0; i < tbl_item.Entries.size(); ++i) {
            auto& e = tbl_item.Entries[i];
            if (e.Name == "item") {
                ItemData m(e.Data.data(), e.Data.size());
                if (AdjustSpacingForBracketed(m.desc)) {
                    e.Data = m.ToBinary();
                }
            } else if (e.Name == "item_e") {
                ItemEData m(e.Data.data(), e.Data.size());
                if (AdjustSpacingForBracketed(m.item.desc)) {
                    e.Data = m.ToBinary();
                }
            } else if (e.Name == "item_q") {
                ItemQData m(e.Data.data(), e.Data.size());
                if (AdjustSpacingForBracketed(m.item.desc)) {
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

        std::vector<char> result_en_vec;
        HyoutaUtils::Stream::MemoryStream result_en(result_en_vec);
        tbl_en.WriteToStream(result_en, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        std::vector<char> result_item_vec;
        HyoutaUtils::Stream::MemoryStream result_item(result_item_vec);
        tbl_item.WriteToStream(result_item, HyoutaUtils::EndianUtils::Endianness::LittleEndian);


        result.emplace_back(
            std::move(result_en_vec), file->Filename, SenPatcher::P3ACompressionType::LZ4);
        result.emplace_back(
            std::move(result_item_vec), file_item->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen5::FileFixes::t_item_magic
