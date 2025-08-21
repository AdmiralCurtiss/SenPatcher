#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_alisa[] = "Fix typo in conversation with Alisa.";
}

namespace SenLib::Sen3::FileFixes::tk_alisa_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_alisa.dat",
            5167,
            HyoutaUtils::Hash::SHA1FromHexString("2f13090b7a21805a96b59f7c0d7405203537556c"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#k#1U(She flew those things all the way to\x01Crossbell? She serously gets around.)"
        // serously -> seriously
        // (chapter 4, 7/17 free time, at the Karel Imperial Villa pavillion)
        patcher.ExtendPartialCommand(0x87f, 0x59, 0x8c3, {{'i'}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_alisa_dat
