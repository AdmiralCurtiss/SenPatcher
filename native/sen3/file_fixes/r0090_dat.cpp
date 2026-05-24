#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_r0090[] = "Fix minor text errors in Chapter 1 Ex. Camp";
}

namespace SenLib::Sen3::FileFixes::r0090_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/r0090.dat",
            284641,
            HyoutaUtils::Hash::SHA1FromHexString("1eef53d4fde2b2e7434fa63960e16c254d951440"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "Oh, yeah. Just stopped by to see how you\x01were doing and take care of an\x01errand."
        // remove the last linebreak, it looks really bad
        // patcher.ReplacePartialCommand(0x31ea2, 0xcd, 0x31ea9, 0x54, "");
        bin[0x31ef5] = ' ';

        // '#E_2#M_0#B_0...Guessing it was some some\x01'supporting gauntlet'-type stuff?'
        // 'some some' -> 'some'
        patcher.RemovePartialCommand(0x32532, 0x61, 0x3256a, 5);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::r0090_dat
