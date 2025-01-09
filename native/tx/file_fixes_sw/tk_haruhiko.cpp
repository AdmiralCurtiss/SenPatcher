#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_haruhiko[] =
    "Text fixes in conversations with Haruhiko.";
}

namespace SenLib::TX::FileFixesSw::tk_haruhiko {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_haruhiko.dat",
            12241,
            HyoutaUtils::Hash::SHA1FromHexString("cd6693f80d2648d864d82e1a03c9ef9ec9f0a9c8"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "I don't want to be a fake,]; I want to be better. To be a real man."
        // keyboard typo that no one caught
        patcher.RemovePartialCommand(0xccb, 0xdc, 0xd2a, 0x2);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_haruhiko
