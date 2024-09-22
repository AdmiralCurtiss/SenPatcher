#include "file_fixes_sw.h"

#include <array>
#include <cstring>
#include <filesystem>
#include <limits>
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
#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/logger.h"
#include "util/text.h"

#define DECLARE_STANDARD_FIXSW(name)                                            \
    namespace SenLib::TX::FileFixesSw::##name {                                 \
        std::string_view GetDescription();                                      \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }
DECLARE_STANDARD_FIXSW(system_dat)
DECLARE_STANDARD_FIXSW(t_dlc)
DECLARE_STANDARD_FIXSW(t_notehelp)
DECLARE_STANDARD_FIXSW(t_text)

#define TRY_APPLY_SW(asset, apply)                      \
    do {                                                \
        logger.Log("Applying: ");                       \
        logger.Log(#asset);                             \
        logger.Log("\n");                               \
        if (!(SenLib::TX::FileFixesSw::asset::apply)) { \
            logger.Log(" -- Failed!\n");                \
            return false;                               \
        }                                               \
    } while (false)

namespace {
static constexpr char Data_SenpatcherModIniEnglish[] = {
    (char)0x5B, (char)0x54, (char)0x58, (char)0x4D, (char)0x6F, (char)0x64, (char)0x5D,
    (char)0x0D, (char)0x0A, (char)0x4C, (char)0x61, (char)0x6E, (char)0x67, (char)0x75,
    (char)0x61, (char)0x67, (char)0x65, (char)0x3D, (char)0x45, (char)0x6E, (char)0x67,
    (char)0x6C, (char)0x69, (char)0x73, (char)0x68, (char)0x0D, (char)0x0A};
static constexpr size_t Length_SenpatcherModIniEnglish = sizeof(Data_SenpatcherModIniEnglish);

static std::string_view StripTrailingNull(std::string_view sv) {
    std::string_view s = sv;
    while (!s.empty() && s.back() == '\0') {
        s = s.substr(0, s.size() - 1);
    }
    return s;
}
} // namespace

namespace SenLib::TX {
static bool AddDifferentSwitchFilesRecursive(HyoutaUtils::Logger& logger,
                                             std::string_view dataDirSwitch,
                                             const SenPatcher::GetCheckedFileCallback& callback,
                                             std::vector<SenPatcher::P3APackFile>& packFiles,
                                             std::string_view relativePath) {
    std::error_code ec;
    std::string currentDir;
    currentDir.append(dataDirSwitch);
    currentDir.push_back('/');
    currentDir.append(relativePath);
    std::filesystem::directory_iterator iterator(
        std::u8string_view((const char8_t*)currentDir.data(),
                           ((const char8_t*)currentDir.data()) + currentDir.size()),
        ec);
    if (ec) {
        return false;
    }
    for (auto const& entry : iterator) {
        std::string relative;
        relative.append(relativePath);
        relative.push_back('/');
        const auto filenameAsPath = entry.path().filename().u8string();
        relative.append(
            std::string_view((const char*)filenameAsPath.data(),
                             ((const char*)filenameAsPath.data()) + filenameAsPath.size()));

        if (entry.is_directory()) {
            if (!AddDifferentSwitchFilesRecursive(
                    logger, dataDirSwitch, callback, packFiles, relative)) {
                return false;
            }
            continue;
        }

        std::array<char, 0x100> fn{};
        if (!SenPatcher::CopyToP3AFilename(fn, relative)) {
            return false;
        }
        fn = SenPatcher::NormalizeP3AFilename(fn);

        logger.Log("Checking Switch file: ").Log(relative).Log("\n");

        HyoutaUtils::IO::File file(entry.path(), HyoutaUtils::IO::OpenMode::Read);
        if (!file.IsOpen()) {
            return false;
        }
        auto filesize = file.GetLength();
        if (!filesize) {
            return false;
        }
        if (*filesize > std::numeric_limits<size_t>::max()) {
            return false;
        }
        std::vector<char> filedata;
        filedata.resize(static_cast<size_t>(*filesize));
        if (file.Read(filedata.data(), filedata.size()) != filedata.size()) {
            return false;
        }

        // check if this file exists in the PC files
        auto hash = HyoutaUtils::Hash::CalculateSHA1(filedata.data(), filedata.size());
        auto pcFiledata = callback(
            StripTrailingNull(std::string_view(fn.data(), fn.size())), filedata.size(), hash);
        if (!pcFiledata || pcFiledata->Data != filedata) {
            logger.Log("Not in PC files\n");
            packFiles.emplace_back(std::move(filedata), fn, SenPatcher::P3ACompressionType::LZ4);
            continue;
        }

        // is in PC files, don't need to pack it
    }
    return true;
}

static bool CollectAssetsSwitch(HyoutaUtils::Logger& logger,
                                std::string_view dataDirSwitch,
                                const SenPatcher::GetCheckedFileCallback& callback,
                                std::vector<SenPatcher::P3APackFile>& packFiles) {
    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniEnglish,
                          Data_SenpatcherModIniEnglish + Length_SenpatcherModIniEnglish),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);

    if (!AddDifferentSwitchFilesRecursive(logger, dataDirSwitch, callback, packFiles, "scripts")) {
        return false;
    }
    if (!AddDifferentSwitchFilesRecursive(logger, dataDirSwitch, callback, packFiles, "text")) {
        return false;
    }
    return true;
}

