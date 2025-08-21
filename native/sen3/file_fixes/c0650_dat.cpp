#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c0650[] = "Fix typos in Villa-Raisins (Crossbell)";
}

namespace SenLib::Sen3::FileFixes::c0650_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c0650.dat",
            13263,
            HyoutaUtils::Hash::SHA1FromHexString("ad9a510097c247fc7b33bb139a0cd5788cc1d83b"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "Hello, hello! I'm the Villa Raisins' caretaker,\x01Sammie."
        // the Villa Raisins -> Villa-Raisins
        // (chapter 2, 5/20 morning)
        bin[0x14D0] = '-';
        patcher.RemovePartialCommand(0x14ae, 0xd7, 0x14c7, 4);

        // "#E_6#M_2I know she'll be back, and as a fan,\x01all I can do it support her!"
        // do it -> do is
        // (chapter 2, 5/21 after Olivier joins)
        // patcher.ReplacePartialCommand(0xd42, 0xd5, 0xdcf, 0x46, "");
        bin[0xE06] = 's';

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c0650_dat
