#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t2000[] = "Fix text issues in Parm";
}

namespace SenLib::Sen3::FileFixes::t2000_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t2000.dat",
            58881,
            HyoutaUtils::Hash::SHA1FromHexString("9bdd3a298549cfe45119b3f668efe6a59bb230c9"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "Okay! You’re on!"
        // wrong apostrophe
        // (chapter 1, 4/22, Vantage Master initial conversation with Clover with Kurt in party)
        patcher.ReplacePartialCommand(0x2e86, 0x37, 0x2eb2, 3, {{'\''}});

        // "Okay! You’re on!"
        // wrong apostrophe
        // (chapter 1, 4/23, Vantage Master initial conversation with Clover without Kurt in party)
        patcher.ReplacePartialCommand(0x3010, 0x36, 0x303b, 3, {{'\''}});

        // "They say it's quite dense... Quite dark...\x01the sort of place people wouldn’t
        // dare\x01trespass. It sounds..."
        // wrong apostrophe
        // (chapter 1, 4/23, NPC dialogue with Orvid)
        patcher.ReplacePartialCommand(0x485e, 0x16e, 0x491d, 3, {{'\''}});

        // "We're going to go and pick up Luna’s clothes\x01now. Once again, thank you so much."
        // wrong apostrophe
        // (chapter 1, 4/23, NPC dialogue with Helena after finishing the Lost Wallet sidequest)
        patcher.ReplacePartialCommand(0x54ab, 0xd1, 0x554a, 3, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t2000_dat
