#include "file_fixes.h"

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "sen/asset_patch.h"
#include "sen/file_getter.h"
#include "util/logger.h"

#define DECLARE_STANDARD_FIX(name)                                              \
    namespace SenLib::Sen5::FileFixes::##name {                                 \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }

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
    TRY_APPLY(t_text, TryApply(callback, packFiles));
    return true;
}

bool CreateAssetPatchIfNeeded(HyoutaUtils::Logger& logger,
                              std::string_view baseDir,
                              SenLib::ModLoad::LoadedP3AData& vanillaP3As) {
    const SenPatcher::GetCheckedFileCallback callback =
        [&](std::string_view path,
            size_t size,
            const HyoutaUtils::Hash::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        return GetCheckedFile(baseDir, &vanillaP3As, path, size, hash);
    };

    return CreateArchiveIfNeeded(logger,
                                 baseDir,
                                 "mods/zzz_senpatcher_revasset.p3a",
                                 [&](SenPatcher::P3APackData& packData) -> bool {
                                     return CollectAssets(
                                         logger, callback, packData.GetMutableFiles());
                                 });
}
} // namespace SenLib::Sen5
