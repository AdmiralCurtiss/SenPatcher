#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3600[] =
    "Fix terminology consistency issues in Ordis (Harbor District).";
}

namespace SenLib::Sen3::FileFixes::t3600_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3600.dat",
            148257,
            HyoutaUtils::Hash::SHA1FromHexString("f20f8c36a44c88fecc44155250b42fa0259cd699"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // the Juno Naval Fortress -> Juno Naval Fortress
        patcher.RemovePartialCommand(0xb336, 0x17e, 0xb3b6, 0x4);

        // "#E_4#M_A#B_0Are you here to admire my yacht!?"
        // !? -> ?!
        // (chapter 3, 6/17, during the Yacht-a Yacht-a Yacht-a sidequest)
        // patcher.ReplacePartialCommand(0x178a0, 0x44, 0x178bd, 0x25, "");
        std::swap(bin[0x178e0], bin[0x178e1]);

        // "#2PA-A sniper!? How can this be?!"
        // !? -> ?!
        // (chapter 3, 6/18 morning, during the A Tale of Two Shops sidequest)
        // patcher.ReplacePartialCommand(0x1da55, 0x2a, 0x1da5c, 0x21, "");
        std::swap(bin[0x1da69], bin[0x1da6a]);

        // "#8KWhat's wrong, guys?\x01Did something bad happen."
        // "happen." -> "happen?"
        // (chapter 3, 6/18 festival night, starting the A Keepsake Lost quest. must already be
        // established that Ash knows Jutta (ie, talk to Jutta when first coming to Ordis))
        // patcher.ReplacePartialCommand(0x1fb56, 0x39, 0x1fb5d, 0x30, "");
        bin[0x1fb8c] = '?';

        // "The group handed Jutta a Magnificient Carnelia."
        // Magnificient -> Magnificent
        // (chapter 3, 6/18 festival night, finishing the A Keepsake Lost quest)
        patcher.RemovePartialCommand(0x229e3, 0x35, 0x22a08, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3600_dat
