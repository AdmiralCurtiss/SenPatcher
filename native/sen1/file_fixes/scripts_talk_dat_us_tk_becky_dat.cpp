#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/memwrite.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_becky[] = "Fix typo in conversation with Becky.";
}

namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_becky_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_us/tk_becky.dat",
            11088,
            HyoutaUtils::Hash::SHA1FromHexString("3eb462def36cc15103e48657aa18b6b02abcd830"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        WriteUInt8(&bin[0x13e], 0x61);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_becky_dat
