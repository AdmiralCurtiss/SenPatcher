#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_r0600_dat {
std::string_view GetDescription() {
    return "Fix formatting issues and text/voice mismatches in western Trista highway.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/r0600.dat",
            73271,
            SenPatcher::SHA1FromHexString("65044a35a4c042fabc4a5a66fd23b0cd8163dfdb"));
        if (!file) {
            return false;
        }

        using namespace HyoutaUtils::MemWrite;

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // formatting issues in Jusis Chapter 6 Day bonding event
        WriteArray(&bin[0xc171], {{0x2e, 0x2e, 0x2e}});
        std::swap(bin[0xbf07], bin[0xbf11]);

        // formatting issues in Alisa Chapter 5 Day bonding event
        std::swap(bin[0xb00b], bin[0xb013]);
        std::swap(bin[0xb2bb], bin[0xb2c0]);
        std::swap(bin[0xb2e4], bin[0xb2e7]);

        // 'U-Umm...' -> 'Umm...' (voice match)
        patcher.RemovePartialCommand(0x208b, 0x35, 0x2094, 0x2);

        // 'Ya' -> 'You' (voice match)
        patcher.ReplacePartialCommand(0x874c, 0x42, 0x8762, 0x1, {{0x6f, 0x75}});

        // 'you' -> 'ya' (voice match)
        patcher.ReplacePartialCommand(0x8f0a, 0x5a, 0x8f5f, 0x2, {{0x61}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_r0600_dat
