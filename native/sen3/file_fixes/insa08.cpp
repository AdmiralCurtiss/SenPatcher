#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "util/bps.h"
#include "util/hash/sha1.h"
#include "util/stream.h"

extern "C" {
// we export a more general string in insa09
//__declspec(dllexport) char SenPatcherFix_9_insa08[] =
//    "Fix incorrect cut in Chapter 3 map travel sequence.";
}

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_insa_08.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::insa08 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, std::vector<char>& result) {
    try {
        auto file = getCheckedFile(
            "data/movie_us/webm/insa_08.webm",
            9103875,
            HyoutaUtils::Hash::SHA1FromHexString("97b37cb0d324014d8db6b2965f1836effdb1ce01"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        HyoutaUtils::Stream::DuplicatableByteArrayStream source(bin.data(), bin.size());
        HyoutaUtils::Stream::DuplicatableByteArrayStream patch(PatchData, PatchLength);
        std::vector<char> target;
        HyoutaUtils::Bps::ApplyPatchToStream(source, patch, target);

        result = std::move(target);
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::insa08
