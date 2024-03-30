#include "pack.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "lz4/lz4.h"
#include "lz4/lz4hc.h"

#include "zstd/common/xxhash.h"
#include "zstd/zstd.h"

#include "p3a/structs.h"

#include "align.h"
#include "file.h"

namespace SenPatcher {
P3APackFile::P3APackFile(std::vector<char> data,
                         const std::array<char, 0x100>& filename,
                         P3ACompressionType desiredCompressionType)
  : Data(std::move(data)), Filename(filename), DesiredCompressionType(desiredCompressionType) {}
P3APackFile::P3APackFile(std::filesystem::path path,
                         const std::array<char, 0x100>& filename,
                         P3ACompressionType desiredCompressionType)
  : Data(std::move(path)), Filename(filename), DesiredCompressionType(desiredCompressionType) {}
P3APackFile::P3APackFile(const P3APackFile& other) = default;
P3APackFile::P3APackFile(P3APackFile&& other) = default;
P3APackFile& P3APackFile::operator=(const P3APackFile& other) = default;
P3APackFile& P3APackFile::operator=(P3APackFile&& other) = default;
P3APackFile::~P3APackFile() = default;

P3APackData::P3APackData() = default;
P3APackData::P3APackData(const P3APackData& other) = default;
P3APackData::P3APackData(P3APackData&& other) = default;
P3APackData& P3APackData::operator=(const P3APackData& other) = default;
P3APackData& P3APackData::operator=(P3APackData&& other) = default;
P3APackData::~P3APackData() = default;

namespace {
struct ZSTD_CDict_Deleter {
    void operator()(ZSTD_CDict* ptr) {
        if (ptr) {
            ZSTD_freeCDict(ptr);
        }
    }
};
using ZSTD_CDict_UniquePtr = std::unique_ptr<ZSTD_CDict, ZSTD_CDict_Deleter>;

struct ZSTD_CCtx_Deleter {
    void operator()(ZSTD_CCtx* ptr) {
        if (ptr) {
            ZSTD_freeCCtx(ptr);
        }
    }
};
using ZSTD_CCtx_UniquePtr = std::unique_ptr<ZSTD_CCtx, ZSTD_CCtx_Deleter>;
} // namespace

static bool CollectEntries(std::vector<P3APackFile>& fileinfos,
                           const std::filesystem::path& rootDir,
                           const std::filesystem::path& currentDir,
                           std::error_code& ec) {
    std::filesystem::directory_iterator iterator(currentDir, ec);
    if (ec) {
        return false;
    }
    for (auto const& entry : iterator) {
        if (entry.is_directory()) {
            if (!CollectEntries(fileinfos, rootDir, entry.path(), ec)) {
                return false;
            }
            continue;
        }

        const auto relativePath = std::filesystem::relative(entry.path(), rootDir, ec);
        if (relativePath.empty()) {
            return false;
        }
        const auto filename = relativePath.u8string();
        const char8_t* filenameC = filename.c_str();

        std::array<char, 0x100> fn{};
        for (size_t i = 0; i < fn.size(); ++i) {
            const char c = static_cast<char>(filenameC[i]);
            if (c == '\0') {
                break;
            }
            fn[i] = (c == '\\' ? '/' : c);
        }
        fileinfos.emplace_back(entry.path(), fn, SenPatcher::P3ACompressionType::None);
    }
    return true;
}

bool PackP3AFromDirectory(const std::filesystem::path& directoryPath,
                          const std::filesystem::path& archivePath,
                          P3ACompressionType desiredCompressionType,
                          const std::filesystem::path& dictPath) {
    P3APackData packData;
    packData.Alignment = 0x40;
    std::error_code ec;
    if (!CollectEntries(packData.Files, directoryPath, directoryPath, ec)) {
        return false;
    }

    // probably not needed but makes the packing order reproduceable
    std::stable_sort(
        packData.Files.begin(),
        packData.Files.end(),
        [](const P3APackFile& lhs, const P3APackFile& rhs) {
            return memcmp(lhs.Filename.data(), rhs.Filename.data(), lhs.Filename.size()) < 0;
        });

    for (auto& packFile : packData.Files) {
        packFile.DesiredCompressionType = desiredCompressionType;
    }

    if (!dictPath.empty()) {
        packData.ZStdDictionary = dictPath;
    }

    return PackP3A(archivePath, packData);
}

bool PackP3A(const std::filesystem::path& archivePath, const P3APackData& packData) {
    IO::File file(archivePath, IO::OpenMode::Write);
    return PackP3A(file, packData);
}

bool PackP3A(SenPatcher::IO::File& file, const P3APackData& packData) {
    if (!file.IsOpen()) {
        return false;
    }

    std::unique_ptr<uint8_t[]> dict;
    uint64_t dictLength = 0;
    ZSTD_CDict_UniquePtr cdict = nullptr;
    if (std::holds_alternative<std::filesystem::path>(packData.ZStdDictionary)) {
        IO::File dictfile(std::get<std::filesystem::path>(packData.ZStdDictionary),
                          IO::OpenMode::Read);
        if (!dictfile.IsOpen()) {
            return false;
        }
        const auto fileLength = dictfile.GetLength();
        if (!fileLength) {
            return false;
        }
        dictLength = *fileLength;
        dict = std::make_unique<uint8_t[]>(dictLength);
        if (!dict) {
            return false;
        }
        if (dictfile.Read(dict.get(), dictLength) != dictLength) {
            return false;
        }
        cdict = ZSTD_CDict_UniquePtr(ZSTD_createCDict(dict.get(), dictLength, 22));
        if (!cdict) {
            return false;
        }
    } else if (std::holds_alternative<std::vector<char>>(packData.ZStdDictionary)) {
        const auto& vec = std::get<std::vector<char>>(packData.ZStdDictionary);
        cdict = ZSTD_CDict_UniquePtr(ZSTD_createCDict(vec.data(), vec.size(), 22));
        if (!cdict) {
            return false;
        }
    }

    const uint64_t alignment = packData.Alignment == 0 ? 0x40 : packData.Alignment;
    const auto& fileinfos = packData.Files;

    uint64_t position = 0;
    {
        P3AHeader header{};
        header.Magic = {{'P', 'H', '3', 'A', 'R', 'C', 'V', '\0'}};
        header.Flags = 0;
        if (dict) {
            header.Flags |= P3AHeaderFlag_HasZstdDict;
        }
        header.Version = 1100;
        header.FileCount = fileinfos.size();
        header.Hash = XXH64(&header, sizeof(P3AHeader) - 8, 0);
        if (file.Write(&header, sizeof(P3AHeader)) != sizeof(P3AHeader)) {
            return false;
        }
        position += sizeof(P3AHeader);
    }
    {
        // we'll fill in the actual data later
        P3AFileInfo tmp{};
        for (size_t i = 0; i < fileinfos.size(); ++i) {
            if (file.Write(&tmp, sizeof(P3AFileInfo)) != sizeof(P3AFileInfo)) {
                return false;
            }
            position += sizeof(P3AFileInfo);
        }
    }
    if (dict) {
        P3ADictHeader dictHeader{};
        dictHeader.Magic = {{'P', '3', 'A', 'D', 'I', 'C', 'T', '\0'}};
        dictHeader.Length = dictLength;
        if (file.Write(&dictHeader, sizeof(P3ADictHeader)) != sizeof(P3ADictHeader)) {
            return false;
        }
        position += sizeof(P3ADictHeader);
        if (file.Write(dict.get(), dictLength) != dictLength) {
            return false;
        }
        position += dictLength;
    }

    for (size_t i = 0; i < fileinfos.size(); ++i) {
        const auto& fileinfo = fileinfos[i];
        if (!AlignFile(file, position, alignment)) {
            return false;
        }

        uint64_t filesize;
        const char* filedata;
        std::unique_ptr<char[]> filedataHolder;
        if (std::holds_alternative<std::filesystem::path>(fileinfo.Data)) {
            IO::File inputfile(std::get<std::filesystem::path>(fileinfo.Data), IO::OpenMode::Read);
            if (!inputfile.IsOpen()) {
                return false;
            }
            const auto maybeFilesize = inputfile.GetLength();
            if (!maybeFilesize) {
                return false;
            }
            filesize = *maybeFilesize;
            filedataHolder = std::make_unique_for_overwrite<char[]>(filesize);
            if (!filedataHolder) {
                return false;
            }
            if (inputfile.Read(filedataHolder.get(), filesize) != filesize) {
                return false;
            }
            filedata = filedataHolder.get();
        } else if (std::holds_alternative<std::vector<char>>(fileinfo.Data)) {
            const auto& vec = std::get<std::vector<char>>(fileinfo.Data);
            filesize = vec.size();
            filedata = vec.data();
        } else {
            return false;
        }

        P3ACompressionType compressionType = fileinfo.DesiredCompressionType;
        uint64_t compressedSize = filesize;
        uint64_t uncompressedSize = filesize;
        uint64_t hash = 0;

        const auto write_uncompressed = [&]() -> bool {
            compressionType = P3ACompressionType::None;
            compressedSize = filesize;
            uncompressedSize = filesize;
            hash = XXH64(filedata, filesize, 0);
            if (file.Write(filedata, filesize) != filesize) {
                return false;
            }
            return true;
        };

        switch (fileinfo.DesiredCompressionType) {
            case P3ACompressionType::LZ4: {
                if (filesize == 0 || filesize > LZ4_MAX_INPUT_SIZE) {
                    if (!write_uncompressed()) {
                        return false;
                    }
                } else {
                    const int signedSize = static_cast<int>(filesize);
                    const int bound = LZ4_compressBound(signedSize);
                    if (bound <= 0) {
                        if (!write_uncompressed()) {
                            return false;
                        }
                    } else {
                        auto compressedData = std::make_unique_for_overwrite<char[]>(
                            static_cast<unsigned int>(bound));
                        if (!compressedData) {
                            return false;
                        }
                        const int lz4return = LZ4_compress_HC(
                            filedata, compressedData.get(), signedSize, bound, LZ4HC_CLEVEL_MAX);
                        if (lz4return <= 0 || static_cast<unsigned int>(lz4return) >= filesize) {
                            // compression failed or pointless, write uncompressed instead
                            if (!write_uncompressed()) {
                                return false;
                            }
                        } else {
                            compressionType = P3ACompressionType::LZ4;
                            compressedSize = static_cast<unsigned int>(lz4return);
                            uncompressedSize = filesize;
                            hash = XXH64(compressedData.get(), compressedSize, 0);
                            if (file.Write(compressedData.get(), compressedSize)
                                != compressedSize) {
                                return false;
                            }
                        }
                    }
                }
                break;
            }
            case P3ACompressionType::ZSTD: {
                size_t bound = ZSTD_compressBound(filesize);
                if (filesize == 0 || ZSTD_isError(bound)) {
                    if (!write_uncompressed()) {
                        return false;
                    }
                } else {
                    auto compressedData = std::make_unique_for_overwrite<char[]>(bound);
                    if (!compressedData) {
                        return false;
                    }
                    const size_t zstdReturn =
                        ZSTD_compress(compressedData.get(), bound, filedata, filesize, 22);
                    if (ZSTD_isError(zstdReturn)) {
                        if (!write_uncompressed()) {
                            return false;
                        }
                    } else {
                        compressionType = P3ACompressionType::ZSTD;
                        compressedSize = zstdReturn;
                        uncompressedSize = filesize;
                        hash = XXH64(compressedData.get(), compressedSize, 0);
                        if (file.Write(compressedData.get(), compressedSize) != compressedSize) {
                            return false;
                        }
                    }
                }
                break;
            }
            case P3ACompressionType::ZSTD_DICT: {
                if (!cdict) {
                    return false;
                }

                size_t bound = ZSTD_compressBound(filesize);
                if (filesize == 0 || ZSTD_isError(bound)) {
                    if (!write_uncompressed()) {
                        return false;
                    }
                } else {
                    auto compressedData = std::make_unique_for_overwrite<char[]>(bound);
                    if (!compressedData) {
                        return false;
                    }
                    size_t zstdReturn;
                    {
                        ZSTD_CCtx_UniquePtr cctx = ZSTD_CCtx_UniquePtr(ZSTD_createCCtx());
                        if (!cctx) {
                            return false;
                        }
                        zstdReturn = ZSTD_compress_usingCDict(cctx.get(),
                                                              compressedData.get(),
                                                              bound,
                                                              filedata,
                                                              filesize,
                                                              cdict.get());
                    }
                    if (ZSTD_isError(zstdReturn)) {
                        if (!write_uncompressed()) {
                            return false;
                        }
                    } else {
                        compressionType = P3ACompressionType::ZSTD_DICT;
                        compressedSize = zstdReturn;
                        uncompressedSize = filesize;
                        hash = XXH64(compressedData.get(), compressedSize, 0);
                        if (file.Write(compressedData.get(), compressedSize) != compressedSize) {
                            return false;
                        }
                    }
                }
                break;
            }
            default: {
                if (!write_uncompressed()) {
                    return false;
                }
                break;
            }
        }


        // fill in header
        P3AFileInfo tmp{};
        tmp.Filename = fileinfo.Filename;
        tmp.CompressionType = compressionType;
        tmp.CompressedSize = compressedSize;
        tmp.UncompressedSize = uncompressedSize;
        tmp.Offset = position;
        tmp.Hash = hash;

        if (!file.SetPosition(sizeof(P3AHeader) + sizeof(P3AFileInfo) * i)) {
            return false;
        }
        if (file.Write(&tmp, sizeof(P3AFileInfo)) != sizeof(P3AFileInfo)) {
            return false;
        }

        position += compressedSize;
        if (!file.SetPosition(position)) {
            return false;
        }
    }

    return true;
}
} // namespace SenPatcher
