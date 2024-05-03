#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

namespace SenLib::Sen4::FileFixes::f4200_dat {
std::string_view GetDescription() {
    return "Fix astrology/astronomy mixup.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/f4200.dat",
            329609,
            HyoutaUtils::Hash::SHA1FromHexString("d1cd26a05828553bb7bf03e370717226f28353a0"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        using namespace HyoutaUtils::Vector;

        // astrology -> astronomy in cocktail with Towa
        WriteAt(bin, 0x39b68, {{0x6e, 0x6f, 0x6d}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen4::FileFixes::f4200_dat
