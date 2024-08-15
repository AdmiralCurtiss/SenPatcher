#include "pack.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
#include <filesystem>
#include <variant>
#endif

#include "lz4/lz4.h"
#include "lz4/lz4hc.h"

#include "zstd/common/xxhash.h"
#include "zstd/zstd.h"

#include "p3a/structs.h"

#include "util/align.h"
#include "util/file.h"

namespace SenPatcher {
struct P3APackFile::Impl {
#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    std::variant<std::vector<char>, std::filesystem::path> Data = std::vector<char>();
#else
    std::vector<char> Data;
#endif
    std::array<char, 0x100> Filename{};
    P3ACompressionType DesiredCompressionType{};
    P3APackFilePrecompressed IsPrecompressed = P3APackFilePrecompressed::No;
    uint64_t DecompressedFilesizeForPrecompressed = 0u;

    Impl(std::vector<char> data,
         const std::array<char, 0x100>& filename,
         P3ACompressionType desiredCompressionType,
         P3APackFilePrecompressed precompressed,
         uint64_t decompressedFilesize)
      : Data(std::move(data))
      , Filename(filename)
      , DesiredCompressionType(desiredCompressionType)
      , IsPrecompressed(precompressed)
      , DecompressedFilesizeForPrecompressed(
            precompressed != P3APackFilePrecompressed::No ? decompressedFilesize : 0u) {}

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    Impl(std::filesystem::path path,
         const std::array<char, 0x100>& filename,
         P3ACompressionType desiredCompressionType,
         P3APackFilePrecompressed precompressed,
         uint64_t decompressedFilesize)
      : Data(std::move(path))
      , Filename(filename)
      , DesiredCompressionType(desiredCompressionType)
      , IsPrecompressed(precompressed)
      , DecompressedFilesizeForPrecompressed(
            precompressed != P3APackFilePrecompressed::No ? decompressedFilesize : 0u) {}
#endif
};

P3APackFile::P3APackFile(std::vector<char> data,
                         const std::array<char, 0x100>& filename,
                         P3ACompressionType desiredCompressionType,
                         P3APackFilePrecompressed precompressed,
                         uint64_t decompressedFilesize)
  : Data(std::make_unique<P3APackFile::Impl>(std::move(data),
                                             filename,
                                             desiredCompressionType,
                                             precompressed,
                                             decompressedFilesize)) {}

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
P3APackFile::P3APackFile(std::filesystem::path path,
                         const std::array<char, 0x100>& filename,
                         P3ACompressionType desiredCompressionType,
                         P3APackFilePrecompressed precompressed,
                         uint64_t decompressedFilesize)
  : Data(std::make_unique<P3APackFile::Impl>(std::move(path),
                                             filename,
                                             desiredCompressionType,
                                             precompressed,
                                             decompressedFilesize)) {}
#endif

P3APackFile::P3APackFile(P3APackFile&& other) = default;
P3APackFile& P3APackFile::operator=(P3APackFile&& other) = default;
P3APackFile::~P3APackFile() = default;

const std::array<char, 0x100>& P3APackFile::GetFilename() const {
    return Data->Filename;
}
P3ACompressionType P3APackFile::GetDesiredCompressionType() const {
    return Data->DesiredCompressionType;
}
bool P3APackFile::IsPrecompressed() const {
    return Data->IsPrecompressed != P3APackFilePrecompressed::No;
}
uint64_t P3APackFile::GetDecompressedFilesizeForPrecompressed() const {
    assert(IsPrecompressed());
    return Data->DecompressedFilesizeForPrecompressed;
}

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
bool P3APackFile::HasVectorData() const {
    return std::holds_alternative<std::vector<char>>(Data->Data);
}
const std::vector<char>& P3APackFile::GetVectorData() const {
    assert(HasVectorData());
    return std::get<std::vector<char>>(Data->Data);
}
bool P3APackFile::HasPathData() const {
    return std::holds_alternative<std::filesystem::path>(Data->Data);
}
const std::filesystem::path& P3APackFile::GetPathData() const {
    assert(HasPathData());
    return std::get<std::filesystem::path>(Data->Data);
}
#else
bool P3APackFile::HasVectorData() const {
    return true;
}
const std::vector<char>& P3APackFile::GetVectorData() const {
    return Data->Data;
}
#endif

struct P3APackData::Impl {
    std::vector<P3APackFile> Files;
#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    std::variant<std::monostate, std::vector<char>, std::filesystem::path> ZStdDictionary =
        std::monostate();
#else
    std::optional<std::vector<char>> ZStdDictionary = std::nullopt;
#endif
    size_t Alignment = 0;
};

P3APackData::P3APackData() : Data(std::make_unique<P3APackData::Impl>()) {}
P3APackData::P3APackData(P3APackData&& other) = default;
P3APackData& P3APackData::operator=(P3APackData&& other) = default;
P3APackData::~P3APackData() = default;

size_t P3APackData::GetAlignment() const {
    return Data->Alignment;
}
void P3APackData::SetAlignment(size_t alignment) {
    Data->Alignment = alignment;
}
const std::vector<P3APackFile>& P3APackData::GetFiles() const {
    return Data->Files;
}
std::vector<P3APackFile>& P3APackData::GetMutableFiles() {
    return Data->Files;
}

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
void P3APackData::ClearZStdDictionaryData() {
    Data->ZStdDictionary = std::monostate();
}
bool P3APackData::HasZStdDictionaryVectorData() const {
    return std::holds_alternative<std::vector<char>>(Data->ZStdDictionary);
}
const std::vector<char>& P3APackData::GetZStdDictionaryVectorData() const {
    assert(HasZStdDictionaryVectorData());
    return std::get<std::vector<char>>(Data->ZStdDictionary);
}
void P3APackData::SetZStdDictionaryVectorData(std::vector<char> data) {
    Data->ZStdDictionary = std::move(data);
}
bool P3APackData::HasZStdDictionaryPathData() const {
    return std::holds_alternative<std::filesystem::path>(Data->ZStdDictionary);
}
const std::filesystem::path& P3APackData::GetZStdDictionaryPathData() const {
    assert(HasZStdDictionaryPathData());
    return std::get<std::filesystem::path>(Data->ZStdDictionary);
}
void P3APackData::SetZStdDictionaryPathData(std::filesystem::path path) {
    Data->ZStdDictionary = std::move(path);
}
#else
void P3APackData::ClearZStdDictionaryData() {
    Data->ZStdDictionary = std::nullopt;
}
bool P3APackData::HasZStdDictionaryVectorData() const {
    return Data->ZStdDictionary.has_value();
}
const std::vector<char>& P3APackData::GetZStdDictionaryVectorData() const {
    assert(HasZStdDictionaryVectorData());
    return *Data->ZStdDictionary;
}
void P3APackData::SetZStdDictionaryVectorData(std::vector<char> data) {
    Data->ZStdDictionary = std::move(data);
}
#endif

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

HyoutaUtils::Result<P3ACompressionResult, P3ACompressionError>
    CompressForP3A(P3ACompressionType desiredCompressionType,
                   const char* filedata,
                   uint64_t uncompressedSize,
                   const void* zstdCDict) {
    switch (desiredCompressionType) {
        case P3ACompressionType::LZ4: {
            if (uncompressedSize == 0 || uncompressedSize > LZ4_MAX_INPUT_SIZE) {
                return P3ACompressionError::InvalidUncompressedSize;
            } else {
                const int signedSize = static_cast<int>(uncompressedSize);
                const int bound = LZ4_compressBound(signedSize);
                if (bound <= 0) {
                    return P3ACompressionError::InvalidUncompressedSize;
                } else {
                    auto bufferLength = static_cast<unsigned int>(bound);
                    auto compressedData = std::make_unique_for_overwrite<char[]>(bufferLength);
                    if (!compressedData) {
                        return P3ACompressionError::MemoryAllocationFailure;
                    }
                    const int lz4return = LZ4_compress_HC(
                        filedata, compressedData.get(), signedSize, bound, LZ4HC_CLEVEL_MAX);
                    if (lz4return <= 0
                        || static_cast<unsigned int>(lz4return) >= uncompressedSize) {
                        // compression failed or pointless, write uncompressed instead
                        return P3ACompressionError::CompressionError;
                    } else {
                        P3ACompressionResult c{};
                        c.Buffer = std::move(compressedData);
                        c.DataLength = static_cast<unsigned int>(lz4return);
                        c.BufferLength = bufferLength;
                        c.CompressionType = P3ACompressionType::LZ4;
                        return c;
                    }
                }
            }
            break;
        }
        case P3ACompressionType::ZSTD: {
            size_t bound = ZSTD_compressBound(uncompressedSize);
            if (uncompressedSize == 0 || ZSTD_isError(bound)) {
                return P3ACompressionError::InvalidUncompressedSize;
            } else {
                auto compressedData = std::make_unique_for_overwrite<char[]>(bound);
                if (!compressedData) {
                    return P3ACompressionError::MemoryAllocationFailure;
                }
                const size_t zstdReturn =
                    ZSTD_compress(compressedData.get(), bound, filedata, uncompressedSize, 22);
                if (ZSTD_isError(zstdReturn)) {
                    return P3ACompressionError::CompressionError;
                } else {
                    P3ACompressionResult c{};
                    c.Buffer = std::move(compressedData);
                    c.DataLength = zstdReturn;
                    c.BufferLength = bound;
                    c.CompressionType = P3ACompressionType::ZSTD;
                    return c;
                }
            }
            break;
        }
        case P3ACompressionType::ZSTD_DICT: {
            if (!zstdCDict) {
                return P3ACompressionError::ArgumentError;
            }

            size_t bound = ZSTD_compressBound(uncompressedSize);
            if (uncompressedSize == 0 || ZSTD_isError(bound)) {
                return P3ACompressionError::InvalidUncompressedSize;
            } else {
                auto compressedData = std::make_unique_for_overwrite<char[]>(bound);
                if (!compressedData) {
                    return P3ACompressionError::MemoryAllocationFailure;
                }
                size_t zstdReturn;
                {
                    ZSTD_CCtx_UniquePtr cctx = ZSTD_CCtx_UniquePtr(ZSTD_createCCtx());
                    if (!cctx) {
                        return P3ACompressionError::MemoryAllocationFailure;
                    }
                    zstdReturn =
                        ZSTD_compress_usingCDict(cctx.get(),
                                                 compressedData.get(),
                                                 bound,
                                                 filedata,
                                                 uncompressedSize,
                                                 static_cast<const ZSTD_CDict*>(zstdCDict));
                }
                if (ZSTD_isError(zstdReturn)) {
                    return P3ACompressionError::CompressionError;
                } else {
                    P3ACompressionResult c{};
                    c.Buffer = std::move(compressedData);
                    c.DataLength = zstdReturn;
                    c.BufferLength = bound;
                    c.CompressionType = P3ACompressionType::ZSTD_DICT;
                    return c;
                }
            }
            break;
        }
        default: break;
    }

    return P3ACompressionError::InvalidCompressionType;
}

bool PackP3A(HyoutaUtils::IO::File& file, const P3APackData& packData) {
    if (!file.IsOpen()) {
        return false;
    }

    std::unique_ptr<uint8_t[]> dict;
    uint64_t dictLength = 0;
    ZSTD_CDict_UniquePtr cdict = nullptr;
#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    if (packData.HasZStdDictionaryPathData()) {
        HyoutaUtils::IO::File dictfile(packData.GetZStdDictionaryPathData(),
                                       HyoutaUtils::IO::OpenMode::Read);
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
    } else
#endif
        if (packData.HasZStdDictionaryVectorData()) {
        const auto& vec = packData.GetZStdDictionaryVectorData();
        cdict = ZSTD_CDict_UniquePtr(ZSTD_createCDict(vec.data(), vec.size(), 22));
        if (!cdict) {
            return false;
        }
    }

