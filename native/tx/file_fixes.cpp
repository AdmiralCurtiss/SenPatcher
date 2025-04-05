#include "file_fixes.h"

#include <array>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "modload/loaded_mods.h"
#include "p3a/pack.h"
#include "p3a/util.h"
#include "sen/asset_patch.h"
#include "sen/file_getter.h"
#include "tx/bra.h"
#include "tx/bra_extract.h"
#include "util/hash/sha1.h"
#include "util/logger.h"

#include "senpatcher_version.h"

static constexpr char Data_SenpatcherModIniEnglish[] =
    "[TXMod]\nMinFeatureLevel=0\nLanguage=English";
static constexpr size_t Length_SenpatcherModIniEnglish = sizeof(Data_SenpatcherModIniEnglish) - 1;
static constexpr char Data_SenpatcherModIniJapanese[] =
    "[TXMod]\nMinFeatureLevel=0\nLanguage=Japanese";
static constexpr size_t Length_SenpatcherModIniJapanese = sizeof(Data_SenpatcherModIniJapanese) - 1;

#define DECLARE_STANDARD_FIX(name)                                              \
    namespace SenLib::TX::FileFixes::##name {                                   \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }
DECLARE_STANDARD_FIX(a0000)
DECLARE_STANDARD_FIX(E_EFOBJ078)
DECLARE_STANDARD_FIX(E_EFOBJ079)
DECLARE_STANDARD_FIX(E_EFOBJ080)

#define TRY_APPLY(asset, apply)                       \
    do {                                              \
        logger.Log("Applying: ");                     \
        logger.Log(#asset);                           \
        if (!(SenLib::TX::FileFixes::asset::apply)) { \
            logger.Log(" -- Failed!\n");              \
            return false;                             \
        }                                             \
        logger.Log("\n");                             \
    } while (false)

namespace SenLib::TX {
static bool CollectAssetsEnglish(HyoutaUtils::Logger& logger,
                                 const SenPatcher::GetCheckedFileCallback& callback,
                                 std::vector<SenPatcher::P3APackFile>& packFiles) {
    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniEnglish,
                          Data_SenpatcherModIniEnglish + Length_SenpatcherModIniEnglish),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);

    TRY_APPLY(a0000, TryApply(callback, packFiles));
    return true;
}

static bool CollectAssetsJapanese(HyoutaUtils::Logger& logger,
                                  const SenPatcher::GetCheckedFileCallback& callback,
                                  std::vector<SenPatcher::P3APackFile>& packFiles) {
    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniJapanese,
                          Data_SenpatcherModIniJapanese + Length_SenpatcherModIniJapanese),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);

    TRY_APPLY(E_EFOBJ078, TryApply(callback, packFiles));
    TRY_APPLY(E_EFOBJ079, TryApply(callback, packFiles));
    TRY_APPLY(E_EFOBJ080, TryApply(callback, packFiles));
    return true;
}

