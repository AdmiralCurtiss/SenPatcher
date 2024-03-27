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
static constexpr char PatchData_ed8m2150[] = {
#include "embed_sen1_ed8m2150.h"
};
static constexpr size_t PatchLength_ed8m2150 = sizeof(PatchData_ed8m2150);
static constexpr char PatchData_ed8m4097[] = {
#include "embed_sen1_ed8m4097.h"
};
static constexpr size_t PatchLength_ed8m4097 = sizeof(PatchData_ed8m4097);
static constexpr char PatchData_ed8m4217[] = {
#include "embed_sen1_ed8m4217.h"
};
static constexpr size_t PatchLength_ed8m4217 = sizeof(PatchData_ed8m4217);
static constexpr char PatchData_pc8v02551_4[] = {
#include "embed_sen1_pc8v02551_4.h"
};
static constexpr size_t PatchLength_pc8v02551_4 = sizeof(PatchData_pc8v02551_4);
static constexpr char PatchData_pc8v10286_15[] = {
#include "embed_sen1_pc8v10286_15.h"
};
static constexpr size_t PatchLength_pc8v10286_15 = sizeof(PatchData_pc8v10286_15);
} // namespace

namespace SenLib::Sen1::FileFixes::missing_audio_files {
std::string_view GetDescription() {
    return "Fix missing sound effects and voice clips";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        // These three sound effects are completely missing in the PC files, for some reason, though
        // the game still references them in script files:

        // Courageous engine noise
        result.emplace_back(SenLib::DecompressFromBuffer(PatchData_ed8m2150, PatchLength_ed8m2150),
                            SenPatcher::InitializeP3AFilename("data/se/wav/ed8m2150.wav"),
                            SenPatcher::P3ACompressionType::None);

        // Bleublanc teleport effect
        result.emplace_back(SenLib::DecompressFromBuffer(PatchData_ed8m4097, PatchLength_ed8m4097),
                            SenPatcher::InitializeP3AFilename("data/se/wav/ed8m4097.wav"),
                            SenPatcher::P3ACompressionType::None);

        // Olivier lute sound
        result.emplace_back(SenLib::DecompressFromBuffer(PatchData_ed8m4217, PatchLength_ed8m4217),
                            SenPatcher::InitializeP3AFilename("data/se/wav/ed8m4217.wav"),
                            SenPatcher::P3ACompressionType::None);

        // And then we have extra two voice clips:

        // Alisa voice clip in Chapter 3, in t1000.dat;
        // This line contains a continuity error that was fixed in PS4 but not backported to PC.
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_pc8v02551_4, PatchLength_pc8v02551_4),
            SenPatcher::InitializeP3AFilename("data/voice/wav/pc8v02551.wav"),
            SenPatcher::P3ACompressionType::None);

        // Sara voice clip in Finale, in t0020.dat;
        // This one got broken in PC patch 1.6, which added a few extra voice clips to previously
        // unvoiced scenes. pc8v10286.wav got replaced with a new voice clip (presumably by
        // accident) but the old reference to the voice clip remained, leading to a very confusing
        // out-of-place line read in the Finale. This reinjects that file at a different filename.
        // See the text_dat_us_t_voice_tbl patch for the update in the voice table so the relevant
        // ID points at this file instead of the replaced one.
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_pc8v10286_15, PatchLength_pc8v10286_15),
            SenPatcher::InitializeP3AFilename("data/voice/wav/pc8v10299.wav"),
            SenPatcher::P3ACompressionType::None);
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::missing_audio_files
