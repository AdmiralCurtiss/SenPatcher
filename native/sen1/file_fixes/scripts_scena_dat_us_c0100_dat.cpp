#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/memread.h"
#include "util/memwrite.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_c0100_dat {
std::string_view GetDescription() {
    return "Fix text and voice errors on Vainqueur Street.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/c0100.dat",
            162620,
            SenPatcher::SHA1FromHexString("9a75c8439ca12500437af585d0ae94744b620dfd"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemRead;
        using namespace HyoutaUtils::MemWrite;

        // fix duplicate line on Imperial Chronicle sign
        SenScriptPatcher patcher(bin);
        patcher.RemovePartialCommand(0x27aed, 0x2c, 0x27b03, 0x13);

        // fix incorrect voice clips for Rean in Alisa's bike scene
        WriteUInt16(&bin[0x10150], 63276);
        WriteUInt16(&bin[0x10253], 63278);

        // fix voice/text mismatches in Millium's bike scene
        patcher.ExtendPartialCommand(
            0x155cd, 0x4e, 0x15618, {{0x20, 0x65, 0x69, 0x74, 0x68, 0x65, 0x72}});
        patcher.ReplacePartialCommand(
            0x168a9, 0x2f, 0x168b1, 0x24, {{0x57, 0x6f, 0x6f, 0x20, 0x68, 0x6f, 0x6f}});

        // swap 1st and 2nd line from summer festival employee handing out stuff (PS4 does this and
        // it makes more sense)
        const auto line1 = ReadArray<0x31>(&bin[0x7b87]);
        const auto line2 = ReadArray<0x31>(&bin[0x7bc1]);
        WriteArray(&bin[0x7b87], line2);
        WriteArray(&bin[0x7bc1], line1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_c0100_dat
