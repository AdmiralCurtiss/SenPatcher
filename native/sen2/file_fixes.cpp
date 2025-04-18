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

static constexpr char Data_SenpatcherModIniDefault[] = "[CS2Mod]\nMinFeatureLevel=0";
static constexpr size_t Length_SenpatcherModIniDefault = sizeof(Data_SenpatcherModIniDefault) - 1;

#define DECLARE_STANDARD_FIX(name)                                              \
    namespace SenLib::Sen2::FileFixes::##name {                                 \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }

DECLARE_STANDARD_FIX(scripts_book_dat_us_book00_dat)
DECLARE_STANDARD_FIX(scripts_book_dat_us_book03_dat)
DECLARE_STANDARD_FIX(scripts_book_dat_us_book04_dat)
DECLARE_STANDARD_FIX(scripts_book_dat_us_book05_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_e7050_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_e7060_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_e7090_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_e7101_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_e7110_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_r0920_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_r1010_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_system_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0001_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t0010_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t1010_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t3060_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t3500_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t3740_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t4000_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t4080_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t5501_dat)
DECLARE_STANDARD_FIX(scripts_scena_dat_us_t6500_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_beryl_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_emily_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_heinrich_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_monica_dat)
DECLARE_STANDARD_FIX(scripts_talk_dat_us_tk_rosine_dat)
DECLARE_STANDARD_FIX(text_dat_us_t_item_tbl_t_magic_tbl)
DECLARE_STANDARD_FIX(text_dat_us_t_notecook_tbl)
DECLARE_STANDARD_FIX(text_dat_us_t_voice_tbl)

#define TRY_APPLY(asset, apply)                         \
    do {                                                \
        logger.Log("Applying: ");                       \
        logger.Log(#asset);                             \
        if (!(SenLib::Sen2::FileFixes::asset::apply)) { \
            logger.Log(" -- Failed!\n");                \
            return false;                               \
        }                                               \
        logger.Log("\n");                               \
    } while (false)

namespace SenLib::Sen2 {
static bool CollectAssets(HyoutaUtils::Logger& logger,
                          const SenPatcher::GetCheckedFileCallback& callback,
                          std::vector<SenPatcher::P3APackFile>& packFiles) {
    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniDefault,
                          Data_SenpatcherModIniDefault + Length_SenpatcherModIniDefault),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);

    TRY_APPLY(scripts_book_dat_us_book00_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_book_dat_us_book03_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_book_dat_us_book04_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_book_dat_us_book05_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_e7050_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_e7060_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_e7090_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_e7101_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_e7110_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_r0920_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_r1010_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_system_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0001_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t0010_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t1010_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t3060_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t3500_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t3740_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t4000_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t4080_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t5501_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_scena_dat_us_t6500_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_beryl_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_emily_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_heinrich_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_monica_dat, TryApply(callback, packFiles));
    TRY_APPLY(scripts_talk_dat_us_tk_rosine_dat, TryApply(callback, packFiles));
    TRY_APPLY(text_dat_us_t_item_tbl_t_magic_tbl, TryApply(callback, packFiles));
    TRY_APPLY(text_dat_us_t_notecook_tbl, TryApply(callback, packFiles));
    TRY_APPLY(text_dat_us_t_voice_tbl, TryApply(callback, packFiles));
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

    return CreateArchiveIfNeeded(logger,
                                 baseDir,
                                 "mods/zzz_senpatcher_cs2asset.p3a",
                                 versionString,
                                 [&](SenPatcher::P3APackData& packData) -> bool {
                                     return CollectAssets(
                                         logger, callback, packData.GetMutableFiles());
                                 });
}
} // namespace SenLib::Sen2
