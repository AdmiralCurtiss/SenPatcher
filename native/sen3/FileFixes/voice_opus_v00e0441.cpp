#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../decompress_helper.h"
#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../p3a/util.h"
#include "../../sha1.h"

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_v00_e0441.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::voice_opus_v00e0441 {
std::string_view GetDescription() {
    return "Fix incorrect voice clip in chapter 2 cutscene ('Inspector').";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        result.emplace_back(SenPatcher::P3APackFile{
            SenLib::DecompressFromBuffer(PatchData, PatchLength),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_e0441.opus"),
            SenPatcher::P3ACompressionType::None});
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::voice_opus_v00e0441
