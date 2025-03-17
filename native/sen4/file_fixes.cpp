#include "file_fixes.h"

#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sen/asset_patch.h"
#include "sen/file_getter.h"
#include "util/logger.h"

#include "senpatcher_version.h"

static constexpr char Data_SenpatcherModIniDefault[] = "[CS4Mod]\nMinFeatureLevel=0";
static constexpr size_t Length_SenpatcherModIniDefault = sizeof(Data_SenpatcherModIniDefault) - 1;

#define DECLARE_STANDARD_FIX(name)                                              \
    namespace SenLib::Sen4::FileFixes::##name {                                 \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }

DECLARE_STANDARD_FIX(f4200_dat)
DECLARE_STANDARD_FIX(m9031_dat)
DECLARE_STANDARD_FIX(t_item)
DECLARE_STANDARD_FIX(t_magic)
DECLARE_STANDARD_FIX(t_mstqrt)
DECLARE_STANDARD_FIX(t3600_dat)

namespace SenLib::Sen4::FileFixes::t_text {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              bool allowSwitchToNightmare);
} // namespace SenLib::Sen4::FileFixes::t_text

#define TRY_APPLY(asset, apply)                         \
    do {                                                \
        logger.Log("Applying: ");                       \
        logger.Log(#asset);                             \
        if (!(SenLib::Sen4::FileFixes::asset::apply)) { \
            logger.Log(" -- Failed!\n");                \
            return false;                               \
        }                                               \
        logger.Log("\n");                               \
    } while (false)

namespace SenLib::Sen4 {
static bool CollectAssets(HyoutaUtils::Logger& logger,
                          const SenPatcher::GetCheckedFileCallback& callback,
                          std::vector<SenPatcher::P3APackFile>& packFiles,
                          bool allowSwitchToNightmare) {
    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniDefault,
                          Data_SenpatcherModIniDefault + Length_SenpatcherModIniDefault),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);

    TRY_APPLY(f4200_dat, TryApply(callback, packFiles));
    TRY_APPLY(m9031_dat, TryApply(callback, packFiles));
    TRY_APPLY(t_item, TryApply(callback, packFiles));
    TRY_APPLY(t_magic, TryApply(callback, packFiles));
    TRY_APPLY(t_mstqrt, TryApply(callback, packFiles));
    TRY_APPLY(t_text, TryApply(callback, packFiles, allowSwitchToNightmare));
    TRY_APPLY(t3600_dat, TryApply(callback, packFiles));
    return true;
}

bool CreateAssetPatchIfNeeded(HyoutaUtils::Logger& logger,
                              std::string_view baseDir,
                              SenLib::ModLoad::LoadedP3AData& vanillaP3As,
                              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                              std::span<const std::string_view> pkaPrefixes,
                              SenLib::ModLoad::LoadedPkaGroupData& pkgsOfPrefix0File0,
                              bool allowSwitchToNightmare) {
    std::string_view versionString(SENPATCHER_VERSION, sizeof(SENPATCHER_VERSION) - 1);
    std::string versionStringWithSettings =
        std::format("{}:{}", versionString, allowSwitchToNightmare ? 1 : 0);

    const SenPatcher::GetCheckedFileCallback callback =
        [&](std::string_view path,
            size_t size,
            const HyoutaUtils::Hash::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        return GetCheckedFile(
            baseDir, vanillaP3As, vanillaPKAs, pkaPrefixes, &pkgsOfPrefix0File0, path, size, hash);
    };

    return CreateArchiveIfNeeded(logger,
                                 baseDir,
                                 "mods/zzz_senpatcher_cs4asset.p3a",
                                 versionStringWithSettings,
                                 [&](SenPatcher::P3APackData& packData) -> bool {
                                     return CollectAssets(logger,
                                                          callback,
                                                          packData.GetMutableFiles(),
                                                          allowSwitchToNightmare);
                                 });
}
} // namespace SenLib::Sen4
