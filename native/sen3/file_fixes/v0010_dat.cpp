#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_v0010[] = "Terminology fixes in Derfflinger Car 1.";
}

namespace SenLib::Sen3::FileFixes::v0010_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/v0010.dat",
            293281,
            HyoutaUtils::Hash::SHA1FromHexString("4c2781204c47f2457e831b6a50413068b498c5b3"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // the Juno Naval Fortress -> Juno Naval Fortress
        patcher.RemovePartialCommand(0x26097, 0xe7, 0x26147, 0x4);
        patcher.RemovePartialCommand(0x262c6, 0x25, 0x262d0, 0x4);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::v0010_dat
