#include "pack.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>
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
    uint64_t DecompressedHashForPrecompressed = 0u;
    std::optional<size_t> SameAsIndex = std::nullopt;

    Impl(std::vector<char> data,
         const std::array<char, 0x100>& filename,
         P3ACompressionType desiredCompressionType,
         P3APackFilePrecompressed precompressed,
         uint64_t decompressedFilesize,
         uint64_t decompressedHash)
      : Data(std::move(data))
      , Filename(filename)
      , DesiredCompressionType(desiredCompressionType)
      , IsPrecompressed(precompressed)
      , DecompressedFilesizeForPrecompressed(
            precompressed != P3APackFilePrecompressed::No ? decompressedFilesize : 0u)
      , DecompressedHashForPrecompressed(
            precompressed != P3APackFilePrecompressed::No ? decompressedHash : 0u) {}

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    Impl(std::filesystem::path path,
         const std::array<char, 0x100>& filename,
         P3ACompressionType desiredCompressionType,
         P3APackFilePrecompressed precompressed,
         uint64_t decompressedFilesize,
         uint64_t decompressedHash)
      : Data(std::move(path))
      , Filename(filename)
      , DesiredCompressionType(desiredCompressionType)
      , IsPrecompressed(precompressed)
      , DecompressedFilesizeForPrecompressed(
            precompressed != P3APackFilePrecompressed::No ? decompressedFilesize : 0u)
      , DecompressedHashForPrecompressed(
            precompressed != P3APackFilePrecompressed::No ? decompressedHash : 0u) {}
#endif
};

P3APackFile::P3APackFile(std::vector<char> data,
                         const std::array<char, 0x100>& filename,
                         P3ACompressionType desiredCompressionType,
                         P3APackFilePrecompressed precompressed,
                         uint64_t decompressedFilesize,
                         uint64_t decompressedHash)
  : Data(std::make_unique<P3APackFile::Impl>(std::move(data),
                                             filename,
                                             desiredCompressionType,
                                             precompressed,
                                             decompressedFilesize,
                                             decompressedHash)) {}

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
P3APackFile::P3APackFile(std::filesystem::path path,
                         const std::array<char, 0x100>& filename,
                         P3ACompressionType desiredCompressionType,
                         P3APackFilePrecompressed precompressed,
                         uint64_t decompressedFilesize,
                         uint64_t decompressedHash)
  : Data(std::make_unique<P3APackFile::Impl>(std::move(path),
                                             filename,
                                             desiredCompressionType,
                                             precompressed,
                                             decompressedFilesize,
                                             decompressedHash)) {}
#endif

P3APackFile::P3APackFile(P3APackFile&& other) = default;
P3APackFile& P3APackFile::operator=(P3APackFile&& other) = default;
P3APackFile::~P3APackFile() = default;

