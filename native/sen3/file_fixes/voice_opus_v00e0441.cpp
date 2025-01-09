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
//__declspec(dllexport) char SenPatcherFix_9_voice1[] =
//"Fix incorrect voice clip in chapter 2 cutscene ('Inspector').";
}

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_v00_e0441.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::voice_opus_v00e0441 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        result.emplace_back(SenLib::DecompressFromBuffer(PatchData, PatchLength),
                            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_e0441.opus"),
                            SenPatcher::P3ACompressionType::None);
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::voice_opus_v00e0441