    const size_t packDataAlignment = packData.GetAlignment();
    const uint64_t alignment = packDataAlignment == 0 ? 0x40 : packDataAlignment;
    const auto& fileinfos = packData.GetFiles();

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
        if (!HyoutaUtils::AlignFile(file, position, alignment)) {
            return false;
        }

        uint64_t filesize;
        const char* filedata;
        std::unique_ptr<char[]> filedataHolder;
#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
        if (fileinfo.HasPathData()) {
            HyoutaUtils::IO::File inputfile(fileinfo.GetPathData(),
                                            HyoutaUtils::IO::OpenMode::Read);
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
        } else
#endif
            if (fileinfo.HasVectorData()) {
            const auto& vec = fileinfo.GetVectorData();
            filesize = vec.size();
            filedata = vec.data();
        } else {
            return false;
        }

        const P3ACompressionType desiredCompressionType = fileinfo.GetDesiredCompressionType();
        P3ACompressionType compressionType = desiredCompressionType;
        uint64_t compressedSize = filesize;
        const uint64_t uncompressedSize = fileinfo.IsPrecompressed()
                                              ? fileinfo.GetDecompressedFilesizeForPrecompressed()
                                              : filesize;
        uint64_t hash = 0;

        const auto write_uncompressed = [&]() -> bool {
            if (fileinfo.IsPrecompressed()) {
                // TODO: this is doable, decompress first
                return false;
            }

            compressionType = P3ACompressionType::None;
            compressedSize = uncompressedSize;
            hash = XXH64(filedata, uncompressedSize, 0);
            if (file.Write(filedata, uncompressedSize) != uncompressedSize) {
                return false;
            }
            return true;
        };
        const auto write_precompressed = [&]() -> bool {
            if (!fileinfo.IsPrecompressed()) {
                return false;
            }

            compressionType = desiredCompressionType;
            compressedSize = filesize;
            hash = XXH64(filedata, filesize, 0);
            if (file.Write(filedata, filesize) != filesize) {
                return false;
            }
            return true;
        };

        if (fileinfo.IsPrecompressed()) {
            if (!write_precompressed()) {
                return false;
            }
        } else {
            auto compressionResult =
                CompressForP3A(desiredCompressionType, filedata, uncompressedSize, cdict.get());
            if (compressionResult.IsSuccess()) {
                auto& c = compressionResult.GetSuccessValue();
                compressionType = c.CompressionType;
                compressedSize = c.DataLength;
                hash = XXH64(c.Buffer.get(), compressedSize, 0);
                if (file.Write(c.Buffer.get(), compressedSize) != compressedSize) {
                    return false;
                }
            } else {
                switch (compressionResult.GetErrorValue()) {
                    case P3ACompressionError::ArgumentError: return false;
                    case P3ACompressionError::MemoryAllocationFailure: return false;
                    default:
                        if (!write_uncompressed()) {
                            return false;
                        }
                        break;
                }
            }
        }

        // fill in header
        P3AFileInfo tmp{};
        tmp.Filename = fileinfo.GetFilename();
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