const std::array<char, 0x100>& P3APackFile::GetFilename() const {
    return Data->Filename;
}
void P3APackFile::SetFilename(const std::array<char, 0x100>& filename) {
    Data->Filename = filename;
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
uint64_t P3APackFile::GetDecompressedHashForPrecompressed() const {
    assert(IsPrecompressed());
    return Data->DecompressedHashForPrecompressed;
}

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
bool P3APackFile::HasVectorData() const {
    return std::holds_alternative<std::vector<char>>(Data->Data);
}
const std::vector<char>& P3APackFile::GetVectorData() const {
    assert(HasVectorData());
    return std::get<std::vector<char>>(Data->Data);
}
void P3APackFile::SetVectorData(std::vector<char> data) {
    Data->Data = std::move(data);
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
void P3APackFile::SetVectorData(std::vector<char> data) {
    Data->Data = std::move(data);
}
#endif

std::optional<size_t> P3APackFile::IsSameAsIndex() const {
    return Data->SameAsIndex;
}

void P3APackFile::SetSameAsIndex(std::optional<size_t> index) {
    Data->SameAsIndex = index;
}

struct P3APackData::Impl {
    std::vector<P3APackFile> Files;
#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    std::variant<std::monostate, std::vector<char>, std::filesystem::path> ZStdDictionary =
        std::monostate();
#else
    std::optional<std::vector<char>> ZStdDictionary = std::nullopt;
#endif
    size_t Alignment = 0;
    uint32_t Version = 0;

    // P3A typically wants all-lowercase in filenames. This can be disabled. SenPatcher handles
    // files with uppercase filenames correctly, but the official implementation may not.
    bool AllowUppercaseInFilenames = false;
};

P3APackData::P3APackData() : Data(std::make_unique<P3APackData::Impl>()) {}
P3APackData::P3APackData(P3APackData&& other) = default;
P3APackData& P3APackData::operator=(P3APackData&& other) = default;
P3APackData::~P3APackData() = default;

uint32_t P3APackData::GetVersion() const {
    return Data->Version;
}
void P3APackData::SetVersion(uint32_t version) {
    Data->Version = version;
}
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

bool P3APackData::IsAllowUppercaseInFilenames() const {
    return Data->AllowUppercaseInFilenames;
}

void P3APackData::SetAllowUppercaseInFilenames(bool allowUppercase) {
    Data->AllowUppercaseInFilenames = allowUppercase;
}

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

std::array<char, 0x100> NormalizeP3AFilename(const std::array<char, 0x100>& filename,
                                             bool allowUppercase) {
    std::array<char, 0x100> output{};
    for (size_t i = 0; i < filename.size(); ++i) {
        char c = filename[i];
        if (c == '\0') {
            break;
        }
        if (c == '\\') {
            output[i] = '/';
        } else if (!allowUppercase && (c >= 'A' && c <= 'Z')) {
            output[i] = static_cast<char>(c + ('a' - 'A'));
        } else {
            output[i] = c;
        }
    }
    return output;
}

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

static const char* GetFileData(uint64_t& filesize,
                               std::unique_ptr<char[]>& filedataHolder,
                               const P3APackFile& fileinfo) {
#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    if (fileinfo.HasPathData()) {
        HyoutaUtils::IO::File inputfile(fileinfo.GetPathData(), HyoutaUtils::IO::OpenMode::Read);
        if (!inputfile.IsOpen()) {
            return nullptr;
        }
        const auto maybeFilesize = inputfile.GetLength();
        if (!maybeFilesize) {
            return nullptr;
        }
        filesize = *maybeFilesize;
        filedataHolder = std::make_unique_for_overwrite<char[]>(filesize);
        if (!filedataHolder) {
            return nullptr;
        }
        if (inputfile.Read(filedataHolder.get(), filesize) != filesize) {
            return nullptr;
        }
        return filedataHolder.get();
    }
#endif

    if (fileinfo.HasVectorData()) {
        const auto& vec = fileinfo.GetVectorData();
        filesize = vec.size();
        return vec.data();
    }

    return nullptr;
}

static bool WriteData(HyoutaUtils::IO::File& file,
                      P3ACompressionType& compressionType,
                      uint64_t& compressedSize,
                      uint64_t& hash,
                      const char* filedata,
                      const uint64_t filesize,
                      const P3ACompressionType filedataCompressionType) {
    compressionType = filedataCompressionType;
    compressedSize = filesize;
    hash = XXH64(filedata, filesize, 0);
    if (file.Write(filedata, filesize) != filesize) {
        return false;
    }
    return true;
}

static bool WriteUncompressed(HyoutaUtils::IO::File& file,
                              P3ACompressionType& compressionType,
                              uint64_t& compressedSize,
                              uint64_t& hash,
                              const P3APackFile& fileinfo,
                              const char* filedata,
                              const uint64_t uncompressedSize) {
    if (fileinfo.IsPrecompressed()) {
        // TODO: this is doable, decompress first
        return false;
    }

    return WriteData(file,
                     compressionType,
                     compressedSize,
                     hash,
                     filedata,
                     uncompressedSize,
                     P3ACompressionType::None);
}

static bool MultithreadedWriteP3AFiles(HyoutaUtils::IO::File& file,
                                       P3AFileInfo* fileHeaders, // same length as fileinfos
                                       const std::vector<P3APackFile>& fileinfos,
                                       const uint64_t alignment,
                                       uint64_t& position,
                                       uint64_t sizePerFileInfo,
                                       uint64_t positionFileInfoStart,
                                       const ZSTD_CDict* cdict,
                                       bool allowUppercase,
                                       size_t numberOfFilesThatNeedCompressing,
                                       size_t threadCount) {
    // This is designed like this:
    // - threadCount worker threads, which do the actual compression of the files
    // - 1 file reader thread, which reads the input files and hands them over to a worker thread
    // - 1 file writer thread, which takes the output of the worker thread and writes them to the
    //   output file

    // TODO: This generally works, but uses more memory that it really needs to because the reader
    // thread just reads all files as fast as it can.

    std::atomic<bool> encounteredError = false;

    struct alignas(64) FileDataToWrite {
        bool IsValid = false;
        const char* Data;
        uint64_t CompressedSize;
        uint64_t UncompressedSize;
        std::unique_ptr<char[]> Holder;
        P3ACompressionType CompressionType;
        uint64_t UncompressedHash;
    };
    auto fileDataToWriteArray = std::make_unique<FileDataToWrite[]>(fileinfos.size());
    std::mutex fileDataToWriteMutex;
    std::condition_variable fileDataToWriteCondVar;
    bool fileDataToWriteAbort = false;

    struct alignas(64) FileDataToCompress {
        const char* Data;
        uint64_t Size;
        std::unique_ptr<char[]> Holder;
        size_t Index;
        P3ACompressionType CompressionType;
    };
    std::deque<FileDataToCompress> fileDataToCompressQueue;
    std::mutex fileDataToCompressMutex;
    std::condition_variable fileDataToCompressCondVar;
    size_t fileDataToCompressLeft = numberOfFilesThatNeedCompressing;

    const auto do_abort = [&]() -> void {
        encounteredError.store(true);

        // break out of file writer thread
        {
            std::unique_lock lock(fileDataToWriteMutex);
            fileDataToWriteAbort = true;
            fileDataToWriteCondVar.notify_all();
        }

        // break out of remaining worker threads
        {
            std::unique_lock lock(fileDataToCompressMutex);
            fileDataToCompressLeft = 0;
            fileDataToCompressCondVar.notify_all();
        }
    };

    // worker threads
    std::vector<std::thread> workerThreads;
    workerThreads.reserve(threadCount);
    for (size_t i = 0; i < threadCount; ++i) {
        workerThreads.emplace_back([&]() -> void {
            while (true) {
                FileDataToCompress workPacket;
                {
                    std::unique_lock lock(fileDataToCompressMutex);
                    if (fileDataToCompressLeft == 0) {
                        return;
                    }
                    if (fileDataToCompressQueue.empty()) {
                        fileDataToCompressCondVar.wait(lock, [&] {
                            return fileDataToCompressLeft == 0 || !fileDataToCompressQueue.empty();
                        });
                        if (fileDataToCompressLeft == 0) {
                            return;
                        }
                    }
                    workPacket = std::move(fileDataToCompressQueue.front());
                    fileDataToCompressQueue.pop_front();
                    --fileDataToCompressLeft;
                }

                // process work packet
                auto compressionResult = CompressForP3A(
                    workPacket.CompressionType, workPacket.Data, workPacket.Size, cdict);
                if (compressionResult.IsSuccess()) {
                    auto& c = compressionResult.GetSuccessValue();
                    const uint64_t uncompressedHash = XXH64(workPacket.Data, workPacket.Size, 0);
                    FileDataToWrite data{.IsValid = true,
                                         .Data = c.Buffer.get(),
                                         .CompressedSize = c.DataLength,
                                         .UncompressedSize = workPacket.Size,
                                         .Holder = std::move(c.Buffer),
                                         .CompressionType = c.CompressionType,
                                         .UncompressedHash = uncompressedHash};

                    {
                        std::unique_lock lock(fileDataToWriteMutex);
                        fileDataToWriteArray[workPacket.Index] = std::move(data);
                        fileDataToWriteCondVar.notify_all();
                    }
                } else {
                    switch (compressionResult.GetErrorValue()) {
                        case P3ACompressionError::ArgumentError:
                        case P3ACompressionError::MemoryAllocationFailure: do_abort(); return;
                        default:
                            // write uncompressed instead
                            const uint64_t uncompressedHash =
                                XXH64(workPacket.Data, workPacket.Size, 0);
                            FileDataToWrite data{.IsValid = true,
                                                 .Data = workPacket.Data,
                                                 .CompressedSize = workPacket.Size,
                                                 .UncompressedSize = workPacket.Size,
                                                 .Holder = std::move(workPacket.Holder),
                                                 .CompressionType = P3ACompressionType::None,
                                                 .UncompressedHash = uncompressedHash};

                            {
                                std::unique_lock lock(fileDataToWriteMutex);
                                fileDataToWriteArray[workPacket.Index] = std::move(data);
                                fileDataToWriteCondVar.notify_all();
                            }
                            break;
                    }
                }
            }
        });
    }

    // file reader thread
    std::thread fileReaderThread = std::thread([&]() -> void {
        for (size_t i = 0; i < fileinfos.size(); ++i) {
            const auto& fileinfo = fileinfos[i];
            if (fileinfo.IsSameAsIndex().has_value()) {
                // nothing to read here
                continue;
            }

            uint64_t filesize = 0u;
            std::unique_ptr<char[]> filedataHolder;
            const char* filedata = GetFileData(filesize, filedataHolder, fileinfo);
            if (!filedata) {
                do_abort();
                return;
            }

            if (!fileinfo.IsPrecompressed()
                && fileinfo.GetDesiredCompressionType() != P3ACompressionType::None) {
                // needs to be compressed, push to worker thread
                FileDataToCompress data{.Data = filedata,
                                        .Size = filesize,
                                        .Holder = std::move(filedataHolder),
                                        .Index = i,
                                        .CompressionType = fileinfo.GetDesiredCompressionType()};

                {
                    std::unique_lock lock(fileDataToCompressMutex);
                    fileDataToCompressQueue.emplace_back(std::move(data));
                    fileDataToCompressCondVar.notify_all();
                }
            } else {
                // can be written as-is to target file, so push to writer thread directly
                const uint64_t uncompressedHash =
                    fileinfo.IsPrecompressed() ? fileinfo.GetDecompressedHashForPrecompressed()
                                               : (XXH64(filedata, filesize, 0));
                FileDataToWrite data{.IsValid = true,
                                     .Data = filedata,
                                     .CompressedSize = filesize,
                                     .UncompressedSize =
                                         fileinfo.IsPrecompressed()
                                             ? fileinfo.GetDecompressedFilesizeForPrecompressed()
                                             : filesize,
                                     .Holder = std::move(filedataHolder),
                                     .CompressionType = fileinfo.GetDesiredCompressionType(),
                                     .UncompressedHash = uncompressedHash};

                {
                    std::unique_lock lock(fileDataToWriteMutex);
                    fileDataToWriteArray[i] = std::move(data);
                    fileDataToWriteCondVar.notify_all();
                }
            }
        }
    });

    // file writer thread
    std::thread fileWriterThread = std::thread([&]() -> void {
        for (size_t i = 0; i < fileinfos.size(); ++i) {
            const auto& fileinfo = fileinfos[i];
            if (fileinfo.IsSameAsIndex().has_value()) {
                // don't write these, we will just copy the header entry afterwards
                continue;
            }

            if (!HyoutaUtils::AlignFile(file, position, alignment)) {
                do_abort();
                return;
            }

            FileDataToWrite data;
            {
                std::unique_lock lock(fileDataToWriteMutex);
                if (fileDataToWriteAbort) {
                    return;
                }
                if (!fileDataToWriteArray[i].IsValid) {
                    fileDataToWriteCondVar.wait(lock, [&] {
                        return fileDataToWriteAbort || fileDataToWriteArray[i].IsValid;
                    });
                    if (fileDataToWriteAbort) {
                        return;
                    }
                }
                data = std::move(fileDataToWriteArray[i]);
            }

            P3ACompressionType compressionType = data.CompressionType;
            uint64_t compressedSize = data.CompressedSize;
            uint64_t compressedHash = 0;
            if (!WriteData(file,
                           compressionType,
                           compressedSize,
                           compressedHash,
                           data.Data,
                           data.CompressedSize,
                           data.CompressionType)) {
                do_abort();
                return;
            }

            // fill in header
            P3AFileInfo& tmp = fileHeaders[i];
            tmp.Filename = NormalizeP3AFilename(fileinfo.GetFilename(), allowUppercase);
            tmp.CompressionType = compressionType;
            tmp.CompressedSize = compressedSize;
            tmp.UncompressedSize = data.UncompressedSize;
            tmp.Offset = position;
            tmp.CompressedHash = compressedHash;
            tmp.UncompressedHash = data.UncompressedHash;
            position += compressedSize;
        }
    });

    // wait for all threads to finish
    for (size_t i = 0; i < threadCount; ++i) {
        workerThreads[i].join();
    }
    fileReaderThread.join();
    fileWriterThread.join();

    return !encounteredError.load();
}

static bool FillHeadersForSameAsFiles(P3AFileInfo* fileHeaders, // same length as fileinfos
                                      const std::vector<P3APackFile>& fileinfos,
                                      bool allowUppercase) {
    for (size_t i = 0; i < fileinfos.size(); ++i) {
        const auto& fileinfo = fileinfos[i];
        auto sameAs = fileinfo.IsSameAsIndex();
        if (!sameAs.has_value()) {
            continue;
        }
        if (*sameAs >= fileinfos.size()) {
            return false;
        }
        if (fileinfos[*sameAs].IsSameAsIndex().has_value()) {
            // reference to a file that doesn't store data is invalid, otherwise we'd need to
            // dependency sort the copy order...
            return false;
        }

        P3AFileInfo& selfHeader = fileHeaders[i];
        P3AFileInfo& otherHeader = fileHeaders[*sameAs];
        selfHeader.Filename = NormalizeP3AFilename(fileinfo.GetFilename(), allowUppercase);
        selfHeader.CompressionType = otherHeader.CompressionType;
        selfHeader.CompressedSize = otherHeader.CompressedSize;
        selfHeader.UncompressedSize = otherHeader.UncompressedSize;
        selfHeader.Offset = otherHeader.Offset;
        selfHeader.CompressedHash = otherHeader.CompressedHash;
        selfHeader.UncompressedHash = otherHeader.UncompressedHash;
    }

    return true;
}

static bool WriteHeaders(HyoutaUtils::IO::File& file,
                         P3AFileInfo* fileHeaders, // same length as fileinfos
                         const std::vector<P3APackFile>& fileinfos,
                         uint64_t sizePerFileInfo,
                         uint64_t positionFileInfoStart) {
    if (!file.SetPosition(positionFileInfoStart)) {
        return false;
    }
    for (size_t i = 0; i < fileinfos.size(); ++i) {
        const P3AFileInfo& tmp = fileHeaders[i];
        if (file.Write(&tmp, sizePerFileInfo) != sizePerFileInfo) {
            return false;
        }
    }
    return true;
}

namespace {
enum class IdentStatus {
    FailedToDetermine,
    IsSame,
    IsNotSame,
};
}

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
static IdentStatus DoFilesHaveSameData(HyoutaUtils::IO::File& lhs, HyoutaUtils::IO::File& rhs) {
    if (!lhs.IsOpen() || !rhs.IsOpen()) {
        return IdentStatus::FailedToDetermine;
    }
    auto size_lhs = lhs.GetLength();
    if (!size_lhs) {
        return IdentStatus::FailedToDetermine;
    }
    auto size_rhs = rhs.GetLength();
    if (!size_rhs) {
        return IdentStatus::FailedToDetermine;
    }
    if (*size_lhs != *size_rhs) {
        return IdentStatus::IsNotSame;
    }

    static constexpr size_t bufferSize = 4096;
    std::array<char, bufferSize> lhs_buffer;
    std::array<char, bufferSize> rhs_buffer;
    uint64_t rest = *size_lhs;
    while (rest > 0) {
        const size_t chunkSize = (rest > bufferSize) ? bufferSize : static_cast<size_t>(rest);
        if (lhs.Read(lhs_buffer.data(), chunkSize) != chunkSize) {
            return IdentStatus::FailedToDetermine;
        }
        if (rhs.Read(rhs_buffer.data(), chunkSize) != chunkSize) {
            return IdentStatus::FailedToDetermine;
        }
        if (std::memcmp(lhs_buffer.data(), rhs_buffer.data(), chunkSize) != 0) {
            return IdentStatus::IsNotSame;
        }

        rest -= chunkSize;
    }

    return IdentStatus::IsSame;
}

static IdentStatus DoFileAndVectorHaveSameData(HyoutaUtils::IO::File& lhs,
                                               const std::vector<char>& rhs) {
    if (!lhs.IsOpen()) {
        return IdentStatus::FailedToDetermine;
    }
    auto size_lhs = lhs.GetLength();
    if (!size_lhs) {
        return IdentStatus::FailedToDetermine;
    }
    if (size_lhs != rhs.size()) {
        return IdentStatus::IsNotSame;
    }

    static constexpr size_t bufferSize = 4096;
    std::array<char, bufferSize> lhs_buffer;
    uint64_t rest = *size_lhs;
    size_t rhs_offset = 0;
    while (rest > 0) {
        const size_t chunkSize = (rest > bufferSize) ? bufferSize : static_cast<size_t>(rest);
        if (lhs.Read(lhs_buffer.data(), chunkSize) != chunkSize) {
            return IdentStatus::FailedToDetermine;
        }
        if (std::memcmp(lhs_buffer.data(), rhs.data() + rhs_offset, chunkSize) != 0) {
            return IdentStatus::IsNotSame;
        }

        rhs_offset += chunkSize;
        rest -= chunkSize;
    }

    return IdentStatus::IsSame;
}
#endif

static IdentStatus IsDuplicate(P3APackFile& lhs, P3APackFile& rhs) {
    if (lhs.GetDesiredCompressionType() != rhs.GetDesiredCompressionType()) {
        // Even if the data is identical, only mark duplicates if the compression type matches.
        return IdentStatus::IsNotSame;
    }

    if (lhs.IsPrecompressed() != rhs.IsPrecompressed()) {
        // TODO: This case is possible to handle.
        return IdentStatus::IsNotSame;
    }

    if (lhs.HasVectorData() && rhs.HasVectorData()) {
        return (lhs.GetVectorData() == rhs.GetVectorData()) ? IdentStatus::IsSame
                                                            : IdentStatus::IsNotSame;
    }

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    if (lhs.HasPathData() && rhs.HasPathData()) {
        HyoutaUtils::IO::File fl(lhs.GetPathData(), HyoutaUtils::IO::OpenMode::Read);
        HyoutaUtils::IO::File fr(rhs.GetPathData(), HyoutaUtils::IO::OpenMode::Read);
        return DoFilesHaveSameData(fl, fr);
    }
    if (lhs.HasPathData() && rhs.HasVectorData()) {
        HyoutaUtils::IO::File f(lhs.GetPathData(), HyoutaUtils::IO::OpenMode::Read);
        return DoFileAndVectorHaveSameData(f, rhs.GetVectorData());
    }
    if (lhs.HasVectorData() && rhs.HasPathData()) {
        HyoutaUtils::IO::File f(rhs.GetPathData(), HyoutaUtils::IO::OpenMode::Read);
        return DoFileAndVectorHaveSameData(f, lhs.GetVectorData());
    }
#endif

    return IdentStatus::FailedToDetermine;
}

std::optional<uint64_t> GetFilesize(P3APackFile& packFile) {
    if (packFile.IsPrecompressed()) {
        return packFile.GetDecompressedFilesizeForPrecompressed();
    }
    if (packFile.HasVectorData()) {
        return packFile.GetVectorData().size();
    }
#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    if (packFile.HasPathData()) {
        return HyoutaUtils::IO::GetFilesize(packFile.GetPathData());
    }
#endif
    return std::nullopt;
}

