#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_gorou[] = "Text fixes in conversations with Gorou.";
}

namespace SenLib::TX::FileFixesSw::tk_gorou {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_gorou.dat",
            16233,
            HyoutaUtils::Hash::SHA1FromHexString("5273d4a8d1faf7992cd830abfe4444d41a0f2b19"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E[1]#M_9You piqued my interest when you\x01said it was a model for hikking."
        // hikking -> hiking
        // patcher.ReplacePartialCommand(0xff0, 0x66, 0x1010, 0x44, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_gorou
