#include <string_view>
#include <vector>

#include "bps.h"
#include "sen/file_getter.h"
#include "sha1.h"
#include "util/stream.h"

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

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, std::vector<char>& result) {
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

        result = std::move(target);
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::insa09