namespace {
struct SizeWithIndex {
    uint64_t Filesize;
    size_t Index;
};

struct BlockState {
    size_t Begin = 0;
    size_t End = 0;
};
bool GetNextBlock(BlockState& state, const SizeWithIndex* sizes, size_t count) {
    const size_t last = state.End;
    if (last == count) {
        return false;
    }

    state.Begin = last;
    for (size_t i = last + 1; i < count; ++i) {
        if (sizes[last].Filesize != sizes[i].Filesize) {
            state.End = i;
            return true;
        }
    }
    state.End = count;
    return true;
}
} // namespace

bool DeduplicateP3APackFiles(P3APackData& packData) {
    std::vector<P3APackFile>& files = packData.GetMutableFiles();
    if (files.empty()) {
        return true;
    }

    // comparing each file with each is very slow due to n^2, so instead we pre-query each file's
    // filesize, sort by filesize, and then only compare within the blocks with the same filesize.
    auto sizes = std::make_unique_for_overwrite<SizeWithIndex[]>(files.size());
    for (size_t i = 0; i < files.size(); ++i) {
        auto fs = GetFilesize(files[i]);
        if (!fs) {
            return false;
        }
        sizes[i].Filesize = *fs;
        sizes[i].Index = i;
    }
    std::stable_sort(sizes.get(),
                     sizes.get() + files.size(),
                     [](const SizeWithIndex& lhs, const SizeWithIndex& rhs) {
                         return lhs.Filesize < rhs.Filesize;
                     });

    BlockState block;
    while (GetNextBlock(block, sizes.get(), files.size())) {
        const size_t begin = block.Begin;
        const size_t end = block.End;
        for (size_t i = begin; i < end; ++i) {
            auto& fi = files[sizes[i].Index];
            if (fi.IsSameAsIndex().has_value()) {
                continue;
            }
            for (size_t j = begin; j < i; ++j) {
                auto& fj = files[sizes[j].Index];
                if (fj.IsSameAsIndex().has_value()) {
                    continue;
                }
                auto dup = IsDuplicate(fi, fj);
                if (dup == IdentStatus::FailedToDetermine) {
                    return false;
                }
                if (dup == IdentStatus::IsSame) {
                    fi.SetSameAsIndex(sizes[j].Index);
                    break;
                }
            }
        }
    }
    return true;
}

