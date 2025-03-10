#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c0250[] =
    "Fix name and terminology consistency in Crossbell (SSS Building).";
}

namespace SenLib::Sen3::FileFixes::c0250_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c0250.dat",
            41409,
            HyoutaUtils::Hash::SHA1FromHexString("e9ef91d4cf274a0fb055303c3d29dcd1af99fd49"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // old battlefield -> ancient battlefield
        patcher.ReplacePartialCommand(
            0x34a3, 0xc4, 0x3502, 0x3, {{0x61, 0x6e, 0x63, 0x69, 0x65, 0x6e, 0x74}});
        patcher.ReplacePartialCommand(
            0x58d2, 0x136, 0x596d, 0x3, {{0x61, 0x6e, 0x63, 0x69, 0x65, 0x6e, 0x74}});

        // royal family -> Imperial family
        patcher.ReplacePartialCommand(
            0x6ef4, 0x26, 0x6f07, 0x3, {{0x49, 0x6d, 0x70, 0x65, 0x72, 0x69}});
        patcher.ReplacePartialCommand(
            0x6f34, 0x5f, 0x6f70, 0x3, {{0x49, 0x6d, 0x70, 0x65, 0x72, 0x69}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c0250_dat
