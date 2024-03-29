#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0032_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in teahouse festival events.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0032.dat",
            95586,
            SenPatcher::SHA1FromHexString("e7854ac057166d50d94c340ec39403d26173ff9f"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        SenScriptPatcher patcher(bin);

        WriteArray(&bin[0x2360], {{0x2e, 0x2e, 0x2e}});
        WriteArray(&bin[0xe22a], {{0x2e, 0x2e, 0x2e}});
        WriteArray(&bin[0x11fde], {{0x2e, 0x2e, 0x2e}});
        WriteArray(&bin[0x137db], {{0x2e, 0x2e, 0x2e}});

        // 'no way I can to quit now' -> 'no way I can quit now'
        patcher.RemovePartialCommand(0x1622b, 0xa5, 0x162c2, 0x3);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0032_dat
