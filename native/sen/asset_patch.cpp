#include "asset_patch.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "file.h"
#include "logger.h"
#include "sha1.h"

#include "p3a/p3a.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sen/file_getter.h"

#include "senpatcher_version.h"

namespace SenLib {
static bool IsSenPatcherVersionFile(const SenPatcher::P3AFileInfo& f) {
    return strcmp(f.Filename.data(), "_senpatcher_version.txt") == 0;
}

static bool CheckArchiveExistsAndIsRightVersion(SenPatcher::Logger& logger, std::string_view path) {
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
    std::string_view baseDir,
    std::string_view archivePath,
    const std::function<bool(SenPatcher::P3APackData& packData)>& collectAssets) {
    std::string fullArchivePath;
    fullArchivePath.reserve(baseDir.size() + 1 + archivePath.size());
    fullArchivePath.append(baseDir);
    fullArchivePath.push_back('/');
    fullArchivePath.append(archivePath);

    // check if the archive already exists and is at the correct version,
    // and if yes don't recreate it
    if (!CheckArchiveExistsAndIsRightVersion(logger, fullArchivePath)) {
        logger.Log("Creating new asset archive.\n");

        std::string tmpPath;
        tmpPath.reserve(fullArchivePath.size() + 4);
        tmpPath.append(fullArchivePath);
        tmpPath.append(".tmp");
        SenPatcher::IO::File newArchive(std::string_view(tmpPath), SenPatcher::IO::OpenMode::Write);
        if (newArchive.IsOpen()) {
            SenPatcher::P3APackData packData;
            packData.Alignment = 0x10;
            AddSenPatcherVersionFile(packData);
            if (collectAssets(packData)) {
                std::stable_sort(
                    packData.Files.begin(),
                    packData.Files.end(),
                    [](const SenPatcher::P3APackFile& lhs, const SenPatcher::P3APackFile& rhs) {
                        const auto& l = lhs.GetFilename();
                        const auto& r = rhs.GetFilename();
                        return memcmp(l.data(), r.data(), l.size()) < 0;
                    });
                if (SenPatcher::PackP3A(newArchive, packData)) {
                    if (newArchive.Rename(std::string_view(fullArchivePath))) {
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

void CreateVideoIfNeeded(SenPatcher::Logger& logger,
                         std::string_view baseDir,
                         std::string_view videoPath,
                         const std::function<bool(std::vector<char>& videoData)>& getVideo) {
    std::string fullVideoPath;
    fullVideoPath.reserve(baseDir.size() + 1 + videoPath.size());
    fullVideoPath.append(baseDir);
    fullVideoPath.push_back('/');
    fullVideoPath.append(videoPath);

    logger.Log("Checking video file.\n");

    // Turns out redirecting video files to a memory stream is nontrivial, so we'll just put
    // modified videos directly into the file system.
    {
        SenPatcher::IO::File f(std::string_view(fullVideoPath), SenPatcher::IO::OpenMode::Read);
        if (f.IsOpen()) {
            logger.Log("Video file exists.\n");
            // assume that it's good if it exists...
            return;
        }
    }

    std::vector<char> data;
    if (!getVideo(data)) {
        logger.Log("Collecting failed.\n");
        return;
    }

    std::string tmpPath;
    tmpPath.reserve(fullVideoPath.size() + 4);
    tmpPath.append(fullVideoPath);
    tmpPath.append(".tmp");
    SenPatcher::IO::File f(std::string_view(tmpPath), SenPatcher::IO::OpenMode::Write);
    if (!f.IsOpen()) {
        logger.Log("Open failed.\n");
        return;
    }

    if (f.Write(data.data(), data.size()) != data.size()) {
        logger.Log("Write failed.\n");
        f.Delete();
        return;
    }

    if (!f.Rename(std::string_view(fullVideoPath))) {
        logger.Log("Rename failed.\n");
        f.Delete();
    }

    logger.Log("Created video file.\n");
}

std::optional<SenPatcher::CheckedFileResult> GetCheckedFile(std::string_view baseDir,
                                                            std::string_view path,
                                                            size_t size,
                                                            const SenPatcher::SHA1& hash) {
    SenPatcher::CheckedFileResult result{};
    if (!SenPatcher::CopyToP3AFilename(result.Filename, path)) {
        return std::nullopt;
    }

    std::string fullFilePath;
    fullFilePath.reserve(baseDir.size() + 1 + path.size());
    fullFilePath.append(baseDir);
    fullFilePath.push_back('/');
    fullFilePath.append(path);

    SenPatcher::IO::File inputfile(std::string_view(fullFilePath), SenPatcher::IO::OpenMode::Read);
    if (!inputfile.IsOpen()) {
        return std::nullopt;
    }
    const auto filesize = inputfile.GetLength();
    if (!filesize || size != *filesize) {
        // printf("filesize wrong -> %d == %s\n", (int)filesize.value_or(0), fullFilePath.data());
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
