#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_luise[] =
    "Fix typography errors in conversation with Luise.";
}

namespace SenLib::Sen3::FileFixes::tk_luise_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_luise.dat",
            32209,
            HyoutaUtils::Hash::SHA1FromHexString("7d32046669a0e608779e9dcd5524ed3fa5029fd7"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "Received Chrono Repeater"
        // missing '!' at the end of the system message
        // (reporting full friends book)
        patcher.ExtendPartialCommand(0x7422, 0x14, 0x7434, {{'!'}});

        // "#kI see-- thank you."
        // remove space after '--'
        // (reporting full friends book)
        patcher.RemovePartialCommand(0x7561, 0x1d, 0x7571, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_luise_dat
