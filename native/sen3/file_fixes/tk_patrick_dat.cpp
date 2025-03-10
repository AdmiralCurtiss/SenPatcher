#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_patrick[] =
    "Fix typography consistency issues in conversations with Patrick.";
}

namespace SenLib::Sen3::FileFixes::tk_patrick_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_patrick.dat",
            2761,
            HyoutaUtils::Hash::SHA1FromHexString("6ed1ba8558121db318927df8771712233f41fcce"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // !? -> ?!
        std::swap(bin[0x2ff], bin[0x300]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_patrick_dat
