#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1010_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in Class VII dorm.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t1010.dat",
            241846,
            SenPatcher::SHA1FromHexString("44c4abb3f8e01dde0e36ca1d11cd433f37c10788"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);
        patcher.RemovePartialCommand(0x25f91, 0x13f, 0x2606b, 1);

        // formatting issues in Elliot Chapter 1 Day bonding event
        std::swap(bin[0x322ae], bin[0x322b6]);
        std::swap(bin[0x322d7], bin[0x322dd]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1010_dat
