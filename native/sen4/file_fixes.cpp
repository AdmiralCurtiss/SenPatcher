#include "file_fixes.h"

#include <string_view>
#include <vector>

#include "util/logger.h"
#include "p3a/pack.h"
#include "sen/asset_patch.h"
#include "sen/file_getter.h"

#define DECLARE_STANDARD_FIX(name)                                              \
    namespace SenLib::Sen4::FileFixes::##name {                                 \
        std::string_view GetDescription();                                      \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }

DECLARE_STANDARD_FIX(f4200_dat)
DECLARE_STANDARD_FIX(m9031_dat)
DECLARE_STANDARD_FIX(t_item)
DECLARE_STANDARD_FIX(t_mstqrt)
DECLARE_STANDARD_FIX(t3600_dat)

namespace SenLib::Sen4::FileFixes::t_text {
std::string_view GetDescription();
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
    TRY_APPLY(f4200_dat, TryApply(callback, packFiles));
    TRY_APPLY(m9031_dat, TryApply(callback, packFiles));
    TRY_APPLY(t_item, TryApply(callback, packFiles));
    TRY_APPLY(t_mstqrt, TryApply(callback, packFiles));
    TRY_APPLY(t_text, TryApply(callback, packFiles, allowSwitchToNightmare));
    TRY_APPLY(t3600_dat, TryApply(callback, packFiles));
    return true;
}

bool CreateAssetPatchIfNeeded(HyoutaUtils::Logger& logger, std::string_view baseDir) {
    // TODO: handle this flag somehow?
    bool allowSwitchToNightmare = true;


    const SenPatcher::GetCheckedFileCallback callback =
        [&](std::string_view path,
            size_t size,
            const HyoutaUtils::Hash::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        return GetCheckedFile(baseDir, path, size, hash);
    };

    return CreateArchiveIfNeeded(logger,
                                 baseDir,
                                 "mods/zzz_senpatcher_cs4asset.p3a",
                                 [&](SenPatcher::P3APackData& packData) -> bool {
                                     return CollectAssets(logger,
                                                          callback,
                                                          packData.GetMutableFiles(),
                                                          allowSwitchToNightmare);
                                 });
}
} // namespace SenLib::Sen4
