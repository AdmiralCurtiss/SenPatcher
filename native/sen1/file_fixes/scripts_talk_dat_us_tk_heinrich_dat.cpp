#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/memwrite.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_heinrich[] =
    "Fix formatting issues in conversation with Heinrich.";
}

namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_heinrich_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_us/tk_heinrich.dat",
            17750,
            HyoutaUtils::Hash::SHA1FromHexString("491c365d592bb90029e7543d893d47bd5e66139d"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        WriteArray(&bin[0x1168], {{0x2e, 0x2e, 0x2e}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_heinrich_dat
