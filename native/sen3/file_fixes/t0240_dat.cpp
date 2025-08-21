#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0240[] = "Fix typo in Clubhouse.";
}

namespace SenLib::Sen3::FileFixes::t0240_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0240.dat",
            87329,
            HyoutaUtils::Hash::SHA1FromHexString("c26dfe7930d2176cb7f0025e00d233e5b950b3a8"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#E_2#M[0]#B_0#1U(I should invite Sydney to shooting\x01practice as soon as I'm able.)"
        // Sydney -> Sidney
        // patcher.ReplacePartialCommand(0x596b, 0x95, 0x59b9, 0x45, "");
        // (chapter 2, 5/14 morning, Sidney's Sadness sidequest, must have taken a bath)
        bin[0x59ce] = 'i';

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0240_dat
