#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/memread.h"

namespace SenLib::Sen2::FileFixes::scripts_talk_dat_us_tk_monica_dat {
std::string_view GetDescription() {
    return "Fix reference to incorrect character in dialogue with Monica.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_us/tk_monica.dat",
            24009,
            SenPatcher::SHA1FromHexString("836140611baf2149c7ac1a28182762b25354b54f"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // Hibelle -> Casper
        // Whoever edited or translated this must have been really tired here...
        const auto casper = HyoutaUtils::MemRead::ReadArray<6>(&bin[0x17c4]);
        patcher.ReplacePartialCommand(0x4b5, 0x83, 0x4dc, 0x7, casper);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_talk_dat_us_tk_monica_dat
