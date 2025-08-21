#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_system3[] = "";
}

namespace SenLib::Sen3::FileFixes::system3_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/system3.dat",
            63425,
            HyoutaUtils::Hash::SHA1FromHexString("d7cf5b2d1ac1dbba25cd929c138069d23316d1d4"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#K#0TThis is rather tasteful fountain\x01pen... Is it for me?"
        // is rather -> is a rather
        // (gift for Kurt, chapter 1 at Demeter Exchange & Antiques)
        patcher.ExtendPartialCommand(0x5af8, 0x43, 0x5b0c, STR_SPAN("a "));

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::system3_dat
