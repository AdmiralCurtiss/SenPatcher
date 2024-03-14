#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen3::FileFixes::m3430_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Dark Dragon's Nest (Area 1).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/m3430.dat",
            43457,
            SenPatcher::SHA1FromHexString("8a880ba00b221cdc176eb66cabb23573750251d5"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // the Juno Naval Fortress -> Juno Naval Fortress
        patcher.RemovePartialCommand(0x93d4, 0x49, 0x9403, 0x4);
        patcher.RemovePartialCommand(0x9483, 0x4c, 0x94b5, 0x4);

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::m3430_dat
