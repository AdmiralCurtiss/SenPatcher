#include "unpackfs.h"

#include <cassert>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <string_view>

#include "lz4/lz4.h"

#include "zstd/common/xxhash.h"
#include "zstd/zstd.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "p3a/structs.h"

#include "util/file.h"
#include "util/result.h"
#include "util/scope.h"
#include "util/text.h"

namespace SenPatcher {
static std::string_view StripTrailingNull(std::string_view sv) {
    std::string_view s = sv;
    while (!s.empty() && s.back() == '\0') {
        s = s.substr(0, s.size() - 1);
    }
    return s;
}

namespace {
struct ZSTD_DDict_Deleter {
    void operator()(ZSTD_DDict* ptr) {
        if (ptr) {
            ZSTD_freeDDict(ptr);
        }
    }
};
using ZSTD_DDict_UniquePtr = std::unique_ptr<ZSTD_DDict, ZSTD_DDict_Deleter>;

struct ZSTD_DCtx_Deleter {
    void operator()(ZSTD_DCtx* ptr) {
        if (ptr) {
            ZSTD_freeDCtx(ptr);
        }
    }
};
using ZSTD_DCtx_UniquePtr = std::unique_ptr<ZSTD_DCtx, ZSTD_DCtx_Deleter>;
} // namespace

HyoutaUtils::Result<UnpackP3AResult, std::string> UnpackP3A(std::string_view archivePath,
                                                            std::string_view extractPath,
                                                            std::string_view pathFilter,
                                                            bool generateJson,
                                                            bool noDecompression) {
    rapidjson::StringBuffer jsonbuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> json(jsonbuffer);
    json.StartObject();

    HyoutaUtils::IO::File f(archivePath, HyoutaUtils::IO::OpenMode::Read);
    if (!f.IsOpen()) {
        return std::format("Could not open P3A archive at '{}'.", archivePath);
    }

    std::filesystem::path extractFsPath = HyoutaUtils::IO::FilesystemPathFromUtf8(extractPath);
    std::error_code ec;
    std::filesystem::create_directories(extractFsPath, ec);
    if (ec) {
        return std::format("Could not create output directory at '{}'.", extractPath);
    }

    P3AHeader header;
    if (f.Read(&header, sizeof(P3AHeader)) != sizeof(P3AHeader)) {
        return std::format("Could not read P3A header of archive at '{}'.", archivePath);
    }
    const auto headerHash = XXH64(&header, sizeof(P3AHeader) - 8, 0);
    if (headerHash != header.Hash) {
        return std::format("P3A header hash error in archive at '{}'.", archivePath);
    }

    P3AExtendedHeader extHeader{};
    bool hasUncompressedHash = false;
    if (header.Version >= 1200) {
        // if this changes this code needs to be updated
        static_assert(P3AExtendedHeaderSize1200 == sizeof(P3AExtendedHeader));

        if (f.Read(&extHeader, P3AExtendedHeaderSize1200) != P3AExtendedHeaderSize1200) {
            return std::format("Could not read P3A extended header of archive at '{}'.",
                               archivePath);
        }
        if (extHeader.Size < P3AExtendedHeaderSize1200) {
            return std::format("Invalid P3A extended header size in archive at '{}'.", archivePath);
        }
        if (extHeader.Size > P3AExtendedHeaderSize1200) {
            // skip remaining extended header, we don't know what it contains
            if (!f.SetPosition(extHeader.Size - P3AExtendedHeaderSize1200,
                               HyoutaUtils::IO::SetPositionMode::Current)) {
                return std::format("Could skip past P3A extended header in archive at '{}'.",
                                   archivePath);
            }
        }
        // TODO: check hash

        hasUncompressedHash = true;
    } else {
        extHeader.Size = 0;
        extHeader.FileInfoSize = P3AFileInfoSize1100;
    }

    json.Key("Version");
    json.Uint(header.Version);

    json.Key("Alignment");
    json.Uint(0);

    auto fileinfos = std::make_unique_for_overwrite<P3AFileInfo[]>(header.FileCount);
    if (!fileinfos) {
        return std::string("Could not allocate memory for P3A file infos.");
    }
    if (extHeader.FileInfoSize == sizeof(P3AFileInfo)) {
        // can read efficiently
        const size_t fileinfoTotalSize = sizeof(P3AFileInfo) * header.FileCount;
        if (f.Read(fileinfos.get(), fileinfoTotalSize) != fileinfoTotalSize) {
            return std::format("Could read P3A file infos from archive at '{}'.", archivePath);
        }
    } else {
        if (extHeader.FileInfoSize < P3AFileInfoSize1100) {
            // P3AFileInfoSize1100 is the minimum size, this can't be valid
            return std::format("Invalid P3A file info size in archive at '{}'.", archivePath);
        }

        if (extHeader.FileInfoSize > sizeof(P3AFileInfo)) {
            // read one at a time, then skip difference
            for (size_t i = 0; i < header.FileCount; ++i) {
                if (f.Read(&fileinfos[i], sizeof(P3AFileInfo)) != sizeof(P3AFileInfo)) {
                    return std::format("Could read P3A file infos from archive at '{}'.",
                                       archivePath);
                }
                if (!f.SetPosition(extHeader.FileInfoSize - sizeof(P3AFileInfo),
                                   HyoutaUtils::IO::SetPositionMode::Current)) {
                    return std::format("Could read P3A file infos from archive at '{}'.",
                                       archivePath);
                }
            }
        } else {
            // read one at a time, keep the unread part zero-filled
            std::memset(fileinfos.get(), 0, sizeof(P3AFileInfo) * header.FileCount);
            for (size_t i = 0; i < header.FileCount; ++i) {
                if (f.Read(&fileinfos[i], extHeader.FileInfoSize) != extHeader.FileInfoSize) {
                    return std::format("Could read P3A file infos from archive at '{}'.",
                                       archivePath);
                }
            }
        }
    }

    ZSTD_DDict_UniquePtr ddict = nullptr;
    if (header.Flags & P3AHeaderFlag_HasZstdDict) {
        P3ADictHeader dictHeader;
        if (f.Read(&dictHeader, sizeof(P3ADictHeader)) != sizeof(P3ADictHeader)) {
            return std::format("Could not read ZSTD dictionary header from archive at '{}'.",
                               archivePath);
        }
        auto dict = std::make_unique_for_overwrite<uint8_t[]>(dictHeader.Length);
        if (!dict) {
            return std::string("Could not allocate memory for ZSTD dictionary.");
        }
        if (f.Read(dict.get(), dictHeader.Length) != dictHeader.Length) {
            return std::format("Could not read ZSTD dictionary from archive at '{}'.", archivePath);
        }
        ddict = ZSTD_DDict_UniquePtr(ZSTD_createDDict(dict.get(), dictHeader.Length));
        if (!ddict) {
            return std::format("Invalid ZSTD dictionary in archive at '{}'.", archivePath);
        }

        std::string dictPath = HyoutaUtils::IO::FilesystemPathToUtf8(
            extractFsPath / HyoutaUtils::IO::FilesystemPathFromUtf8("__zstd_dictionary.bin"));
        if (!HyoutaUtils::IO::WriteFileAtomic(dictPath, dict.get(), dictHeader.Length)) {
            return std::format("Could not write ZSTD dictionary to '{}'.", dictPath);
        }

        json.Key("ZStdDictionaryPath");
        json.String("__zstd_dictionary.bin");
    }

    json.Key("Files");
    json.StartArray();

    for (size_t i = 0; i < header.FileCount; ++i) {
        const auto& fileinfo = fileinfos[i];
        auto filename =
            StripTrailingNull(std::string_view(fileinfo.Filename.begin(), fileinfo.Filename.end()));
        if (!HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches(filename, pathFilter)) {
            continue;
        }

        std::filesystem::path relativePath = HyoutaUtils::IO::FilesystemPathFromUtf8(filename);
        if (relativePath.is_absolute()) {
            return std::format(
                "Refusing to extract file with absolute path '{}' from P3A archive at '{}'.",
                filename,
                archivePath);
        }

        json.StartObject();
        {
            json.Key("NameInArchive");
            json.String(filename.data(), filename.size());

            auto relPathStr = relativePath.u8string();
            json.Key("PathOnDisk");
            json.String((const char*)relPathStr.data(), relPathStr.size());
        }

        std::filesystem::path fullPath = extractFsPath / relativePath;
        std::filesystem::path parentPath = fullPath.parent_path();
        std::filesystem::create_directories(parentPath, ec);
        if (ec) {
            return std::format("Could not create output directory at '{}'.",
                               HyoutaUtils::IO::FilesystemPathToUtf8(parentPath));
        }

        std::string fullPathStr = HyoutaUtils::IO::FilesystemPathToUtf8(fullPath);
        HyoutaUtils::IO::File f2;
        if (!f2.OpenWithTempFilename(fullPathStr, HyoutaUtils::IO::OpenMode::Write)) {
            return std::format("Could not open temp output file for '{}'.", fullPathStr);
        }
        auto outfileScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { f2.Delete(); });

        bool uncompressed = false;
        json.Key("Compression");
        if (fileinfo.CompressionType == P3ACompressionType::LZ4) {
            json.String("lz4");
        } else if (fileinfo.CompressionType == P3ACompressionType::ZSTD) {
            json.String("zStd");
        } else if (fileinfo.CompressionType == P3ACompressionType::ZSTD_DICT) {
            json.String("zStdDict");
        } else if (fileinfo.CompressionType == P3ACompressionType::None) {
            json.String("none");
            uncompressed = true;
        } else {
            return std::format("Unknown compression type for file '{}' in P3A archive at '{}'.",
                               filename,
                               archivePath);
        }

        if (uncompressed || noDecompression) {
            if (uncompressed) {
                if (fileinfo.CompressedSize != fileinfo.UncompressedSize) {
                    return std::format(
                        "Filesize mismatch for uncompressed file '{}' in P3A archive at '{}'.",
                        filename,
                        archivePath);
                }
                if (hasUncompressedHash && fileinfo.CompressedHash != fileinfo.UncompressedHash) {
                    return std::format(
                        "Hash mismatch for uncompressed file '{}' in P3A archive at '{}'.",
                        filename,
                        archivePath);
                }
            }

            uint64_t filedataSize = fileinfo.CompressedSize;
            if (!f.SetPosition(fileinfo.Offset)) {
                return std::format("Failed to seek to data for file '{}' in P3A archive at '{}'.",
                                   filename,
                                   archivePath);
            }

            XXH64_state_t* hashState = XXH64_createState();
            if (hashState == nullptr) {
                return std::string("Failed to allocate memory for XXH64 state.");
            }
            auto hashStateGuard =
                HyoutaUtils::MakeScopeGuard([&hashState] { XXH64_freeState(hashState); });
            XXH64_reset(hashState, 0);

            static constexpr size_t bufferSize = 4096;
            std::array<char, bufferSize> buffer;
            uint64_t rest = filedataSize;
            while (rest > 0) {
                size_t blockSize = (rest > bufferSize) ? bufferSize : static_cast<size_t>(rest);
                if (f.Read(buffer.data(), blockSize) != blockSize) {
                    return std::format("Failed to read data for file '{}' in P3A archive at '{}'.",
                                       filename,
                                       archivePath);
                }
                XXH64_update(hashState, buffer.data(), blockSize);
                if (f2.Write(buffer.data(), blockSize) != blockSize) {
                    return std::format(
                        "Failed to write data to file '{}' extracting from P3A archive at '{}'.",
                        fullPathStr,
                        archivePath);
                }
                rest -= blockSize;
            }
            if (fileinfo.CompressedHash != XXH64_digest(hashState)) {
                return std::format(
                    "Data hash error for file '{}' in P3A archive at '{}'.", filename, archivePath);
            }
        } else {
            auto filedata = std::make_unique_for_overwrite<char[]>(fileinfo.CompressedSize);
            uint64_t filedataSize = fileinfo.CompressedSize;
            if (!filedata) {
                return std::format(
                    "Failed to allocate memory for compressed data for file '{}' in P3A archive at "
                    "'{}'.",
                    filename,
                    archivePath);
            }
            if (!f.SetPosition(fileinfo.Offset)) {
                return std::format("Failed to seek to data for file '{}' in P3A archive at '{}'.",
                                   filename,
                                   archivePath);
            }
            if (f.Read(filedata.get(), fileinfo.CompressedSize) != fileinfo.CompressedSize) {
                return std::format("Failed to read data for file '{}' in P3A archive at '{}'.",
                                   filename,
                                   archivePath);
            }

            if (fileinfo.CompressedHash != XXH64(filedata.get(), fileinfo.CompressedSize, 0)) {
                return std::format(
                    "Compressed data hash error for file '{}' in P3A archive at '{}'.",
                    filename,
                    archivePath);
            }

            if (fileinfo.CompressionType == P3ACompressionType::LZ4) {
                if (!noDecompression) {
                    auto decomp = std::make_unique_for_overwrite<char[]>(fileinfo.UncompressedSize);
                    if (!decomp) {
                        return std::format(
                            "Failed to allocate memory for uncompressed data for file '{}' in P3A "
                            "archive at "
                            "'{}'.",
                            filename,
                            archivePath);
                    }
                    if (LZ4_decompress_safe(filedata.get(),
                                            decomp.get(),
                                            fileinfo.CompressedSize,
                                            fileinfo.UncompressedSize)
                        != fileinfo.UncompressedSize) {
                        return std::format("Failed to decompress file '{}' in P3A archive at '{}'.",
                                           filename,
                                           archivePath);
                    }
                    decomp.swap(filedata);
                    filedataSize = fileinfo.UncompressedSize;

                    if (hasUncompressedHash
                        && fileinfo.UncompressedHash != XXH64(filedata.get(), filedataSize, 0)) {
                        return std::format(
                            "Uncompressed data hash error for file '{}' in P3A archive at '{}'.",
                            filename,
                            archivePath);
                    }
                }
            } else if (fileinfo.CompressionType == P3ACompressionType::ZSTD) {
                if (!noDecompression) {
                    auto decomp = std::make_unique_for_overwrite<char[]>(fileinfo.UncompressedSize);
                    if (!decomp) {
                        return std::format(
                            "Failed to allocate memory for uncompressed data for file '{}' in P3A "
                            "archive at "
                            "'{}'.",
                            filename,
                            archivePath);
                    }
                    if (ZSTD_decompress(decomp.get(),
                                        fileinfo.UncompressedSize,
                                        filedata.get(),
                                        fileinfo.CompressedSize)
                        != fileinfo.UncompressedSize) {
                        return std::format("Failed to decompress file '{}' in P3A archive at '{}'.",
                                           filename,
                                           archivePath);
                    }
                    decomp.swap(filedata);
                    filedataSize = fileinfo.UncompressedSize;

                    if (hasUncompressedHash
                        && fileinfo.UncompressedHash != XXH64(filedata.get(), filedataSize, 0)) {
                        return std::format(
                            "Uncompressed data hash error for file '{}' in P3A archive at '{}'.",
                            filename,
                            archivePath);
                    }
                }
            } else if (fileinfo.CompressionType == P3ACompressionType::ZSTD_DICT) {
                if (!noDecompression) {
                    if (!ddict) {
                        return std::format(
                            "File '{}' in P3A archive at '{}' claims ZSTD dictionary compression, "
                            "but no dictionary provided.",
                            filename,
                            archivePath);
                    }

                    auto decomp = std::make_unique_for_overwrite<char[]>(fileinfo.UncompressedSize);
                    if (!decomp) {
                        return std::format(
                            "Failed to allocate memory for uncompressed data for file '{}' in P3A "
                            "archive at "
                            "'{}'.",
                            filename,
                            archivePath);
                    }
                    ZSTD_DCtx_UniquePtr dctx = ZSTD_DCtx_UniquePtr(ZSTD_createDCtx());
                    if (!dctx) {
                        return std::string(
                            "Could not allocate memory for ZSTD dictionary decompression context.");
                    }
                    if (ZSTD_decompress_usingDDict(dctx.get(),
                                                   decomp.get(),
                                                   fileinfo.UncompressedSize,
                                                   filedata.get(),
                                                   fileinfo.CompressedSize,
                                                   ddict.get())
                        != fileinfo.UncompressedSize) {
                        return std::format("Failed to decompress file '{}' in P3A archive at '{}'.",
                                           filename,
                                           archivePath);
                    }

                    decomp.swap(filedata);
                    filedataSize = fileinfo.UncompressedSize;

                    if (hasUncompressedHash
                        && fileinfo.UncompressedHash != XXH64(filedata.get(), filedataSize, 0)) {
                        return std::format(
                            "Uncompressed data hash error for file '{}' in P3A archive at '{}'.",
                            filename,
                            archivePath);
                    }
                }
            } else {
                assert(0); // should never come here
                return std::string("Internal error.");
            }

            if (f2.Write(filedata.get(), filedataSize) != filedataSize) {
                return std::format("Could not write to temp output file for '{}'.", fullPathStr);
            }
        }

        if (!f2.Rename(fullPath)) {
            return std::format("Could not rename temp output file to '{}'.", fullPathStr);
        }
        outfileScope.Dispose();

        if (noDecompression) {
            json.Key("Precompressed");
            json.Bool(true);
            json.Key("UncompressedFilesize");
            json.Uint64(fileinfo.UncompressedSize);
            if (header.Version >= 1200) {
                json.Key("UncompressedHash");
                json.Uint64(fileinfo.UncompressedHash);
            }
        }

        json.EndObject();
    }

    json.EndArray();
    json.EndObject();

    if (generateJson) {
        std::string jsonPath = HyoutaUtils::IO::FilesystemPathToUtf8(
            extractFsPath / HyoutaUtils::IO::FilesystemPathFromUtf8("__p3a.json"));
        const char* jsonstring = jsonbuffer.GetString();
        const size_t jsonstringsize = jsonbuffer.GetSize();
        if (!HyoutaUtils::IO::WriteFileAtomic(jsonPath, jsonstring, jsonstringsize)) {
            return std::format("Could not write json output file at '{}'.", jsonPath);
        }
    }

    return UnpackP3AResult::Success;
}
} // namespace SenPatcher
