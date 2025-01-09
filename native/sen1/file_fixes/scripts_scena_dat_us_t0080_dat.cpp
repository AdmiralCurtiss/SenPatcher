#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/memwrite.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0080[] =
    "Fix formatting issues in Student Union building.";
}

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0080_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0080.dat",
            279576,
            HyoutaUtils::Hash::SHA1FromHexString("d5805f2f25de668a4ececc8f6cad0aaae64a3cf8"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        // double space
        SenScriptPatcher patcher(bin);
        patcher.RemovePartialCommand(0x1f278, 0x88, 0x1f2df, 1);

        // formatting issues in Towa Chapter 1 Evening bonding event
        std::swap(bin[0x31889], bin[0x3188f]);
        std::swap(bin[0x31ac0], bin[0x31ac2]);
        std::swap(bin[0x31b09], bin[0x31b0e]);
        std::swap(bin[0x31b2d], bin[0x31b35]);
        std::swap(bin[0x3205c], bin[0x32066]);
        std::swap(bin[0x320a5], bin[0x320ab]);
        std::swap(bin[0x32175], bin[0x32179]);

        // formatting issues in Machias Chapter 3 Day bonding event
        std::swap(bin[0x2d214], bin[0x2d21d]);
        std::swap(bin[0x2d32f], bin[0x2d332]);

        // formatting issues in Jusis Chapter 4 Day bonding event
        std::swap(bin[0x2c35f], bin[0x2c362]);
        std::swap(bin[0x2c467], bin[0x2c46a]);
        std::swap(bin[0x2c4dd], bin[0x2c4e2]);

        // formatting issues in Emma Chapter 4 Day bonding event
        WriteUInt8(&bin[0x303f1], 0x01);
        std::swap(bin[0x3042b], bin[0x3042f]);
        std::swap(bin[0x305c0], bin[0x305c4]);
        std::swap(bin[0x30b71], bin[0x30b75]);
        std::swap(bin[0x30b96], bin[0x30b9f]);
        std::swap(bin[0x30df1], bin[0x30dfc]);
        WriteUInt8(&bin[0x30e17], 0x01);
        std::swap(bin[0x30f4e], bin[0x30f53]);

        // formatting issues in Machias Chapter 5 Evening bonding event
        std::swap(bin[0x2e4ca], bin[0x2e4cd]);
        std::swap(bin[0x2efa4], bin[0x2efa8]);
        std::swap(bin[0x2fd55], bin[0x2fd61]);

        // formatting issues in Emma/Fie midterms event
        std::swap(bin[0x33b77], bin[0x33b7c]);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0080_dat
