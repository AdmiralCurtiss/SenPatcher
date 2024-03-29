#include "file_fixes.h"

#include <algorithm>
#include <cstring>
#include <optional>
#include <string_view>
#include <vector>

#include "file.h"
#include "file_getter.h"
#include "logger.h"
#include "p3a/p3a.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sha1.h"

#include "senpatcher_version.h"

#define DECLARE_STANDARD_FIX(name)                                              \
    namespace SenLib::Sen3::FileFixes::##name {                                 \
        std::string_view GetDescription();                                      \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }

DECLARE_STANDARD_FIX(alchr022_dat)
DECLARE_STANDARD_FIX(a0417_dat)
DECLARE_STANDARD_FIX(book05_dat)
DECLARE_STANDARD_FIX(book06_dat)
DECLARE_STANDARD_FIX(book07_dat)
DECLARE_STANDARD_FIX(c0000_dat)
DECLARE_STANDARD_FIX(c0200_dat)
DECLARE_STANDARD_FIX(c0250_dat)
DECLARE_STANDARD_FIX(c0400_dat)
DECLARE_STANDARD_FIX(c0420_dat)
DECLARE_STANDARD_FIX(c0430_dat)
DECLARE_STANDARD_FIX(c0820_dat)
DECLARE_STANDARD_FIX(c0830_dat)
DECLARE_STANDARD_FIX(c2430_dat)
DECLARE_STANDARD_FIX(c2440_dat)
DECLARE_STANDARD_FIX(c2610_dat)
DECLARE_STANDARD_FIX(c3000_dat)
DECLARE_STANDARD_FIX(c3010_dat)
DECLARE_STANDARD_FIX(c3210_dat)
DECLARE_STANDARD_FIX(c3610_dat)
DECLARE_STANDARD_FIX(f0010_dat)
DECLARE_STANDARD_FIX(f2000_dat)
DECLARE_STANDARD_FIX(I_CVIS0061_pkg)
DECLARE_STANDARD_FIX(I_CVIS1008_pkg)
DECLARE_STANDARD_FIX(I_JMP009_pkg)
DECLARE_STANDARD_FIX(insa05)
DECLARE_STANDARD_FIX(insa08)
DECLARE_STANDARD_FIX(insa09)
DECLARE_STANDARD_FIX(m0000_dat)
DECLARE_STANDARD_FIX(m0100_dat)
DECLARE_STANDARD_FIX(m0300_dat)
DECLARE_STANDARD_FIX(m0600_dat)
DECLARE_STANDARD_FIX(m3000_dat)
DECLARE_STANDARD_FIX(m3420_dat)
DECLARE_STANDARD_FIX(m3430_dat)
DECLARE_STANDARD_FIX(m4004_dat)
DECLARE_STANDARD_FIX(r0210_dat)
DECLARE_STANDARD_FIX(r2290_dat)
DECLARE_STANDARD_FIX(r3000_dat)
DECLARE_STANDARD_FIX(r3090_dat)
DECLARE_STANDARD_FIX(r3200_dat)
DECLARE_STANDARD_FIX(r3430_dat)
DECLARE_STANDARD_FIX(r4200_dat)
DECLARE_STANDARD_FIX(r4290_dat)
DECLARE_STANDARD_FIX(t_item)
DECLARE_STANDARD_FIX(t_jump)
DECLARE_STANDARD_FIX(t_magic)
DECLARE_STANDARD_FIX(t_mons)
DECLARE_STANDARD_FIX(t_mstqrt)
DECLARE_STANDARD_FIX(t_name)
DECLARE_STANDARD_FIX(t_notecook)
DECLARE_STANDARD_FIX(t_place)
DECLARE_STANDARD_FIX(t_vctiming_us)
DECLARE_STANDARD_FIX(t0000_dat)
DECLARE_STANDARD_FIX(t0010_dat)
DECLARE_STANDARD_FIX(t0080_dat)
DECLARE_STANDARD_FIX(t0100_dat)
DECLARE_STANDARD_FIX(t0200_dat)
DECLARE_STANDARD_FIX(t0210_dat)
DECLARE_STANDARD_FIX(t0250_dat)
DECLARE_STANDARD_FIX(t0260_dat)
DECLARE_STANDARD_FIX(t0400_dat)
DECLARE_STANDARD_FIX(t0410_dat)
DECLARE_STANDARD_FIX(t3000_dat)
DECLARE_STANDARD_FIX(t3200_dat)
DECLARE_STANDARD_FIX(t3220_dat)
DECLARE_STANDARD_FIX(t3400_dat)
DECLARE_STANDARD_FIX(t3510_dat)
DECLARE_STANDARD_FIX(t3600_dat)
DECLARE_STANDARD_FIX(tk_ada_dat)
DECLARE_STANDARD_FIX(tk_linde_dat)
DECLARE_STANDARD_FIX(tk_patrick_dat)
DECLARE_STANDARD_FIX(tk_stark_dat)
DECLARE_STANDARD_FIX(tk_tovar_dat)
DECLARE_STANDARD_FIX(tk_zessica_dat)
DECLARE_STANDARD_FIX(v0010_dat)
DECLARE_STANDARD_FIX(v0030_dat)
DECLARE_STANDARD_FIX(v0050_dat)
DECLARE_STANDARD_FIX(voice_opus_ps4_103)
DECLARE_STANDARD_FIX(voice_opus_v00e0441)
DECLARE_STANDARD_FIX(voice_opus_v00s2728)

