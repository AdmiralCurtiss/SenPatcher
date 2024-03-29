#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0010_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in Thors (bottom floor).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0010.dat",
            254098,
            SenPatcher::SHA1FromHexString("8a76ff88baf96b5e72e675d0d5d3b75a72cc3989"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        SenScriptPatcher patcher(bin);
        patcher.RemovePartialCommand(0xfbbf, 0x53, 0xfbcf, 1);

        // formatting issues in Towa Chapter 3 Day bonding event
        std::swap(bin[0x292ef], bin[0x292f4]);
        std::swap(bin[0x29cf4], bin[0x29cf8]);
        WriteUInt8(&bin[0x29d48], 0x01);
        std::swap(bin[0x29e57], bin[0x29e5b]);
        std::swap(bin[0x29e78], bin[0x29e7e]);
        std::swap(bin[0x2a13e], bin[0x2a143]);
        std::swap(bin[0x2a3fc], bin[0x2a3ff]);
        std::swap(bin[0x2a421], bin[0x2a42a]);
        std::swap(bin[0x2a6e3], bin[0x2a6e6]);
        std::swap(bin[0x2a72a], bin[0x2a72f]);

        // formatting issues in Fie Chapter 4 Day bonding event
        std::swap(bin[0x2b045], bin[0x2b04a]);
        std::swap(bin[0x2b069], bin[0x2b070]);

        // formatting issues in Fie Chapter 5 Evening bonding event
        std::swap(bin[0x2bda3], bin[0x2bda7]);
        std::swap(bin[0x2bdc6], bin[0x2bdcb]);
        std::swap(bin[0x2c02e], bin[0x2c032]);

        // missing 'to' in Rufus line (voice match, also makes more sense) (Chapter 6 intro)
        patcher.ExtendPartialCommand(0x2585e, 0x48, 0x25883, {{0x20, 0x74, 0x6f}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0010_dat