bool CreateAssetPatchIfNeeded(HyoutaUtils::Logger& logger,
                              std::string_view baseDir,
                              SenLib::ModLoad::LoadedP3AData& vanillaP3As,
                              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                              std::span<const std::string_view> pkaPrefixes,
                              bool isJp) {
    std::string_view versionString(SENPATCHER_VERSION, sizeof(SENPATCHER_VERSION) - 1);

    // lazy init these so that they won't get inited if no one asks for a file
    std::optional<SenPatcher::BRA> asset1_bra;
    std::optional<SenPatcher::BRA> asset2_bra;
    std::optional<SenPatcher::BRA> asset3_bra;
    std::optional<SenPatcher::BRA> asset4_bra;
    std::optional<SenPatcher::BRA> audio_bra;
    std::optional<SenPatcher::BRA> japanese_bra;
    std::optional<SenPatcher::BRA> system_bra;
    const auto get_bra = [&](std::optional<SenPatcher::BRA>& bra,
                             std::string_view bra_filename) -> SenPatcher::BRA& {
        if (!bra) {
            std::string fullPath;
            fullPath.reserve(baseDir.size() + 1 + bra_filename.size());
            fullPath.append(baseDir);
            fullPath.push_back('/');
            fullPath.append(bra_filename);

            bra.emplace();
            bra->Load(fullPath);
            for (size_t i = 0; i < bra->FileCount; ++i) {
                SenLib::ModLoad::FilterP3APath(bra->FileInfo[i].Path.data(),
                                               bra->FileInfo[i].Path.size());
            }
        }
        return *bra;
    };
    const auto get_file_from_bra =
        [](SenPatcher::BRA& bra,
           std::string_view path,
           size_t size,
           const HyoutaUtils::Hash::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        if (path.size() > 0x60) {
            return std::nullopt;
        }

        std::array<char, 0x60> tmp{};
        static_assert(sizeof(tmp) == sizeof(SenPatcher::BRAFileInfo::Path));
        std::memcpy(tmp.data(), path.data(), path.size());
        SenLib::ModLoad::FilterP3APath(tmp.data(), tmp.size());
        for (size_t i = 0; i < bra.FileCount; ++i) {
            const auto& fi = bra.FileInfo[i];
            const uint32_t compressedSize = fi.CompressedSize;
            const uint32_t uncompressedSize = fi.UncompressedSize;
            if (uncompressedSize == size
                && std::strncmp(tmp.data(), fi.Path.data(), tmp.size()) == 0) {
                auto r = [&]() -> std::optional<SenPatcher::CheckedFileResult> {
                    SenPatcher::CheckedFileResult result;
                    if (!SenPatcher::CopyToP3AFilename(result.Filename, path)) {
                        return std::nullopt;
                    }
                    result.Data.resize(uncompressedSize);
                    auto compressedBuffer = std::make_unique_for_overwrite<char[]>(compressedSize);
                    if (!bra.FileHandle.SetPosition(fi.DataPosition)) {
                        return std::nullopt;
                    }
                    if (bra.FileHandle.Read(compressedBuffer.get(), compressedSize)
                        != compressedSize) {
                        return std::nullopt;
                    }
                    if (!SenPatcher::ExtractAndDecompressBraFile(result.Data.data(),
                                                                 uncompressedSize,
                                                                 compressedBuffer.get(),
                                                                 compressedSize)) {
                        return std::nullopt;
                    }
                    if (HyoutaUtils::Hash::CalculateSHA1(result.Data.data(), uncompressedSize)
                        != hash) {
                        return std::nullopt;
                    }
                    return result;
                }();
                if (r) {
                    return r;
                }
            }
        }
        return std::nullopt;
    };

    const SenPatcher::GetCheckedFileCallback callback =
        [&](std::string_view path,
            size_t size,
            const HyoutaUtils::Hash::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        auto fJ = get_file_from_bra(get_bra(japanese_bra, "Japanese.bra"), path, size, hash);
        if (fJ) {
            return fJ;
        }
        auto fA = get_file_from_bra(get_bra(audio_bra, "Audio.bra"), path, size, hash);
        if (fA) {
            return fA;
        }
        auto fS = get_file_from_bra(get_bra(system_bra, "System.bra"), path, size, hash);
        if (fS) {
            return fS;
        }
        auto f1 = get_file_from_bra(get_bra(asset1_bra, "Asset1.bra"), path, size, hash);
        if (f1) {
            return f1;
        }
        auto f2 = get_file_from_bra(get_bra(asset2_bra, "Asset2.bra"), path, size, hash);
        if (f2) {
            return f2;
        }
        auto f3 = get_file_from_bra(get_bra(asset3_bra, "Asset3.bra"), path, size, hash);
        if (f3) {
            return f3;
        }
        auto f4 = get_file_from_bra(get_bra(asset4_bra, "Asset4.bra"), path, size, hash);
        if (f4) {
            return f4;
        }
        return GetCheckedFile(
            baseDir, vanillaP3As, vanillaPKAs, pkaPrefixes, nullptr, path, size, hash);
    };

    bool success = true;
    if (isJp) {
        success = CreateArchiveIfNeeded(logger,
                                        baseDir,
                                        "mods/zzz_senpatcher_txjapanese.p3a",
                                        versionString,
                                        [&](SenPatcher::P3APackData& packData) -> bool {
                                            return CollectAssetsJapanese(
                                                logger, callback, packData.GetMutableFiles());
                                        })
                  && success;
    } else {
        success = CreateArchiveIfNeeded(logger,
                                        baseDir,
                                        "mods/zzz_senpatcher_txenglish.p3a",
                                        versionString,
                                        [&](SenPatcher::P3APackData& packData) -> bool {
                                            return CollectAssetsEnglish(
                                                logger, callback, packData.GetMutableFiles());
                                        })
                  && success;
    }
    return success;
}
} // namespace SenLib::TX
