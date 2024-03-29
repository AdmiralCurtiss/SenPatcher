#include "asset_patch.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "file.h"
#include "file_getter.h"
#include "logger.h"
#include "sha1.h"

#include "p3a/p3a.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"

#include "senpatcher_version.h"

namespace SenLib {
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

void CreateArchiveIfNeeded(
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

void CreateVideoIfNeeded(
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

std::optional<SenPatcher::CheckedFileResult> GetCheckedFile(const std::filesystem::path& baseDir,
                                                            std::string_view path,
                                                            size_t size,
                                                            const SenPatcher::SHA1& hash) {
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
}
} // namespace SenLib
