#include <array>
#include <cstring>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

#include "../file_fixes_sw/dungeon_names.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_a0000[] = "Translate the main debug map.";
}

#define STR_WITH_LEN(text) text, (sizeof(text) - 1)
#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixes::a0000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "scripts/scena/dat/a0000.dat",
            140545,
            HyoutaUtils::Hash::SHA1FromHexString("0e39c642792a2874f1b62bae549a0fd707ab11c7"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);


        // clang-format off

        patcher.ReplacePartialCommand(0x110f, 0x5a, 0x111b, 0x10, STR_SPAN("NiAR Debug"));
        patcher.ReplacePartialCommand(0x1169, 0x6a, 0x1175, 0x21, STR_SPAN("Quest Jump"));
        patcher.ReplacePartialCommand(0x11d3, 0x65, 0x11df, 0x1e, STR_SPAN("Event Jump"));
        patcher.ReplacePartialCommand(0x1238, 0x61, 0x1244, 0x15, STR_SPAN("Battle Debug"));
        patcher.ReplacePartialCommand(0x1299, 0x67, 0x12a5, 0x1b, STR_SPAN("System Debug"));
        patcher.ReplacePartialCommand(0x1300, 0x6c, 0x130c, 0x1e, STR_SPAN("Minigame Debug"));
        patcher.ReplacePartialCommand(0x136c, 0x65, 0x1378, 0x1b, STR_SPAN("Shop Debug"));

        // TK_Battle_Debug
        patcher.ReplacePartialCommand(0x14a6, 0x12, 0x14a9, 0xc, STR_SPAN("Open Preparation Screen"));
        patcher.ReplacePartialCommand(0x14b8, 0x18, 0x14bb, 0x12, STR_SPAN("Reset Skills"));
        patcher.ReplacePartialCommand(0x14d0, 0x15, 0x14d3, 0xf, STR_SPAN("Get Skills"));
        patcher.ReplacePartialCommand(0x14e5, 0x2a, 0x14e8, 0x24, STR_SPAN("Unlock Grid"));
        patcher.ReplacePartialCommand(0x150f, 0x27, 0x1512, 0x21, STR_SPAN("Set to Chapter 1-5"));
        patcher.ReplacePartialCommand(0x1536, 0x30, 0x1539, 0x2a, STR_SPAN("Set to Intermission-Final"));
        patcher.ReplacePartialCommand(0x1566, 0x21, 0x1569, 0x1b, STR_SPAN("Remove Master Core"));
        patcher.ReplacePartialCommand(0x1587, 0x1e, 0x158a, 0x18, STR_SPAN("Clear all dungeons"));
        patcher.ReplacePartialCommand(0x15a5, 0x21, 0x15a8, 0x1b, STR_SPAN("Switch XS availability"));
        patcher.ReplacePartialCommand(0x165e, 0x1e, 0x1661, 0x18, STR_SPAN("Kou: All Lv1"));
        patcher.ReplacePartialCommand(0x167c, 0x1e, 0x167f, 0x18, STR_SPAN("Kou: All Lv2"));
        patcher.ReplacePartialCommand(0x169a, 0x1e, 0x169d, 0x18, STR_SPAN("Kou: All Lv3"));
        patcher.ReplacePartialCommand(0x16b8, 0x1e, 0x16bb, 0x18, STR_SPAN("Asuka: All Lv1"));
        patcher.ReplacePartialCommand(0x16d6, 0x1e, 0x16d9, 0x18, STR_SPAN("Asuka: All Lv2"));
        patcher.ReplacePartialCommand(0x16f4, 0x1e, 0x16f7, 0x18, STR_SPAN("Asuka: All Lv3"));
        patcher.ReplacePartialCommand(0x1712, 0x1e, 0x1715, 0x18, STR_SPAN("Sora: All Lv1"));
        patcher.ReplacePartialCommand(0x1730, 0x1e, 0x1733, 0x18, STR_SPAN("Sora: All Lv2"));
        patcher.ReplacePartialCommand(0x174e, 0x1e, 0x1751, 0x18, STR_SPAN("Sora: All Lv3"));
        patcher.ReplacePartialCommand(0x176c, 0x1e, 0x176f, 0x18, STR_SPAN("Yuuki: All Lv1"));
        patcher.ReplacePartialCommand(0x178a, 0x1e, 0x178d, 0x18, STR_SPAN("Yuuki: All Lv2"));
        patcher.ReplacePartialCommand(0x17a8, 0x1e, 0x17ab, 0x18, STR_SPAN("Yuuki: All Lv3"));
        patcher.ReplacePartialCommand(0x17c6, 0x1e, 0x17c9, 0x18, STR_SPAN("Shio: All Lv1"));
        patcher.ReplacePartialCommand(0x17e4, 0x1e, 0x17e7, 0x18, STR_SPAN("Shio: All Lv2"));
        patcher.ReplacePartialCommand(0x1802, 0x1e, 0x1805, 0x18, STR_SPAN("Shio: All Lv3"));
        patcher.ReplacePartialCommand(0x1820, 0x1e, 0x1823, 0x18, STR_SPAN("Mitsuki: All Lv1"));
        patcher.ReplacePartialCommand(0x183e, 0x1e, 0x1841, 0x18, STR_SPAN("Mitsuki: All Lv2"));
        patcher.ReplacePartialCommand(0x185c, 0x1e, 0x185f, 0x18, STR_SPAN("Mitsuki: All Lv3"));
        patcher.ReplacePartialCommand(0x187a, 0x1e, 0x187d, 0x18, STR_SPAN("Rion: All Lv1"));
        patcher.ReplacePartialCommand(0x1898, 0x1e, 0x189b, 0x18, STR_SPAN("Rion: All Lv2"));
        patcher.ReplacePartialCommand(0x18b6, 0x1e, 0x18b9, 0x18, STR_SPAN("Rion: All Lv3"));
        patcher.ReplacePartialCommand(0x18d4, 0x1e, 0x18d7, 0x18, STR_SPAN("Gorou: All Lv1"));
        patcher.ReplacePartialCommand(0x18f2, 0x1e, 0x18f5, 0x18, STR_SPAN("Gorou: All Lv2"));
        patcher.ReplacePartialCommand(0x1910, 0x1e, 0x1913, 0x18, STR_SPAN("Gorou: All Lv3"));
        patcher.ReplacePartialCommand(0x192e, 0x1e, 0x1931, 0x18, STR_SPAN("Everyone: Ranged Lv1"));
        patcher.ReplacePartialCommand(0x194c, 0x1e, 0x194f, 0x18, STR_SPAN("Everyone: Power Lv1"));
        patcher.ReplacePartialCommand(0x196a, 0x1e, 0x196d, 0x18, STR_SPAN("Everyone: Aerial Lv1"));
        patcher.ReplacePartialCommand(0x1d5a, 0x3b, 0x1d5d, 0x35, STR_SPAN("Kou: All Grid Max"));
        patcher.ReplacePartialCommand(0x1d95, 0x3b, 0x1d98, 0x35, STR_SPAN("Asuka: All Grid Max"));
        patcher.ReplacePartialCommand(0x1dd0, 0x3b, 0x1dd3, 0x35, STR_SPAN("Sora: All Grid Max"));
        patcher.ReplacePartialCommand(0x1e0b, 0x3b, 0x1e0e, 0x35, STR_SPAN("Yuuki: All Grid Max"));
        patcher.ReplacePartialCommand(0x1e46, 0x3b, 0x1e49, 0x35, STR_SPAN("Shio: All Grid Max"));
        patcher.ReplacePartialCommand(0x1e81, 0x3b, 0x1e84, 0x35, STR_SPAN("Mitsuki: All Grid Max"));
        patcher.ReplacePartialCommand(0x1ebc, 0x3b, 0x1ebf, 0x35, STR_SPAN("Rion: All Grid Max"));
        patcher.ReplacePartialCommand(0x1ef7, 0x3b, 0x1efa, 0x35, STR_SPAN("Gorou: All Grid Max"));
        patcher.ReplacePartialCommand(0x1f32, 0x36, 0x1f35, 0x30, STR_SPAN("Everyone: All Grid Lv0"));
        patcher.ReplacePartialCommand(0x1f68, 0x36, 0x1f6b, 0x30, STR_SPAN("Everyone: All Grid Lv1"));
        patcher.ReplacePartialCommand(0x1f9e, 0x36, 0x1fa1, 0x30, STR_SPAN("Everyone: All Grid Lv2"));
        patcher.ReplacePartialCommand(0x1fd4, 0x36, 0x1fd7, 0x30, STR_SPAN("Everyone: All Grid Lv3"));
        patcher.ReplacePartialCommand(0x23d1, 0x15, 0x23d4, 0xf, STR_SPAN("Game Start"));
        patcher.ReplacePartialCommand(0x23e6, 0x18, 0x23e9, 0x12, STR_SPAN("Chapter 1 Boss"));
        patcher.ReplacePartialCommand(0x23fe, 0x18, 0x2401, 0x12, STR_SPAN("Chapter 2 Midboss"));
        patcher.ReplacePartialCommand(0x2416, 0x15, 0x2419, 0xf, STR_SPAN("Chapter 2 Boss"));
        patcher.ReplacePartialCommand(0x242b, 0x18, 0x242e, 0x12, STR_SPAN("Chapter 3 Midboss"));
        patcher.ReplacePartialCommand(0x2443, 0x15, 0x2446, 0xf, STR_SPAN("Chapter 3 Boss"));
        patcher.ReplacePartialCommand(0x2458, 0x18, 0x245b, 0x12, STR_SPAN("Chapter 4 Midboss"));
        patcher.ReplacePartialCommand(0x2470, 0x15, 0x2473, 0xf, STR_SPAN("Chapter 4 Boss"));
        patcher.ReplacePartialCommand(0x2485, 0x1b, 0x2488, 0x15, STR_SPAN("Chapter 5 Midboss 1"));
        patcher.ReplacePartialCommand(0x24a0, 0x1b, 0x24a3, 0x15, STR_SPAN("Chapter 5 Midboss 2"));
        patcher.ReplacePartialCommand(0x24bb, 0x1b, 0x24be, 0x15, STR_SPAN("Chapter 5 Midboss 3"));
        patcher.ReplacePartialCommand(0x24d6, 0x15, 0x24d9, 0xf, STR_SPAN("Chapter 5 Boss"));
        patcher.ReplacePartialCommand(0x26ca, 0x15, 0x26cd, 0xf, STR_SPAN("Intermission Boss"));
        patcher.ReplacePartialCommand(0x26df, 0x1b, 0x26e2, 0x15, STR_SPAN("Chapter 6 Midboss 1"));
        patcher.ReplacePartialCommand(0x26fa, 0x1b, 0x26fd, 0x15, STR_SPAN("Chapter 6 Midboss 2"));
        patcher.ReplacePartialCommand(0x2715, 0x1b, 0x2718, 0x15, STR_SPAN("Chapter 6 Midboss 3"));
        patcher.ReplacePartialCommand(0x2730, 0x15, 0x2733, 0xf, STR_SPAN("Chapter 6 Boss"));
        patcher.ReplacePartialCommand(0x2745, 0x1b, 0x2748, 0x15, STR_SPAN("Chapter 7 Midboss 1"));
        patcher.ReplacePartialCommand(0x2760, 0x1b, 0x2763, 0x15, STR_SPAN("Chapter 7 Midboss 2"));
        patcher.ReplacePartialCommand(0x277b, 0x1b, 0x277e, 0x15, STR_SPAN("Chapter 7 Midboss 3"));
        patcher.ReplacePartialCommand(0x2796, 0x15, 0x2799, 0xf, STR_SPAN("Chapter 7 Boss"));
        patcher.ReplacePartialCommand(0x27ab, 0x1b, 0x27ae, 0x15, STR_SPAN("Chapter 8 Midboss 1"));
        patcher.ReplacePartialCommand(0x27c6, 0x1b, 0x27c9, 0x15, STR_SPAN("Chapter 8 Midboss 2"));
        patcher.ReplacePartialCommand(0x27e1, 0x1b, 0x27e4, 0x15, STR_SPAN("Chapter 8 Midboss 3"));
        patcher.ReplacePartialCommand(0x27fc, 0x1b, 0x27ff, 0x15, STR_SPAN("Chapter 8 Midboss 4"));
        patcher.ReplacePartialCommand(0x2817, 0x18, 0x281a, 0x12, STR_SPAN("Chapter 8 Boss 1"));
        patcher.ReplacePartialCommand(0x2a7e, 0x11, 0x2a81, 0xc, STR_SPAN("XS locked"));
        patcher.ReplacePartialCommand(0x2aa0, 0x11, 0x2aa3, 0xc, STR_SPAN("XS unlocked"));

        // TK_System_Debug
        patcher.ReplacePartialCommand(0x2ad2, 0x27, 0x2ad5, 0x21, STR_SPAN("Active Voice test"));
        patcher.ReplacePartialCommand(0x2af9, 0x21, 0x2afc, 0x1b, STR_SPAN("Voicemail test"));
        patcher.ReplacePartialCommand(0x2b1a, 0x2d, 0x2b1d, 0x27, STR_SPAN("Real-time announcement test"));
        patcher.ReplacePartialCommand(0x2b47, 0x3e, 0x2b4a, 0x38, STR_SPAN("Real-time announcement test (during event)"));
        patcher.ReplacePartialCommand(0x2b85, 0x21, 0x2b88, 0x1b, STR_SPAN("Soul level test"));
        patcher.ReplacePartialCommand(0x2ba6, 0x21, 0x2ba9, 0x1b, STR_SPAN("Event image test"));
        patcher.ReplacePartialCommand(0x2bc7, 0x1b, 0x2bca, 0x15, STR_SPAN("Character name test"));
        patcher.ReplacePartialCommand(0x2be2, 0x20, 0x2be5, 0x1a, STR_SPAN("Face popup test"));
        patcher.ReplacePartialCommand(0x2c02, 0x21, 0x2c05, 0x1b, STR_SPAN("Face popup test 2"));
        patcher.ReplacePartialCommand(0x2c23, 0x24, 0x2c26, 0x1e, STR_SPAN("Bustup test"));
        patcher.ReplacePartialCommand(0x2c47, 0x1b, 0x2c4a, 0x15, STR_SPAN("Calendar test"));
        patcher.ReplacePartialCommand(0x2c62, 0x30, 0x2c65, 0x2a, STR_SPAN("Calendar test (with generated noise)"));
        patcher.ReplacePartialCommand(0x2c92, 0x1e, 0x2c95, 0x18, STR_SPAN("Book test"));
        patcher.ReplacePartialCommand(0x2cb0, 0x1b, 0x2cb3, 0x15, STR_SPAN("Location select test"));
        patcher.ReplacePartialCommand(0x2ccb, 0x24, 0x2cce, 0x1e, STR_SPAN("Map jump test"));
        patcher.ReplacePartialCommand(0x2cef, 0x1b, 0x2cf2, 0x15, STR_SPAN("Button menu test"));
        patcher.ReplacePartialCommand(0x2d0a, 0x24, 0x2d0d, 0x1e, STR_SPAN("[QS0402] Shoe Quest test"));
        patcher.ReplacePartialCommand(0x2d2e, 0x24, 0x2d31, 0x1e, STR_SPAN("Gate Search test"));
        patcher.ReplacePartialCommand(0x2d52, 0x21, 0x2d55, 0x1b, STR_SPAN("Affinity Shard test"));
        patcher.ReplacePartialCommand(0x2d73, 0x15, 0x2d76, 0xf, STR_SPAN("Font test"));
        patcher.ReplacePartialCommand(0x2d88, 0x18, 0x2d8b, 0x12, STR_SPAN("Save test"));
        patcher.ReplacePartialCommand(0x2da0, 0x19, 0x2da3, 0x13, STR_SPAN("Loading test"));
        patcher.ReplacePartialCommand(0x2db9, 0x14, 0x2dbc, 0xe, STR_SPAN("FMV test"));
        patcher.ReplacePartialCommand(0x2dcd, 0x1e, 0x2dd0, 0x18, STR_SPAN("Monotone test"));
        patcher.ReplacePartialCommand(0x2deb, 0x24, 0x2dee, 0x1e, STR_SPAN("Switch temporary item list"));
        patcher.ReplacePartialCommand(0x2e0f, 0x1b, 0x2e12, 0x15, STR_SPAN("Blur test"));

        // TK_System_Debug_AVoiceMail
        patcher.ReplacePartialCommand(0x322a, 0x18, 0x322d, 0x12, STR_SPAN("Voicemail"));
        patcher.ReplacePartialCommand(0x3242, 0x1b, 0x3245, 0x15, STR_SPAN("E-Card"));
        patcher.ReplacePartialCommand(0x325d, 0x1e, 0x3260, 0x18, STR_SPAN("Remove Voicemail"));
        patcher.ReplacePartialCommand(0x327b, 0x18, 0x327e, 0x12, STR_SPAN("Money Check"));
        patcher.ReplacePartialCommand(0x3307, 0x15, 0x330a, 0x10, STR_SPAN("Yen >= 1000"));
        patcher.ReplacePartialCommand(0x3322, 0x15, 0x3325, 0x10, STR_SPAN("Yen < 1000"));
        patcher.ReplacePartialCommand(0x3355, 0x15, 0x3358, 0x10, STR_SPAN("Gems >= 1000"));
        patcher.ReplacePartialCommand(0x3370, 0x15, 0x3373, 0x10, STR_SPAN("Gems < 1000"));
        patcher.ReplacePartialCommand(0x33a3, 0x18, 0x33a6, 0x13, STR_SPAN("Medals >= 1000"));
        patcher.ReplacePartialCommand(0x33c1, 0x18, 0x33c4, 0x13, STR_SPAN("Medals < 1000"));
        patcher.ReplacePartialCommand(0x33f2, 0x75, 0x33f5, 0x70, STR_SPAN("Voicemails only available in town." "\x01" "Town flag force-enabled for testing."));

        // TK_System_DebugSoulLevel
        patcher.ReplacePartialCommand(0x35e2, 0x27, 0x35e5 + 3, 0x21 - 3, STR_SPAN("Use Soul System"));
        patcher.ReplacePartialCommand(0x360e, 0x27, 0x3611 + 3, 0x21 - 3, STR_SPAN("Use Soul System"));
        patcher.ReplacePartialCommand(0x3635, 0x27, 0x3638, 0x21, STR_SPAN("Soul Level +1"));
        patcher.ReplacePartialCommand(0x365c, 0x1f, 0x365f, 0x19, STR_SPAN("Soul Points +200"));
        patcher.ReplacePartialCommand(0x367b, 0x20, 0x367e, 0x1a, STR_SPAN("Soul Points +9000"));
        patcher.ReplacePartialCommand(0x369b, 0x21, 0x369e, 0x1b, STR_SPAN("Soul Level Max"));
        patcher.ReplacePartialCommand(0x36bc, 0x19, 0x36bf, 0x13, STR_SPAN("Soul Level 0"));
        patcher.ReplacePartialCommand(0x36d5, 0x33, 0x36d8, 0x2d, STR_SPAN("Soul Point gain test (one)"));
        patcher.ReplacePartialCommand(0x3708, 0x33, 0x370b, 0x2d, STR_SPAN("Soul Point gain test (multiple)"));
        patcher.ReplacePartialCommand(0x373b, 0x5d, 0x373e, 0x57, STR_SPAN("Soul Point gain test (multiple, only display level up)"));
        patcher.ReplacePartialCommand(0x3798, 0x40, 0x379b, 0x3a, STR_SPAN("Soul Level limit 4 (invalid since v1.01)"));
        patcher.ReplacePartialCommand(0x37d8, 0x40, 0x37db, 0x3a, STR_SPAN("Soul Level limit 5 (invalid since v1.01)"));

        // TK_System_Debug_ScreenSelect
        patcher.ReplacePartialCommand(0x4ba6, 0x2e, 0x4bab + 5, 0x27 - 5, STR_SPAN("Invalid location"));
        patcher.ReplacePartialCommand(0x4bea, 0x16, 0x4bef + 5, 0xf - 5, STR_SPAN("Station Square"));
        patcher.ReplacePartialCommand(0x4c16, 0x16, 0x4c1b + 5, 0xf - 5, STR_SPAN("Memorial Park"));
        patcher.ReplacePartialCommand(0x4c42, 0x19, 0x4c47 + 5, 0x12 - 5, STR_SPAN("Nanahoshi Mall"));

        // TK_System_Debug_MapJump
        patcher.ReplacePartialCommand(0x4ca9, 0x21, 0x4cac, 0x1b, STR_SPAN("Open map jump"));
        patcher.ReplacePartialCommand(0x4cca, 0x27, 0x4ccd, 0x21, STR_SPAN("Event icon test"));
        patcher.ReplacePartialCommand(0x4cf1, 0x2c, 0x4cf4, 0x26, STR_SPAN("Add Otherworld tab (all dungeons)"));
        patcher.ReplacePartialCommand(0x4d1d, 0x21, 0x4d20, 0x1b, STR_SPAN("Add Affinity Shards"));
        patcher.ReplacePartialCommand(0x4d3e, 0x3f, 0x4d41, 0x39, STR_SPAN("No destinations (only map shown)"));
        patcher.ReplacePartialCommand(0x4d7d, 0x32, 0x4d80, 0x2c - 15, STR_SPAN("Disallow map jump L/R "));
        patcher.ReplacePartialCommand(0x4daf, 0x27, 0x4db2, 0x21, STR_SPAN("Partially disallow map jump"));
        patcher.ReplacePartialCommand(0x4dd6, 0x47, 0x4dd9, 0x41 - 20, STR_SPAN("Map jump after event "));
        patcher.ReplacePartialCommand(0x4e1d, 0x48, 0x4e20, 0x42, STR_SPAN("Map jump event (select Nanahoshi Mall)"));
        patcher.ReplacePartialCommand(0x4e65, 0x24, 0x4e68, 0x1e, STR_SPAN("Map jump (night)"));
        patcher.ReplacePartialCommand(0x4e90, 0x23, 0x4e93, 0x15, STR_SPAN("Disallow cancel"));
        patcher.ReplacePartialCommand(0x502e, 0x2f, 0x5031, 0x2a, STR_SPAN("Affinity Shards were added."));
        patcher.ReplacePartialCommand(0x5065, 0x51, 0x5068 + 3, 0x4c - 3, STR_SPAN("Test this via event [00_03_04]."));
        patcher.ReplacePartialCommand(0x50be, 0x51, 0x50c1 + 3, 0x4c - 3, STR_SPAN("Test this via event [01_04_00]."));
        patcher.ReplacePartialCommand(0x5117, 0x51, 0x511a + 3, 0x4c - 3, STR_SPAN("Test this via event [02_09_02]."));
        patcher.ReplacePartialCommand(0x5170, 0x62, 0x5173 + 3, 0x5d - 3, STR_SPAN("Test this via event [01_04_00]. (select \"Leave School\")"));

        // TK_System_Debug_ButtonMenu
        patcher.ReplacePartialCommand(0x5236, 0xf, 0x5239, 0x9, STR_SPAN("Option 1"));
        patcher.ReplacePartialCommand(0x5245, 0xf, 0x5248, 0x9, STR_SPAN("Option 2"));
        patcher.ReplacePartialCommand(0x5254, 0x24, 0x5257, 0x1e, STR_SPAN("Option 3 (disabled)"));
        patcher.ReplacePartialCommand(0x5278, 0x24, 0x527b, 0x1e, STR_SPAN("Option 4 (disabled)"));
        patcher.ReplacePartialCommand(0x529c, 0xf, 0x529f, 0x9, STR_SPAN("Option 5"));

        // TK_System_FreePoint
        patcher.ReplacePartialCommand(0x532d, 0x12, 0x5330, 0xc, STR_SPAN("Add item"));
        patcher.ReplacePartialCommand(0x533f, 0x24, 0x5342, 0x1e, STR_SPAN("Shard acquire test"));
        patcher.ReplacePartialCommand(0x5363, 0x28, 0x5366, 0x22 - 4, STR_SPAN("Shard use test "));
        patcher.ReplacePartialCommand(0x538b, 0x28, 0x538e, 0x22 - 4, STR_SPAN("Shard use test "));
        patcher.ReplacePartialCommand(0x53e9, 0x78, 0x53ec, 0x73, STR_SPAN("Affinity Shards can only be found in town." "\x01" "Town flag force-enabled for testing."));
        patcher.ReplacePartialCommand(0x5573, 0x1e, 0x5576, 0x18, STR_SPAN("Use Affinity Shard"));
        patcher.ReplacePartialCommand(0x5591, 0x12, 0x5594, 0xc, STR_SPAN("Do Not Use"));
        patcher.ReplacePartialCommand(0x5651, 0x1e, 0x5654, 0x18, STR_SPAN("Use Affinity Shard"));
        patcher.ReplacePartialCommand(0x566f, 0x12, 0x5672, 0xc, STR_SPAN("Do Not Use"));

        // TK_SaveTest
        patcher.ReplacePartialCommand(0x58ed, 0x15, 0x58f0, 0xf, STR_SPAN("Save between chapters"));
        patcher.ReplacePartialCommand(0x5902, 0x18, 0x5905, 0x12, STR_SPAN("Clear data"));
        patcher.ReplacePartialCommand(0x591a, 0x1e, 0x591d, 0x18, STR_SPAN("After normal ending"));
        patcher.ReplacePartialCommand(0x5938, 0x2a, 0x593b, 0x24, STR_SPAN("Carryover reflection (Status)"));
        patcher.ReplacePartialCommand(0x5962, 0x27, 0x5965, 0x21, STR_SPAN("Carryover reflection (Items)"));
        patcher.ReplacePartialCommand(0x5b2a, 0x3c, 0x5b2d, 0x37, STR_SPAN("#1CThe path to the True Ending is now open."));
        patcher.ReplacePartialCommand(0x5b6b, 0x6a, 0x5b6e, 0x65, STR_SPAN("#1CYou can access it by #3Cwatching the epilogue again#1C."));
        patcher.ReplacePartialCommand(0x5be5, 0xb5, 0x5be8, 0xb0, STR_SPAN("#1CYou can load your save data from either the #3Cbeginning of the" "\x01" "epilogue#1C or just before the #3Cthe final battle#1C."));

        // TK_NowLoadingTest
        patcher.ReplacePartialCommand(0x5d07, 0x2c, 0x5d0a, 0x27, STR_SPAN("Please wait a moment."));

        // TK_System_Debug_BlurTest
        patcher.ReplacePartialCommand(0x5e3d, 0x3f, 0x5e40, 0x39, STR_SPAN("Blur test (regular) *No effect if not moving"));
        patcher.ReplacePartialCommand(0x5e7c, 0x20, 0x5e7f, 0x1a, STR_SPAN("Blur test (zoom)"));
        patcher.ReplacePartialCommand(0x5e9c, 0x29, 0x5e9f, 0x23, STR_SPAN("Blur test (dash)"));

        // TK_MiniGame_Debug
        patcher.ReplacePartialCommand(0x5f65, 0x19, 0x5f68, 0x13 - 7, STR_SPAN("Skateboarding "));
        patcher.ReplacePartialCommand(0x5f7e, 0x24, 0x5f81, 0x1e, STR_SPAN("Open all skateboarding courses"));
        patcher.ReplacePartialCommand(0x5fa2, 0x1e, 0x5fa5, 0x18, STR_SPAN("Mishy Panic"));
        patcher.ReplacePartialCommand(0x5fc0, 0xc, 0x5fc3, 0x6, STR_SPAN("Fishing"));
        patcher.ReplacePartialCommand(0x5fcc, 0x15, 0x5fcf, 0xf, STR_SPAN("Blade II"));
        patcher.ReplacePartialCommand(0x5fe1, 0x1a, 0x5fe4, 0x14 - 5, STR_SPAN("Get medals "));
        patcher.ReplacePartialCommand(0x5ffb, 0x1a, 0x5ffe, 0x14 - 5, STR_SPAN("Get medals "));
        patcher.ReplacePartialCommand(0x6015, 0x1b, 0x6018, 0x15 - 6, STR_SPAN("Get medals "));
        patcher.ReplacePartialCommand(0x6030, 0x1b, 0x6033, 0x15 - 6, STR_SPAN("Get medals "));
        patcher.ReplacePartialCommand(0x604b, 0x15, 0x604e, 0xf, STR_SPAN("Remove medals"));
        patcher.ReplacePartialCommand(0x6060, 0xc, 0x6063, 0x6, STR_SPAN("Swimming"));

        // TK_Shop_Debug
        patcher.ReplacePartialCommand(0x6ff9, 0xf, 0x6ffc, 0x9, STR_SPAN("Shop"));
        patcher.ReplacePartialCommand(0x7008, 0x21, 0x700b, 0x1b, STR_SPAN("Soul Device Enhancement"));
        patcher.ReplacePartialCommand(0x7029, 0x21, 0x702c, 0x1b, STR_SPAN("Element Shop"));
        patcher.ReplacePartialCommand(0x704a, 0xf, 0x704d, 0x9, STR_SPAN("Pawn Shop"));
        patcher.ReplacePartialCommand(0x7059, 0x12, 0x705c, 0xc, STR_SPAN("Medal Exchange"));
        patcher.ReplacePartialCommand(0x706b, 0xc, 0x706e, 0x6, STR_SPAN("Book Shop"));
        patcher.ReplacePartialCommand(0x7077, 0x12, 0x707a, 0xc, STR_SPAN("Relic"));
        patcher.ReplacePartialCommand(0x7089, 0x24, 0x708c, 0x1e, STR_SPAN("Add Money & Materials"));
        patcher.ReplacePartialCommand(0x71a1, 0x46, 0x71a4, 0x40, STR_SPAN("Obtained materials, money, and gems!"));

        // TK_NiAR_Debug
        patcher.ReplacePartialCommand(0x7206, 0x1b, 0x7209, 0x15, STR_SPAN("Camp menu test"));
        patcher.ReplacePartialCommand(0x7221, 0x13, 0x7224, 0xd, STR_SPAN("NiAR test"));
        patcher.ReplacePartialCommand(0x7280, 0x1d, 0x7283, 0xf, STR_SPAN("Disable saving"));
        patcher.ReplacePartialCommand(0x72a4, 0x29, 0x72a7, 0x1b, STR_SPAN("Disable Soul Level screen"));
        patcher.ReplacePartialCommand(0x72cd, 0x33, 0x72d0, 0x2d, STR_SPAN("Add Asuka as guest"));
        patcher.ReplacePartialCommand(0x7300, 0x33, 0x7303, 0x2d, STR_SPAN("Add Yuuki as guest"));
        patcher.ReplacePartialCommand(0x7333, 0x30, 0x7336, 0x2a, STR_SPAN("Add Shio as guest"));
        patcher.ReplacePartialCommand(0x73b7, 0x4f, 0x73ba, 0x49, STR_SPAN("Asuka has been added as guest (hidden in camp etc.)"));
        patcher.ReplacePartialCommand(0x742f, 0x4f, 0x7432, 0x49, STR_SPAN("Yuuki has been added as guest (hidden in camp etc.)"));
        patcher.ReplacePartialCommand(0x74ab, 0x4c, 0x74ae, 0x46, STR_SPAN("Shio has been added as guest (hidden in camp etc.)"));
        patcher.ReplacePartialCommand(0x7550, 0xf, 0x7553, 0x9, STR_SPAN("Register all"));
        patcher.ReplacePartialCommand(0x755f, 0x1b, 0x7562, 0x15, STR_SPAN("Register main"));
        patcher.ReplacePartialCommand(0x757a, 0x1e, 0x757d, 0x18, STR_SPAN("Register quests"));
        patcher.ReplacePartialCommand(0x7598, 0x1e, 0x759b, 0x18, STR_SPAN(" - Completed"));
        patcher.ReplacePartialCommand(0x75b6, 0x24, 0x75b9, 0x1e, STR_SPAN(" - Expired"));
        patcher.ReplacePartialCommand(0x75da, 0x21, 0x75dd, 0x1b, STR_SPAN("Register dungeons"));
        patcher.ReplacePartialCommand(0x75fb, 0x24, 0x75fe, 0x1e, STR_SPAN("Clear dungeons"));
        patcher.ReplacePartialCommand(0x761f, 0x18, 0x7622, 0x12, STR_SPAN("Register monsters"));
        patcher.ReplacePartialCommand(0x7637, 0x38, 0x763a, 0x32, STR_SPAN(" - Register event monsters (test)"));
        patcher.ReplacePartialCommand(0x766f, 0x1b, 0x7672, 0x15, STR_SPAN("Register friends"));
        patcher.ReplacePartialCommand(0x768a, 0x2d, 0x768d, 0x27, STR_SPAN(" - Switch info"));
        patcher.ReplacePartialCommand(0x76b7, 0x2e, 0x76ba, 0x28, STR_SPAN(" - Switch info 2"));
        patcher.ReplacePartialCommand(0x76e5, 0x1b, 0x76e8, 0x15, STR_SPAN("Register recipes"));
        patcher.ReplacePartialCommand(0x7700, 0x1b, 0x7703, 0x15, STR_SPAN(" - Including variants"));
        patcher.ReplacePartialCommand(0x771b, 0x1b, 0x771e, 0x15, STR_SPAN(" - Remove all"));
        patcher.ReplacePartialCommand(0x7736, 0x1e, 0x7739, 0x18, STR_SPAN(" - Give ingredients"));
        patcher.ReplacePartialCommand(0x7754, 0x2a, 0x7757, 0x24, STR_SPAN("Register Active Voice"));
        patcher.ReplacePartialCommand(0x777e, 0x15, 0x7781, 0xf, STR_SPAN("Register books"));
        patcher.ReplacePartialCommand(0x7793, 0x1b, 0x7796, 0x15, STR_SPAN("Register help"));
        patcher.ReplacePartialCommand(0x77ae, 0x1e, 0x77b1, 0x18, STR_SPAN("Unlock NiAR"));
        patcher.ReplacePartialCommand(0x7a97, 0x21, 0x7a9a, 0x1c, STR_SPAN("Books have been unlocked."));

        // TK_EV_Jump
        patcher.ReplacePartialCommand(0x7e77, 0x16, 0x7e7a + 7, 0x10 - 7, STR_SPAN(" Chapter 1"));
        patcher.ReplacePartialCommand(0x7e8d, 0x16, 0x7e90 + 7, 0x10 - 7, STR_SPAN(" Chapter 2"));
        patcher.ReplacePartialCommand(0x7ea3, 0x16, 0x7ea6 + 7, 0x10 - 7, STR_SPAN(" Chapter 3"));
        patcher.ReplacePartialCommand(0x7eb9, 0x16, 0x7ebc + 7, 0x10 - 7, STR_SPAN(" Chapter 4"));
        patcher.ReplacePartialCommand(0x7ecf, 0x16, 0x7ed2 + 7, 0x10 - 7, STR_SPAN(" Chapter 5"));
        patcher.ReplacePartialCommand(0x7ee5, 0x16, 0x7ee8 + 7, 0x10 - 7, STR_SPAN(" Intermission"));
        patcher.ReplacePartialCommand(0x7efb, 0x16, 0x7efe + 7, 0x10 - 7, STR_SPAN(" Chapter 6"));
        patcher.ReplacePartialCommand(0x7f11, 0x16, 0x7f14 + 7, 0x10 - 7, STR_SPAN(" Chapter 7"));
        patcher.ReplacePartialCommand(0x7f27, 0x16, 0x7f2a + 7, 0x10 - 7, STR_SPAN(" Final Chapter"));
        patcher.ReplacePartialCommand(0x7f3d, 0x1a, 0x7f40, 0x14, STR_SPAN(" \xe2\x87\x92" "Side/After Story"));
        patcher.ReplacePartialCommand(0x80aa, 0x20, 0x80ad + 7, 0x1a - 7, STR_SPAN(" Chapter 1 Side Story"));
        patcher.ReplacePartialCommand(0x80ca, 0x20, 0x80cd + 7, 0x1a - 7, STR_SPAN(" Chapter 2 Side Story"));
        patcher.ReplacePartialCommand(0x80ea, 0x20, 0x80ed + 7, 0x1a - 7, STR_SPAN(" Chapter 3 Side Story"));
        patcher.ReplacePartialCommand(0x810a, 0x20, 0x810d + 7, 0x1a - 7, STR_SPAN(" Chapter 4 Side Story"));
        patcher.ReplacePartialCommand(0x812a, 0x20, 0x812d + 7, 0x1a - 7, STR_SPAN(" Chapter 5 Side Story"));
        patcher.ReplacePartialCommand(0x814a, 0x20, 0x814d + 7, 0x1a - 7, STR_SPAN(" Intermission Side Story"));
        patcher.ReplacePartialCommand(0x816a, 0x20, 0x816d + 7, 0x1a - 7, STR_SPAN(" Chapter 6 Side Story"));
        patcher.ReplacePartialCommand(0x818a, 0x17, 0x818d + 7, 0x11 - 7, STR_SPAN(" After Story"));
        patcher.ReplacePartialCommand(0x81a1, 0xf, 0x81a4, 0x9, STR_SPAN(" \xe2\x87\x92" "Main Story"));

        // EV_Jump_00_00
        patcher.ReplacePartialCommand(0x831e, 0x42, 0x8321 + 10, 0x3c - 10, STR_SPAN(" Kou sees Asuka in Houraichou"));
        patcher.ReplacePartialCommand(0x8360, 0x42, 0x8363 + 10, 0x3c - 10, STR_SPAN(" Encounter with the Otherworld at the underpass"));
        patcher.ReplacePartialCommand(0x83a2, 0x42, 0x83a5 + 10, 0x3c - 10, STR_SPAN(" Waking up in the Otherworld"));
        patcher.ReplacePartialCommand(0x83e4, 0x42, 0x83e7 + 10, 0x3c - 10, STR_SPAN(" Asuka fights the Greed"));
        patcher.ReplacePartialCommand(0x8426, 0x42, 0x8429 + 10, 0x3c - 10, STR_SPAN(" Erasing Kou's memory"));
        patcher.ReplacePartialCommand(0x8468, 0x42, 0x846b + 10, 0x3c - 10, STR_SPAN(" Kou's dream"));
        patcher.ReplacePartialCommand(0x84aa, 0x42, 0x84ad + 10, 0x3c - 10, STR_SPAN(" Morning at school"));
        patcher.ReplacePartialCommand(0x84ec, 0x42, 0x84ef + 10, 0x3c - 10, STR_SPAN(" Towa's class"));
        patcher.ReplacePartialCommand(0x852e, 0x42, 0x8531 + 10, 0x3c - 10, STR_SPAN(" Lunch break"));
        patcher.ReplacePartialCommand(0x8570, 0x42, 0x8573 + 10, 0x3c - 10, STR_SPAN(" Conversation after lunch"));
        patcher.ReplacePartialCommand(0x85b2, 0x42, 0x85b5 + 10, 0x3c - 10, STR_SPAN(" After school"));
        patcher.ReplacePartialCommand(0x85f4, 0x42, 0x85f7 + 10, 0x3c - 10, STR_SPAN(" Meeting Sora"));
        patcher.ReplacePartialCommand(0x8636, 0x42, 0x8639 + 10, 0x3c - 10, STR_SPAN(" Start working?"));
        patcher.ReplacePartialCommand(0x8678, 0x42, 0x867b + 10, 0x3c - 10, STR_SPAN(" Part-time work at Yukino's"));
        patcher.ReplacePartialCommand(0x86ba, 0x42, 0x86bd + 10, 0x3c - 10, STR_SPAN(" Shiori running away"));
        patcher.ReplacePartialCommand(0x86fc, 0x42, 0x86ff + 10, 0x3c - 10, STR_SPAN(" Shiori gets pulled into Gate"));
        patcher.ReplacePartialCommand(0x873e, 0x42, 0x8741 + 10, 0x3c - 10, STR_SPAN(" Entering Ruins of Abstraction"));
        patcher.ReplacePartialCommand(0x8780, 0x42, 0x8783 + 10, 0x3c - 10, STR_SPAN(" After tutorial"));
        patcher.ReplacePartialCommand(0x87d5, 0x42, 0x87d8 + 10, 0x3c - 10, STR_SPAN(" Finding Shiori (boss fight)"));
        patcher.ReplacePartialCommand(0x8817, 0x42, 0x881a + 10, 0x3c - 10, STR_SPAN(" Leaving Eclipse"));
        patcher.ReplacePartialCommand(0x8859, 0x42, 0x885c + 10, 0x3c - 10, STR_SPAN(" Shiori rescured"));
        patcher.ReplacePartialCommand(0x889b, 0x1b, 0x889e, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 1] Next Page"));
        patcher.ReplacePartialCommand(0x88b6, 0x1b, 0x88b9, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 1] Previous Page"));

        // EV_Jump_01_00
        patcher.ReplacePartialCommand(0x8bc2, 0x42, 0x8bc5 + 10, 0x3c - 10, STR_SPAN(" Early morning at Kou's"));
        patcher.ReplacePartialCommand(0x8c04, 0x42, 0x8c07 + 10, 0x3c - 10, STR_SPAN(" Kou leaving home"));
        patcher.ReplacePartialCommand(0x8c46, 0x42, 0x8c49 + 10, 0x3c - 10, STR_SPAN(" Visiting Kokonoe Shrine"));
        patcher.ReplacePartialCommand(0x8c88, 0x42, 0x8c8b + 10, 0x3c - 10, STR_SPAN(" At shrine with Sora"));
        patcher.ReplacePartialCommand(0x8cca, 0x42, 0x8ccd + 10, 0x3c - 10, STR_SPAN(" Outside school"));
        patcher.ReplacePartialCommand(0x8d0c, 0x42, 0x8d0f + 10, 0x3c - 10, STR_SPAN(" Gorou's class"));
        patcher.ReplacePartialCommand(0x8d4e, 0x42, 0x8d51 + 10, 0x3c - 10, STR_SPAN(" Lunch break"));
        patcher.ReplacePartialCommand(0x8d90, 0x42, 0x8d93 + 10, 0x3c - 10, STR_SPAN(" 10 days ago"));
        patcher.ReplacePartialCommand(0x8dd2, 0x42, 0x8dd5 + 10, 0x3c - 10, STR_SPAN(" After school"));
        patcher.ReplacePartialCommand(0x8e14, 0x42, 0x8e17 + 10, 0x3c - 10, STR_SPAN(" Running into Chiaki"));
        patcher.ReplacePartialCommand(0x8e56, 0x42, 0x8e59 + 10, 0x3c - 10, STR_SPAN(" Visiting the Karate Club"));
        patcher.ReplacePartialCommand(0x8e98, 0x42, 0x8e9b + 10, 0x3c - 10, STR_SPAN(" Meeting Rion"));
        patcher.ReplacePartialCommand(0x8eda, 0x42, 0x8edd + 10, 0x3c - 10, STR_SPAN(" Start working?"));
        patcher.ReplacePartialCommand(0x8f1c, 0x42, 0x8f1f + 10, 0x3c - 10, STR_SPAN(" Part-time work at Yanagi's"));
        patcher.ReplacePartialCommand(0x8f5e, 0x42, 0x8f61 + 10, 0x3c - 10, STR_SPAN(" After work"));
        patcher.ReplacePartialCommand(0x8fa0, 0x42, 0x8fa3 + 10, 0x3c - 10, STR_SPAN(" Argument between Sora and Chiaki"));
        patcher.ReplacePartialCommand(0x8fe2, 0x42, 0x8fe5 + 10, 0x3c - 10, STR_SPAN(" Entering Amber Labyrinth"));
        patcher.ReplacePartialCommand(0x9024, 0x42, 0x9027 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x9079, 0x42, 0x907c + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x90bb, 0x42, 0x90be + 10, 0x3c - 10, STR_SPAN(" Back in real world"));
        patcher.ReplacePartialCommand(0x90fd, 0x42, 0x9100 + 10, 0x3c - 10, STR_SPAN(" Outside school the next day"));
        patcher.ReplacePartialCommand(0x913f, 0x42, 0x9142 + 10, 0x3c - 10, STR_SPAN(" Sora practicing in the morning"));
        patcher.ReplacePartialCommand(0x9181, 0x42, 0x9184 + 10, 0x3c - 10, STR_SPAN(" Conversation with Asuka in clubhouse"));
        patcher.ReplacePartialCommand(0x91c3, 0x42, 0x91c6 + 10, 0x3c - 10, STR_SPAN(" Conversation with Chiaki at lunch break"));
        patcher.ReplacePartialCommand(0x9205, 0x42, 0x9208 + 10, 0x3c - 10, STR_SPAN(" After school, Chiaki missing"));
        patcher.ReplacePartialCommand(0x9247, 0x42, 0x924a + 10, 0x3c - 10, STR_SPAN(" Searching the park"));
        patcher.ReplacePartialCommand(0x9289, 0x42, 0x928c + 10, 0x3c - 10, STR_SPAN(" Finding Chiaki"));
        patcher.ReplacePartialCommand(0x92cb, 0x42, 0x92ce + 10, 0x3c - 10, STR_SPAN(" Entering Moonlit Garden"));
        patcher.ReplacePartialCommand(0x930d, 0x42, 0x9310 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x934f, 0x42, 0x9352 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x9391, 0x42, 0x9394 + 10, 0x3c - 10, STR_SPAN(" Reconciliation between Sora and Chaiki"));
        patcher.ReplacePartialCommand(0x93d3, 0x42, 0x93d6 + 10, 0x3c - 10, STR_SPAN(" Officially teaming up with Asuka"));
        patcher.ReplacePartialCommand(0x9415, 0x1b, 0x9418, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 2] Next Page"));
        patcher.ReplacePartialCommand(0x9430, 0x1b, 0x9433, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 2] Previous Page"));

        // EV_Jump_02_00
        patcher.ReplacePartialCommand(0x98b2, 0x42, 0x98b5 + 10, 0x3c - 10, STR_SPAN(" Yuuki's Room"));
        patcher.ReplacePartialCommand(0x98f4, 0x42, 0x98f7 + 10, 0x3c - 10, STR_SPAN(" Outside school"));
        patcher.ReplacePartialCommand(0x9936, 0x42, 0x9939 + 10, 0x3c - 10, STR_SPAN(" Talk about God's App"));
        patcher.ReplacePartialCommand(0x9978, 0x42, 0x997b + 10, 0x3c - 10, STR_SPAN(" After school"));
        patcher.ReplacePartialCommand(0x99ba, 0x42, 0x99bd + 10, 0x3c - 10, STR_SPAN(" Meeting Shio"));
        patcher.ReplacePartialCommand(0x99fc, 0x42, 0x99ff + 10, 0x3c - 10, STR_SPAN(" Start working?"));
        patcher.ReplacePartialCommand(0x9a3e, 0x42, 0x9a41 + 10, 0x3c - 10, STR_SPAN(" Part-time work at Hama's"));
        patcher.ReplacePartialCommand(0x9a80, 0x42, 0x9a83 + 10, 0x3c - 10, STR_SPAN(" Houraichou at night"));
        patcher.ReplacePartialCommand(0x9ac2, 0x42, 0x9ac5 + 10, 0x3c - 10, STR_SPAN(" Gorou visits the caf\xc3\xa9"));
        patcher.ReplacePartialCommand(0x9b04, 0x42, 0x9b07 + 10, 0x3c - 10, STR_SPAN(" Call from Ryouta"));
        patcher.ReplacePartialCommand(0x9b46, 0x42, 0x9b49 + 10, 0x3c - 10, STR_SPAN(" Next day, Ryouta is safe"));
        patcher.ReplacePartialCommand(0x9b88, 0x42, 0x9b8b + 10, 0x3c - 10, STR_SPAN(" After school, looking into God's App"));
        patcher.ReplacePartialCommand(0x9bca, 0x42, 0x9bcd + 10, 0x3c - 10, STR_SPAN(" Let's visit the student council"));
        patcher.ReplacePartialCommand(0x9c0c, 0x42, 0x9c0f + 10, 0x3c - 10, STR_SPAN(" Getting info about Yuuki from Mitsuki"));
        patcher.ReplacePartialCommand(0x9c4e, 0x42, 0x9c51 + 10, 0x3c - 10, STR_SPAN(" Entering the park"));
        patcher.ReplacePartialCommand(0x9c90, 0x42, 0x9c93 + 10, 0x3c - 10, STR_SPAN(" Enter the apartment?"));
        patcher.ReplacePartialCommand(0x9cd2, 0x42, 0x9cd5 + 10, 0x3c - 10, STR_SPAN(" Meeting Yuuki in his room"));
        patcher.ReplacePartialCommand(0x9d14, 0x42, 0x9d17 + 10, 0x3c - 10, STR_SPAN(" Kicked out, talk with Aoi"));
        patcher.ReplacePartialCommand(0x9d69, 0x42, 0x9d6c + 10, 0x3c - 10, STR_SPAN(" Next day, outside Kou's house"));
        patcher.ReplacePartialCommand(0x9dab, 0x42, 0x9dae + 10, 0x3c - 10, STR_SPAN(" Message from Yuuki"));
        patcher.ReplacePartialCommand(0x9ded, 0x42, 0x9df0 + 10, 0x3c - 10, STR_SPAN(" Meeting Yuuki in the park"));
        patcher.ReplacePartialCommand(0x9e2f, 0x42, 0x9e32 + 10, 0x3c - 10, STR_SPAN(" Entering Brick Alley"));
        patcher.ReplacePartialCommand(0x9e71, 0x42, 0x9e74 + 10, 0x3c - 10, STR_SPAN(" Enter art gallery?"));
        patcher.ReplacePartialCommand(0x9eb3, 0x42, 0x9eb6 + 10, 0x3c - 10, STR_SPAN(" Aoi is gone"));
        patcher.ReplacePartialCommand(0x9ef5, 0x42, 0x9ef8 + 10, 0x3c - 10, STR_SPAN(" Entering Fairy Corridor"));
        patcher.ReplacePartialCommand(0x9f37, 0x42, 0x9f3a + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x9f79, 0x42, 0x9f7c + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x9fbb, 0x42, 0x9fbe + 10, 0x3c - 10, STR_SPAN(" Back outside, Aoi doesn't wake up"));
        patcher.ReplacePartialCommand(0x9ffd, 0x42, 0xa000 + 10, 0x3c - 10, STR_SPAN(" Investigation from Yuuki's room"));
        patcher.ReplacePartialCommand(0xa03f, 0x42, 0xa042 + 10, 0x3c - 10, STR_SPAN(" Outside school"));
        patcher.ReplacePartialCommand(0xa081, 0x42, 0xa084 + 10, 0x3c - 10, STR_SPAN(" Investigation in computer lab"));
        patcher.ReplacePartialCommand(0xa0c3, 0x42, 0xa0c6 + 10, 0x3c - 10, STR_SPAN(" Go to Nanahoshi Mall? (softlocks game)"));
        patcher.ReplacePartialCommand(0xa105, 0x42, 0xa108 + 10, 0x3c - 10, STR_SPAN(" Outside Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0xa147, 0x42, 0xa14a + 10, 0x3c - 10, STR_SPAN(" Entering Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0xa189, 0x42, 0xa18c + 10, 0x3c - 10, STR_SPAN(" Enter server room?"));
        patcher.ReplacePartialCommand(0xa1cb, 0x42, 0xa1ce + 10, 0x3c - 10, STR_SPAN(" Yuuki's awakening"));
        patcher.ReplacePartialCommand(0xa220, 0x42, 0xa223 + 10, 0x3c - 10, STR_SPAN(" Entering Second Spiritron Barrier"));
        patcher.ReplacePartialCommand(0xa262, 0x42, 0xa265 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0xa2a4, 0x42, 0xa2a7 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0xa2e6, 0x42, 0xa2e9 + 10, 0x3c - 10, STR_SPAN(" Aoi regains consciousness"));
        patcher.ReplacePartialCommand(0xa328, 0x42, 0xa32b + 10, 0x3c - 10, STR_SPAN(" Yuuki calling Aoi"));
        patcher.ReplacePartialCommand(0xa36a, 0x42, 0xa36d + 10, 0x3c - 10, STR_SPAN(" Yuuki comes to school"));
        patcher.ReplacePartialCommand(0xa3ac, 0x1b, 0xa3af, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 3] Next Page"));
        patcher.ReplacePartialCommand(0xa3c7, 0x1b, 0xa3ca, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 3] Previous Page"));

        // EV_Jump_03_00
        patcher.ReplacePartialCommand(0xa996, 0x42, 0xa999 + 10, 0x3c - 10, STR_SPAN(" Jun assaulted by Blaze"));
        patcher.ReplacePartialCommand(0xa9d8, 0x42, 0xa9db + 10, 0x3c - 10, STR_SPAN(" Akihiro at Gemini"));
        patcher.ReplacePartialCommand(0xaa1a, 0x42, 0xaa1d + 10, 0x3c - 10, STR_SPAN(" Outside school"));
        patcher.ReplacePartialCommand(0xaa5c, 0x42, 0xaa5f + 10, 0x3c - 10, STR_SPAN(" Jun enters class with injury"));
        patcher.ReplacePartialCommand(0xaa9e, 0x42, 0xaaa1 + 10, 0x3c - 10, STR_SPAN(" Talking with Jun about Blaze"));
        patcher.ReplacePartialCommand(0xaae0, 0x42, 0xaae3 + 10, 0x3c - 10, STR_SPAN(" After school"));
        patcher.ReplacePartialCommand(0xab22, 0x42, 0xab25 + 10, 0x3c - 10, STR_SPAN(" Meeting Yuuki and Mitsuki at school gate"));
        patcher.ReplacePartialCommand(0xab64, 0x42, 0xab67 + 10, 0x3c - 10, STR_SPAN(" Mitsuki picked up by Kyouka"));
        patcher.ReplacePartialCommand(0xaba6, 0x42, 0xaba9 + 10, 0x3c - 10, STR_SPAN(" Parting with Yuuki"));
        patcher.ReplacePartialCommand(0xabe8, 0x42, 0xabeb + 10, 0x3c - 10, STR_SPAN(" Start working?"));
        patcher.ReplacePartialCommand(0xac2a, 0x42, 0xac2d + 10, 0x3c - 10, STR_SPAN(" Rion's encounter with Blaze"));
        patcher.ReplacePartialCommand(0xac6c, 0x42, 0xac6f + 10, 0x3c - 10, STR_SPAN(" Next day, discussing Blaze's abnormal strength"));
        patcher.ReplacePartialCommand(0xacae, 0x42, 0xacb1 + 10, 0x3c - 10, STR_SPAN(" Split into two groups"));
        patcher.ReplacePartialCommand(0xacf0, 0x42, 0xacf3 + 10, 0x3c - 10, STR_SPAN(" Entering Brick Alley"));
        patcher.ReplacePartialCommand(0xad32, 0x42, 0xad35 + 10, 0x3c - 10, STR_SPAN(" Chance meeting with Rion"));
        patcher.ReplacePartialCommand(0xad74, 0x42, 0xad77 + 10, 0x3c - 10, STR_SPAN(" Entering Houraichou"));
        patcher.ReplacePartialCommand(0xadb6, 0x42, 0xadb9 + 10, 0x3c - 10, STR_SPAN(" Encounter with Gotou"));
        patcher.ReplacePartialCommand(0xadf8, 0x42, 0xadfb + 10, 0x3c - 10, STR_SPAN(" Enter Gemini?"));
        patcher.ReplacePartialCommand(0xae4d, 0x42, 0xae50 + 10, 0x3c - 10, STR_SPAN(" Outside Gemini"));
        patcher.ReplacePartialCommand(0xae8f, 0x42, 0xae92 + 10, 0x3c - 10, STR_SPAN(" Inside Gemini"));
        patcher.ReplacePartialCommand(0xaed1, 0x42, 0xaed4 + 10, 0x3c - 10, STR_SPAN(" Rescued by Shio"));
        patcher.ReplacePartialCommand(0xaf13, 0x42, 0xaf16 + 10, 0x3c - 10, STR_SPAN(" Encounter with Akihiro"));
        patcher.ReplacePartialCommand(0xaf55, 0x42, 0xaf58 + 10, 0x3c - 10, STR_SPAN(" Entering Cave of the Fallen Dragon"));
        patcher.ReplacePartialCommand(0xaf97, 0x42, 0xaf9a + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0xafd9, 0x42, 0xafdc + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0xb01b, 0x42, 0xb01e + 10, 0x3c - 10, STR_SPAN(" Filling Shio in at the park"));
        patcher.ReplacePartialCommand(0xb05d, 0x42, 0xb060 + 10, 0x3c - 10, STR_SPAN(" Next day, outside Yuuki's apartment"));
        patcher.ReplacePartialCommand(0xb09f, 0x42, 0xb0a2 + 10, 0x3c - 10, STR_SPAN(" Investigating Blaze in Yuuki's room"));
        patcher.ReplacePartialCommand(0xb0e1, 0x42, 0xb0e4 + 10, 0x3c - 10, STR_SPAN(" Entering Houraichou"));
        patcher.ReplacePartialCommand(0xb123, 0x42, 0xb126 + 10, 0x3c - 10, STR_SPAN(" Enter Gemini?"));
        patcher.ReplacePartialCommand(0xb165, 0x42, 0xb168 + 10, 0x3c - 10, STR_SPAN(" Outside Gemini, Asuka putting worker to sleep"));
        patcher.ReplacePartialCommand(0xb1a7, 0x42, 0xb1aa + 10, 0x3c - 10, STR_SPAN(" Start investigating Gemini"));
        patcher.ReplacePartialCommand(0xb1e9, 0x42, 0xb1ec + 10, 0x3c - 10, STR_SPAN(" Finding Heat"));
        patcher.ReplacePartialCommand(0xb22b, 0x42, 0xb22e + 10, 0x3c - 10, STR_SPAN(" Leaving Gemini, waking worker back up"));
        patcher.ReplacePartialCommand(0xb26d, 0x42, 0xb270 + 10, 0x3c - 10, STR_SPAN(" Call from Sora"));
        patcher.ReplacePartialCommand(0xb2af, 0x42, 0xb2b2 + 10, 0x3c - 10, STR_SPAN(" Confrontation at the underpass"));
        patcher.ReplacePartialCommand(0xb304, 0x42, 0xb307 + 10, 0x3c - 10, STR_SPAN(" Call from Sora (continued)"));
        patcher.ReplacePartialCommand(0xb346, 0x42, 0xb349 + 10, 0x3c - 10, STR_SPAN(" Arriving at the underpass"));
        patcher.ReplacePartialCommand(0xb388, 0x42, 0xb38b + 10, 0x3c - 10, STR_SPAN(" Shio waking up at shrine"));
        patcher.ReplacePartialCommand(0xb3ca, 0x42, 0xb3cd + 10, 0x3c - 10, STR_SPAN(" Outside shrine"));
        patcher.ReplacePartialCommand(0xb40c, 0x42, 0xb40f + 10, 0x3c - 10, STR_SPAN(" Blaze's past"));
        patcher.ReplacePartialCommand(0xb44e, 0x42, 0xb451 + 10, 0x3c - 10, STR_SPAN(" Leaving for the abandoned factory"));
        patcher.ReplacePartialCommand(0xb490, 0x42, 0xb493 + 10, 0x3c - 10, STR_SPAN(" Go to factory? (softlocks game)"));
        patcher.ReplacePartialCommand(0xb4d2, 0x42, 0xb4d5 + 10, 0x3c - 10, STR_SPAN(" Outside factory"));
        patcher.ReplacePartialCommand(0xb514, 0x42, 0xb517 + 10, 0x3c - 10, STR_SPAN(" Akihiro's recklessness"));
        patcher.ReplacePartialCommand(0xb556, 0x42, 0xb559 + 10, 0x3c - 10, STR_SPAN(" Entering Azure Ruins"));
        patcher.ReplacePartialCommand(0xb598, 0x42, 0xb59b + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0xb5da, 0x42, 0xb5dd + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0xb61c, 0x42, 0xb61f + 10, 0x3c - 10, STR_SPAN(" Akihiro's despair"));
        patcher.ReplacePartialCommand(0xb65e, 0x42, 0xb661 + 10, 0x3c - 10, STR_SPAN(" Onlookers from outside"));
        patcher.ReplacePartialCommand(0xb6a0, 0x1b, 0xb6a3, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 4] Next Page"));
        patcher.ReplacePartialCommand(0xb6bb, 0x1b, 0xb6be, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 4] Previous Page"));

        // EV_Jump_04_00
        patcher.ReplacePartialCommand(0xbdce, 0x42, 0xbdd1 + 10, 0x3c - 10, STR_SPAN(" Station Square in fog"));
        patcher.ReplacePartialCommand(0xbe10, 0x42, 0xbe13 + 10, 0x3c - 10, STR_SPAN(" Houraichou in fog"));
        patcher.ReplacePartialCommand(0xbe52, 0x42, 0xbe55 + 10, 0x3c - 10, STR_SPAN(" Memorial Park in fog"));
        patcher.ReplacePartialCommand(0xbe94, 0x42, 0xbe97 + 10, 0x3c - 10, STR_SPAN(" School in fog"));
        patcher.ReplacePartialCommand(0xbed6, 0x42, 0xbed9 + 10, 0x3c - 10, STR_SPAN(" Mitsuki called by Seijuurou"));
        patcher.ReplacePartialCommand(0xbf18, 0x42, 0xbf1b + 10, 0x3c - 10, STR_SPAN(" Brick Alley in fog"));
        patcher.ReplacePartialCommand(0xbf5a, 0x42, 0xbf5d + 10, 0x3c - 10, STR_SPAN(" Asuka visits Yukino"));
        patcher.ReplacePartialCommand(0xbf9c, 0x42, 0xbf9f + 10, 0x3c - 10, STR_SPAN(" Outside school"));
        patcher.ReplacePartialCommand(0xbfde, 0x42, 0xbfe1 + 10, 0x3c - 10, STR_SPAN(" Ryouta's ghost story"));
        patcher.ReplacePartialCommand(0xc020, 0x42, 0xc023 + 10, 0x3c - 10, STR_SPAN(" After school"));
        patcher.ReplacePartialCommand(0xc062, 0x42, 0xc065 + 10, 0x3c - 10, STR_SPAN(" Encounter with Asuka and Mitsuki"));
        patcher.ReplacePartialCommand(0xc0a4, 0x42, 0xc0a7 + 10, 0x3c - 10, STR_SPAN(" Outside Acros Tower"));
        patcher.ReplacePartialCommand(0xc0e6, 0x42, 0xc0e9 + 10, 0x3c - 10, STR_SPAN(" Entering Acros Tower"));
        patcher.ReplacePartialCommand(0xc128, 0x42, 0xc12b + 10, 0x3c - 10, STR_SPAN(" Asking about job details"));
        patcher.ReplacePartialCommand(0xc16a, 0x42, 0xc16d + 10, 0x3c - 10, STR_SPAN(" Start working?"));
        patcher.ReplacePartialCommand(0xc1ac, 0x42, 0xc1af + 10, 0x3c - 10, STR_SPAN(" Part-time work at theater"));
        patcher.ReplacePartialCommand(0xc1ee, 0x42, 0xc1f1 + 10, 0x3c - 10, STR_SPAN(" SPiKA concert"));
        patcher.ReplacePartialCommand(0xc230, 0x42, 0xc233 + 10, 0x3c - 10, STR_SPAN(" Leaving Acros Tower, encounter with Shio"));
        patcher.ReplacePartialCommand(0xc285, 0x42, 0xc288 + 10, 0x3c - 10, STR_SPAN(" Outside Shopping Street"));
        patcher.ReplacePartialCommand(0xc2c7, 0x42, 0xc2ca + 10, 0x3c - 10, STR_SPAN(" Shio serving Kou a self-made meal"));
        patcher.ReplacePartialCommand(0xc309, 0x42, 0xc30c + 10, 0x3c - 10, STR_SPAN(" Fog appears in Shopping Street"));
        patcher.ReplacePartialCommand(0xc34b, 0x42, 0xc34e + 10, 0x3c - 10, STR_SPAN(" Next day, outside school"));
        patcher.ReplacePartialCommand(0xc38d, 0x42, 0xc390 + 10, 0x3c - 10, STR_SPAN(" Group discussion about fog"));
        patcher.ReplacePartialCommand(0xc3cf, 0x42, 0xc3d2 + 10, 0x3c - 10, STR_SPAN(" Investigation with Asuka"));
        patcher.ReplacePartialCommand(0xc411, 0x42, 0xc414 + 10, 0x3c - 10, STR_SPAN(" Encounter with Mikuriya"));
        patcher.ReplacePartialCommand(0xc453, 0x42, 0xc456 + 10, 0x3c - 10, STR_SPAN(" Call from Mayu (Nanahoshi Mall)"));
        patcher.ReplacePartialCommand(0xc495, 0x42, 0xc498 + 10, 0x3c - 10, STR_SPAN(" Call from Mayu (Acros Tower)"));
        patcher.ReplacePartialCommand(0xc4d7, 0x42, 0xc4da + 10, 0x3c - 10, STR_SPAN(" Call from Mayu (Station Square)"));
        patcher.ReplacePartialCommand(0xc519, 0x42, 0xc51c + 10, 0x3c - 10, STR_SPAN(" Go to Shopping Street? (softlocks game)"));
        patcher.ReplacePartialCommand(0xc55b, 0x42, 0xc55e + 10, 0x3c - 10, STR_SPAN(" Mayu kidnapped"));
        patcher.ReplacePartialCommand(0xc59d, 0x42, 0xc5a0 + 10, 0x3c - 10, STR_SPAN(" Entering Temple of Evil Mist"));
        patcher.ReplacePartialCommand(0xc5df, 0x42, 0xc5e2 + 10, 0x3c - 10, STR_SPAN(" Finding Mayu mid-dungeon"));
        patcher.ReplacePartialCommand(0xc621, 0x42, 0xc624 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0xc663, 0x42, 0xc666 + 10, 0x3c - 10, STR_SPAN(" After boss fight, Asuka is gone"));
        patcher.ReplacePartialCommand(0xc6a5, 0x42, 0xc6a8 + 10, 0x3c - 10, STR_SPAN(" Outside hospital"));
        patcher.ReplacePartialCommand(0xc6e7, 0x42, 0xc6ea + 10, 0x3c - 10, STR_SPAN(" Mayu resting in bed"));
        patcher.ReplacePartialCommand(0xc73c, 0x42, 0xc73f + 10, 0x3c - 10, STR_SPAN(" Conversation with Mitsuki"));
        patcher.ReplacePartialCommand(0xc77e, 0x42, 0xc781 + 10, 0x3c - 10, STR_SPAN(" Outside school at night"));
        patcher.ReplacePartialCommand(0xc7c0, 0x42, 0xc7c3 + 10, 0x3c - 10, STR_SPAN(" Mitsuki explains Zodiac and Nemesis"));
        patcher.ReplacePartialCommand(0xc802, 0x42, 0xc805 + 10, 0x3c - 10, STR_SPAN(" Next day, outside school"));
        patcher.ReplacePartialCommand(0xc844, 0x42, 0xc847 + 10, 0x3c - 10, STR_SPAN(" Shiori talks about fairy tale"));
        patcher.ReplacePartialCommand(0xc886, 0x42, 0xc889 + 10, 0x3c - 10, STR_SPAN(" Starting investigation during lunch break"));
        patcher.ReplacePartialCommand(0xc8c8, 0x42, 0xc8cb + 10, 0x3c - 10, STR_SPAN(" Splitting up and leaving school"));
        patcher.ReplacePartialCommand(0xc90a, 0x42, 0xc90d + 10, 0x3c - 10, STR_SPAN(" Entering Brick Alley, encounter with Mikuriya"));
        patcher.ReplacePartialCommand(0xc94c, 0x42, 0xc94f + 10, 0x3c - 10, STR_SPAN(" Enter caf\xc3\xa9?"));
        patcher.ReplacePartialCommand(0xc98e, 0x42, 0xc991 + 10, 0x3c - 10, STR_SPAN(" Conversation with Yamaoka"));
        patcher.ReplacePartialCommand(0xc9d0, 0x42, 0xc9d3 + 10, 0x3c - 10, STR_SPAN(" Enter antique shop?"));
        patcher.ReplacePartialCommand(0xca12, 0x42, 0xca15 + 10, 0x3c - 10, STR_SPAN(" Conversation with Yukino"));
        patcher.ReplacePartialCommand(0xca54, 0x42, 0xca57 + 10, 0x3c - 10, STR_SPAN(" Encounter with black hound (from antique shop)"));
        patcher.ReplacePartialCommand(0xca96, 0x42, 0xca99 + 10, 0x3c - 10, STR_SPAN(" Encounter with black hound (from caf\xc3\xa9)"));
        patcher.ReplacePartialCommand(0xcad8, 0x42, 0xcadb + 10, 0x3c - 10, STR_SPAN(" Entering Scarlet Labyrinth"));
        patcher.ReplacePartialCommand(0xcb1a, 0x42, 0xcb1d + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0xcb5c, 0x42, 0xcb5f + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0xcb9e, 0x42, 0xcba1 + 10, 0x3c - 10, STR_SPAN(" Leaving for Memorial Park"));
        patcher.ReplacePartialCommand(0xcbf3, 0x42, 0xcbf6 + 10, 0x3c - 10, STR_SPAN(" Entering Memorial Park"));
        patcher.ReplacePartialCommand(0xcc35, 0x42, 0xcc38 + 10, 0x3c - 10, STR_SPAN(" Enter grove?"));
        patcher.ReplacePartialCommand(0xcc77, 0x42, 0xcc7a + 10, 0x3c - 10, STR_SPAN(" Gate at grove"));
        patcher.ReplacePartialCommand(0xccb9, 0x42, 0xccbc + 10, 0x3c - 10, STR_SPAN(" Entering Birdcage Corridor"));
        patcher.ReplacePartialCommand(0xccfb, 0x42, 0xccfe + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0xcd3d, 0x42, 0xcd40 + 10, 0x3c - 10, STR_SPAN(" After boss fight, rescuing Asuka"));
        patcher.ReplacePartialCommand(0xcd7f, 0x42, 0xcd82 + 10, 0x3c - 10, STR_SPAN(" Getting through Asuka's thick skull"));
        patcher.ReplacePartialCommand(0xcdc1, 0x42, 0xcdc4 + 10, 0x3c - 10, STR_SPAN(" Arriving at school turned castle"));
        patcher.ReplacePartialCommand(0xce03, 0x42, 0xce06 + 10, 0x3c - 10, STR_SPAN(" Entering Witch's Briar Castle"));
        patcher.ReplacePartialCommand(0xce45, 0x42, 0xce48 + 10, 0x3c - 10, STR_SPAN(" Finding Ryouta/Jun/etc. mid-dungeon"));
        patcher.ReplacePartialCommand(0xce87, 0x42, 0xce8a + 10, 0x3c - 10, STR_SPAN(" Finding Towa/Rion/Gorou/etc. mid-dungeon"));
        patcher.ReplacePartialCommand(0xcec9, 0x42, 0xcecc + 10, 0x3c - 10, STR_SPAN(" Finding Shiori at end of dungeon"));
        patcher.ReplacePartialCommand(0xcf0b, 0x42, 0xcf0e + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0xcf4d, 0x42, 0xcf50 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0xcf8f, 0x42, 0xcf92 + 10, 0x3c - 10, STR_SPAN(" Waking up on school roof"));
        patcher.ReplacePartialCommand(0xcfd1, 0x42, 0xcfd4 + 10, 0x3c - 10, STR_SPAN(" Everyone is safe, but Towa/Rion/Gorou remember"));
        patcher.ReplacePartialCommand(0xd013, 0x42, 0xd016 + 10, 0x3c - 10, STR_SPAN(" Mikuriya hands off fairy tale book to White Shroud"));
        patcher.ReplacePartialCommand(0xd055, 0x1b, 0xd058, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 5] Next Page"));
        patcher.ReplacePartialCommand(0xd070, 0x1b, 0xd073, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 5] Previous Page"));

        // EV_Jump_05_00
        patcher.ReplacePartialCommand(0xd9fa, 0x42, 0xd9fd + 10, 0x3c - 10, STR_SPAN(" Road to hot springs"));
        patcher.ReplacePartialCommand(0xda3c, 0x42, 0xda3f + 10, 0x3c - 10, STR_SPAN(" Conversation in car"));
        patcher.ReplacePartialCommand(0xda7e, 0x42, 0xda81 + 10, 0x3c - 10, STR_SPAN(" Flashback to 3 days ago"));
        patcher.ReplacePartialCommand(0xdac0, 0x42, 0xdac3 + 10, 0x3c - 10, STR_SPAN(" Conversation in car (continued)"));
        patcher.ReplacePartialCommand(0xdb02, 0x42, 0xdb05 + 10, 0x3c - 10, STR_SPAN(" Arriving at hot springs"));
        patcher.ReplacePartialCommand(0xdb44, 0x42, 0xdb47 + 10, 0x3c - 10, STR_SPAN(" Encounter with Rion"));
        patcher.ReplacePartialCommand(0xdb86, 0x42, 0xdb89 + 10, 0x3c - 10, STR_SPAN(" Encounter with Gorou"));
        patcher.ReplacePartialCommand(0xdbc8, 0x42, 0xdbcb + 10, 0x3c - 10, STR_SPAN(" Open-air hot springs (boys)"));
        patcher.ReplacePartialCommand(0xdc0a, 0x42, 0xdc0d + 10, 0x3c - 10, STR_SPAN(" Open-air hot springs (girls)"));
        patcher.ReplacePartialCommand(0xdc4c, 0x42, 0xdc4f + 10, 0x3c - 10, STR_SPAN(" After bath"));
        patcher.ReplacePartialCommand(0xdc8e, 0x42, 0xdc91 + 10, 0x3c - 10, STR_SPAN(" Conversation with Gorou at shrine"));
        patcher.ReplacePartialCommand(0xdcd0, 0x42, 0xdcd3 + 10, 0x3c - 10, STR_SPAN(" Conversation with Rion/Wakaba/Akira in garden"));
        patcher.ReplacePartialCommand(0xdd12, 0x42, 0xdd15 + 10, 0x3c - 10, STR_SPAN(" Head to dinner?"));
        patcher.ReplacePartialCommand(0xdd54, 0x42, 0xdd57 + 10, 0x3c - 10, STR_SPAN(" Outside hot springs"));
        patcher.ReplacePartialCommand(0xdd96, 0x42, 0xdd99 + 10, 0x3c - 10, STR_SPAN(" The Otherworld and the Tokyo Twilight Disaster"));
        patcher.ReplacePartialCommand(0xddd8, 0x42, 0xdddb + 10, 0x3c - 10, STR_SPAN(" After dinner"));
        patcher.ReplacePartialCommand(0xde1a, 0x42, 0xde1d + 10, 0x3c - 10, STR_SPAN(" Asuka's past and Kou's determination"));
        patcher.ReplacePartialCommand(0xde5c, 0x42, 0xde5f + 10, 0x3c - 10, STR_SPAN(" Take a bath?"));
        patcher.ReplacePartialCommand(0xdeb1, 0x42, 0xdeb4 + 10, 0x3c - 10, STR_SPAN(" Conversation with Shiori in hot springs"));
        patcher.ReplacePartialCommand(0xdef3, 0x42, 0xdef6 + 10, 0x3c - 10, STR_SPAN(" Time stopped, gathering party"));
        patcher.ReplacePartialCommand(0xdf35, 0x42, 0xdf38 + 10, 0x3c - 10, STR_SPAN(" Heading towards shrine"));
        patcher.ReplacePartialCommand(0xdf77, 0x42, 0xdf7a + 10, 0x3c - 10, STR_SPAN(" Encounter with Rem"));
        patcher.ReplacePartialCommand(0xdfb9, 0x42, 0xdfbc + 10, 0x3c - 10, STR_SPAN(" Entering Sacred Land of Eternity"));
        patcher.ReplacePartialCommand(0xdffb, 0x42, 0xdffe + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0xe03d, 0x42, 0xe040 + 10, 0x3c - 10, STR_SPAN(" After boss fight, Kou's decision"));
        patcher.ReplacePartialCommand(0xe07f, 0x42, 0xe082 + 10, 0x3c - 10, STR_SPAN(" Everyone's decision"));
        patcher.ReplacePartialCommand(0xe0c1, 0x42, 0xe0c4 + 10, 0x3c - 10, STR_SPAN(" Leaving hot springs, forming the XRC"));
        patcher.ReplacePartialCommand(0xe103, 0x42, 0xe106 + 10, 0x3c - 10, STR_SPAN(" (scene for capture)"));
        patcher.ReplacePartialCommand(0xe145, 0x18, 0xe148, 0x12, STR_SPAN(" \xe2\x87\x92" "[Intermission] Next Page"));
        patcher.ReplacePartialCommand(0xe15d, 0x18, 0xe160, 0x12, STR_SPAN(" \xe2\x87\x92" "[Intermission] Previous Page"));

        // EV_Jump_06_00
        patcher.ReplacePartialCommand(0xe61e, 0x42, 0xe621 + 10, 0x3c - 10, STR_SPAN(" SPiKA concert"));
        patcher.ReplacePartialCommand(0xe660, 0x42, 0xe663 + 10, 0x3c - 10, STR_SPAN(" White Shroud overlooking Station Square"));
        patcher.ReplacePartialCommand(0xe6a2, 0x42, 0xe6a5 + 10, 0x3c - 10, STR_SPAN(" After school, heading to XRC room"));
        patcher.ReplacePartialCommand(0xe6e4, 0x42, 0xe6e7 + 10, 0x3c - 10, STR_SPAN(" Outside school"));
        patcher.ReplacePartialCommand(0xe726, 0x41, 0xe729 + 10, 0x3b - 10, STR_SPAN(" First activities of the XRC"));
        patcher.ReplacePartialCommand(0xe767, 0x42, 0xe76a + 10, 0x3c - 10, STR_SPAN(" Arriving at Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0xe7a9, 0x42, 0xe7ac + 10, 0x3c - 10, STR_SPAN(" Entering Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0xe7eb, 0x42, 0xe7ee + 10, 0x3c - 10, STR_SPAN(" Talk to Wakana?"));
        patcher.ReplacePartialCommand(0xe82d, 0x42, 0xe830 + 10, 0x3c - 10, STR_SPAN(" Finding a Gate in the alarm clock"));
        patcher.ReplacePartialCommand(0xe86f, 0x42, 0xe872 + 10, 0x3c - 10, STR_SPAN(" Entering Ruin of Fading Memories"));
        patcher.ReplacePartialCommand(0xe8b1, 0x42, 0xe8b4 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0xe8f3, 0x42, 0xe8f6 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0xe935, 0x42, 0xe938 + 10, 0x3c - 10, STR_SPAN(" Back in mall"));
        patcher.ReplacePartialCommand(0xe977, 0x42, 0xe97a + 10, 0x3c - 10, STR_SPAN(" Finishing XRC activities for the day"));
        patcher.ReplacePartialCommand(0xe9b9, 0x42, 0xe9bc + 10, 0x3c - 10, STR_SPAN(" Outside school, splitting up"));
        patcher.ReplacePartialCommand(0xe9fb, 0x42, 0xe9fe + 10, 0x3c - 10, STR_SPAN(" Walking home with Towa"));
        patcher.ReplacePartialCommand(0xea3d, 0x42, 0xea40 + 10, 0x3c - 10, STR_SPAN(" Arriving at the shrine, meeting Seijuurou"));
        patcher.ReplacePartialCommand(0xea7f, 0x42, 0xea82 + 10, 0x3c - 10, STR_SPAN(" Next day, outside school (unused)"));
        patcher.ReplacePartialCommand(0xead4, 0x42, 0xead7 + 10, 0x3c - 10, STR_SPAN(" Rion teasing Kou in the school halls"));
        patcher.ReplacePartialCommand(0xeb16, 0x42, 0xeb19 + 10, 0x3c - 10, STR_SPAN(" After school"));
        patcher.ReplacePartialCommand(0xeb58, 0x42, 0xeb5b + 10, 0x3c - 10, STR_SPAN(" Encounter with Rion while leaving school"));
        patcher.ReplacePartialCommand(0xeb9a, 0x42, 0xeb9d + 10, 0x3c - 10, STR_SPAN(" Start working?"));
        patcher.ReplacePartialCommand(0xebdc, 0x42, 0xebdf + 10, 0x3c - 10, STR_SPAN(" Encounter with Wakaba and Akira, Rion missing"));
        patcher.ReplacePartialCommand(0xec1e, 0x42, 0xec21 + 10, 0x3c - 10, STR_SPAN(" Flashback, Rion dejected"));
        patcher.ReplacePartialCommand(0xec60, 0x42, 0xec63 + 10, 0x3c - 10, STR_SPAN(" Wakaba and Akira talk about weird things happening"));
        patcher.ReplacePartialCommand(0xeca2, 0x42, 0xeca5 + 10, 0x3c - 10, STR_SPAN(" Go to abandoned factory? (softlocks game)"));
        patcher.ReplacePartialCommand(0xece4, 0x42, 0xece7 + 10, 0x3c - 10, STR_SPAN(" Arriving at factory"));
        patcher.ReplacePartialCommand(0xed26, 0x42, 0xed29 + 10, 0x3c - 10, STR_SPAN(" Conversation with Rion, Rion's angelic possession"));
        patcher.ReplacePartialCommand(0xed68, 0x42, 0xed6b + 10, 0x3c - 10, STR_SPAN(" Next day, outside hospital"));
        patcher.ReplacePartialCommand(0xedaa, 0x42, 0xedad + 10, 0x3c - 10, STR_SPAN(" Inside hospital"));
        patcher.ReplacePartialCommand(0xedec, 0x42, 0xedef + 10, 0x3c - 10, STR_SPAN(" Discussing Rion's situation"));
        patcher.ReplacePartialCommand(0xee2e, 0x42, 0xee31 + 10, 0x3c - 10, STR_SPAN(" Outside Acros Tower"));
        patcher.ReplacePartialCommand(0xee70, 0x42, 0xee73 + 10, 0x3c - 10, STR_SPAN(" Entering Acros Tower"));
        patcher.ReplacePartialCommand(0xeeb2, 0x42, 0xeeb5 + 10, 0x3c - 10, STR_SPAN(" Talking with the receptionist about Rion"));
        patcher.ReplacePartialCommand(0xeef4, 0x42, 0xeef7 + 10, 0x3c - 10, STR_SPAN(" Talking with SPiKA about Rion"));
        patcher.ReplacePartialCommand(0xef36, 0x42, 0xef39 + 10, 0x3c - 10, STR_SPAN(" Continuing the investigation"));
        patcher.ReplacePartialCommand(0xef8b, 0x42, 0xef8e + 10, 0x3c - 10, STR_SPAN(" Entering Houraichou"));
        patcher.ReplacePartialCommand(0xefcd, 0x42, 0xefd0 + 10, 0x3c - 10, STR_SPAN(" Enter karaoke shop?"));
        patcher.ReplacePartialCommand(0xf00f, 0x42, 0xf012 + 10, 0x3c - 10, STR_SPAN(" Inside the karaoke shop"));
        patcher.ReplacePartialCommand(0xf051, 0x42, 0xf054 + 10, 0x3c - 10, STR_SPAN(" Enter room 107?"));
        patcher.ReplacePartialCommand(0xf093, 0x42, 0xf096 + 10, 0x3c - 10, STR_SPAN(" Gate in room 107"));
        patcher.ReplacePartialCommand(0xf0d5, 0x42, 0xf0d8 + 10, 0x3c - 10, STR_SPAN(" Entering Fourth Spiritron Barrier"));
        patcher.ReplacePartialCommand(0xf117, 0x42, 0xf11a + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0xf159, 0x42, 0xf15c + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0xf19b, 0x42, 0xf19e + 10, 0x3c - 10, STR_SPAN(" Back in karaoke shop, Rion's past"));
        patcher.ReplacePartialCommand(0xf1dd, 0x42, 0xf1e0 + 10, 0x3c - 10, STR_SPAN(" Mikuriya in Rion's hospital room"));
        patcher.ReplacePartialCommand(0xf21f, 0x42, 0xf222 + 10, 0x3c - 10, STR_SPAN(" Encounter with White Shroud in karaoke shop"));
        patcher.ReplacePartialCommand(0xf261, 0x42, 0xf264 + 10, 0x3c - 10, STR_SPAN(" Concert is starting without Rion"));
        patcher.ReplacePartialCommand(0xf2a3, 0x42, 0xf2a6 + 10, 0x3c - 10, STR_SPAN(" Arriving at hospital turned Eclipse"));
        patcher.ReplacePartialCommand(0xf2e5, 0x42, 0xf2e8 + 10, 0x3c - 10, STR_SPAN(" Entering Palace of the Winged God"));
        patcher.ReplacePartialCommand(0xf327, 0x42, 0xf32a + 10, 0x3c - 10, STR_SPAN(" Rescuing Rion"));
        patcher.ReplacePartialCommand(0xf369, 0x42, 0xf36c + 10, 0x3c - 10, STR_SPAN(" Message from SPiKA to Rion"));
        patcher.ReplacePartialCommand(0xf3ab, 0x42, 0xf3ae + 10, 0x3c - 10, STR_SPAN(" Rion's awakening"));
        patcher.ReplacePartialCommand(0xf3ed, 0x42, 0xf3f0 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0xf442, 0x42, 0xf445 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0xf484, 0x42, 0xf487 + 10, 0x3c - 10, STR_SPAN(" Back in hospital, Mikuriya taken into custody"));
        patcher.ReplacePartialCommand(0xf4c6, 0x42, 0xf4c9 + 10, 0x3c - 10, STR_SPAN(" End of the concert, Rion arrives in time"));
        patcher.ReplacePartialCommand(0xf508, 0x42, 0xf50b + 10, 0x3c - 10, STR_SPAN(" White Shroud and Gorou outside Acros Tower"));
        patcher.ReplacePartialCommand(0xf54a, 0x1b, 0xf54d, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 6] Next Page"));
        patcher.ReplacePartialCommand(0xf565, 0x1b, 0xf568, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 6] Previous Page"));

        // EV_Jump_07_00
        patcher.ReplacePartialCommand(0x0fdc6, 0x42, 0x0fdc9 + 10, 0x3c - 10, STR_SPAN(" Memories of the Tokyo Twilight Disaster"));
        patcher.ReplacePartialCommand(0x0fe08, 0x42, 0x0fe0b + 10, 0x3c - 10, STR_SPAN(" Going to school with Shiori, Rem watches"));
        patcher.ReplacePartialCommand(0x0fe4a, 0x42, 0x0fe4d + 10, 0x3c - 10, STR_SPAN(" Outside school"));
        patcher.ReplacePartialCommand(0x0fe8c, 0x42, 0x0fe8f + 10, 0x3c - 10, STR_SPAN(" Swimming classes"));
        patcher.ReplacePartialCommand(0x0fece, 0x42, 0x0fed1 + 10, 0x3c - 10, STR_SPAN(" Swimming classes (after minigame)"));
        patcher.ReplacePartialCommand(0x0ff10, 0x42, 0x0ff13 + 10, 0x3c - 10, STR_SPAN(" Outside school (unused)"));
        patcher.ReplacePartialCommand(0x0ff52, 0x42, 0x0ff55 + 10, 0x3c - 10, STR_SPAN(" Rion joins the XRC"));
        patcher.ReplacePartialCommand(0x0ff94, 0x41, 0x0ff97 + 10, 0x3b - 10, STR_SPAN(" XRC enters Shopping Street"));
        patcher.ReplacePartialCommand(0x0ffd5, 0x42, 0x0ffd8 + 10, 0x3c - 10, STR_SPAN(" Enter Yanagi Sports?"));
        patcher.ReplacePartialCommand(0x10017, 0x42, 0x1001a + 10, 0x3c - 10, STR_SPAN(" Yanagi Sports slashed up"));
        patcher.ReplacePartialCommand(0x10059, 0x42, 0x1005c + 10, 0x3c - 10, STR_SPAN(" Leaving Yanagi Sports"));
        patcher.ReplacePartialCommand(0x1009b, 0x42, 0x1009e + 10, 0x3c - 10, STR_SPAN(" Go to Houraichou? (softlocks game)"));
        patcher.ReplacePartialCommand(0x100dd, 0x42, 0x100e0 + 10, 0x3c - 10, STR_SPAN(" Commotion at Houraichou"));
        patcher.ReplacePartialCommand(0x1011f, 0x42, 0x10122 + 10, 0x3c - 10, STR_SPAN(" Otherworld fire at Takahane building"));
        patcher.ReplacePartialCommand(0x10161, 0x42, 0x10164 + 10, 0x3c - 10, STR_SPAN(" Entering Cave of Divine Flame"));
        patcher.ReplacePartialCommand(0x101a3, 0x42, 0x101a6 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x101e5, 0x42, 0x101e8 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x10227, 0x42, 0x1022a + 10, 0x3c - 10, STR_SPAN(" The fire disappears"));
        patcher.ReplacePartialCommand(0x1027c, 0x42, 0x1027f + 10, 0x3c - 10, STR_SPAN(" Finishing XRC activities for the day"));
        patcher.ReplacePartialCommand(0x102be, 0x42, 0x102c1 + 10, 0x3c - 10, STR_SPAN(" Houraichou at night"));
        patcher.ReplacePartialCommand(0x10300, 0x42, 0x10303 + 10, 0x3c - 10, STR_SPAN(" Part-time work at Hama's, encounter with Gorou"));
        patcher.ReplacePartialCommand(0x10342, 0x42, 0x10345 + 10, 0x3c - 10, STR_SPAN(" Next day, outside school (unused)"));
        patcher.ReplacePartialCommand(0x10384, 0x42, 0x10387 + 10, 0x3c - 10, STR_SPAN(" Gorou absent from school"));
        patcher.ReplacePartialCommand(0x103c6, 0x42, 0x103c9 + 10, 0x3c - 10, STR_SPAN(" After school"));
        patcher.ReplacePartialCommand(0x10408, 0x42, 0x1040b + 10, 0x3c - 10, STR_SPAN(" Entering Memorial Park"));
        patcher.ReplacePartialCommand(0x1044a, 0x42, 0x1044d + 10, 0x3c - 10, STR_SPAN(" Start working?"));
        patcher.ReplacePartialCommand(0x1048c, 0x42, 0x1048f + 10, 0x3c - 10, STR_SPAN(" Lake freezes over"));
        patcher.ReplacePartialCommand(0x104ce, 0x42, 0x104d1 + 10, 0x3c - 10, STR_SPAN(" Look for gate north of lake?"));
        patcher.ReplacePartialCommand(0x10510, 0x42, 0x10513 + 10, 0x3c - 10, STR_SPAN(" Entering Garden of Frozen Trees"));
        patcher.ReplacePartialCommand(0x10552, 0x42, 0x10555 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x10594, 0x42, 0x10597 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x105d6, 0x42, 0x105d9 + 10, 0x3c - 10, STR_SPAN(" Lake thaws, Hollow Quake occurs"));
        patcher.ReplacePartialCommand(0x10618, 0x42, 0x1061b + 10, 0x3c - 10, STR_SPAN(" Hollow Quake (Station Square)"));
        patcher.ReplacePartialCommand(0x1065a, 0x42, 0x1065d + 10, 0x3c - 10, STR_SPAN(" Hollow Quake (Shopping Street)"));
        patcher.ReplacePartialCommand(0x1069c, 0x42, 0x1069f + 10, 0x3c - 10, STR_SPAN(" Hollow Quake (Brick Alley)"));
        patcher.ReplacePartialCommand(0x106de, 0x42, 0x106e1 + 10, 0x3c - 10, STR_SPAN(" Hollow Quake (School)"));
        patcher.ReplacePartialCommand(0x10733, 0x42, 0x10736 + 10, 0x3c - 10, STR_SPAN(" Hollow Quake (Library)"));
        patcher.ReplacePartialCommand(0x10775, 0x42, 0x10778 + 10, 0x3c - 10, STR_SPAN(" Outside school at night"));
        patcher.ReplacePartialCommand(0x107b7, 0x42, 0x107ba + 10, 0x3c - 10, STR_SPAN(" XRC discussing the Hollow Quake"));
        patcher.ReplacePartialCommand(0x107f9, 0x42, 0x107fc + 10, 0x3c - 10, STR_SPAN(" Next day, outside school (unused)"));
        patcher.ReplacePartialCommand(0x1083b, 0x42, 0x1083e + 10, 0x3c - 10, STR_SPAN(" Shiori absent from class"));
        patcher.ReplacePartialCommand(0x1087d, 0x42, 0x10880 + 10, 0x3c - 10, STR_SPAN(" Flashback to Shiori calling Kou"));
        patcher.ReplacePartialCommand(0x108bf, 0x42, 0x108c2 + 10, 0x3c - 10, STR_SPAN(" Gorou is absent, too"));
        patcher.ReplacePartialCommand(0x10901, 0x42, 0x10904 + 10, 0x3c - 10, STR_SPAN(" XRC investigating Gorou"));
        patcher.ReplacePartialCommand(0x10943, 0x42, 0x10946 + 10, 0x3c - 10, STR_SPAN(" Entering Houraichou"));
        patcher.ReplacePartialCommand(0x10985, 0x42, 0x10988 + 10, 0x3c - 10, STR_SPAN(" Enter Gorou's apartment?"));
        patcher.ReplacePartialCommand(0x109c7, 0x42, 0x109ca + 10, 0x3c - 10, STR_SPAN(" Inside Gorou's apartment"));
        patcher.ReplacePartialCommand(0x10a09, 0x42, 0x10a0c + 10, 0x3c - 10, STR_SPAN(" Finding photo of Futaba"));
        patcher.ReplacePartialCommand(0x10a4b, 0x42, 0x10a4e + 10, 0x3c - 10, STR_SPAN(" Finding map with note on back"));
        patcher.ReplacePartialCommand(0x10a8d, 0x42, 0x10a90 + 10, 0x3c - 10, STR_SPAN(" Found everything"));
        patcher.ReplacePartialCommand(0x10acf, 0x42, 0x10ad2 + 10, 0x3c - 10, STR_SPAN(" Outside Yuuki's apartment (unused)"));
        patcher.ReplacePartialCommand(0x10b11, 0x42, 0x10b14 + 10, 0x3c - 10, STR_SPAN(" Pinpointing \"Point Q\""));
        patcher.ReplacePartialCommand(0x10b53, 0x42, 0x10b56 + 10, 0x3c - 10, STR_SPAN(" Entering Station Square"));
        patcher.ReplacePartialCommand(0x10b95, 0x42, 0x10b98 + 10, 0x3c - 10, STR_SPAN(" Opening the door to the underground"));
        patcher.ReplacePartialCommand(0x10bea, 0x42, 0x10bed + 10, 0x3c - 10, STR_SPAN(" Finding the gate"));
        patcher.ReplacePartialCommand(0x10c2c, 0x42, 0x10c2f + 10, 0x3c - 10, STR_SPAN(" Entering Purgatory of Molten Blood"));
        patcher.ReplacePartialCommand(0x10c6e, 0x42, 0x10c71 + 10, 0x3c - 10, STR_SPAN(" Another Hollow Quake mid-dungeon"));
        patcher.ReplacePartialCommand(0x10cb0, 0x42, 0x10cb3 + 10, 0x3c - 10, STR_SPAN(" Finding Gorou, boss fight"));
        patcher.ReplacePartialCommand(0x10cf2, 0x42, 0x10cf5 + 10, 0x3c - 10, STR_SPAN(" After boss fight, Gorou's identity revealed"));
        patcher.ReplacePartialCommand(0x10d34, 0x42, 0x10d37 + 10, 0x3c - 10, STR_SPAN(" Military helicopter flying over town"));
        patcher.ReplacePartialCommand(0x10d76, 0x42, 0x10d79 + 10, 0x3c - 10, STR_SPAN(" Inside helicopter"));
        patcher.ReplacePartialCommand(0x10db8, 0x42, 0x10dbb + 10, 0x3c - 10, STR_SPAN(" NDF base, attacking the breed, breed escapes"));
        patcher.ReplacePartialCommand(0x10dfa, 0x42, 0x10dfd + 10, 0x3c - 10, STR_SPAN(" Entering Fortress of False Gods, splitting into two teams"));
        patcher.ReplacePartialCommand(0x10e3c, 0x42, 0x10e3f + 10, 0x3c - 10, STR_SPAN(" Team A entering dungeon"));
        patcher.ReplacePartialCommand(0x10e7e, 0x42, 0x10e81 + 10, 0x3c - 10, STR_SPAN(" Team B entering dungeon"));
        patcher.ReplacePartialCommand(0x10ec0, 0x42, 0x10ec3 + 10, 0x3c - 10, STR_SPAN(" Team A halfway through"));
        patcher.ReplacePartialCommand(0x10f02, 0x42, 0x10f05 + 10, 0x3c - 10, STR_SPAN(" Team B halfway through"));
        patcher.ReplacePartialCommand(0x10f44, 0x42, 0x10f47 + 10, 0x3c - 10, STR_SPAN(" Teams meet back up"));
        patcher.ReplacePartialCommand(0x10f86, 0x42, 0x10f89 + 10, 0x3c - 10, STR_SPAN(" Enter boss fight?"));
        patcher.ReplacePartialCommand(0x10fc8, 0x42, 0x10fcb + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x1100a, 0x42, 0x1100d + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x1104c, 0x42, 0x1104f + 10, 0x3c - 10, STR_SPAN(" Waking up at NDF base, call from Shiori"));
        patcher.ReplacePartialCommand(0x110a1, 0x42, 0x110a4 + 10, 0x3c - 10, STR_SPAN(" Hollow Quake at Acros Tower, Twilight Apostle awakens"));
        patcher.ReplacePartialCommand(0x110e3, 0x42, 0x110e6 + 10, 0x3c - 10, STR_SPAN(" Otherworld spreads out from Acros Tower"));
        patcher.ReplacePartialCommand(0x11125, 0x42, 0x11128 + 10, 0x3c - 10, STR_SPAN(" Everyone at NDF base pulled into Otherworld"));
        patcher.ReplacePartialCommand(0x11167, 0x1b, 0x1116a, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 7] Next Page"));
        patcher.ReplacePartialCommand(0x11182, 0x1b, 0x11185, 0x15, STR_SPAN(" \xe2\x87\x92" "[Chapter 7] Previous Page"));

        // EV_Jump_08_00
        patcher.ReplacePartialCommand(0x11b9a, 0x42, 0x11b9d + 10, 0x3c - 10, STR_SPAN(" Kou's nightmare, waking up at shrine"));
        patcher.ReplacePartialCommand(0x11bdc, 0x42, 0x11bdf + 10, 0x3c - 10, STR_SPAN(" Outside, the town has transformed"));
        patcher.ReplacePartialCommand(0x11c1e, 0x42, 0x11c21 + 10, 0x3c - 10, STR_SPAN(" Planning your next actions"));
        patcher.ReplacePartialCommand(0x11c60, 0x42, 0x11c63 + 10, 0x3c - 10, STR_SPAN(" Setting out"));
        patcher.ReplacePartialCommand(0x11ca2, 0x42, 0x11ca5 + 10, 0x3c - 10, STR_SPAN(" Leaving the shrine area"));
        patcher.ReplacePartialCommand(0x11ce4, 0x42, 0x11ce7 + 10, 0x3c - 10, STR_SPAN(" Battle at Shopping Street"));
        patcher.ReplacePartialCommand(0x11d26, 0x42, 0x11d29 + 10, 0x3c - 10, STR_SPAN(" After battle, meeting with residents"));
        patcher.ReplacePartialCommand(0x11d68, 0x42, 0x11d6b + 10, 0x3c - 10, STR_SPAN(" Conversation at Kuguraya"));
        patcher.ReplacePartialCommand(0x11daa, 0x42, 0x11dad + 10, 0x3c - 10, STR_SPAN(" Securing Shopping Street with barrier devices"));
        patcher.ReplacePartialCommand(0x11dec, 0x42, 0x11def + 10, 0x3c - 10, STR_SPAN(" Receive some barrier devices from Jihei"));
        patcher.ReplacePartialCommand(0x11e2e, 0x42, 0x11e31 + 10, 0x3c - 10, STR_SPAN(" Leaving for Brick Alley"));
        patcher.ReplacePartialCommand(0x11e70, 0x42, 0x11e73 + 10, 0x3c - 10, STR_SPAN(" Battle at Brick Alley"));
        patcher.ReplacePartialCommand(0x11eb2, 0x42, 0x11eb5 + 10, 0x3c - 10, STR_SPAN(" After battle, meeting with Yukino and Yamaoka"));
        patcher.ReplacePartialCommand(0x11ef4, 0x42, 0x11ef7 + 10, 0x3c - 10, STR_SPAN(" Securing Brick Alley with barrier devices"));
        patcher.ReplacePartialCommand(0x11f36, 0x42, 0x11f39 + 10, 0x3c - 10, STR_SPAN(" Conversation in Caf\xc3\xa9 17"));
        patcher.ReplacePartialCommand(0x11f78, 0x42, 0x11f7b + 10, 0x3c - 10, STR_SPAN(" Battle at Memorial Park"));
        patcher.ReplacePartialCommand(0x11fba, 0x42, 0x11fbd + 10, 0x3c - 10, STR_SPAN(" After battle, meeting with Airi and Takeuchi"));
        patcher.ReplacePartialCommand(0x11ffc, 0x42, 0x11fff + 10, 0x3c - 10, STR_SPAN(" Enter the pillar?"));
        patcher.ReplacePartialCommand(0x12051, 0x42, 0x12054 + 10, 0x3c - 10, STR_SPAN(" Entering Cocytus Pillar"));
        patcher.ReplacePartialCommand(0x12093, 0x42, 0x12096 + 10, 0x3c - 10, STR_SPAN(" Catching up with Sora and Yuuki mid-dungeon"));
        patcher.ReplacePartialCommand(0x120d5, 0x42, 0x120d8 + 10, 0x3c - 10, STR_SPAN(" After battle"));
        patcher.ReplacePartialCommand(0x12117, 0x42, 0x1211a + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x12159, 0x42, 0x1215c + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x1219b, 0x42, 0x1219e + 10, 0x3c - 10, STR_SPAN(" Markings disappear at Memorial Park"));
        patcher.ReplacePartialCommand(0x121dd, 0x42, 0x121e0 + 10, 0x3c - 10, STR_SPAN(" First seal disappears at Acros Tower"));
        patcher.ReplacePartialCommand(0x1221f, 0x42, 0x12222 + 10, 0x3c - 10, STR_SPAN(" Rescuing Chiaki and Aoi"));
        patcher.ReplacePartialCommand(0x12261, 0x42, 0x12264 + 10, 0x3c - 10, STR_SPAN(" Deciding to head to school"));
        patcher.ReplacePartialCommand(0x122a3, 0x42, 0x122a6 + 10, 0x3c - 10, STR_SPAN(" Go to school? (softlocks game)"));
        patcher.ReplacePartialCommand(0x122e5, 0x42, 0x122e8 + 10, 0x3c - 10, STR_SPAN(" Greed at school, Jun saves the day but gets injured"));
        patcher.ReplacePartialCommand(0x12327, 0x42, 0x1232a + 10, 0x3c - 10, STR_SPAN(" Securing school with barrier devices"));
        patcher.ReplacePartialCommand(0x12369, 0x42, 0x1236c + 10, 0x3c - 10, STR_SPAN(" Relief at karate club"));
        patcher.ReplacePartialCommand(0x123ab, 0x42, 0x123ae + 10, 0x3c - 10, STR_SPAN(" Yuuki in computer room"));
        patcher.ReplacePartialCommand(0x123ed, 0x42, 0x123f0 + 10, 0x3c - 10, STR_SPAN(" Jun reveals his identity at infirmary"));
        patcher.ReplacePartialCommand(0x1242f, 0x42, 0x12432 + 10, 0x3c - 10, STR_SPAN(" Forming a plan at the XRC clubroom"));
        patcher.ReplacePartialCommand(0x12471, 0x42, 0x12474 + 10, 0x3c - 10, STR_SPAN(" Go to Station Square? (softlocks game)"));
        patcher.ReplacePartialCommand(0x124b3, 0x42, 0x124b6 + 10, 0x3c - 10, STR_SPAN(" Battle at Station Square"));
        patcher.ReplacePartialCommand(0x12508, 0x42, 0x1250b + 10, 0x3c - 10, STR_SPAN(" After battle, meeting with Akane and Mizuhara"));
        patcher.ReplacePartialCommand(0x1254a, 0x42, 0x1254d + 10, 0x3c - 10, STR_SPAN(" Enter the pillar?"));
        patcher.ReplacePartialCommand(0x1258c, 0x42, 0x1258f + 10, 0x3c - 10, STR_SPAN(" Entering Lethe Pillar"));
        patcher.ReplacePartialCommand(0x125ce, 0x42, 0x125d1 + 10, 0x3c - 10, STR_SPAN(" Catching up with Mitsuki and Rion mid-dungeon"));
        patcher.ReplacePartialCommand(0x12610, 0x42, 0x12613 + 10, 0x3c - 10, STR_SPAN(" After battle"));
        patcher.ReplacePartialCommand(0x12652, 0x42, 0x12655 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x12694, 0x42, 0x12697 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x126d6, 0x42, 0x126d9 + 10, 0x3c - 10, STR_SPAN(" Markings disappear at Station Square"));
        patcher.ReplacePartialCommand(0x12718, 0x42, 0x1271b + 10, 0x3c - 10, STR_SPAN(" Second seal disappears at Acros Tower"));
        patcher.ReplacePartialCommand(0x1275a, 0x42, 0x1275d + 10, 0x3c - 10, STR_SPAN(" Rescuing Seijuurou and Mikuriya"));
        patcher.ReplacePartialCommand(0x1279c, 0x42, 0x1279f + 10, 0x3c - 10, STR_SPAN(" Mikuriya wants to act independently"));
        patcher.ReplacePartialCommand(0x127de, 0x42, 0x127e1 + 10, 0x3c - 10, STR_SPAN(" Outside school"));
        patcher.ReplacePartialCommand(0x12820, 0x42, 0x12823 + 10, 0x3c - 10, STR_SPAN(" XRC clubroom, Rion is suspicious of Shiori"));
        patcher.ReplacePartialCommand(0x12862, 0x42, 0x12865 + 10, 0x3c - 10, STR_SPAN(" Go to Houraichou? (softlocks game)"));
        patcher.ReplacePartialCommand(0x128a4, 0x42, 0x128a7 + 10, 0x3c - 10, STR_SPAN(" Battle at Houraichou"));
        patcher.ReplacePartialCommand(0x128e6, 0x42, 0x128e9 + 10, 0x3c - 10, STR_SPAN(" After battle, meeting Hama and Taozou"));
        patcher.ReplacePartialCommand(0x12928, 0x42, 0x1292b + 10, 0x3c - 10, STR_SPAN(" Enter the pillar?"));
        patcher.ReplacePartialCommand(0x1296a, 0x42, 0x1296d + 10, 0x3c - 10, STR_SPAN(" Entering Phlegethon Pillar"));
        patcher.ReplacePartialCommand(0x129bf, 0x42, 0x129c2 + 10, 0x3c - 10, STR_SPAN(" Catching up with Shio and Gorou mid-dungeon"));
        patcher.ReplacePartialCommand(0x12a01, 0x42, 0x12a04 + 10, 0x3c - 10, STR_SPAN(" After battle"));
        patcher.ReplacePartialCommand(0x12a43, 0x42, 0x12a46 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x12a85, 0x42, 0x12a88 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x12ac7, 0x42, 0x12aca + 10, 0x3c - 10, STR_SPAN(" Markings disappear at Houraichou"));
        patcher.ReplacePartialCommand(0x12b09, 0x42, 0x12b0c + 10, 0x3c - 10, STR_SPAN(" Last seal disappears at Acros Tower, Greed appear"));
        patcher.ReplacePartialCommand(0x12b4b, 0x42, 0x12b4e + 10, 0x3c - 10, STR_SPAN(" Deciding to return to school"));
        patcher.ReplacePartialCommand(0x12b8d, 0x42, 0x12b90 + 10, 0x3c - 10, STR_SPAN(" Outside school, Ryouta stands guard"));
        patcher.ReplacePartialCommand(0x12bcf, 0x42, 0x12bd2 + 10, 0x3c - 10, STR_SPAN(" Meeting in XRC clubroom"));
        patcher.ReplacePartialCommand(0x12c11, 0x42, 0x12c14 + 10, 0x3c - 10, STR_SPAN(" Head to auditorium? (softlocks game)"));
        patcher.ReplacePartialCommand(0x12c53, 0x42, 0x12c56 + 10, 0x3c - 10, STR_SPAN(" Night before the final battle, school roof with Asuka"));
        patcher.ReplacePartialCommand(0x12c95, 0x42, 0x12c98 + 10, 0x3c - 10, STR_SPAN(" Next day, outside school"));
        patcher.ReplacePartialCommand(0x12cd7, 0x42, 0x12cda + 10, 0x3c - 10, STR_SPAN(" Briefing about the attack on Pandora"));
        patcher.ReplacePartialCommand(0x12d19, 0x42, 0x12d1c + 10, 0x3c - 10, STR_SPAN(" Final preparations"));
        patcher.ReplacePartialCommand(0x12d5b, 0x42, 0x12d5e + 10, 0x3c - 10, STR_SPAN(" Go to Pandora? (softlocks game)"));
        patcher.ReplacePartialCommand(0x12d9d, 0x42, 0x12da0 + 10, 0x3c - 10, STR_SPAN(" Attack on Pandora commences"));
        patcher.ReplacePartialCommand(0x12ddf, 0x42, 0x12de2 + 10, 0x3c - 10, STR_SPAN(" XRC waits in car"));
        patcher.ReplacePartialCommand(0x12e21, 0x42, 0x12e24 + 10, 0x3c - 10, STR_SPAN(" Your schoolmates are here, too!"));
        patcher.ReplacePartialCommand(0x12e76, 0x42, 0x12e79 + 10, 0x3c - 10, STR_SPAN(" In car, seeing the breed appear"));
        patcher.ReplacePartialCommand(0x12eb8, 0x42, 0x12ebb + 10, 0x3c - 10, STR_SPAN(" Mikuriya wipes out the breed with helicopters"));
        patcher.ReplacePartialCommand(0x12efa, 0x42, 0x12efd + 10, 0x3c - 10, STR_SPAN(" In car, thankful for help"));
        patcher.ReplacePartialCommand(0x12f3c, 0x42, 0x12f3f + 10, 0x3c - 10, STR_SPAN(" Asuka releases her limiters, everyone praying for success"));
        patcher.ReplacePartialCommand(0x12f7e, 0x42, 0x12f81 + 10, 0x3c - 10, STR_SPAN(" Entering Pandora, split into teams"));
        patcher.ReplacePartialCommand(0x12fc0, 0x42, 0x12fc3 + 10, 0x3c - 10, STR_SPAN(" Team A entering dungeon"));
        patcher.ReplacePartialCommand(0x13002, 0x42, 0x13005 + 10, 0x3c - 10, STR_SPAN(" Team B entering dungeon"));
        patcher.ReplacePartialCommand(0x13044, 0x42, 0x13047 + 10, 0x3c - 10, STR_SPAN(" Team C entering dungeon"));
        patcher.ReplacePartialCommand(0x13086, 0x42, 0x13089 + 10, 0x3c - 10, STR_SPAN(" Team D entering dungeon"));
        patcher.ReplacePartialCommand(0x130c8, 0x42, 0x130cb + 10, 0x3c - 10, STR_SPAN(" Kou realizes the truth"));
        patcher.ReplacePartialCommand(0x1310a, 0x42, 0x1310d + 10, 0x3c - 10, STR_SPAN(" Reuniting at final corridor"));
        patcher.ReplacePartialCommand(0x1314c, 0x42, 0x1314f + 10, 0x3c - 10, STR_SPAN(" Reached the final gate"));
        patcher.ReplacePartialCommand(0x1318e, 0x42, 0x13191 + 10, 0x3c - 10, STR_SPAN(" Enter final boss fight?"));
        patcher.ReplacePartialCommand(0x131d0, 0x42, 0x131d3 + 10, 0x3c - 10, STR_SPAN(" Confronting Shiori"));
        patcher.ReplacePartialCommand(0x13212, 0x42, 0x13215 + 10, 0x3c - 10, STR_SPAN(" Final boss phase 2"));
        patcher.ReplacePartialCommand(0x13254, 0x42, 0x13257 + 10, 0x3c - 10, STR_SPAN(" After battle, Kou jumps after Shiori"));
        patcher.ReplacePartialCommand(0x13296, 0x42, 0x13299 + 10, 0x3c - 10, STR_SPAN(" Shiori disappears"));
        patcher.ReplacePartialCommand(0x132d8, 0x42, 0x132db + 10, 0x3c - 10, STR_SPAN(" Final boss phase 1 complete"));
        patcher.ReplacePartialCommand(0x1332d, 0x42, 0x13330 + 10, 0x3c - 10, STR_SPAN(" Two weeks later, Outside school"));
        patcher.ReplacePartialCommand(0x1336f, 0x42, 0x13372 + 10, 0x3c - 10, STR_SPAN(" Meeting everyone at school 2F"));
        patcher.ReplacePartialCommand(0x133b1, 0x42, 0x133b4 + 10, 0x3c - 10, STR_SPAN(" Meeting everyone at school 1F"));
        patcher.ReplacePartialCommand(0x133f3, 0x42, 0x133f6 + 10, 0x3c - 10, STR_SPAN(" Outside school, leaving with Asuka"));
        patcher.ReplacePartialCommand(0x13435, 0x42, 0x13438 + 10, 0x3c - 10, STR_SPAN(" Bitter End"));
        patcher.ReplacePartialCommand(0x13477, 0x42, 0x1347a + 10, 0x3c - 10, STR_SPAN(" Normal/True End split"));
        patcher.ReplacePartialCommand(0x134b9, 0x42, 0x134bc + 10, 0x3c - 10, STR_SPAN(" Rem appears at Brick Alley"));
        patcher.ReplacePartialCommand(0x134fb, 0x42, 0x134fe + 10, 0x3c - 10, STR_SPAN(" Radiant gate in front of Shiori's house"));
        patcher.ReplacePartialCommand(0x1353d, 0x42, 0x13540 + 10, 0x3c - 10, STR_SPAN(" Enter radiant gate?"));
        patcher.ReplacePartialCommand(0x1357f, 0x42, 0x13582 + 10, 0x3c - 10, STR_SPAN(" Confrontation with Nine-Tailed Beast"));
        patcher.ReplacePartialCommand(0x135c1, 0x42, 0x135c4 + 10, 0x3c - 10, STR_SPAN(" Recovering Shiori"));
        patcher.ReplacePartialCommand(0x13603, 0x42, 0x13606 + 10, 0x3c - 10, STR_SPAN(" True End"));
        patcher.ReplacePartialCommand(0x13645, 0x1b, 0x13648, 0x15, STR_SPAN(" \xe2\x87\x92" "[Final Chapter] Next Page"));
        patcher.ReplacePartialCommand(0x13660, 0x1b, 0x13663, 0x15, STR_SPAN(" \xe2\x87\x92" "[Final Chapter] Previous Page"));

        // EV_Jump_10_00
        patcher.ReplacePartialCommand(0x14879, 0x42, 0x1487c + 10, 0x3c - 10, STR_SPAN(" Outside hospital"));
        patcher.ReplacePartialCommand(0x148bb, 0x42, 0x148be + 10, 0x3c - 10, STR_SPAN(" White Shroud appears"));
        patcher.ReplacePartialCommand(0x148fd, 0x42, 0x14900 + 10, 0x3c - 10, STR_SPAN(" Entering Abandoned Path"));
        patcher.ReplacePartialCommand(0x1493f, 0x42, 0x14942 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x14981, 0x42, 0x14984 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x149c3, 0x42, 0x149c6 + 10, 0x3c - 10, STR_SPAN(" Back in hospital"));

        // EV_Jump_11_00
        patcher.ReplacePartialCommand(0x14b71, 0x42, 0x14b74 + 10, 0x3c - 10, STR_SPAN(" Sora's morning in Shopping Street"));
        patcher.ReplacePartialCommand(0x14bb3, 0x42, 0x14bb6 + 10, 0x3c - 10, STR_SPAN(" Training with Chiaki in Memorial Park"));
        patcher.ReplacePartialCommand(0x14bf5, 0x42, 0x14bf8 + 10, 0x3c - 10, STR_SPAN(" Following Asuka (Brick Alley)"));
        patcher.ReplacePartialCommand(0x14c37, 0x42, 0x14c3a + 10, 0x3c - 10, STR_SPAN(" Following Asuka (Station Square)"));
        patcher.ReplacePartialCommand(0x14c79, 0x42, 0x14c7c + 10, 0x3c - 10, STR_SPAN(" Following Asuka (Shopping Street)"));
        patcher.ReplacePartialCommand(0x14cbb, 0x42, 0x14cbe + 10, 0x3c - 10, STR_SPAN(" Outside Shopping Street in the evening"));
        patcher.ReplacePartialCommand(0x14cfd, 0x42, 0x14d00 + 10, 0x3c - 10, STR_SPAN(" Conversation at Kuguraya"));
        patcher.ReplacePartialCommand(0x14d3f, 0x42, 0x14d42 + 10, 0x3c - 10, STR_SPAN(" Finding a Gate by Sora's apartment"));
        patcher.ReplacePartialCommand(0x14d81, 0x42, 0x14d84 + 10, 0x3c - 10, STR_SPAN(" Entering Amber Road"));
        patcher.ReplacePartialCommand(0x14dc3, 0x42, 0x14dc6 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x14e05, 0x42, 0x14e08 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x14e47, 0x42, 0x14e4a + 10, 0x3c - 10, STR_SPAN(" Back outside Sora's apartment"));

        // EV_Jump_12_00
        patcher.ReplacePartialCommand(0x15071, 0x42, 0x15074 + 10, 0x3c - 10, STR_SPAN(" Brick Alley at night"));
        patcher.ReplacePartialCommand(0x150b3, 0x42, 0x150b6 + 10, 0x3c - 10, STR_SPAN(" Caf\xc3\xa9 17 after-hours"));
        patcher.ReplacePartialCommand(0x150f5, 0x42, 0x150f8 + 10, 0x3c - 10, STR_SPAN(" Outside school"));
        patcher.ReplacePartialCommand(0x15137, 0x42, 0x1513a + 10, 0x3c - 10, STR_SPAN(" Asuka leaving school"));
        patcher.ReplacePartialCommand(0x15179, 0x42, 0x1517c + 10, 0x3c - 10, STR_SPAN(" Outside Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0x151bb, 0x42, 0x151be + 10, 0x3c - 10, STR_SPAN(" Talk with Tetsuo"));
        patcher.ReplacePartialCommand(0x151fd, 0x42, 0x15200 + 10, 0x3c - 10, STR_SPAN(" Gate outside Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0x1523f, 0x42, 0x15242 + 10, 0x3c - 10, STR_SPAN(" Entering " DUNGEON_NAME_m3800));
        patcher.ReplacePartialCommand(0x15281, 0x42, 0x15284 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x152c3, 0x42, 0x152c6 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x15305, 0x42, 0x15308 + 10, 0x3c - 10, STR_SPAN(" Back outside"));
        patcher.ReplacePartialCommand(0x15347, 0x42, 0x1534a + 10, 0x3c - 10, STR_SPAN(" Asuka arrives at Caf\xc3\xa9 17"));
        patcher.ReplacePartialCommand(0x15389, 0x42, 0x1538c + 10, 0x3c - 10, STR_SPAN(" Surprise birthday party"));
        patcher.ReplacePartialCommand(0x153cb, 0x42, 0x153ce + 10, 0x3c - 10, STR_SPAN(" Outside Caf\xc3\xa9 17"));

        // EV_Jump_13_00
        patcher.ReplacePartialCommand(0x15645, 0x42, 0x15648 + 10, 0x3c - 10, STR_SPAN(" Yuuki wakes up at desk"));
        patcher.ReplacePartialCommand(0x15687, 0x42, 0x1568a + 10, 0x3c - 10, STR_SPAN(" Outside school, Sora runs into Yuuki"));
        patcher.ReplacePartialCommand(0x156c9, 0x42, 0x156cc + 10, 0x3c - 10, STR_SPAN(" Helping with carrying books"));
        patcher.ReplacePartialCommand(0x1570b, 0x42, 0x1570e + 10, 0x3c - 10, STR_SPAN(" Helping with looking up recipes"));
        patcher.ReplacePartialCommand(0x1574d, 0x42, 0x15750 + 10, 0x3c - 10, STR_SPAN(" Homemade lunch from Sora"));
        patcher.ReplacePartialCommand(0x1578f, 0x42, 0x15792 + 10, 0x3c - 10, STR_SPAN(" Confrontation with karate club member"));
        patcher.ReplacePartialCommand(0x157d1, 0x42, 0x157d4 + 10, 0x3c - 10, STR_SPAN(" Outside Station Square"));
        patcher.ReplacePartialCommand(0x15813, 0x42, 0x15816 + 10, 0x3c - 10, STR_SPAN(" Conversation with Sora at Star Camera"));
        patcher.ReplacePartialCommand(0x15855, 0x42, 0x15858 + 10, 0x3c - 10, STR_SPAN(" Gate below Station Square"));
        patcher.ReplacePartialCommand(0x15897, 0x42, 0x1589a + 10, 0x3c - 10, STR_SPAN(" Entering Garden of Stagnant Water"));
        patcher.ReplacePartialCommand(0x158d9, 0x42, 0x158dc + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x1591b, 0x42, 0x1591e + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x1595d, 0x42, 0x15960 + 10, 0x3c - 10, STR_SPAN(" Back at Station Square"));
        patcher.ReplacePartialCommand(0x1599f, 0x42, 0x159a2 + 10, 0x3c - 10, STR_SPAN(" Next day, getting thanks from schoolmates"));

        // EV_Jump_14_00
        patcher.ReplacePartialCommand(0x15c45, 0x42, 0x15c48 + 10, 0x3c - 10, STR_SPAN(" Outside Shopping Street"));
        patcher.ReplacePartialCommand(0x15c87, 0x42, 0x15c8a + 10, 0x3c - 10, STR_SPAN(" Shio working at soba shop"));
        patcher.ReplacePartialCommand(0x15cc9, 0x42, 0x15ccc + 10, 0x3c - 10, STR_SPAN(" Shio returns after deliveries"));
        patcher.ReplacePartialCommand(0x15d0b, 0x42, 0x15d0e + 10, 0x3c - 10, STR_SPAN(" Mitsuki was waiting for Shio"));
        patcher.ReplacePartialCommand(0x15d4d, 0x42, 0x15d50 + 10, 0x3c - 10, STR_SPAN(" Leaving Shopping Street with Mitsuki"));
        patcher.ReplacePartialCommand(0x15d8f, 0x42, 0x15d92 + 10, 0x3c - 10, STR_SPAN(" Next day, outside pawn shop"));
        patcher.ReplacePartialCommand(0x15dd1, 0x42, 0x15dd4 + 10, 0x3c - 10, STR_SPAN(" Mitsuki's past with Shio and Blaze"));
        patcher.ReplacePartialCommand(0x15e13, 0x42, 0x15e16 + 10, 0x3c - 10, STR_SPAN(" Entering Ruins of Poisonous Filth"));
        patcher.ReplacePartialCommand(0x15e55, 0x42, 0x15e58 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x15e97, 0x42, 0x15e9a + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x15ed9, 0x42, 0x15edc + 10, 0x3c - 10, STR_SPAN(" Back in pawn shop"));
        patcher.ReplacePartialCommand(0x15f1b, 0x42, 0x15f1e + 10, 0x3c - 10, STR_SPAN(" Friendship between Shio and Mitsuki"));

        // EV_Jump_15_00
        patcher.ReplacePartialCommand(0x161c1, 0x42, 0x161c4 + 10, 0x3c - 10, STR_SPAN(" White Shroud outside school"));
        patcher.ReplacePartialCommand(0x16203, 0x42, 0x16206 + 10, 0x3c - 10, STR_SPAN(" Mysterious shadow in school halls"));
        patcher.ReplacePartialCommand(0x16245, 0x42, 0x16248 + 10, 0x3c - 10, STR_SPAN(" Searching the library"));
        patcher.ReplacePartialCommand(0x16287, 0x42, 0x1628a + 10, 0x3c - 10, STR_SPAN(" Searching the classrooms"));
        patcher.ReplacePartialCommand(0x162c9, 0x42, 0x162cc + 10, 0x3c - 10, STR_SPAN(" Gate on the roof, meetup with Gorou"));
        patcher.ReplacePartialCommand(0x1630b, 0x42, 0x1630e + 10, 0x3c - 10, STR_SPAN(" Entering Castle of the Stygian Sky"));
        patcher.ReplacePartialCommand(0x1634d, 0x42, 0x16350 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x1638f, 0x42, 0x16392 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x163d1, 0x42, 0x163d4 + 10, 0x3c - 10, STR_SPAN(" Return to school roof"));

        // EV_Jump_16_00
        patcher.ReplacePartialCommand(0x1665d, 0x42, 0x16660 + 10, 0x3c - 10, STR_SPAN(" Rion getting a checkup at the hospital"));
        patcher.ReplacePartialCommand(0x1669f, 0x42, 0x166a2 + 10, 0x3c - 10, STR_SPAN(" Rion leaving hospital"));
        patcher.ReplacePartialCommand(0x166e1, 0x42, 0x166e4 + 10, 0x3c - 10, STR_SPAN(" Car ride"));
        patcher.ReplacePartialCommand(0x16723, 0x42, 0x16726 + 10, 0x3c - 10, STR_SPAN(" Flashback to examination results"));
        patcher.ReplacePartialCommand(0x16765, 0x42, 0x16768 + 10, 0x3c - 10, STR_SPAN(" Car ride (continued)"));
        patcher.ReplacePartialCommand(0x167a7, 0x42, 0x167aa + 10, 0x3c - 10, STR_SPAN(" Rion's childhood memories at the park"));
        patcher.ReplacePartialCommand(0x167e9, 0x42, 0x167ec + 10, 0x3c - 10, STR_SPAN(" Gate at the grove"));
        patcher.ReplacePartialCommand(0x1682b, 0x42, 0x1682e + 10, 0x3c - 10, STR_SPAN(" Entering Palace of the Luminous Sky"));
        patcher.ReplacePartialCommand(0x1686d, 0x42, 0x16870 + 10, 0x3c - 10, STR_SPAN(" Boss fight"));
        patcher.ReplacePartialCommand(0x168af, 0x42, 0x168b2 + 10, 0x3c - 10, STR_SPAN(" After boss fight"));
        patcher.ReplacePartialCommand(0x168f1, 0x42, 0x168f4 + 10, 0x3c - 10, STR_SPAN(" Back at the grove"));

        // EV_Jump_17_00
        patcher.ReplacePartialCommand(0x16bf6, 0x42, 0x16bf9 + 10, 0x3c - 10, STR_SPAN(" Halloween preparations at Station Square"));
        patcher.ReplacePartialCommand(0x16c38, 0x42, 0x16c3b + 10, 0x3c - 10, STR_SPAN(" Halloween preparations at Shopping Street"));
        patcher.ReplacePartialCommand(0x16c7a, 0x42, 0x16c7d + 10, 0x3c - 10, STR_SPAN(" Outside school"));
        patcher.ReplacePartialCommand(0x16cbc, 0x42, 0x16cbf + 10, 0x3c - 10, STR_SPAN(" Tanabe's class"));
        patcher.ReplacePartialCommand(0x16cfe, 0x42, 0x16d01 + 10, 0x3c - 10, STR_SPAN(" Conversation after school"));
        patcher.ReplacePartialCommand(0x16d40, 0x42, 0x16d43 + 10, 0x3c - 10, STR_SPAN(" Flashback to last night"));
        patcher.ReplacePartialCommand(0x16d82, 0x42, 0x16d85 + 10, 0x3c - 10, STR_SPAN(" Conversation after school (continued)"));
        patcher.ReplacePartialCommand(0x16dc4, 0x42, 0x16dc7 + 10, 0x3c - 10, STR_SPAN(" Encounter with Gorou"));
        patcher.ReplacePartialCommand(0x16e06, 0x42, 0x16e09 + 10, 0x3c - 10, STR_SPAN(" Report work and end free time?"));
        patcher.ReplacePartialCommand(0x16e48, 0x42, 0x16e4b + 10, 0x3c - 10, STR_SPAN(" Conversation with Yukino"));
        patcher.ReplacePartialCommand(0x16e8a, 0x42, 0x16e8d + 10, 0x3c - 10, STR_SPAN(" Leaving Brick Alley with Shiori and Asuka"));
        patcher.ReplacePartialCommand(0x16ecc, 0x42, 0x16ecf + 10, 0x3c - 10, STR_SPAN(" Arriving at Sunshine Road"));
        patcher.ReplacePartialCommand(0x16f0e, 0x42, 0x16f11 + 10, 0x3c - 10, STR_SPAN(" Kou wakes up"));
        patcher.ReplacePartialCommand(0x16f50, 0x42, 0x16f53 + 10, 0x3c - 10, STR_SPAN(" Exploring Station Square"));
        patcher.ReplacePartialCommand(0x16f92, 0x42, 0x16f95 + 10, 0x3c - 10, STR_SPAN(" Hearing a bell"));
        patcher.ReplacePartialCommand(0x16fd4, 0x42, 0x16fd7 + 10, 0x3c - 10, STR_SPAN(" Found the boy"));
        patcher.ReplacePartialCommand(0x17016, 0x42, 0x17019 + 10, 0x3c - 10, STR_SPAN(" Entering Boundary of Blood"));
        patcher.ReplacePartialCommand(0x17058, 0x42, 0x1705b + 10, 0x3c - 10, STR_SPAN(" Finding Rion and Towa"));
        patcher.ReplacePartialCommand(0x170ad, 0x42, 0x170b0 + 10, 0x3c - 10, STR_SPAN(" After boss"));
        patcher.ReplacePartialCommand(0x170ef, 0x42, 0x170f2 + 10, 0x3c - 10, STR_SPAN(" Searching with pendulum"));
        patcher.ReplacePartialCommand(0x17131, 0x42, 0x17134 + 10, 0x3c - 10, STR_SPAN(" Exploring Shopping Street"));
        patcher.ReplacePartialCommand(0x17173, 0x42, 0x17176 + 10, 0x3c - 10, STR_SPAN(" Finding the Gate at Yanagi Sports"));
        patcher.ReplacePartialCommand(0x171b5, 0x42, 0x171b8 + 10, 0x3c - 10, STR_SPAN(" Enter Gate?"));
        patcher.ReplacePartialCommand(0x171f7, 0x42, 0x171fa + 10, 0x3c - 10, STR_SPAN(" Entering " DUNGEON_NAME_m9810));
        patcher.ReplacePartialCommand(0x17239, 0x42, 0x1723c + 10, 0x3c - 10, STR_SPAN(" Finding Sora and Shio"));
        patcher.ReplacePartialCommand(0x1727b, 0x42, 0x1727e + 10, 0x3c - 10, STR_SPAN(" After miniboss"));
        patcher.ReplacePartialCommand(0x172bd, 0x42, 0x172c0 + 10, 0x3c - 10, STR_SPAN(" Continuing Search (Shopping Street)"));
        patcher.ReplacePartialCommand(0x172ff, 0x42, 0x17302 + 10, 0x3c - 10, STR_SPAN(" Outside Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0x17341, 0x42, 0x17344 + 10, 0x3c - 10, STR_SPAN(" Exploring Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0x17383, 0x42, 0x17386 + 10, 0x3c - 10, STR_SPAN(" Finding the Gate in the Server Room"));
        patcher.ReplacePartialCommand(0x173c5, 0x42, 0x173c8 + 10, 0x3c - 10, STR_SPAN(" Enter Gate?"));
        patcher.ReplacePartialCommand(0x17407, 0x42, 0x1740a + 10, 0x3c - 10, STR_SPAN(" Entering " DUNGEON_NAME_m9820));
        patcher.ReplacePartialCommand(0x17449, 0x42, 0x1744c + 10, 0x3c - 10, STR_SPAN(" Finding Mitsuki and Yuuki"));
        patcher.ReplacePartialCommand(0x1748b, 0x42, 0x1748e + 10, 0x3c - 10, STR_SPAN(" After miniboss"));
        patcher.ReplacePartialCommand(0x174cd, 0x42, 0x174d0 + 10, 0x3c - 10, STR_SPAN(" Continuing Search (Nanahoshi Mall)"));
        patcher.ReplacePartialCommand(0x1750f, 0x42, 0x17512 + 10, 0x3c - 10, STR_SPAN(" Searching with Horus (Shopping Street)"));
        patcher.ReplacePartialCommand(0x17551, 0x42, 0x17554 + 10, 0x3c - 10, STR_SPAN(" Searching with Horus (Nanahoshi Mall)"));
        patcher.ReplacePartialCommand(0x175a6, 0x42, 0x175a9 + 10, 0x3c - 10, STR_SPAN(" Exploring Brick Alley"));
        patcher.ReplacePartialCommand(0x175e8, 0x42, 0x175eb + 10, 0x3c - 10, STR_SPAN(" Finding the Gate by Rion's house"));
        patcher.ReplacePartialCommand(0x1762a, 0x42, 0x1762d + 10, 0x3c - 10, STR_SPAN(" Enter the Gate?"));
        patcher.ReplacePartialCommand(0x1766c, 0x42, 0x1766f + 10, 0x3c - 10, STR_SPAN(" Entering " DUNGEON_NAME_m9830));
        patcher.ReplacePartialCommand(0x176ae, 0x42, 0x176b1 + 10, 0x3c - 10, STR_SPAN(" Finding Gorou"));
        patcher.ReplacePartialCommand(0x176f0, 0x42, 0x176f3 + 10, 0x3c - 10, STR_SPAN(" Miniboss fight"));
        patcher.ReplacePartialCommand(0x17732, 0x42, 0x17735 + 10, 0x3c - 10, STR_SPAN(" After miniboss"));
        patcher.ReplacePartialCommand(0x17774, 0x42, 0x17777 + 10, 0x3c - 10, STR_SPAN(" Back in Brick Alley"));
        patcher.ReplacePartialCommand(0x177b6, 0x42, 0x177b9 + 10, 0x3c - 10, STR_SPAN(" Go to NDF base? (crashes game)"));
        patcher.ReplacePartialCommand(0x177f8, 0x42, 0x177fb + 10, 0x3c - 10, STR_SPAN(" Asuka and Shiori wake up"));
        patcher.ReplacePartialCommand(0x1783a, 0x42, 0x1783d + 10, 0x3c - 10, STR_SPAN(" Party arrives at NDF Base"));
        patcher.ReplacePartialCommand(0x1787c, 0x42, 0x1787f + 10, 0x3c - 10, STR_SPAN(" Entering Boundary of Blue Tears"));
        patcher.ReplacePartialCommand(0x178be, 0x42, 0x178c1 + 10, 0x3c - 10, STR_SPAN(" Finding Asuka and Shiori"));
        patcher.ReplacePartialCommand(0x17900, 0x42, 0x17903 + 10, 0x3c - 10, STR_SPAN(" After boss"));
        patcher.ReplacePartialCommand(0x17942, 0x42, 0x17945 + 10, 0x3c - 10, STR_SPAN(" Shiori points towards boy"));
        patcher.ReplacePartialCommand(0x17984, 0x42, 0x17987 + 10, 0x3c - 10, STR_SPAN(" Finding the Gate in Sunshine Road"));
        patcher.ReplacePartialCommand(0x179c6, 0x42, 0x179c9 + 10, 0x3c - 10, STR_SPAN(" Enter the Gate?"));
        patcher.ReplacePartialCommand(0x17a08, 0x42, 0x17a0b + 10, 0x3c - 10, STR_SPAN(" Entering Sky Tomb Boundary"));
        patcher.ReplacePartialCommand(0x17a5d, 0x42, 0x17a60 + 10, 0x3c - 10, STR_SPAN(" Shiori realizes the boy's identity"));
        patcher.ReplacePartialCommand(0x17a9f, 0x42, 0x17aa2 + 10, 0x3c - 10, STR_SPAN(" End of dungeon"));
        patcher.ReplacePartialCommand(0x17ae1, 0x42, 0x17ae4 + 10, 0x3c - 10, STR_SPAN(" Enter final boss fight?"));
        patcher.ReplacePartialCommand(0x17b23, 0x42, 0x17b26 + 10, 0x3c - 10, STR_SPAN(" Confronting Twilight Apostle"));
        patcher.ReplacePartialCommand(0x17b65, 0x42, 0x17b68 + 10, 0x3c - 10, STR_SPAN(" Defeated Twilight Apostle"));
        patcher.ReplacePartialCommand(0x17ba7, 0x42, 0x17baa + 10, 0x3c - 10, STR_SPAN(" Back to reality"));
        patcher.ReplacePartialCommand(0x17be9, 0x42, 0x17bec + 10, 0x3c - 10, STR_SPAN(" Spend time with Asuka"));
        patcher.ReplacePartialCommand(0x17c2b, 0x42, 0x17c2e + 10, 0x3c - 10, STR_SPAN(" Spend time with Sora"));
        patcher.ReplacePartialCommand(0x17c6d, 0x42, 0x17c70 + 10, 0x3c - 10, STR_SPAN(" Spend time with Yuuki"));
        patcher.ReplacePartialCommand(0x17caf, 0x42, 0x17cb2 + 10, 0x3c - 10, STR_SPAN(" Spend time with Shio"));
        patcher.ReplacePartialCommand(0x17cf1, 0x42, 0x17cf4 + 10, 0x3c - 10, STR_SPAN(" Spend time with Mitsuki"));
        patcher.ReplacePartialCommand(0x17d33, 0x42, 0x17d36 + 10, 0x3c - 10, STR_SPAN(" Spend time with Rion"));
        patcher.ReplacePartialCommand(0x17d75, 0x42, 0x17d78 + 10, 0x3c - 10, STR_SPAN(" Spend time with Gorou"));
        patcher.ReplacePartialCommand(0x17db7, 0x42, 0x17dba + 10, 0x3c - 10, STR_SPAN(" Spend time with Ryouta"));
        patcher.ReplacePartialCommand(0x17df9, 0x42, 0x17dfc + 10, 0x3c - 10, STR_SPAN(" Spend time with Jun"));
        patcher.ReplacePartialCommand(0x17e3b, 0x42, 0x17e3e + 10, 0x3c - 10, STR_SPAN(" Spend time with Shiori"));
        patcher.ReplacePartialCommand(0x17e7d, 0x42, 0x17e80 + 10, 0x3c - 10, STR_SPAN(" Spend time with Towa"));
        patcher.ReplacePartialCommand(0x17ebf, 0x42, 0x17ec2 + 10, 0x3c - 10, STR_SPAN(" Spend time with Asuka and Shiori"));
        patcher.ReplacePartialCommand(0x17f14, 0x42, 0x17f17 + 10, 0x3c - 10, STR_SPAN(" Approach the girl?"));
        patcher.ReplacePartialCommand(0x17f56, 0x42, 0x17f59 + 10, 0x3c - 10, STR_SPAN(" Talk to the girl"));
        patcher.ReplacePartialCommand(0x17f98, 0x42, 0x17f9b + 10, 0x3c - 10, STR_SPAN(" Conversation with Rem"));
        patcher.ReplacePartialCommand(0x17fda, 0x42, 0x17fdd + 10, 0x3c - 10, STR_SPAN(" Renewing your resolve"));
        patcher.ReplacePartialCommand(0x1801c, 0x42, 0x1801f + 10, 0x3c - 10, STR_SPAN(" Concert & Ending"));
        patcher.ReplacePartialCommand(0x1805e, 0x1c, 0x18061, 0x16, STR_SPAN(" \xe2\x87\x92" "[After Story] Next Page"));
        patcher.ReplacePartialCommand(0x1807a, 0x1c, 0x1807d, 0x16, STR_SPAN(" \xe2\x87\x92" "[After Story] Previous Page"));

        // EV_DoJump_17
        patcher.ReplacePartialCommand(0x185f0, 0x30, 0x185f3, 0x2a, STR_SPAN("Haven't visited Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0x18620, 0x21, 0x18623, 0x1b, STR_SPAN("Visited Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0x18641, 0x27, 0x18644, 0x21, STR_SPAN("Found Gate in Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0x18668, 0x1b, 0x1866b, 0x15, STR_SPAN("Cleared Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0x186e8, 0x2a, 0x186eb, 0x24, STR_SPAN("Haven't visited Shopping Street"));
        patcher.ReplacePartialCommand(0x18712, 0x1b, 0x18715, 0x15, STR_SPAN("Visited Shopping Street"));
        patcher.ReplacePartialCommand(0x1872d, 0x21, 0x18730, 0x1b, STR_SPAN("Found Gate in Shopping Street"));
        patcher.ReplacePartialCommand(0x1874e, 0x1e, 0x18751, 0x18, STR_SPAN("Cleared Shopping Street"));
        patcher.ReplacePartialCommand(0x18828, 0xf, 0x1882b, 0x9, STR_SPAN("Asuka"));
        patcher.ReplacePartialCommand(0x18837, 0xc, 0x1883a, 0x6, STR_SPAN("Sora"));
        patcher.ReplacePartialCommand(0x18843, 0xf, 0x18846, 0x9, STR_SPAN("Yuuki"));
        patcher.ReplacePartialCommand(0x18852, 0xc, 0x18855, 0x6, STR_SPAN("Shio"));
        patcher.ReplacePartialCommand(0x1885e, 0xf, 0x18861, 0x9, STR_SPAN("Mitsuki"));
        patcher.ReplacePartialCommand(0x1886d, 0xf, 0x18870, 0x9, STR_SPAN("Rion"));
        patcher.ReplacePartialCommand(0x1887c, 0xf, 0x1887f, 0x9, STR_SPAN("Gorou"));
        patcher.ReplacePartialCommand(0x1888b, 0x12, 0x1888e, 0xc, STR_SPAN("Ryouta"));
        patcher.ReplacePartialCommand(0x1889d, 0xf, 0x188a0, 0x9, STR_SPAN("Jun"));
        patcher.ReplacePartialCommand(0x188ac, 0xf, 0x188af, 0x9, STR_SPAN("Shiori"));
        patcher.ReplacePartialCommand(0x188bb, 0xc, 0x188be, 0x6, STR_SPAN("Towa"));
        patcher.ReplacePartialCommand(0x188c7, 0x1b, 0x188ca, 0x15, STR_SPAN("Asuka and Shiori"));
        patcher.ReplacePartialCommand(0x1916e, 0x35, 0x19171, 0x27, STR_SPAN("Did quest with Asuka"));
        patcher.ReplacePartialCommand(0x191aa, 0x32, 0x191ad, 0x24, STR_SPAN("Did quest with Sora"));
        patcher.ReplacePartialCommand(0x191e3, 0x35, 0x191e6, 0x27, STR_SPAN("Did quest with Yuuki"));
        patcher.ReplacePartialCommand(0x1921f, 0x32, 0x19222, 0x24, STR_SPAN("Did quest with Shio"));
        patcher.ReplacePartialCommand(0x19258, 0x35, 0x1925b, 0x27, STR_SPAN("Did quest with Mitsuki"));
        patcher.ReplacePartialCommand(0x19294, 0x35, 0x19297, 0x27, STR_SPAN("Did quest with Rion"));
        patcher.ReplacePartialCommand(0x192d0, 0x35, 0x192d3, 0x27, STR_SPAN("Did quest with Gorou"));
        patcher.ReplacePartialCommand(0x1930c, 0x32, 0x1930f, 0x24, STR_SPAN("Did quest with Towa"));
        patcher.ReplacePartialCommand(0x19345, 0x35, 0x19348, 0x27, STR_SPAN("Did quest with Shiori"));
        patcher.ReplacePartialCommand(0x19381, 0x38, 0x19384, 0x2a, STR_SPAN("Did quest with Ryouta"));
        patcher.ReplacePartialCommand(0x193c0, 0x35, 0x193c3, 0x27, STR_SPAN("Did quest with Jun"));
        patcher.ReplacePartialCommand(0x193fc, 0x38, 0x193ff, 0x2a, STR_SPAN("Saw both Asuka's and Shiori's ending"));
        patcher.ReplacePartialCommand(0x19434, 0x36, 0x19437, 0x30, STR_SPAN("[Proceed to Event]"));

        // TK_QuestJump
        patcher.ReplacePartialCommand(0x1af75, 0x15, 0x1af78 + 3, 0xf - 3, STR_SPAN(" Quests"));
        patcher.ReplacePartialCommand(0x1af8a, 0x21, 0x1af8d + 3, 0x1b - 3, STR_SPAN(" Affinity Events"));
        patcher.ReplacePartialCommand(0x1afab, 0x1b, 0x1afae + 3, 0x15 - 3, STR_SPAN(" Subevents"));
        patcher.ReplacePartialCommand(0x1afc6, 0x1b, 0x1afc9 + 3, 0x15 - 3, STR_SPAN(" Anytime Events"));
        patcher.ReplacePartialCommand(0x1afe1, 0x1b, 0x1afe4 + 3, 0x15 - 3, STR_SPAN(" Modify Parameters"));

        // EV_QuestJump_Sousuke
        patcher.ReplacePartialCommand(0x1b0d5, 0x12, 0x1b0d8, 0xc, STR_SPAN("Report"));

        // EV_QuestJump_Point
        patcher.ReplacePartialCommand(0x1b17c, 0x1e, 0x1b17f, 0x18, STR_SPAN("Report to Sousuke"));
        patcher.ReplacePartialCommand(0x1b19a, 0x2a, 0x1b19d, 0x24, STR_SPAN("Reset reports"));
        patcher.ReplacePartialCommand(0x1b1c4, 0x15, 0x1b1c7, 0xf, STR_SPAN("Wisdom +1"));
        patcher.ReplacePartialCommand(0x1b1d9, 0x18, 0x1b1dc, 0x12, STR_SPAN("Wisdom +10"));
        patcher.ReplacePartialCommand(0x1b1f1, 0x1b, 0x1b1f4, 0x15, STR_SPAN("Reset Wisdom"));
        patcher.ReplacePartialCommand(0x1b20c, 0x15, 0x1b20f, 0xf, STR_SPAN("Courage +1"));
        patcher.ReplacePartialCommand(0x1b221, 0x18, 0x1b224, 0x12, STR_SPAN("Courage +10"));
        patcher.ReplacePartialCommand(0x1b239, 0x1b, 0x1b23c, 0x15, STR_SPAN("Reset Courage"));
        patcher.ReplacePartialCommand(0x1b254, 0x15, 0x1b257, 0xf, STR_SPAN("Virtue +1"));
        patcher.ReplacePartialCommand(0x1b269, 0x18, 0x1b26c, 0x12, STR_SPAN("Virtue +10"));
        patcher.ReplacePartialCommand(0x1b281, 0x1b, 0x1b284, 0x15, STR_SPAN("Reset Virtue"));
        patcher.ReplacePartialCommand(0x1b29c, 0x16, 0x1b29f, 0x10, STR_SPAN("All Max"));
        patcher.ReplacePartialCommand(0x1b2b2, 0x19, 0x1b2b5, 0x13, STR_SPAN("Reset all"));
        patcher.ReplacePartialCommand(0x1b2cb, 0x13, 0x1b2ce, 0xd, STR_SPAN("Increase all by 1 level"));

        // EV_BustshotTest
        patcher.ReplacePartialCommand(0x1b4c8, 0x43, 0x1b4d4, 0x6, STR_SPAN("Kou"));
        patcher.ReplacePartialCommand(0x1b50b, 0x50, 0x1b51b, 0x9, STR_SPAN("Mitsuki"));
        patcher.ReplacePartialCommand(0x1b6d3, 0x39, 0x1b6d6, 0x34, STR_SPAN("Camera distance in Vita version"));

        // EV_QuestJump_0
        patcher.ReplacePartialCommand(0x1b822, 0x23, 0x1b825, 0x1d, STR_SPAN("Chapter 2 & 3 Quests"));
        patcher.ReplacePartialCommand(0x1b845, 0x23, 0x1b848, 0x1d, STR_SPAN("Chapter 4 & 5 Quests"));
        patcher.ReplacePartialCommand(0x1b868, 0x23, 0x1b86b, 0x1d, STR_SPAN("Chapter 6 & 7 Quests"));
        patcher.ReplacePartialCommand(0x1b88b, 0x2f, 0x1b88e, 0x29, STR_SPAN("Final Chapter & Epilogue Quests"));
        patcher.ReplacePartialCommand(0x1b8ba, 0x32, 0x1b8bd, 0x2c, STR_SPAN("After Story Quests"));

        // EV_QuestJump_0_1
        patcher.ReplacePartialCommand(0x1b9c6, 0x34, 0x1b9c9 + 8, 0x2e - 8, STR_SPAN(" Morimiya Runs on Fury (Required)"));
        patcher.ReplacePartialCommand(0x1b9fa, 0x22, 0x1b9fd + 8, 0x1c - 8, STR_SPAN(" My Precious"));
        patcher.ReplacePartialCommand(0x1ba1c, 0x3a, 0x1ba1f + 8, 0x34 - 8, STR_SPAN(" Rift Compatible (Required)"));
        patcher.ReplacePartialCommand(0x1ba56, 0x2b, 0x1ba59 + 8, 0x25 - 8, STR_SPAN(" Due Today, Not Tome-Orrow"));
        patcher.ReplacePartialCommand(0x1ba81, 0x1f, 0x1ba84 + 8, 0x19 - 8, STR_SPAN(" Tofu Trader's Trusty Trumpet"));
        patcher.ReplacePartialCommand(0x1baa0, 0x3a, 0x1baa3 + 8, 0x34 - 8, STR_SPAN(" Struggling in the Database (Hidden)"));

        // EV_QuestJump_0_2
        patcher.ReplacePartialCommand(0x1bb1e, 0x34, 0x1bb21 + 8, 0x2e - 8, STR_SPAN(" Poor Unfortunate Soles"));
        patcher.ReplacePartialCommand(0x1bb52, 0x2e, 0x1bb55 + 8, 0x28 - 8, STR_SPAN(" Run, Sora, Run"));
        patcher.ReplacePartialCommand(0x1bb80, 0x2b, 0x1bb83 + 8, 0x25 - 8, STR_SPAN(" Shine On, You Crazy Diamond"));
        patcher.ReplacePartialCommand(0x1bbab, 0x37, 0x1bbae + 8, 0x31 - 8, STR_SPAN(" This is Sparta...n Training! (Hidden)"));
        patcher.ReplacePartialCommand(0x1bbe2, 0x25, 0x1bbe5 + 8, 0x1f - 8, STR_SPAN(" Do It for the Vine"));
        patcher.ReplacePartialCommand(0x1bc07, 0x25, 0x1bc0a + 8, 0x1f - 8, STR_SPAN(" Ingredients, Dive-Ins, and Dives"));
        patcher.ReplacePartialCommand(0x1bc2c, 0x2b, 0x1bc2f + 8, 0x25 - 8, STR_SPAN(" Morimiya's Next Top Model"));
        patcher.ReplacePartialCommand(0x1bc57, 0x2e, 0x1bc5a + 8, 0x28 - 8, STR_SPAN(" Once More Unto the Brooch (Hidden)"));

        // EV_QuestJump_0_3
        patcher.ReplacePartialCommand(0x1bcca, 0x25, 0x1bccd + 8, 0x1f - 8, STR_SPAN(" Lights! Camera! ACTION!"));
        patcher.ReplacePartialCommand(0x1bcef, 0x28, 0x1bcf2 + 8, 0x22 - 8, STR_SPAN(" Looking for Law-Breaking Liquids"));
        patcher.ReplacePartialCommand(0x1bd17, 0x25, 0x1bd1a + 8, 0x1f - 8, STR_SPAN(" Tsubasa's Day Off"));
        patcher.ReplacePartialCommand(0x1bd3c, 0x2e, 0x1bd3f + 8, 0x28 - 8, STR_SPAN(" Strapped for Keepsakes (Hidden)"));
        patcher.ReplacePartialCommand(0x1bd6a, 0x27, 0x1bd6d + 8, 0x21 - 8, STR_SPAN(" What the XRC Does in the Shadows"));
        patcher.ReplacePartialCommand(0x1bd91, 0x22, 0x1bd94 + 8, 0x1c - 8, STR_SPAN(" Where in the World Is Airi?"));
        patcher.ReplacePartialCommand(0x1bdb3, 0x25, 0x1bdb6 + 8, 0x1f - 8, STR_SPAN(" Treasure That Transcends Time"));
        patcher.ReplacePartialCommand(0x1bdd8, 0x31, 0x1bddb + 8, 0x2b - 8, STR_SPAN(" Friends With Morimaru"));
        patcher.ReplacePartialCommand(0x1be09, 0x2b, 0x1be0c + 8, 0x25 - 8, STR_SPAN(" Splashing Free (Hidden)"));

        // EV_QuestJump_0_4
        patcher.ReplacePartialCommand(0x1be7a, 0x28, 0x1be7d + 8, 0x22 - 8, STR_SPAN(" Armored Chore (Hidden)"));
        patcher.ReplacePartialCommand(0x1bea2, 0x31, 0x1bea5 + 8, 0x2b - 8, STR_SPAN(" Antidote for Otherworld Ailments (Hidden)"));
        patcher.ReplacePartialCommand(0x1bed3, 0x34, 0x1bed6 + 8, 0x2e - 8, STR_SPAN(" SPiKA-Boo!"));
        patcher.ReplacePartialCommand(0x1bf07, 0x34, 0x1bf0a + 8, 0x2e - 8, STR_SPAN(" Soup Kitchen from Another World (Hidden)"));
        patcher.ReplacePartialCommand(0x1bf3b, 0x2b, 0x1bf3e + 8, 0x25 - 8, STR_SPAN(" Lights! Camera! ACTION! 2 Electric Boogaloo"));
        patcher.ReplacePartialCommand(0x1bf66, 0x37, 0x1bf69 + 8, 0x31 - 8, STR_SPAN(" You Will (Not) Avoid Home (Hidden/Required)"));
        patcher.ReplacePartialCommand(0x1bf9d, 0x31, 0x1bfa0 + 8, 0x2b - 8, STR_SPAN(" Stepping in for the Idol Masters"));
        patcher.ReplacePartialCommand(0x1bfce, 0x34, 0x1bfd1 + 8, 0x2e - 8, STR_SPAN(" International Girl of Mystery (Hidden)"));

        // EV_QuestJump_0_5
        patcher.ReplacePartialCommand(0x1c046, 0x37, 0x1c049 + 8, 0x31 - 8, STR_SPAN(" This Is Halloween"));
        patcher.ReplacePartialCommand(0x1c07d, 0x2e, 0x1c080 + 8, 0x28 - 8, STR_SPAN(" Pumpkin Spice and Everything Nice"));
        patcher.ReplacePartialCommand(0x1c0ab, 0x2b, 0x1c0ae + 8, 0x25 - 8, STR_SPAN(" Lager Than Life"));
        patcher.ReplacePartialCommand(0x1c0d6, 0x2e, 0x1c0d9 + 8, 0x28 - 8, STR_SPAN(" Nice Boat!"));
        patcher.ReplacePartialCommand(0x1c104, 0x34, 0x1c107 + 8, 0x2e - 8, STR_SPAN(" Dance, Morimaru, Dance!"));

        // EV_DoQuestJump_0
        patcher.ReplacePartialCommand(0x1cb3f, 0x1f, 0x1cb42 + 12, 0x19 - 12, STR_SPAN(" From beginning"));
        patcher.ReplacePartialCommand(0x1cb5e, 0x28, 0x1cb61 + 18, 0x22 - 18, STR_SPAN(" Returning to Yanagi"));
        patcher.ReplacePartialCommand(0x1cc31, 0x1c, 0x1cc34 + 12, 0x16 - 12, STR_SPAN(" From beginning"));
        patcher.ReplacePartialCommand(0x1cc4d, 0x22, 0x1cc50 + 15, 0x1c - 15, STR_SPAN(" Boarding the boat"));
        patcher.ReplacePartialCommand(0x1cc6f, 0x2b, 0x1cc72 + 12, 0x25 - 12, STR_SPAN(" On the lake"));
        patcher.ReplacePartialCommand(0x1cd46, 0x24, 0x1cd49 + 12, 0x1e - 12, STR_SPAN(" From beginning"));
        patcher.ReplacePartialCommand(0x1cd6a, 0x2d, 0x1cd6d + 12, 0x27 - 12, STR_SPAN(" Dance event"));
        patcher.ReplacePartialCommand(0x1cd97, 0x21, 0x1cd9a + 18, 0x1b - 18, STR_SPAN(" After dance"));

        // EV_QuestJump_1
        patcher.ReplacePartialCommand(0x1ce7e, 0x2c, 0x1ce81, 0x26, STR_SPAN("Asuka Affinity Events"));
        patcher.ReplacePartialCommand(0x1ceaa, 0x29, 0x1cead, 0x23, STR_SPAN("Sora Affinity Events"));
        patcher.ReplacePartialCommand(0x1ced3, 0x2c, 0x1ced6, 0x26, STR_SPAN("Yuuki Affinity Events"));
        patcher.ReplacePartialCommand(0x1ceff, 0x29, 0x1cf02, 0x23, STR_SPAN("Shio Affinity Events"));
        patcher.ReplacePartialCommand(0x1cf28, 0x2c, 0x1cf2b, 0x26, STR_SPAN("Mitsuki Affinity Events"));
        patcher.ReplacePartialCommand(0x1cf54, 0x2c, 0x1cf57, 0x26, STR_SPAN("Rion Affinity Events"));
        patcher.ReplacePartialCommand(0x1cf80, 0x2c, 0x1cf83, 0x26, STR_SPAN("Gorou Affinity Events"));
        patcher.ReplacePartialCommand(0x1cfac, 0x29, 0x1cfaf, 0x23, STR_SPAN("Towa Affinity Events"));
        patcher.ReplacePartialCommand(0x1cfd5, 0x2c, 0x1cfd8, 0x26, STR_SPAN("Shiori Affinity Events"));
        patcher.ReplacePartialCommand(0x1d001, 0x2f, 0x1d004, 0x29, STR_SPAN("Ryouta Affinity Events"));
        patcher.ReplacePartialCommand(0x1d030, 0x2c, 0x1d033, 0x26, STR_SPAN("Jun Affinity Events"));

        // EV_QuestJump_1_01
        patcher.ReplacePartialCommand(0x1d236, 0x37, 0x1d239 + 8, 0x31 - 8, STR_SPAN(" Asuka Affinity Event 1"));
        patcher.ReplacePartialCommand(0x1d26d, 0x37, 0x1d270 + 8, 0x31 - 8, STR_SPAN(" Asuka Affinity Event 2"));
        patcher.ReplacePartialCommand(0x1d2a4, 0x37, 0x1d2a7 + 8, 0x31 - 8, STR_SPAN(" Asuka Affinity Event 3"));
        patcher.ReplacePartialCommand(0x1d2db, 0x37, 0x1d2de + 8, 0x31 - 8, STR_SPAN(" Asuka Affinity Event 4"));
        patcher.ReplacePartialCommand(0x1d312, 0x37, 0x1d315 + 8, 0x31 - 8, STR_SPAN(" Asuka Affinity Event 5"));

        // EV_QuestJump_1_02
        patcher.ReplacePartialCommand(0x1d38e, 0x34, 0x1d391 + 8, 0x2e - 8, STR_SPAN(" Sora Affinity Event 1"));
        patcher.ReplacePartialCommand(0x1d3c2, 0x34, 0x1d3c5 + 8, 0x2e - 8, STR_SPAN(" Sora Affinity Event 2"));
        patcher.ReplacePartialCommand(0x1d3f6, 0x34, 0x1d3f9 + 8, 0x2e - 8, STR_SPAN(" Sora Affinity Event 3"));
        patcher.ReplacePartialCommand(0x1d42a, 0x34, 0x1d42d + 8, 0x2e - 8, STR_SPAN(" Sora Affinity Event 4"));
        patcher.ReplacePartialCommand(0x1d45e, 0x34, 0x1d461 + 8, 0x2e - 8, STR_SPAN(" Sora Affinity Event 5"));
        patcher.ReplacePartialCommand(0x1d492, 0x34, 0x1d495 + 8, 0x2e - 8, STR_SPAN(" Sora Affinity Event Final"));

        // EV_QuestJump_1_03
        patcher.ReplacePartialCommand(0x1d50a, 0x37, 0x1d50d + 8, 0x31 - 8, STR_SPAN(" Yuuki Affinity Event 1"));
        patcher.ReplacePartialCommand(0x1d541, 0x37, 0x1d544 + 8, 0x31 - 8, STR_SPAN(" Yuuki Affinity Event 2"));
        patcher.ReplacePartialCommand(0x1d578, 0x37, 0x1d57b + 8, 0x31 - 8, STR_SPAN(" Yuuki Affinity Event 3"));
        patcher.ReplacePartialCommand(0x1d5af, 0x37, 0x1d5b2 + 8, 0x31 - 8, STR_SPAN(" Yuuki Affinity Event 4"));
        patcher.ReplacePartialCommand(0x1d5e6, 0x37, 0x1d5e9 + 8, 0x31 - 8, STR_SPAN(" Yuuki Affinity Event Final"));

        // EV_QuestJump_1_04
        patcher.ReplacePartialCommand(0x1d662, 0x34, 0x1d665 + 8, 0x2e - 8, STR_SPAN(" Shio Affinity Event 1"));
        patcher.ReplacePartialCommand(0x1d696, 0x34, 0x1d699 + 8, 0x2e - 8, STR_SPAN(" Shio Affinity Event 2"));
        patcher.ReplacePartialCommand(0x1d6ca, 0x34, 0x1d6cd + 8, 0x2e - 8, STR_SPAN(" Shio Affinity Event 3"));
        patcher.ReplacePartialCommand(0x1d6fe, 0x34, 0x1d701 + 8, 0x2e - 8, STR_SPAN(" Shio Affinity Event 4"));
        patcher.ReplacePartialCommand(0x1d732, 0x34, 0x1d735 + 8, 0x2e - 8, STR_SPAN(" Shio Affinity Event Final"));

        // EV_QuestJump_1_05
        patcher.ReplacePartialCommand(0x1d7aa, 0x37, 0x1d7ad + 8, 0x31 - 8, STR_SPAN(" Mitsuki Affinity Event 1"));
        patcher.ReplacePartialCommand(0x1d7e1, 0x37, 0x1d7e4 + 8, 0x31 - 8, STR_SPAN(" Mitsuki Affinity Event 2"));
        patcher.ReplacePartialCommand(0x1d818, 0x37, 0x1d81b + 8, 0x31 - 8, STR_SPAN(" Mitsuki Affinity Event 3"));
        patcher.ReplacePartialCommand(0x1d84f, 0x37, 0x1d852 + 8, 0x31 - 8, STR_SPAN(" Mitsuki Affinity Event 4"));
        patcher.ReplacePartialCommand(0x1d886, 0x37, 0x1d889 + 8, 0x31 - 8, STR_SPAN(" Mitsuki Affinity Event Final"));

        // EV_QuestJump_1_06
        patcher.ReplacePartialCommand(0x1d902, 0x37, 0x1d905 + 8, 0x31 - 8, STR_SPAN(" Rion Affinity Event 1"));
        patcher.ReplacePartialCommand(0x1d939, 0x37, 0x1d93c + 8, 0x31 - 8, STR_SPAN(" Rion Affinity Event 2"));
        patcher.ReplacePartialCommand(0x1d970, 0x37, 0x1d973 + 8, 0x31 - 8, STR_SPAN(" Rion Affinity Event 3"));
        patcher.ReplacePartialCommand(0x1d9a7, 0x37, 0x1d9aa + 8, 0x31 - 8, STR_SPAN(" Rion Affinity Event 4"));
        patcher.ReplacePartialCommand(0x1d9de, 0x37, 0x1d9e1 + 8, 0x31 - 8, STR_SPAN(" Rion Affinity Event Final"));

        // EV_QuestJump_1_07
        patcher.ReplacePartialCommand(0x1da5a, 0x37, 0x1da5d + 8, 0x31 - 8, STR_SPAN(" Gorou Affinity Event 1"));
        patcher.ReplacePartialCommand(0x1da91, 0x37, 0x1da94 + 8, 0x31 - 8, STR_SPAN(" Gorou Affinity Event 2"));
        patcher.ReplacePartialCommand(0x1dac8, 0x37, 0x1dacb + 8, 0x31 - 8, STR_SPAN(" Gorou Affinity Event 3"));
        patcher.ReplacePartialCommand(0x1daff, 0x37, 0x1db02 + 8, 0x31 - 8, STR_SPAN(" Gorou Affinity Event Final"));

        // EV_QuestJump_1_08
        patcher.ReplacePartialCommand(0x1db7a, 0x34, 0x1db7d + 8, 0x2e - 8, STR_SPAN(" Towa Affinity Event 1"));
        patcher.ReplacePartialCommand(0x1dbae, 0x34, 0x1dbb1 + 8, 0x2e - 8, STR_SPAN(" Towa Affinity Event 2"));
        patcher.ReplacePartialCommand(0x1dbe2, 0x34, 0x1dbe5 + 8, 0x2e - 8, STR_SPAN(" Towa Affinity Event 3"));
        patcher.ReplacePartialCommand(0x1dc16, 0x34, 0x1dc19 + 8, 0x2e - 8, STR_SPAN(" Towa Affinity Event Final"));

        // EV_QuestJump_1_09
        patcher.ReplacePartialCommand(0x1dc8e, 0x37, 0x1dc91 + 8, 0x31 - 8, STR_SPAN(" Shiori Affinity Event 1"));
        patcher.ReplacePartialCommand(0x1dcc5, 0x37, 0x1dcc8 + 8, 0x31 - 8, STR_SPAN(" Shiori Affinity Event 2"));
        patcher.ReplacePartialCommand(0x1dcfc, 0x37, 0x1dcff + 8, 0x31 - 8, STR_SPAN(" Shiori Affinity Event 3"));
        patcher.ReplacePartialCommand(0x1dd33, 0x37, 0x1dd36 + 8, 0x31 - 8, STR_SPAN(" Shiori Affinity Event Final"));

        // EV_QuestJump_1_10
        patcher.ReplacePartialCommand(0x1ddae, 0x3a, 0x1ddb1 + 8, 0x34 - 8, STR_SPAN(" Ryouta Affinity Event 1"));
        patcher.ReplacePartialCommand(0x1dde8, 0x3a, 0x1ddeb + 8, 0x34 - 8, STR_SPAN(" Ryouta Affinity Event 2"));
        patcher.ReplacePartialCommand(0x1de22, 0x3a, 0x1de25 + 8, 0x34 - 8, STR_SPAN(" Ryouta Affinity Event 3"));
        patcher.ReplacePartialCommand(0x1de5c, 0x3a, 0x1de5f + 8, 0x34 - 8, STR_SPAN(" Ryouta Affinity Event Final"));

        // EV_QuestJump_1_11
        patcher.ReplacePartialCommand(0x1deda, 0x37, 0x1dedd + 8, 0x31 - 8, STR_SPAN(" Jun Affinity Event 1"));
        patcher.ReplacePartialCommand(0x1df11, 0x37, 0x1df14 + 8, 0x31 - 8, STR_SPAN(" Jun Affinity Event 2"));
        patcher.ReplacePartialCommand(0x1df48, 0x37, 0x1df4b + 8, 0x31 - 8, STR_SPAN(" Jun Affinity Event 3"));
        patcher.ReplacePartialCommand(0x1df7f, 0x37, 0x1df82 + 8, 0x31 - 8, STR_SPAN(" Jun Affinity Event Final"));

        // EV_QuestJump_2
        patcher.ReplacePartialCommand(0x1ed6e, 0x2c, 0x1ed71, 0x26, STR_SPAN("Chapter 2 & 3 Subevents"));
        patcher.ReplacePartialCommand(0x1ed9a, 0x2c, 0x1ed9d, 0x26, STR_SPAN("Chapter 4 & 5 Subevents"));
        patcher.ReplacePartialCommand(0x1edc6, 0x2c, 0x1edc9, 0x26, STR_SPAN("Chapter 6 & 7 Subevents"));
        patcher.ReplacePartialCommand(0x1edf2, 0x35, 0x1edf5, 0x2f, STR_SPAN("Final Chapter & Epilogue Subevents"));
        patcher.ReplacePartialCommand(0x1ee27, 0x24, 0x1ee2a, 0x1e, STR_SPAN("\xe2\x98\x85" "Complete required Subevents"));
        patcher.ReplacePartialCommand(0x1ee4b, 0x38, 0x1ee4e, 0x32, STR_SPAN("After Story Subevents"));

        // EV_QuestJump_2_1
        patcher.ReplacePartialCommand(0x1ef7a, 0x24, 0x1ef7d, 0x1e, STR_SPAN("Chapter 2: " "\xe2\x96\xbc" " Searching for Chiaki"));
        patcher.ReplacePartialCommand(0x1ef9e, 0x51, 0x1efa1, 0x4b, STR_SPAN("Chapter 2: " "\xe2\x96\xbc" " Sealing a Gate in Brick Alley"));
        patcher.ReplacePartialCommand(0x1efef, 0x3f, 0x1eff2, 0x39, STR_SPAN("Chapter 3: " "\xe2\x96\xbd" " Towa and Gorou setting up the Computer Lab"));
        patcher.ReplacePartialCommand(0x1f02e, 0x3c, 0x1f031, 0x36, STR_SPAN("Chapter 3: " "\xe2\x96\xbd" " Preparations for art gallery in Brick Alley"));
        patcher.ReplacePartialCommand(0x1f06a, 0x2a, 0x1f06d, 0x24, STR_SPAN("Chapter 3: " "\xe2\x96\xbc" " Investigation of God's App"));

        // EV_QuestJump_2_2
        patcher.ReplacePartialCommand(0x1f14a, 0x3f, 0x1f14d, 0x39, STR_SPAN("Chapter 4: " "\xe2\x96\xbd" " Mitsuki returns to school"));
        patcher.ReplacePartialCommand(0x1f189, 0x2a, 0x1f18c, 0x24, STR_SPAN("Chapter 4: " "\xe2\x96\xbc" " Investigation of Blaze"));
        patcher.ReplacePartialCommand(0x1f1b3, 0x2d, 0x1f1b6, 0x27, STR_SPAN("Chapter 4: " "\xe2\x96\xbc" " Searching the dance club"));
        patcher.ReplacePartialCommand(0x1f1e0, 0x30, 0x1f1e3, 0x2a, STR_SPAN("Chapter 4: " "\xe2\x96\xbc" " Talking to guests in Gemini"));
        patcher.ReplacePartialCommand(0x1f210, 0x33, 0x1f213, 0x2d, STR_SPAN("Chapter 4: " "\xe2\x96\xbc" " Situation in Horaichou"));
        patcher.ReplacePartialCommand(0x1f243, 0x1e, 0x1f246, 0x18, STR_SPAN("Chapter 5: " "\xe2\x96\xbc" " Investigation of the fog"));
        patcher.ReplacePartialCommand(0x1f261, 0x21, 0x1f264, 0x1b, STR_SPAN("Chapter 5: " "\xe2\x96\xbd" " Investigation of the dense fog"));
        patcher.ReplacePartialCommand(0x1f282, 0x1e, 0x1f285, 0x18, STR_SPAN("Intermission: " "\xe2\x96\xbd" " Talking to everyone at night"));

        // EV_QuestJump_2_3
        patcher.ReplacePartialCommand(0x1f6e6, 0x30, 0x1f6e9, 0x2a, STR_SPAN("Chapter 6: Gate in Star Camera"));
        patcher.ReplacePartialCommand(0x1f716, 0x4b, 0x1f719, 0x45, STR_SPAN("Chapter 6: " "\xe2\x96\xbc" " Call with Kyouka and XRC after telling SPiKA about Rion's state"));
        patcher.ReplacePartialCommand(0x1f761, 0x30, 0x1f764, 0x2a, STR_SPAN("Chapter 7: " "\xe2\x96\xbc" " Investigation of frozen lake"));
        patcher.ReplacePartialCommand(0x1f791, 0x33, 0x1f794, 0x2d, STR_SPAN("Chapter 7: " "\xe2\x96\xbc" " Searching for underground in Station Square"));
        patcher.ReplacePartialCommand(0x1f7c4, 0x2d, 0x1f7c7, 0x27, STR_SPAN("Chapter 7: " "\xe2\x96\xbd" " Rion's glasses disguise (leave for Houraichou)"));

        // EV_QuestJump_2_4
        patcher.ReplacePartialCommand(0x1f8e6, 0x54, 0x1f8e9, 0x4e, STR_SPAN("Final Chapter: " "\xe2\x96\xbd" " Visiting Pandora (two seals left)"));
        patcher.ReplacePartialCommand(0x1f93a, 0x54, 0x1f93d, 0x4e, STR_SPAN("Final Chapter: " "\xe2\x96\xbd" " Visiting Pandora (one seal left)"));
        patcher.ReplacePartialCommand(0x1f98e, 0x3f, 0x1f991, 0x39, STR_SPAN("Final Chapter: " "\xe2\x96\xbc" " Stronghold at Nanahoshi Mall"));
        patcher.ReplacePartialCommand(0x1f9cd, 0x30, 0x1f9d0, 0x2a, STR_SPAN("Final Chapter: " "\xe2\x96\xbd" " Rion visiting her familiy"));
        patcher.ReplacePartialCommand(0x1f9fd, 0x3f, 0x1fa00, 0x39, STR_SPAN("Final Chapter: " "\xe2\x96\xbc" " Night before the final battle, Asuka in the library"));
        patcher.ReplacePartialCommand(0x1fa3c, 0x30, 0x1fa3f, 0x2a, STR_SPAN("Final Chapter: Missing Shiori photos (Residential Area)"));
        patcher.ReplacePartialCommand(0x1fa6c, 0x30, 0x1fa6f, 0x2a, STR_SPAN("Final Chapter: " "\xe2\x96\xbd" " Stopping by Gorou's apartment"));
        patcher.ReplacePartialCommand(0x1fa9c, 0x39, 0x1fa9f, 0x33, STR_SPAN("Final Chapter: " "\xe2\x96\xbd" " Event at Yanagi Sports"));
        patcher.ReplacePartialCommand(0x1fad5, 0x30, 0x1fad8, 0x2a, STR_SPAN("Final Chapter: " "\xe2\x96\xbc" " Final dungeon boss fights"));
        patcher.ReplacePartialCommand(0x1fb05, 0x2a, 0x1fb08, 0x24, STR_SPAN("Epilogue: " "\xe2\x96\xbc" " Key collecting"));

        // EV_QuestJump_2_5
        patcher.ReplacePartialCommand(0x1fc96, 0x30, 0x1fc99, 0x2a, STR_SPAN("Chapter 6 Star Camera investigation"));

        // EV_QuestJump_2_6
        #define AFTER_STORY_SUBEVENT(commandLocation, commandLength, replacementLocation, replacementLength, str) do { \
            static constexpr size_t len = sizeof(str) - 1; \
            std::array<char, len + 26> replacementData; \
            std::memcpy(replacementData.data(), "After Story: ", 13); \
            std::memcpy(replacementData.data() + 13, &bin[replacementLocation + 15], 13); \
            std::memcpy(replacementData.data() + 26, str, len); \
            patcher.ReplacePartialCommand(commandLocation, commandLength, replacementLocation, replacementLength, replacementData); \
        } while (false)
        AFTER_STORY_SUBEVENT(0x1fd1a, 0x55, 0x1fd1d, 0x4f, " Rion and Haruna in XRC clubroom");
        AFTER_STORY_SUBEVENT(0x1fd6f, 0x55, 0x1fd72, 0x4f, " Reika, Wakaba, and Akira in karate dojo");
        AFTER_STORY_SUBEVENT(0x1fdc4, 0x55, 0x1fdc7, 0x4f, " Meeting Asuka and Shiori in Caf\xc3\xa9 17");
        AFTER_STORY_SUBEVENT(0x1fe19, 0x55, 0x1fe1c, 0x4f, " First time at Sunshine Road");
        AFTER_STORY_SUBEVENT(0x1fe6e, 0x55, 0x1fe71, 0x4f, " Encounter with Mikuriya");
        AFTER_STORY_SUBEVENT(0x1fec3, 0x58, 0x1fec6, 0x52, " Crystals in Station Square");
        AFTER_STORY_SUBEVENT(0x1ff1b, 0x55, 0x1ff1e, 0x4f, " Unsealing any crystal");
        AFTER_STORY_SUBEVENT(0x1ff70, 0x55, 0x1ff73, 0x4f, " Unsealing Mizuhara");
        AFTER_STORY_SUBEVENT(0x1ffc5, 0x55, 0x1ffc8, 0x4f, " Unsealing Akane and Tsubasa");
        AFTER_STORY_SUBEVENT(0x2001a, 0x55, 0x2001d, 0x4f, " Unsealing Chizuru");
        AFTER_STORY_SUBEVENT(0x2006f, 0x55, 0x20072, 0x4f, " Unsealing Mayu and Jihei");
        AFTER_STORY_SUBEVENT(0x200c4, 0x55, 0x200c7, 0x4f, " Unsealing Tetsuo and Karen");
        AFTER_STORY_SUBEVENT(0x20119, 0x55, 0x2011c, 0x4f, " Unsealing Sadie");
        AFTER_STORY_SUBEVENT(0x2016e, 0x55, 0x20171, 0x4f, " Unsealing Yukino and Yamaoka");
        AFTER_STORY_SUBEVENT(0x201c3, 0x55, 0x201c6, 0x4f, " Unsealing Kaoru");
        AFTER_STORY_SUBEVENT(0x20218, 0x55, 0x2021b, 0x4f, " Finding the barrier towards Kokonoe Shrine");
        AFTER_STORY_SUBEVENT(0x2026d, 0x55, 0x20270, 0x4f, " Having Chizuru take care of the cat");

        // EV_QuestJump_3
        patcher.ReplacePartialCommand(0x20806, 0x2d, 0x20809, 0x27, STR_SPAN("Weekly Lottery"));
        patcher.ReplacePartialCommand(0x20833, 0x21, 0x20836, 0x1b, STR_SPAN("Drawing Fortunes (Shrine)"));
        patcher.ReplacePartialCommand(0x20854, 0x18, 0x20857, 0x12, STR_SPAN("Sadie's Fortune Telling"));
        patcher.ReplacePartialCommand(0x2086c, 0x2a, 0x2086f, 0x24, STR_SPAN("Report books"));
        patcher.ReplacePartialCommand(0x20896, 0x2d, 0x20899, 0x27, STR_SPAN("Report Friend pages"));
        patcher.ReplacePartialCommand(0x208c3, 0x24, 0x208c6, 0x1e, STR_SPAN("Hand over food"));
        patcher.ReplacePartialCommand(0x208e7, 0x1b, 0x208ea, 0x15, STR_SPAN("Skate Park"));
        patcher.ReplacePartialCommand(0x20902, 0x24, 0x20905, 0x1e, STR_SPAN("Gate of Avalon"));
        patcher.ReplacePartialCommand(0x20926, 0x2a, 0x20929, 0x24, STR_SPAN("Report Parameters"));
        patcher.ReplacePartialCommand(0x20950, 0x1e, 0x20953, 0x18, STR_SPAN("Room decoration"));

        // YR_Jump_COLLECT
        patcher.ReplacePartialCommand(0x20e52, 0x55, 0x20e55 + 16, 0x4f - 16, STR_SPAN(" Introduction (after warp, exit Brick Alley)"));
        patcher.ReplacePartialCommand(0x20ea7, 0x31, 0x20eaa + 18, 0x2b - 18, STR_SPAN(" Entering Kou's Room"));
        patcher.ReplacePartialCommand(0x20ed8, 0x47, 0x20edb + 22, 0x41 - 22, STR_SPAN(" Ryouta & Jun Event"));
        patcher.ReplacePartialCommand(0x20f1f, 0x3e, 0x20f22 + 22, 0x38 - 22, STR_SPAN(" Sora & Towa Event"));
        patcher.ReplacePartialCommand(0x20f5d, 0x41, 0x20f60 + 22, 0x3b - 22, STR_SPAN(" Yuuki & Shio Event"));
        patcher.ReplacePartialCommand(0x20f9e, 0x44, 0x20fa1 + 22, 0x3e - 22, STR_SPAN(" Asuka & Mitsuki Event"));
        patcher.ReplacePartialCommand(0x20fe2, 0x44, 0x20fe5 + 22, 0x3e - 22, STR_SPAN(" Rion & Shiori Event"));
        patcher.ReplacePartialCommand(0x21026, 0x21, 0x21029,      0x1b,      STR_SPAN("Collected everything"));

        // clang-format on

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixes::a0000
