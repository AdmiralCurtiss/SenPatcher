#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../bps.h"
#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sha1.h"
#include "../../util/stream.h"

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_insa_09.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::insa09 {
std::string_view GetDescription() {
    return "Fix duplicate Chapter 3 map travel sequence.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/movie_us/webm/insa_09.webm",
            9103875,
            SenPatcher::SHA1FromHexString("97b37cb0d324014d8db6b2965f1836effdb1ce01"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenLib::DuplicatableByteArrayStream source(bin.data(), bin.size());
        SenLib::DuplicatableByteArrayStream patch(PatchData, PatchLength);
        std::vector<char> target;
        HyoutaUtils::Bps::ApplyPatchToStream(source, patch, target);

        // FIXME: update output filename here

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(target), file->Filename, SenPatcher::P3ACompressionType::None});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::insa09
