#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sen/decompress_helper.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
// we don't need to display this separately, the ps4 103 fix suffices
//__declspec(dllexport) char SenPatcherFix_9_voice2[] =
// "Fix incorrect voice clips for Rean's Ogre S-Crafts.";
}

namespace {
static constexpr char PatchData27[] = {
#include "embed_sen3_v00_s0027.h"
};
static constexpr size_t PatchLength27 = sizeof(PatchData27);
static constexpr char PatchData28[] = {
#include "embed_sen3_v00_s0028.h"
};
static constexpr size_t PatchLength28 = sizeof(PatchData28);
} // namespace

namespace SenLib::Sen3::FileFixes::voice_opus_v00s2728 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto d27 = SenLib::DecompressFromBuffer(PatchData27, PatchLength27);
        if (!d27) {
            return false;
        }
        auto d28 = SenLib::DecompressFromBuffer(PatchData28, PatchLength28);
        if (!d28) {
            return false;
        }
        result.emplace_back(std::move(*d27),
                            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_s0027.opus"),
                            SenPatcher::P3ACompressionType::None);
        result.emplace_back(std::move(*d28),
                            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_s0028.opus"),
                            SenPatcher::P3ACompressionType::None);
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::voice_opus_v00s2728
