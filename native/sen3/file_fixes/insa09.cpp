#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "util/bps.h"
#include "util/hash/sha1.h"
#include "util/stream.h"

extern "C" {
//__declspec(dllexport) char SenPatcherFix_9_insa09[] =
//    "Fix duplicate Chapter 3 map travel sequence.";
__declspec(dllexport) char SenPatcherFix_9_insa[] = "Fix errors in train travel sequences.";
}

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_insa_09.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::insa09 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, std::vector<char>& result) {
    try {
        auto file = getCheckedFile(
            "data/movie_us/webm/insa_09.webm",
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
} // namespace SenLib::Sen3::FileFixes::insa09
