#include "file_fixes.h"

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sen/asset_patch.h"
#include "sen/file_getter.h"
#include "util/logger.h"

#include "senpatcher_version.h"

static constexpr char Data_SenpatcherModIniDefault[] = "[CS1Mod]\nMinFeatureLevel=0";
static constexpr size_t Length_SenpatcherModIniDefault = sizeof(Data_SenpatcherModIniDefault) - 1;

#define DECLARE_STANDARD_FIX(name)                                              \
    namespace SenLib::Sen1::FileFixes::##name {                                 \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }

DECLARE_STANDARD_FIX(missing_audio_files)
DECLARE_STANDARD_FIX(scripts_book_dat_us_book00_dat)
DECLARE_STANDARD_FIX(scripts_book_dat_us_book01_dat)
DECLARE_STANDARD_FIX(scripts_book_dat_us_book02_dat)
DECLARE_STANDARD_FIX(scripts_book_dat_us_book03_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_a0006_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_c0100_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_c0110_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_m0040_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_m2130_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_m3008_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_r0600_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_r0601_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_r0610_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_r0800_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0000_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0000c_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0010_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0020_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0031_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0032_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0050_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0060_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0070_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0080_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0090_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t1000_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t1010_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t1020_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t1030_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t1040_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t1050_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t1110_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t1500_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t3500_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t4610_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t5660_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_angelica_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_becky_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_beryl_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_edel_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_heinrich_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_laura_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_thomas_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_vandyck_dat)
DECLARE_STANDARD_FIX(se_wav_ed8m2123_wav)
DECLARE_STANDARD_FIX(text_dat_t_item_tbl)
DECLARE_STANDARD_FIX(text_dat_t_voice_tbl)
DECLARE_STANDARD_FIX(text_dat_us_t_item_tbl_t_magic_tbl)
DECLARE_STANDARD_FIX(text_dat_us_t_notecook_tbl)
DECLARE_STANDARD_FIX(text_dat_us_t_voice_tbl)

#define TRY_APPLY(asset, apply)                         \
    do {                                                \
        logger.Log("Applying: ");                       \
        logger.Log(#asset);                             \
        if (!(SenLib::Sen1::FileFixes::asset::apply)) { \
            logger.Log(" -- Failed!\n");                \
            return false;                               \
        }                                               \
        logger.Log("\n");                               \
    } while (false)

namespace SenLib::Sen1 {
static bool CollectAssets(HyoutaUtils::Logger& logger,
                          const SenPatcher::GetCheckedFileCallback& callback,
                          std::vector<SenPatcher::P3APackFile>& packFiles) {
    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniDefault,
                          Data_SenpatcherModIniDefault + Length_SenpatcherModIniDefault),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);

    TRY_APPLY(scripts_book_dat_us_book00_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_book_dat_us_book01_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_book_dat_us_book02_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_book_dat_us_book03_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_a0006_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_c0100_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_c0110_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_m0040_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_m2130_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_m3008_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_r0600_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_r0601_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_r0610_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_r0800_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0000_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0000c_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0010_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0020_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0031_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0032_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0050_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0060_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0070_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0080_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0090_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t1000_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t1010_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t1020_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t1030_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t1040_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t1050_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t1110_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t1500_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t3500_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t4610_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t5660_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_angelica_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_becky_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_beryl_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_edel_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_heinrich_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_laura_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_thomas_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_vandyck_dat, TryApply(callback, packFiles));
    TRY_APPLY(text_dat_t_item_tbl, TryApply(callback, packFiles));
    TRY_APPLY(text_dat_t_voice_tbl, TryApply(callback, packFiles));
    TRY_APPLY(text_dat_us_t_item_tbl_t_magic_tbl, TryApply(callback, packFiles));
    TRY_APPLY(text_dat_us_t_notecook_tbl, TryApply(callback, packFiles));
    TRY_APPLY(text_dat_us_t_voice_tbl, TryApply(callback, packFiles));
    return true;
}

static bool CollectAudio(HyoutaUtils::Logger& logger,
                         const SenPatcher::GetCheckedFileCallback& callback,
                         std::vector<SenPatcher::P3APackFile>& packFiles) {
    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniDefault,
                          Data_SenpatcherModIniDefault + Length_SenpatcherModIniDefault),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);

    TRY_APPLY(missing_audio_files, TryApply(callback, packFiles));
    TRY_APPLY(se_wav_ed8m2123_wav, TryApply(callback, packFiles));
    return true;
}

bool CreateAssetPatchIfNeeded(HyoutaUtils::Logger& logger,
                              std::string_view baseDir,
                              SenLib::ModLoad::LoadedP3AData& vanillaP3As,
                              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                              std::span<const std::string_view> pkaPrefixes) {
    const SenPatcher::GetCheckedFileCallback callback =
        [&](std::string_view path,
            size_t size,
            const HyoutaUtils::Hash::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        return GetCheckedFile(
            baseDir, vanillaP3As, vanillaPKAs, pkaPrefixes, nullptr, path, size, hash);
    };

    std::string_view versionString(SENPATCHER_VERSION, sizeof(SENPATCHER_VERSION) - 1);

    bool success = true;
    success = CreateArchiveIfNeeded(logger,
                                    baseDir,
                                    "mods/zzz_senpatcher_cs1asset.p3a",
                                    versionString,
                                    [&](SenPatcher::P3APackData& packData) -> bool {
                                        return CollectAssets(
                                            logger, callback, packData.GetMutableFiles());
                                    })
              && success;
    success =
        CreateArchiveIfNeeded(logger,
                              baseDir,
                              "mods/zzz_senpatcher_cs1audio.p3a",
                              versionString,
                              [&](SenPatcher::P3APackData& packData) -> bool {
                                  return CollectAudio(logger, callback, packData.GetMutableFiles());
                              })
        && success;
    return success;
}
} // namespace SenLib::Sen1
