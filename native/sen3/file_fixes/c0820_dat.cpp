#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen3::FileFixes::c0820_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Crossbell (IBC).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c0820.dat",
            38025,
            HyoutaUtils::Hash::SHA1FromHexString("a5da1241a9c17b2fbb2fc9c93dc691d51471ec8a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // St. Ursula's hospital -> St. Ursula Medical College
        patcher.ReplacePartialCommand(0x85d6,
                                      0xaa,
                                      0x8606,
                                      0xb,
                                      {{0x20,
                                        0x4d,
                                        0x65,
                                        0x64,
                                        0x69,
                                        0x63,
                                        0x61,
                                        0x6c,
                                        0x20,
                                        0x43,
                                        0x6f,
                                        0x6c,
                                        0x6c,
                                        0x65,
                                        0x67,
                                        0x65}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c0820_dat
