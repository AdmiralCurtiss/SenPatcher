#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3030[] = "Fix text formatting in Ordis (Kleist Mall).";
}

namespace SenLib::Sen3::FileFixes::t3030_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3030.dat",
            37073,
            HyoutaUtils::Hash::SHA1FromHexString("76efa2027ffe7d39e8a1218d49b80b69b6da85cf"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "I'm from here in Ordis.\x01 I inherited my father's store."
        // extra space
        // (chapter 3, 6/17, before leaving for Raquel)
        patcher.RemovePartialCommand(0x1316, 0x1b0, 0x1335, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3030_dat
