#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s3100[] = "Text fixes in Acros Tower Lobby.";
}

namespace SenLib::TX::FileFixesSw::s3100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s3100.dat",
            93313,
            HyoutaUtils::Hash::SHA1FromHexString("8d9da5b698e0410bf4d0707d116db7bb9f24a670"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Kinda of a bummer that it's so rainy\x01right now, but I figured I'd come\x01early
        // before the show starts."
        // Remove the 'a' from 'Kinda'
        patcher.RemovePartialCommand(0xa2e8, 0xb2, 0xa2ef, 0x1);

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s3100
