#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "decompress_helper.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sha1.h"

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
std::string_view GetDescription() {
    return "Fix incorrect voice clips for Rean's Ogre S-Crafts.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        result.emplace_back(SenLib::DecompressFromBuffer(PatchData27, PatchLength27),
                            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_s0027.opus"),
                            SenPatcher::P3ACompressionType::None);
        result.emplace_back(SenLib::DecompressFromBuffer(PatchData28, PatchLength28),
                            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_s0028.opus"),
                            SenPatcher::P3ACompressionType::None);
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::voice_opus_v00s2728
