#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/memread.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t5660_dat {
std::string_view GetDescription() {
    return "Text/Voice matches in Garellia Fortress.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t5660.dat",
            64627,
            SenPatcher::SHA1FromHexString("78aa964124be90f4b9c5da90418a724c0b46cbc3"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemRead;

        SenScriptPatcher patcher(bin);

        // voice clip matches (Chapter 5, scene when seeing the railway guns for the first time)
        // i think these happened when they re-recorded a few lines here for the PC version but only
        // updated the text in the Prologue variant and not the Chapter 5 variant
        patcher.RemovePartialCommand(0x3776, 0x19, 0x3780, 0x3);
        patcher.ReplacePartialCommand(0x4a05, 0x3e, 0x4a2e, 0x6, {{0x6e, 0x6e, 0x61}});

        // remove 'Oh,' (not in voice clip, from PS4) (scene after Scarlet boss fight)
        const auto oh = ReadArray<5>(&bin[0xa871]);
        patcher.ReplacePartialCommand(0xa864, 0x24, 0xa871, 0x5, {{0x54}});

        // add 'Oh,' (it's in voice clip, from PS4) (scene after Scarlet boss fight)
        patcher.ReplacePartialCommand(0xb4db, 0x1b, 0xb4e3, 0x1, oh);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t5660_dat
