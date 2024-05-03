#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0090_dat {
std::string_view GetDescription() {
    return "Fix typo in Engineering Building.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0090.dat",
            74485,
            HyoutaUtils::Hash::SHA1FromHexString("a823fb0d4b8a4ffbdeaa6eb407bb38807048b226"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // typo fix, lession -> lesson (Start of Chapter 6, 9/18, conversation with
        // Crow/Angelica/George)
        patcher.RemovePartialCommand(0x56be, 0x75, 0x56ff, 0x1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0090_dat
