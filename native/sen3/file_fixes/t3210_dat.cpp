#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3210[] =
    "Typography fixes in Ordis (Sea Breeze Tavern & Inn)";
}

namespace SenLib::Sen3::FileFixes::t3210_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3210.dat",
            35345,
            HyoutaUtils::Hash::SHA1FromHexString("88ed5200c0d11645d92cbdea397971a04b0c6218"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#E[5]#M_4#B_0My mom told me there’d be loooads of handsome\x01men coming in for the
        // Summer Festival!"
        // wrong apostrophe
        // (chapter 3, 6/18 festival night, NPC dialogue with Betty)
        patcher.ReplacePartialCommand(0x25b6, 0xbb, 0x25e8, 3, {{'\''}});

        // "#E[1]#M_2She’s actually not too bad\x01at cooking and\x01taking care of the house..."
        // wrong apostrophe
        // (chapter 3, 6/18 festival night, NPC dialogue with Blanca)
        patcher.ReplacePartialCommand(0x2783, 0xe4, 0x27cd, 3, {{'\''}});

        // "#E[1]#M_A#B_0I honestly couldn’t be more relieved."
        // wrong apostrophe
        // (chapter 3, 6/18 festival night, NPC dialogue with Edmond)
        patcher.ReplacePartialCommand(0x2e47, 0xf8, 0x2eb5, 3, {{'\''}});

        // "#E_0#M_A#B_0They’re as fatty and delicious\x01as we can get. Go on and give\x01it a
        // taste!"
        // wrong apostrophe
        // (chapter 3, 6/17 morning, NPC dialogue with Edmond)
        patcher.ReplacePartialCommand(0x32dc, 0xb9, 0x334d, 3, {{'\''}});

        // "Don’t be shy! We're serving\x01everyone who comes in today."
        // wrong apostrophe
        // (chapter 3, 6/18 festival night, NPC dialogue with Marina)
        patcher.ReplacePartialCommand(0x34a9, 0x76, 0x34b3, 3, {{'\''}});

        // "#E_0#M_A#B[7]He’d come to drink here at my\x01father's restaurant every day."
        // wrong apostrophe
        // (chapter 3, 6/18 morning, NPC dialogue with Marina)
        patcher.ReplacePartialCommand(0x3569, 0x154, 0x362c, 3, {{'\''}});

        // "We got busy, so I was gonna\x01ask her to help out, but I\x01suppose I’m too late..."
        // wrong apostrophe
        // (chapter 3, 6/17 morning, NPC dialogue with Marina)
        patcher.ReplacePartialCommand(0x3a38, 0x7a, 0x3aa0, 3, {{'\''}});

        // "#E[1]#M_AEverything’s happening so fast,\x01but I'm glad I came."
        // wrong apostrophe
        // (chapter 3, 6/17, NPC dialogue with Reins before leaving Ordis)
        patcher.ReplacePartialCommand(0x438d, 0x77, 0x43d6, 3, {{'\''}});

        // "I didn't get any information on\x01the treasure, but I think it’s provided\x01me
        // splendid inspiration."
        // "#E[5]#M_A#B_0Now, it’s time I throw myself into\x01the arms of this joyful night."
        // wrong apostrophe
        // (chapter 3, 6/18 festival night, NPC dialogue with Aleister when you *started but didn't
        // finish* the Mystery of the Old Map quest)
        patcher.ReplacePartialCommand(0x4f5e, 0xbd, 0x4fdd, 3, {{'\''}});
        patcher.ReplacePartialCommand(0x4f5e, 0xbd, 0x4fa1, 3, {{'\''}});

        // "#8KYeah,  she asked for something more\x01inland, so I had to think about it, but..."
        // extra space
        // (chapter 3, 6/18 afternoon, during the search for Millium)
        patcher.RemovePartialCommand(0x84b3, 0x5a, 0x84c3, 1);

        // "Thank you. You’ve been a great help."
        // wrong apostrophe
        // (chapter 3, 6/18 afternoon, during the search for Millium)
        patcher.ReplacePartialCommand(0x863d, 0x5d, 0x8652, 3, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3210_dat
