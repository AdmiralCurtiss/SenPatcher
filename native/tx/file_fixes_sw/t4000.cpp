#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t4000[] = "Text fixes in Brick Alley.";
}

namespace SenLib::TX::FileFixesSw::t4000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t4000.dat",
            321313,
            HyoutaUtils::Hash::SHA1FromHexString("a3f92b5e719ed89aefd289ed539cc971ac67d669"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "(Only place I can think of that fits that description)"
        // missing period
        patcher.ExtendPartialCommand(0x27d90, 0xc5, 0x27e24, {{0x2e}});

        // "Something must've dumped it here..." -> Someone
        patcher.ReplacePartialCommand(0x3f45f, 0x81, 0x3f469, 0x5, {{0x6f, 0x6e, 0x65}});

        // "#E[3]#M_A#3K#800WWhat are you even doing here#12W,\x01#200WKugayama?"
        // patcher.ReplacePartialCommand(0x1b9c9, 0x4b, 0x1ba04, 0xe, "");


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t4000
