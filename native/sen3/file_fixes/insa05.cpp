#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "util/bps.h"
#include "util/hash/sha1.h"
#include "util/stream.h"

extern "C" {
// we export a more general string in insa09
//__declspec(dllexport) char SenPatcherFix_9_insa05[] =
//    "Fix incorrect cut in Chapter 2 map travel sequence.";
}

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_insa_05.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::insa05 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, std::vector<char>& result) {
    try {
        auto file = getCheckedFile(
            "data/movie_us/webm/insa_05.webm",
            11333624,
            HyoutaUtils::Hash::SHA1FromHexString("6c33cf8b1cf93950d802f5ac9de1b213c0b45033"));
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
} // namespace SenLib::Sen3::FileFixes::insa05
