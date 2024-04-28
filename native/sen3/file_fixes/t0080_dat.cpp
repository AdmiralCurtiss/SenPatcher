#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen3::FileFixes::t0080_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Leeves (Radio Trista).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0080.dat",
            84689,
            SenPatcher::SHA1FromHexString("dacda5af52ca6ab3efee4f9b51606a5e3d676a7e"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // St. Ursula's -> St. Ursula
        patcher.RemovePartialCommand(0x1115e, 0x7f, 0x111b4, 0x2);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0080_dat
