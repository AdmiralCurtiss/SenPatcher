#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "bps.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sha1.h"
#include "util/stream.h"

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_insa_05.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::insa05 {
std::string_view GetDescription() {
    return "Fix incorrect cut in Chapter 2 map travel sequence.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/movie_us/webm/insa_05.webm",
            11333624,
            SenPatcher::SHA1FromHexString("6c33cf8b1cf93950d802f5ac9de1b213c0b45033"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenLib::DuplicatableByteArrayStream source(bin.data(), bin.size());
        SenLib::DuplicatableByteArrayStream patch(PatchData, PatchLength);
        std::vector<char> target;
        HyoutaUtils::Bps::ApplyPatchToStream(source, patch, target);

        // FIXME: update output filename here

        result.emplace_back(
            std::move(target), file->Filename, SenPatcher::P3ACompressionType::None);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::insa05
