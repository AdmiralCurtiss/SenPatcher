#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0200[] =
    "Fix terminology consistency issues in Branch Campus grounds.";
}

namespace SenLib::Sen3::FileFixes::t0200_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0200.dat",
            541089,
            HyoutaUtils::Hash::SHA1FromHexString("77098ac08dc593d1b4eb1f2be2b5fe5665de36e4"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);
        using namespace HyoutaUtils::Vector;

        // Your Majesty's -> Your Highness'
        WriteAt(bin, 0x2506e, {{0x48, 0x69, 0x67, 0x68, 0x6e, 0x65, 0x73, 0x73, 0x27}});

        // "#E_8#M_0#B_0Looked like you were really going out of your\x01way to help all us
        // students. You're just a\x01stand-up guy, ain't'cha?"
        // ain't'cha -> ain'tcha for consistency
        // (EV_01_03_01; chapter 1, 4/15, scene while leaving school)
        patcher.RemovePartialCommand(0x11bf8, 0x13d, 0x11cb2, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0200_dat
