#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen3::FileFixes::c0400_dat {
std::string_view GetDescription() {
    return "Typo in Crossbell East Street.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c0400.dat",
            45641,
            SenPatcher::SHA1FromHexString("91fe1bb9b0861e93a37835c7678dd389ad365655"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // IIt's -> It's
        patcher.RemovePartialCommand(0x397b, 0xf3, 0x3983, 0x1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c0400_dat
