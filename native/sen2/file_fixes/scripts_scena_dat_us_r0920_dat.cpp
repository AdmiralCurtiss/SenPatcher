#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_r0920_dat {
std::string_view GetDescription() {
    return "Fix logic error in Gone Air sidequest.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/r0920.dat",
            48425,
            SenPatcher::SHA1FromHexString("1e69ef573a1888e5545850c97d6252fb1f5b5596"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // this is a rather fun error that I totally get how it happened
        // if Millium is in the party, the conversation goes
        // Rean: i'm impressed you figured out this stuff [about the enemy mercenaries]
        // Millium: did you get some info from the RMP? you definitely have their weapons!
        // Klein: haha, yeah you're right
        // Klein: we're working with them
        // but if Millium is NOT in the party, the line from Millium and Klein's reply to it get
        // skipped which, in the English script, gives the odd impression that Klein is working with
        // the enemy mercenaries, instead of with the RMP. this fixes that by just explicitly
        // mentioning the RMP.
        std::swap(bin[0x7059], bin[0x7060]);
        patcher.ReplacePartialCommand(0x7038, 0x33, 0x7058, 0x1, {{0x20, 0x52, 0x4d, 0x50}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_r0920_dat
