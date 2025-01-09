#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/memread.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_e7060[] =
    "Fix text/voice inconsistency at start of Act 2 (farewell scene with Sharon).";
}

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_e7060_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/e7060.dat",
            161585,
            HyoutaUtils::Hash::SHA1FromHexString("8000797757b16c64656b0403584bafa1e85af342"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // I feel like I have so much more{n}that I can learn from you. -> I feel like I have so
        // much more{n}I could learn from you. to match VO (two instances)
        patcher.ReplacePartialCommand(
            0x8b90, 0xda, 0x8c4e, 0xa, {{0x49, 0x20, 0x63, 0x6f, 0x75, 0x6c, 0x64}});
        patcher.ReplacePartialCommand(
            0x2435e, 0xda, 0x2441c, 0xa, {{0x49, 0x20, 0x63, 0x6f, 0x75, 0x6c, 0x64}});

        // Fix inconsistency in Alfin line between game and memories menu, match memories to game.
        patcher.RemovePartialCommand(0x1ac12, 0x43, 0x1ac40, 0xe);
        patcher.RemovePartialCommand(0x1e8d2, 0x43, 0x1e900, 0xe);
        patcher.RemovePartialCommand(0x22642, 0x43, 0x22670, 0xe);

        // Fix inconsistency in Sara line between game and memories menu, match memories to game.
        const auto saraline = HyoutaUtils::MemRead::ReadArray<0x41>(&bin[0x616e]);
        patcher.ReplaceCommand(0x1ad23, 0x39, saraline);
        patcher.ReplaceCommand(0x1e9e3, 0x37, saraline);
        patcher.ReplaceCommand(0x22753, 0x39, saraline);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_e7060_dat
