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
__declspec(dllexport) char SenPatcherFix_0_magic[] = "Fix errors in craft descriptions.";
}

namespace SenLib::Sen5::FileFixes::t_magic {
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

        auto& bin = file->Data;

        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

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
        // 'Restores x% HP for n turns' but here it's listed as 'HP+x% for n turns'
        {
            auto& e = tbl_en.Entries[113];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 70, 6, "Restores 50% HP");
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[114];
            MagicData m(e.Data.data(), e.Data.size());
            m.desc = HyoutaUtils::TextUtils::ReplaceSubstring(m.desc, 70, 6, "Restores 50% HP");
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

        // remove Z flag to compare with autogenerated descriptions
        //for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
        //    auto& e = tbl_en.Entries[i];
        //    if (e.Name == "magic") {
        //        MagicData m(e.Data.data(), e.Data.size());
        //        auto pos = m.flags.find('Z');
        //        if (pos != std::string::npos) {
        //            m.flags.erase(m.flags.begin() + pos);
        //        }
        //        e.Data = m.ToBinary();
        //    }
        //}

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
} // namespace SenLib::Sen5::FileFixes::t_magic
