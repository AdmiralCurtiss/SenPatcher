#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0010[] =
    "Fix various text issues in Branch Campus Dorm.";
}

namespace SenLib::Sen3::FileFixes::t0010_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0010.dat",
            367201,
            HyoutaUtils::Hash::SHA1FromHexString("53812a2a94c94c7bc9068c4e43d8973eb32d5034"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // Master Yun -> Master Ka-fai
        patcher.ReplacePartialCommand(
            0x1be3a, 0x50, 0x1be84, 0x3, {{0x4b, 0x61, 0x2d, 0x66, 0x61, 0x69}});

        // Hah. Guess so... -> Hmph. True. (to match existing voice line)
        patcher.ReplacePartialCommand(
            0x39b4e, 0x1e, 0x39b5b, 0xe, {{0x6d, 0x70, 0x68, 0x2e, 0x20, 0x54, 0x72, 0x75, 0x65}});

        // "#E_2#M[0]#B_0#1U(I should invite Sydney to shooting\x01practice as soon as I'm able.)"
        // Sydney -> Sidney
        // patcher.ReplacePartialCommand(0x37936, 0x8c, 0x3797b, 0x45, "");
        // (chapter 2, 5/14 morning, Sidney's Sadness sidequest, must not have taken a bath)
        bin[0x37990] = 'i';

        // "#2POh, yeah, you're from some famous\x01hot springs town, ain't'cha?"
        // ain't'cha -> ain'tcha for consistency
        // (chapter 3, 6/11 morning, take a bath)
        patcher.RemovePartialCommand(0x4d1af, 0x81, 0x4d1f2, 1);

        // "#E_0#M_2#B_0So I figured she got it out of her system and,\x01there wasn't anything to
        // worry about..."
        // misplaced comma, move after 'system'. for reference, there's a second variant of this
        // line where that's already the case -- depends on whether you've seen Maya yell at Joseph
        // in Raquel, I assume
        // (chapter 4, 7/9 afternoon, talk to Randy during the Maya's Malaise sidequest)
        // patcher.ReplacePartialCommand(0x3aafd, 0x15e, 0x3abff, 0x5a, "");
        HyoutaUtils::Vector::ShiftData(bin, 0x3ac30, 0x3ac2c, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0010_dat
