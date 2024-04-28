#include <string_view>
#include <vector>

#include "util/bps.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/stream.h"

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_insa_08.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::insa08 {
std::string_view GetDescription() {
    return "Fix incorrect cut in Chapter 3 map travel sequence.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, std::vector<char>& result) {
    try {
        auto file = getCheckedFile(
            "data/movie_us/webm/insa_08.webm",
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
} // namespace SenLib::Sen3::FileFixes::insa08
