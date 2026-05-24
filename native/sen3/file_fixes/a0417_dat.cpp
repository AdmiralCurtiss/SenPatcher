#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_a0417[] =
    "Fix minor text errors in Chapter 1 Ex. Camp (early copy)";
}

namespace SenLib::Sen3::FileFixes::a0417_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    auto file = getCheckedFile(
        "data/scripts/scena/dat_en/a0417.dat",
        112657,
        HyoutaUtils::Hash::SHA1FromHexString("2b0b345b51b705b71ddc5f7989836dc3139f62f5"));
    if (!file) {
        return false;
    }

    auto& bin = file->Data;
    SenScriptPatcher patcher(bin);

    // PS4 1.03 change, but this is actually inconsistent with the rest of the script
    // so leave it as-is
    // Imperial government -> Imperial Government
    // bin[0xfe3b] = 0x47;

    // "#E_0#M_0#B_0...Guessing it was some some\x01'supporting gauntlet'-type stuff?"
    // 'some some' -> 'some'
    patcher.RemovePartialCommand(0x13e2f, 0x61, 0x13e67, 5);

    result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

    return true;
}
} // namespace SenLib::Sen3::FileFixes::a0417_dat
