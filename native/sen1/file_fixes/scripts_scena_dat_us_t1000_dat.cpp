#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/memwrite.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1000[] =
    "Fix continuity error in chapter 3 and a few formatting issues in the main Trista area.";
}

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1000_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t1000.dat",
            271771,
            HyoutaUtils::Hash::SHA1FromHexString("84d3de50b7318f20b4fe48836404d134a124be52"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        SenScriptPatcher patcher(bin);

        // fix line from Alisa about the upcoming exams
        // this has been fixed in the PS4 version but for some reason was not patched back into the
        // PC version
        patcher.ReplacePartialCommand(
            0x25b3e, 0x38, 0x25b59, 0x14, {{0x66, 0x65, 0x65, 0x6c, 0x3f, 0x20, 0x41, 0x6c, 0x6c,
                                            0x01, 0x70, 0x72, 0x65, 0x70, 0x70, 0x65, 0x64, 0x20,
                                            0x61, 0x6e, 0x64, 0x20, 0x72, 0x65, 0x61, 0x64, 0x79,
                                            0x20, 0x66, 0x6f, 0x72, 0x20, 0x6f, 0x75, 0x72}});

        // two lines later, linebreak was moved to a nicer spot, might as well apply that too
        std::swap(bin[0x25bee], bin[0x25bfb]);

        // formatting issues in Machias Chapter 1 Day bonding event
        std::swap(bin[0x39da5], bin[0x39dac]);
        WriteUInt8(&bin[0x39f87], 0x01);

        // formatting issues in Fie Chapter 3 Day bonding event
        WriteUInt8(&bin[0x3914a], 0x01);
        std::swap(bin[0x391b3], bin[0x391b8]);
        std::swap(bin[0x39202], bin[0x39208]);
        WriteUInt8(&bin[0x39227], 0x01);
        std::swap(bin[0x39430], bin[0x39436]);
        std::swap(bin[0x3945b], bin[0x3945f]);

        // formatting issues in Emma Chapter 5 Day bonding event
        std::swap(bin[0x3aa47], bin[0x3aa4c]);
        std::swap(bin[0x3ab64], bin[0x3ab6c]);
        std::swap(bin[0x3ac71], bin[0x3ac74]);

        // formatting issues in Millium Chapter 6 Evening bonding event
        std::swap(bin[0x3c1fb], bin[0x3c201]);
        std::swap(bin[0x3c4db], bin[0x3c4e4]);
        std::swap(bin[0x3c500], bin[0x3c510]);
        std::swap(bin[0x3c558], bin[0x3c55c]);
        std::swap(bin[0x3c581], bin[0x3c585]);
        std::swap(bin[0x3c5d0], bin[0x3c5d5]);
        std::swap(bin[0x3c5fa], bin[0x3c5ff]);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1000_dat
