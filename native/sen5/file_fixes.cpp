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

static constexpr char Data_SenpatcherModIniDefault[] = "[ReverieMod]\nMinFeatureLevel=0";
static constexpr size_t Length_SenpatcherModIniDefault = sizeof(Data_SenpatcherModIniDefault) - 1;

#define DECLARE_STANDARD_FIX(name)                                              \
    namespace SenLib::Sen5::FileFixes::##name {                                 \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }

DECLARE_STANDARD_FIX(t_item_magic)
DECLARE_STANDARD_FIX(t_notecook)
DECLARE_STANDARD_FIX(t_text)

#define TRY_APPLY(asset, apply)                         \
    do {                                                \
        logger.Log("Applying: ");                       \
        logger.Log(#asset);                             \
        if (!(SenLib::Sen5::FileFixes::asset::apply)) { \
            logger.Log(" -- Failed!\n");                \
            return false;                               \
        }                                               \
        logger.Log("\n");                               \
    } while (false)

namespace SenLib::Sen5 {
static bool CollectAssets(HyoutaUtils::Logger& logger,
                          const SenPatcher::GetCheckedFileCallback& callback,
                          std::vector<SenPatcher::P3APackFile>& packFiles) {
    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniDefault,
                          Data_SenpatcherModIniDefault + Length_SenpatcherModIniDefault),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);

    TRY_APPLY(t_item_magic, TryApply(callback, packFiles));
    TRY_APPLY(t_notecook, TryApply(callback, packFiles));
    TRY_APPLY(t_text, TryApply(callback, packFiles));
    return true;
}

bool CreateAssetPatchIfNeeded(HyoutaUtils::Logger& logger,
                              std::string_view baseDir,
                              SenLib::ModLoad::LoadedP3AData& vanillaP3As,
                              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                              std::span<const std::string_view> pkaPrefixes,
                              SenLib::ModLoad::LoadedPkaGroupData& pkgsOfPrefix0File0) {
    std::string_view versionString(SENPATCHER_VERSION, sizeof(SENPATCHER_VERSION) - 1);

    const SenPatcher::GetCheckedFileCallback callback =
        [&](std::string_view path,
            size_t size,
            const HyoutaUtils::Hash::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        return GetCheckedFile(
            baseDir, vanillaP3As, vanillaPKAs, pkaPrefixes, &pkgsOfPrefix0File0, path, size, hash);
    };

    return CreateArchiveIfNeeded(logger,
                                 baseDir,
                                 "mods/zzz_senpatcher_revasset.p3a",
                                 versionString,
                                 [&](SenPatcher::P3APackData& packData) -> bool {
                                     return CollectAssets(
                                         logger, callback, packData.GetMutableFiles());
                                 });
}
} // namespace SenLib::Sen5
