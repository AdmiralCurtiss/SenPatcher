#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t4080_dat {
std::string_view GetDescription() {
    return "Fix various minor English voice issues (missing and wrong voice clips) in hot spring "
           "scenes.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t4080.dat",
            163289,
            SenPatcher::SHA1FromHexString("18c7ad19818f25d7a2ad61c3f7c711bf16cff933"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        using namespace HyoutaUtils::MemWrite;

        // Elise in Prologue
        patcher.ExtendPartialCommand(0x3176, 0x8, 0x3179, {{0x11, 0x2b, (char)0xff, 0x00, 0x00}});
        patcher.ExtendPartialCommand(0x34bc, 0x31, 0x34d8, {{0x11, 0x2c, (char)0xff, 0x00, 0x00}});

        // Emma
        WriteUInt16(&bin[0xf678], 65326);
        patcher.ExtendPartialCommand(0xf674, 0x11c, 0xf747, {{0x11, 0x2f, (char)0xff, 0x00, 0x00}});
        WriteUInt16(&bin[0xf9a0], 65328);
        WriteUInt16(&bin[0xf9d2], 65329);
        WriteUInt16(&bin[0xfa0c], 65330);

        // Jusis
        WriteUInt16(&bin[0x113a2], 65331);

        // Fie
        WriteUInt16(&bin[0x12bb8], 65332);

        // Gaius
        WriteUInt16(&bin[0x14208], 65333);

        // Claire
        WriteUInt16(&bin[0x18718], 65334);
        WriteUInt16(&bin[0x18dc4], 65335);
        WriteUInt16(&bin[0x18df8], 65336);

        // Sharon
        WriteUInt16(&bin[0x1a18f], 65337);
        WriteUInt16(&bin[0x1a820], 65338);

        // Sara
        WriteUInt16(&bin[0x1bc08], 65339);
        WriteUInt16(&bin[0x1bc72], 65340);
        WriteUInt16(&bin[0x1c1d1], 65341);
        WriteUInt16(&bin[0x1c205], 65342);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t4080_dat
