#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/memread.h"
#include "util/memwrite.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0031_dat {
std::string_view GetDescription() {
    return "Fix text errors in Stella Garten.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0031.dat",
            68772,
            SenPatcher::SHA1FromHexString("66a22b79517c7214b00b2a7a4ac898bc5f231fd8"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemRead;
        using namespace HyoutaUtils::MemWrite;

        SenScriptPatcher patcher(bin);

        // both double space and missing dot in this one...
        {
            auto b = ReadArray<0x3f>(&bin[0xb5be]);
            for (int i = 0x23; i >= 0x11; --i) {
                b[i] = b[i - 1];
            }
            WriteArray(&bin[0xb5be], b);
        }

        // double spaces in various lines
        patcher.RemovePartialCommand(0xeb28, 0x49, 0xeb39, 1);
        patcher.RemovePartialCommand(0xeef6, 0xc0, 0xef20, 1);

        // broken alisa line (missing space/linebreak)
        WriteUInt8(&bin[0x1f7a], 0x01);
        patcher.ExtendPartialCommand(0x1f5c, 0x3c, 0x1f83, {{0x20}});

        // missing word
        patcher.ExtendPartialCommand(0x1c93, 0x93, 0x1cac, {{0x69, 0x73, 0x20}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0031_dat
