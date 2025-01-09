#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m3000[] =
    "Terminology fixes in Juno Naval Fortress (Sub 1).";
}

namespace SenLib::Sen3::FileFixes::m3000_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/m3000.dat",
            54289,
            HyoutaUtils::Hash::SHA1FromHexString("03bec649d56e048772de4fd1ce013005b651d784"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        using namespace HyoutaUtils::Vector;

        // General Aurelia -> General Le Guin
        WriteAt(bin, 0x6847, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::m3000_dat
