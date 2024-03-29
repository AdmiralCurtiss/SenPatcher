#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0050_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in the Gymnasium.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0050.dat",
            221932,
            SenPatcher::SHA1FromHexString("d363fc2114ec8421c24b47c29a4a2baded31cfb5"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        // double space
        SenScriptPatcher patcher(bin);
        patcher.RemovePartialCommand(0x11b02, 0x44, 0x11b10, 1);

        // formatting issues in Laura Chapter 1 Day bonding event
        std::swap(bin[0x14e92], bin[0x14e95]);
        std::swap(bin[0x1505a], bin[0x15061]);
        std::swap(bin[0x15186], bin[0x1518e]);
        std::swap(bin[0x153a1], bin[0x153a6]);

        // formatting issues in Laura Chapter 4 Evening bonding event
        std::swap(bin[0x162a6], bin[0x162ab]);
        std::swap(bin[0x164ae], bin[0x164b4]);
        std::swap(bin[0x164d8], bin[0x164dd]);
        std::swap(bin[0x16627], bin[0x1662a]);

        // formatting issues in Laura Chapter 5 Day bonding event
        WriteUInt8(&bin[0x17070], 0x01);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0050_dat