namespace SenLib::Sen3::FileFixes::t_text {
std::string_view GetDescription();
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              bool allowSwitchToNightmare);
} // namespace SenLib::Sen3::FileFixes::t_text

#define TRY_APPLY(asset, apply)                         \
    do {                                                \
        logger.Log("Applying: ");                       \
        logger.Log(#asset);                             \
        if (!(SenLib::Sen3::FileFixes::asset::apply)) { \
            logger.Log(" -- Failed!\n");                \
            return false;                               \
        }                                               \
        logger.Log("\n");                               \
    } while (false)

namespace SenLib::Sen3 {
static bool CollectAssets(SenPatcher::Logger& logger,
                          const SenPatcher::GetCheckedFileCallback& callback,
                          SenPatcher::P3APackData& packData,
                          bool allowSwitchToNightmare) {
    TRY_APPLY(a0417_dat, TryApply(callback, packData.Files));
    TRY_APPLY(alchr022_dat, TryApply(callback, packData.Files));
    TRY_APPLY(book05_dat, TryApply(callback, packData.Files));
    TRY_APPLY(book06_dat, TryApply(callback, packData.Files));
    TRY_APPLY(book07_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c0000_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c0200_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c0250_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c0400_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c0420_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c0430_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c0820_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c0830_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c2430_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c2440_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c2610_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c3000_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c3010_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c3210_dat, TryApply(callback, packData.Files));
    TRY_APPLY(c3610_dat, TryApply(callback, packData.Files));
    TRY_APPLY(f0010_dat, TryApply(callback, packData.Files));
    TRY_APPLY(f2000_dat, TryApply(callback, packData.Files));
    TRY_APPLY(I_CVIS0061_pkg, TryApply(callback, packData.Files));
    TRY_APPLY(I_CVIS1008_pkg, TryApply(callback, packData.Files));
    TRY_APPLY(I_JMP009_pkg, TryApply(callback, packData.Files));
    TRY_APPLY(m0000_dat, TryApply(callback, packData.Files));
    TRY_APPLY(m0100_dat, TryApply(callback, packData.Files));
    TRY_APPLY(m0300_dat, TryApply(callback, packData.Files));
    TRY_APPLY(m0600_dat, TryApply(callback, packData.Files));
    TRY_APPLY(m3000_dat, TryApply(callback, packData.Files));
    TRY_APPLY(m3420_dat, TryApply(callback, packData.Files));
    TRY_APPLY(m3430_dat, TryApply(callback, packData.Files));
    TRY_APPLY(m4004_dat, TryApply(callback, packData.Files));
    TRY_APPLY(r0210_dat, TryApply(callback, packData.Files));
    TRY_APPLY(r2290_dat, TryApply(callback, packData.Files));
    TRY_APPLY(r3000_dat, TryApply(callback, packData.Files));
    TRY_APPLY(r3090_dat, TryApply(callback, packData.Files));
    TRY_APPLY(r3200_dat, TryApply(callback, packData.Files));
    TRY_APPLY(r3430_dat, TryApply(callback, packData.Files));
    TRY_APPLY(r4200_dat, TryApply(callback, packData.Files));
    TRY_APPLY(r4290_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t_item, TryApply(callback, packData.Files));
    TRY_APPLY(t_jump, TryApply(callback, packData.Files));
    TRY_APPLY(t_magic, TryApply(callback, packData.Files));
    TRY_APPLY(t_mons, TryApply(callback, packData.Files));
    TRY_APPLY(t_mstqrt, TryApply(callback, packData.Files));
    TRY_APPLY(t_name, TryApply(callback, packData.Files));
    TRY_APPLY(t_notecook, TryApply(callback, packData.Files));
    TRY_APPLY(t_place, TryApply(callback, packData.Files));
    TRY_APPLY(t_text, TryApply(callback, packData.Files, allowSwitchToNightmare));
    TRY_APPLY(t_vctiming_us, TryApply(callback, packData.Files));
    TRY_APPLY(t0000_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t0010_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t0080_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t0100_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t0200_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t0210_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t0250_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t0260_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t0400_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t0410_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t3000_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t3200_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t3220_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t3400_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t3510_dat, TryApply(callback, packData.Files));
    TRY_APPLY(t3600_dat, TryApply(callback, packData.Files));
    TRY_APPLY(tk_ada_dat, TryApply(callback, packData.Files));
    TRY_APPLY(tk_linde_dat, TryApply(callback, packData.Files));
    TRY_APPLY(tk_patrick_dat, TryApply(callback, packData.Files));
    TRY_APPLY(tk_stark_dat, TryApply(callback, packData.Files));
    TRY_APPLY(tk_tovar_dat, TryApply(callback, packData.Files));
    TRY_APPLY(tk_zessica_dat, TryApply(callback, packData.Files));
    TRY_APPLY(v0010_dat, TryApply(callback, packData.Files));
    TRY_APPLY(v0030_dat, TryApply(callback, packData.Files));
    TRY_APPLY(v0050_dat, TryApply(callback, packData.Files));
    return true;
}

static bool CollectAudio(SenPatcher::Logger& logger,
                         const SenPatcher::GetCheckedFileCallback& callback,
                         SenPatcher::P3APackData& packData) {
    TRY_APPLY(voice_opus_ps4_103, TryApply(callback, packData.Files));
    TRY_APPLY(voice_opus_v00e0441, TryApply(callback, packData.Files));
    TRY_APPLY(voice_opus_v00s2728, TryApply(callback, packData.Files));
    return true;
}

static bool IsSenPatcherVersionFile(const SenPatcher::P3AFileInfo& f) {
    return strcmp((const char*)f.Filename.data(), "_senpatcher_version.txt") == 0;
}

static bool CheckArchiveExistsAndIsRightVersion(SenPatcher::Logger& logger,
                                                const std::filesystem::path& path) {
    logger.Log("Checking for existing asset archive.\n");

    SenPatcher::P3A p3a;
    if (p3a.Load(path)) {
        for (size_t i = 0; i < p3a.FileCount; ++i) {
            const auto& f = p3a.FileInfo[i];
            if (IsSenPatcherVersionFile(f)) {
                std::array<char, sizeof(SENPATCHER_VERSION) - 1> versionInFile;
                if (f.CompressionType == SenPatcher::P3ACompressionType::None
                    && f.CompressedSize == versionInFile.size()
                    && p3a.FileHandle.SetPosition(f.Offset)
                    && p3a.FileHandle.Read(versionInFile.data(), versionInFile.size())
                           == versionInFile.size()) {
                    if (std::string_view(SENPATCHER_VERSION)
                        == std::string_view(versionInFile.begin(), versionInFile.end())) {
                        logger.Log("Found archive with correct version.\n");
                        return true;
                    }
                }
                logger.Log("Found archive, but wrong version.\n");
                return false;
            }
        }
    }
    return false;
}

static void AddSenPatcherVersionFile(SenPatcher::P3APackData& packData) {
    std::string_view sv(SENPATCHER_VERSION);
    std::vector<char> bin(sv.begin(), sv.end());
    packData.Files.emplace_back(std::move(bin),
                                SenPatcher::InitializeP3AFilename("_senpatcher_version.txt"),
                                SenPatcher::P3ACompressionType::None);
}

static void CreateArchiveIfNeeded(
    SenPatcher::Logger& logger,
    const std::filesystem::path& archivePath,
    const std::function<bool(SenPatcher::P3APackData& packData)>& collectAssets) {
    // check if the archive already exists and is at the correct version,
    // and if yes don't recreate it
    if (!CheckArchiveExistsAndIsRightVersion(logger, archivePath)) {
        logger.Log("Creating new asset archive.\n");

        auto tmpPath = archivePath;
        tmpPath += L".tmp";
        SenPatcher::IO::File newArchive(tmpPath, SenPatcher::IO::OpenMode::Write);
        if (newArchive.IsOpen()) {
            SenPatcher::P3APackData packData;
            packData.Alignment = 0x10;
            AddSenPatcherVersionFile(packData);
            if (collectAssets(packData)) {
                std::stable_sort(
                    packData.Files.begin(),
                    packData.Files.end(),
                    [](const SenPatcher::P3APackFile& lhs, const SenPatcher::P3APackFile& rhs) {
                        return memcmp(lhs.Filename.data(), rhs.Filename.data(), lhs.Filename.size())
                               < 0;
                    });
                if (SenPatcher::PackP3A(newArchive, packData)) {
                    if (newArchive.Rename(archivePath)) {
                        logger.Log("Created archive!\n");
                    } else {
                        logger.Log("Renaming failed.\n");
                        newArchive.Delete();
                    }
                } else {
                    logger.Log("Packing failed.\n");
                    newArchive.Delete();
                }
            } else {
                logger.Log("Collecting failed.\n");
                newArchive.Delete();
            }
        }
    }
}

static void CreateVideoIfNeeded(
    SenPatcher::Logger& logger,
    const std::filesystem::path& videoPath,
    const std::function<bool(SenPatcher::P3APackData& packData)>& collectAssets) {
    logger.Log("Checking video file.\n");

    // Turns out redirecting video files to a memory stream is nontrivial, so we'll just put
    // modified videos directly into the file system.
    {
        SenPatcher::IO::File f(videoPath, SenPatcher::IO::OpenMode::Read);
        if (f.IsOpen()) {
            logger.Log("Video file exists.\n");
            // assume that it's good if it exists...
            return;
        }
    }

    SenPatcher::P3APackData packData;
    if (!collectAssets(packData)) {
        logger.Log("Collecting failed.\n");
        return;
    }

    // This should give us exactly one file.
    if (packData.Files.size() != 1) {
        logger.Log("Collecting failed (wrong count).\n");
        return;
    }
    if (!std::holds_alternative<std::vector<char>>(packData.Files[0].Data)) {
        logger.Log("Collecting failed (wrong type).\n");
        return;
    }
    const auto& data = std::get<std::vector<char>>(packData.Files[0].Data);

    auto tmpPath = videoPath;
    tmpPath += L".tmp";
    SenPatcher::IO::File f(tmpPath, SenPatcher::IO::OpenMode::Write);
    if (!f.IsOpen()) {
        logger.Log("Open failed.\n");
        return;
    }

    if (f.Write(data.data(), data.size()) != data.size()) {
        logger.Log("Write failed.\n");
        f.Delete();
        return;
    }

    if (!f.Rename(videoPath)) {
        logger.Log("Rename failed.\n");
        f.Delete();
    }

    logger.Log("Created video file.\n");
}

void CreateAssetPatchIfNeeded(SenPatcher::Logger& logger, const std::filesystem::path& baseDir) {
    // TODO: handle this flag somehow?
    bool allowSwitchToNightmare = true;


    const SenPatcher::GetCheckedFileCallback callback =
        [&](std::string_view path,
            size_t size,
            const SenPatcher::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        SenPatcher::CheckedFileResult result{};
        if (!SenPatcher::CopyToP3AFilename(result.Filename, path)) {
            return std::nullopt;
        }

        SenPatcher::IO::File inputfile(
            baseDir
                / std::u8string_view((const char8_t*)path.data(),
                                     (const char8_t*)(path.data() + path.size())),
            SenPatcher::IO::OpenMode::Read);
        if (!inputfile.IsOpen()) {
            return std::nullopt;
        }
        const auto filesize = inputfile.GetLength();
        if (!filesize || size != *filesize) {
            // printf("filesize wrong -> %d == %s\n", (int)filesize.value_or(0), path.data());
            return std::nullopt;
        }
        std::vector<char>& vec = result.Data;
        vec.resize(size);
        if (inputfile.Read(vec.data(), size) != size) {
            return std::nullopt;
        }
        inputfile.Close();

        if (SenPatcher::CalculateSHA1(vec.data(), size) != hash) {
            return std::nullopt;
        }

        return result;
    };

    CreateArchiveIfNeeded(logger,
                          baseDir / L"mods/zzz_senpatcher_cs3asset.p3a",
                          [&](SenPatcher::P3APackData& packData) -> bool {
                              return CollectAssets(
                                  logger, callback, packData, allowSwitchToNightmare);
                          });
    CreateArchiveIfNeeded(logger,
                          baseDir / L"mods/zzz_senpatcher_cs3audio.p3a",
                          [&](SenPatcher::P3APackData& packData) -> bool {
                              return CollectAudio(logger, callback, packData);
                          });
    CreateVideoIfNeeded(logger,
                        baseDir / L"data/movie_us/webm/insa_f5.webm",
                        [&](SenPatcher::P3APackData& packData) -> bool {
                            return SenLib::Sen3::FileFixes::insa05::TryApply(callback,
                                                                             packData.Files);
                        });
    CreateVideoIfNeeded(logger,
                        baseDir / L"data/movie_us/webm/insa_f8.webm",
                        [&](SenPatcher::P3APackData& packData) -> bool {
                            return SenLib::Sen3::FileFixes::insa08::TryApply(callback,
                                                                             packData.Files);
                        });
    CreateVideoIfNeeded(logger,
                        baseDir / L"data/movie_us/webm/insa_f9.webm",
                        [&](SenPatcher::P3APackData& packData) -> bool {
                            return SenLib::Sen3::FileFixes::insa09::TryApply(callback,
                                                                             packData.Files);
                        });
}
} // namespace SenLib::Sen3