static bool ApplyFixesSwitch(HyoutaUtils::Logger& logger,
                             const SenPatcher::GetCheckedFileCallback& callback,
                             std::vector<SenPatcher::P3APackFile>& packFiles) {
    TRY_APPLY_SW(system_dat, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_dlc, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_notehelp, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_text, TryApply(callback, packFiles));
    return true;
}

SenPatcher::P3APackFile* FindAlreadyPackedFile(std::vector<SenPatcher::P3APackFile>& packFiles,
                                               std::string_view path,
                                               size_t size,
                                               const HyoutaUtils::Hash::SHA1& hash) {
    for (auto& pf : packFiles) {
        if (!pf.HasVectorData()) {
            continue;
        }
        auto& data = pf.GetVectorData();
        if (data.size() != size) {
            continue;
        }
        auto& pfname = pf.GetFilename();
        if (StripTrailingNull(std::string_view(pfname.data(), pfname.size())) != path) {
            continue;
        }
        if (HyoutaUtils::Hash::CalculateSHA1(data.data(), data.size()) != hash) {
            continue;
        }
        return &pf;
    }
    return nullptr;
}

bool CreateSwitchScriptArchive(HyoutaUtils::Logger& logger,
                               std::string_view dataDirSwitch,
                               std::string_view baseDirPc,
                               SenLib::ModLoad::LoadedP3AData& vanillaP3As) {
    // lazy init these so that they won't get inited if no one asks for a file
    bool allowJapaneseBra = false;
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
            fullPath.reserve(baseDirPc.size() + 1 + bra_filename.size());
            fullPath.append(baseDirPc);
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
        for (size_t i = 0; i < bra.FileCount; ++i) {
            const auto& fi = bra.FileInfo[i];
            const uint32_t compressedSize = fi.CompressedSize;
            const uint32_t uncompressedSize = fi.UncompressedSize;
            if (uncompressedSize == size
                && std::memcmp(tmp.data(), fi.Path.data(), tmp.size()) == 0) {
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
        if (allowJapaneseBra) {
            auto fJ = get_file_from_bra(get_bra(japanese_bra, "Japanese.bra"), path, size, hash);
            if (fJ) {
                logger.Log("Found ").Log(path).Log(" in Japanese.bra\n");
                return fJ;
            }
        }
        auto fA = get_file_from_bra(get_bra(audio_bra, "Audio.bra"), path, size, hash);
        if (fA) {
            logger.Log("Found ").Log(path).Log(" in Audio.bra\n");
            return fA;
        }
        auto fS = get_file_from_bra(get_bra(system_bra, "System.bra"), path, size, hash);
        if (fS) {
            logger.Log("Found ").Log(path).Log(" in System.bra\n");
            return fS;
        }
        auto f1 = get_file_from_bra(get_bra(asset1_bra, "Asset1.bra"), path, size, hash);
        if (f1) {
            logger.Log("Found ").Log(path).Log(" in Asset1.bra\n");
            return f1;
        }
        auto f2 = get_file_from_bra(get_bra(asset2_bra, "Asset2.bra"), path, size, hash);
        if (f2) {
            logger.Log("Found ").Log(path).Log(" in Asset2.bra\n");
            return f2;
        }
        auto f3 = get_file_from_bra(get_bra(asset3_bra, "Asset3.bra"), path, size, hash);
        if (f3) {
            logger.Log("Found ").Log(path).Log(" in Asset3.bra\n");
            return f3;
        }
        auto f4 = get_file_from_bra(get_bra(asset4_bra, "Asset4.bra"), path, size, hash);
        if (f4) {
            logger.Log("Found ").Log(path).Log(" in Asset4.bra\n");
            return f4;
        }
        return GetCheckedFile(baseDirPc, &vanillaP3As, path, size, hash);
    };

    std::string_view archivePath = "mods/tx_switch_script.p3a";

    std::string fullArchivePath;
    fullArchivePath.reserve(baseDirPc.size() + 1 + archivePath.size());
    fullArchivePath.append(baseDirPc);
    fullArchivePath.push_back('/');
    fullArchivePath.append(archivePath);

    logger.Log("Creating Switch script archive.\n");
    std::string tmpPath;
    tmpPath.reserve(fullArchivePath.size() + 4);
    tmpPath.append(fullArchivePath);
    tmpPath.append(".tmp");
    HyoutaUtils::IO::File newArchive(std::string_view(tmpPath), HyoutaUtils::IO::OpenMode::Write);
    if (!newArchive.IsOpen()) {
        logger.Log("Opening new archive failed.\n");
        return false;
    }

    SenPatcher::P3APackData packData;
    packData.SetVersion(SenPatcher::P3AHighestSupportedVersion);
    packData.SetAlignment(0x10);

    // files must be in the english-loaded archives to count as existing
    allowJapaneseBra = false;
    if (!CollectAssetsSwitch(logger, dataDirSwitch, callback, packData.GetMutableFiles())) {
        logger.Log("Collecting failed.\n");
        newArchive.Delete();
        return false;
    }

    // once we're applying fixes it's okay to load JP files
    allowJapaneseBra = true;
    if (!ApplyFixesSwitch(logger, callback, packData.GetMutableFiles())) {
        logger.Log("Applying fixes failed.\n");
        newArchive.Delete();
        return false;
    }

    auto& packFiles = packData.GetMutableFiles();
    for (auto& pf : packFiles) {
        auto normalized = SenPatcher::NormalizeP3AFilename(pf.GetFilename());
        pf.SetFilename(normalized);
    }
    std::stable_sort(packFiles.begin(),
                     packFiles.end(),
                     [](const SenPatcher::P3APackFile& lhs, const SenPatcher::P3APackFile& rhs) {
                         const auto& l = lhs.GetFilename();
                         const auto& r = rhs.GetFilename();
                         return memcmp(l.data(), r.data(), l.size()) < 0;
                     });
    if (!SenPatcher::PackP3A(newArchive, packData, 1)) {
        logger.Log("Packing failed.\n");
        newArchive.Delete();
        return false;
    }

    if (!newArchive.Rename(std::string_view(fullArchivePath))) {
        logger.Log("Renaming failed.\n");
        newArchive.Delete();
        return false;
    }

    logger.Log("Created archive!\n");
    return true;
}
} // namespace SenLib::TX
