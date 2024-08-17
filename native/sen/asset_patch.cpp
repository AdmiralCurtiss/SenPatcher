#include "asset_patch.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/logger.h"

#include "modload/loaded_mods.h"
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

static bool CheckArchiveExistsAndIsRightVersion(HyoutaUtils::Logger& logger,
                                                std::string_view path) {
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
    packData.GetMutableFiles().emplace_back(
        std::move(bin),
        SenPatcher::InitializeP3AFilename("_senpatcher_version.txt"),
        SenPatcher::P3ACompressionType::None);
}

bool CreateArchiveIfNeeded(
    HyoutaUtils::Logger& logger,
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
    if (CheckArchiveExistsAndIsRightVersion(logger, fullArchivePath)) {
        return true;
    }

    logger.Log("Creating new asset archive.\n");
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
    AddSenPatcherVersionFile(packData);
    if (!collectAssets(packData)) {
        logger.Log("Collecting failed.\n");
        newArchive.Delete();
        return false;
    }

    auto& packFiles = packData.GetMutableFiles();
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

bool CreateVideoIfNeeded(HyoutaUtils::Logger& logger,
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
        HyoutaUtils::IO::File f(std::string_view(fullVideoPath), HyoutaUtils::IO::OpenMode::Read);
        if (f.IsOpen()) {
            logger.Log("Video file exists.\n");
            // assume that it's good if it exists...
            return true;
        }
    }

    std::vector<char> data;
    if (!getVideo(data)) {
        logger.Log("Collecting failed.\n");
        return false;
    }

    std::string tmpPath;
    tmpPath.reserve(fullVideoPath.size() + 4);
    tmpPath.append(fullVideoPath);
    tmpPath.append(".tmp");
    HyoutaUtils::IO::File f(std::string_view(tmpPath), HyoutaUtils::IO::OpenMode::Write);
    if (!f.IsOpen()) {
        logger.Log("Open failed.\n");
        return false;
    }

    if (f.Write(data.data(), data.size()) != data.size()) {
        logger.Log("Write failed.\n");
        f.Delete();
        return false;
    }

    if (!f.Rename(std::string_view(fullVideoPath))) {
        logger.Log("Rename failed.\n");
        f.Delete();
        return false;
    }

    logger.Log("Created video file.\n");
    return true;
}

std::optional<SenPatcher::CheckedFileResult>
    GetCheckedFile(std::string_view baseDir,
                   SenLib::ModLoad::LoadedP3AData* vanillaP3As,
                   std::string_view path,
                   size_t size,
                   const HyoutaUtils::Hash::SHA1& hash) {
    SenPatcher::CheckedFileResult result{};
    if (!SenPatcher::CopyToP3AFilename(result.Filename, path)) {
        return std::nullopt;
    }

    std::vector<char>& vec = result.Data;
    vec.resize(size);

    std::string fullFilePath;
    fullFilePath.reserve(baseDir.size() + 1 + path.size());
    fullFilePath.append(baseDir);
    fullFilePath.push_back('/');
    fullFilePath.append(path);

    if (vanillaP3As) {
        const auto* ref = SenLib::ModLoad::FindP3AFileRef(*vanillaP3As, result.Filename);
        void* out_memory = nullptr;
        uint64_t out_filesize = 0;
        if (ref && ref->FileInfo->UncompressedSize == size
            && SenLib::ModLoad::ExtractP3AFileToMemory(
                *ref,
                0x8000'0000,
                out_memory,
                out_filesize,
                [](size_t length) { return malloc(length); },
                [](void* memory) { free(memory); })) {
            if (size != out_filesize
                || HyoutaUtils::Hash::CalculateSHA1(out_memory, size) != hash) {
                free(out_memory);
                return std::nullopt;
            }

            std::memcpy(vec.data(), out_memory, size);
            free(out_memory);
            return result;
        }
    }

    HyoutaUtils::IO::File inputfile(std::string_view(fullFilePath),
                                    HyoutaUtils::IO::OpenMode::Read);
    if (!inputfile.IsOpen()) {
        return std::nullopt;
    }
    const auto filesize = inputfile.GetLength();
    if (!filesize || size != *filesize) {
        // printf("filesize wrong -> %d == %s\n", (int)filesize.value_or(0), fullFilePath.data());
        return std::nullopt;
    }
    if (inputfile.Read(vec.data(), size) != size) {
        return std::nullopt;
    }
    inputfile.Close();

    if (HyoutaUtils::Hash::CalculateSHA1(vec.data(), size) != hash) {
        return std::nullopt;
    }

    return result;
}
} // namespace SenLib
