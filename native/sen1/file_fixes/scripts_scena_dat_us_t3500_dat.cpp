#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/memwrite.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3500[] = "Fix formatting issues in main Roer area.";
}

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t3500_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t3500.dat",
            191658,
            HyoutaUtils::Hash::SHA1FromHexString("d18f9880c045b969afd8c6a8836ee6e86810aa4e"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        WriteArray(&bin[0x9ce6], {{0x2e, 0x2e, 0x2e}});
        WriteArray(&bin[0x28b20], {{0x2e, 0x2e, 0x2e}});
        WriteArray(&bin[0x29b60], {{0x2e, 0x2e, 0x2e}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t3500_dat
