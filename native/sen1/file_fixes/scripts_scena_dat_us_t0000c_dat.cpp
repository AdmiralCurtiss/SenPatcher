#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0000c_dat {
std::string_view GetDescription() {
    return "Fix text/voice mismatch in dance scenes.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0000c.dat",
            137276,
            SenPatcher::SHA1FromHexString("8bf43f6d4c7f0de5ba13e4c14301da05bd1c919a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // extra word in voice but not text
        patcher.ExtendPartialCommand(0xcdd1, 0x8c, 0xce58, {{0x2c, 0x20, 0x68, 0x75, 0x68}});

        // repeated voice clip where there shouldn't be one
        patcher.RemovePartialCommand(0xd1d1, 0x3d, 0xd1d4, 5);

        // extra word in voice but not text
        patcher.ExtendPartialCommand(0x1f46b, 0x10, 0x1f476, {{0x4d, 0x6d, 0x2d, 0x68, 0x6d}});

        // 'Ditto my dad.' -> 'Ditto my father.' to match voice clip
        std::swap(bin[0x20a2f], bin[0x20a34]);
        patcher.ReplacePartialCommand(
            0x20a03, 0x58, 0x20a14, 0x3, {{0x66, 0x61, 0x74, 0x68, 0x65, 0x72}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0000c_dat