bool PackP3A(HyoutaUtils::IO::File& file, const P3APackData& packData, size_t desiredThreadCount) {
    if (!file.IsOpen()) {
        return false;
    }

    const uint32_t archiveVersion = packData.GetVersion();
    {
        auto it = std::find(SenPatcher::P3ASupportedVersions.begin(),
                            SenPatcher::P3ASupportedVersions.end(),
                            archiveVersion);
        if (it == SenPatcher::P3ASupportedVersions.end()) {
            return false;
        }
    }

    std::unique_ptr<uint8_t[]> dict;
    uint64_t dictLength = 0;
    ZSTD_CDict_UniquePtr cdict = nullptr;
    const bool allowUppercase = packData.IsAllowUppercaseInFilenames();
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
    const uint64_t alignment = packDataAlignment == 0 ? 1 : packDataAlignment;
    const auto& fileinfos = packData.GetFiles();

    uint64_t position = 0;
    {
        P3AHeader header{};
        header.Magic = {{'P', 'H', '3', 'A', 'R', 'C', 'V', '\0'}};
        header.Flags = 0;
        if (dict) {
            header.Flags |= P3AHeaderFlag_HasZstdDict;
        }
        header.Version = archiveVersion;
        header.FileCount = fileinfos.size();
        header.Hash = XXH64(&header, sizeof(P3AHeader) - 8, 0);
        if (file.Write(&header, sizeof(P3AHeader)) != sizeof(P3AHeader)) {
            return false;
        }
        position += sizeof(P3AHeader);
    }

    uint32_t sizePerFileInfo;
    uint32_t sizeExtendedHeader = 0;
    if (archiveVersion == 1100) {
        sizePerFileInfo = SenPatcher::P3AFileInfoSize1100;
    } else if (archiveVersion == 1200) {
        sizePerFileInfo = SenPatcher::P3AFileInfoSize1200;
        sizeExtendedHeader = SenPatcher::P3AExtendedHeaderSize1200;
    } else {
        assert(0);
        return false;
    }

    if (archiveVersion >= 1200) {
        P3AExtendedHeader extHeader{};
        extHeader.Size = sizeExtendedHeader;
        extHeader.FileInfoSize = sizePerFileInfo;
        extHeader.Hash = XXH64(((const char*)&extHeader) + 8, sizeExtendedHeader - 8, 0);
        if (file.Write(&extHeader, sizeExtendedHeader) != sizeExtendedHeader) {
            return false;
        }
        position += sizeExtendedHeader;
    }

    const uint64_t positionFileInfoStart = position;
    {
        // we'll fill in the actual data later
        P3AFileInfo tmp{};
        for (size_t i = 0; i < fileinfos.size(); ++i) {
            if (file.Write(&tmp, sizePerFileInfo) != sizePerFileInfo) {
                return false;
            }
            position += sizePerFileInfo;
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

    size_t threadCount =
        desiredThreadCount == 0 ? std::thread::hardware_concurrency() : desiredThreadCount;
    auto fileHeaders = std::make_unique<P3AFileInfo[]>(fileinfos.size());
    if (threadCount >= 2) {
        size_t numberOfFilesThatNeedCompressing = 0;
        for (size_t i = 0; i < fileinfos.size(); ++i) {
            const auto& fileinfo = fileinfos[i];
            if (!fileinfo.IsSameAsIndex().has_value() && !fileinfo.IsPrecompressed()
                && fileinfo.GetDesiredCompressionType() != P3ACompressionType::None) {
                ++numberOfFilesThatNeedCompressing;
            }
        }
        if (numberOfFilesThatNeedCompressing >= 2) {
            if (!MultithreadedWriteP3AFiles(
                    file,
                    fileHeaders.get(),
                    fileinfos,
                    alignment,
                    position,
                    sizePerFileInfo,
                    positionFileInfoStart,
                    cdict.get(),
                    allowUppercase,
                    numberOfFilesThatNeedCompressing,
                    std::min(numberOfFilesThatNeedCompressing, threadCount))) {
                return false;
            }
            if (!FillHeadersForSameAsFiles(fileHeaders.get(), fileinfos, allowUppercase)) {
                return false;
            }
            if (!WriteHeaders(
                    file, fileHeaders.get(), fileinfos, sizePerFileInfo, positionFileInfoStart)) {
                return false;
            }
            return true;
        }
    }

    for (size_t i = 0; i < fileinfos.size(); ++i) {
        const auto& fileinfo = fileinfos[i];
        if (fileinfo.IsSameAsIndex().has_value()) {
            // don't write these, we will just copy the header entry afterwards
            continue;
        }

        if (!HyoutaUtils::AlignFile(file, position, alignment)) {
            return false;
        }

        uint64_t filesize = 0u;
        std::unique_ptr<char[]> filedataHolder;
        const char* filedata = GetFileData(filesize, filedataHolder, fileinfo);
        if (filedata == nullptr) {
            return false;
        }

        const P3ACompressionType desiredCompressionType = fileinfo.GetDesiredCompressionType();
        P3ACompressionType compressionType = desiredCompressionType;
        uint64_t compressedSize = filesize;
        const uint64_t uncompressedSize = fileinfo.IsPrecompressed()
                                              ? fileinfo.GetDecompressedFilesizeForPrecompressed()
                                              : filesize;
        uint64_t compressedHash = 0;
        uint64_t uncompressedHash = 0;

        if (fileinfo.IsPrecompressed()) {
            if (!WriteData(file,
                           compressionType,
                           compressedSize,
                           compressedHash,
                           filedata,
                           filesize,
                           desiredCompressionType)) {
                return false;
            }
            uncompressedHash = fileinfo.GetDecompressedHashForPrecompressed();
        } else {
            auto compressionResult =
                CompressForP3A(desiredCompressionType, filedata, uncompressedSize, cdict.get());
            if (compressionResult.IsSuccess()) {
                auto& c = compressionResult.GetSuccessValue();
                if (!WriteData(file,
                               compressionType,
                               compressedSize,
                               compressedHash,
                               c.Buffer.get(),
                               c.DataLength,
                               c.CompressionType)) {
                    return false;
                }
                uncompressedHash = XXH64(filedata, uncompressedSize, 0);
            } else {
                switch (compressionResult.GetErrorValue()) {
                    case P3ACompressionError::ArgumentError: return false;
                    case P3ACompressionError::MemoryAllocationFailure: return false;
                    default:
                        if (!WriteUncompressed(file,
                                               compressionType,
                                               compressedSize,
                                               compressedHash,
                                               fileinfo,
                                               filedata,
                                               uncompressedSize)) {
                            return false;
                        }
                        uncompressedHash = compressedHash;
                        break;
                }
            }
        }

        // fill in header
        P3AFileInfo& tmp = fileHeaders[i];
        tmp.Filename = NormalizeP3AFilename(fileinfo.GetFilename(), allowUppercase);
        tmp.CompressionType = compressionType;
        tmp.CompressedSize = compressedSize;
        tmp.UncompressedSize = uncompressedSize;
        tmp.Offset = position;
        tmp.CompressedHash = compressedHash;
        tmp.UncompressedHash = uncompressedHash;
        position += compressedSize;
    }

    if (!FillHeadersForSameAsFiles(fileHeaders.get(), fileinfos, allowUppercase)) {
        return false;
    }
    if (!WriteHeaders(file, fileHeaders.get(), fileinfos, sizePerFileInfo, positionFileInfoStart)) {
        return false;
    }

    return true;
}
} // namespace SenPatcher
