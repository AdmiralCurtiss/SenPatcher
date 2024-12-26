#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
#include <filesystem>
#endif

#include "util/result.h"

namespace HyoutaUtils::IO {
struct File;
}

namespace SenPatcher {
enum class P3ACompressionType : uint64_t;

enum class P3APackFilePrecompressed : bool { No, Yes };

struct P3APackFile {
    P3APackFile(std::vector<char> data,
                const std::array<char, 0x100>& filename,
                P3ACompressionType desiredCompressionType,
                P3APackFilePrecompressed precompressed = P3APackFilePrecompressed::No,
                uint64_t decompressedFilesize = 0, // ignored if not precompressed
                uint64_t decompressedHash = 0      // ignored if not precompressed
    );
    P3APackFile(const P3APackFile& other) = delete;
    P3APackFile(P3APackFile&& other);
    P3APackFile& operator=(const P3APackFile& other) = delete;
    P3APackFile& operator=(P3APackFile&& other);
    ~P3APackFile();

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    P3APackFile(std::filesystem::path path,
                const std::array<char, 0x100>& filename,
                P3ACompressionType desiredCompressionType,
                P3APackFilePrecompressed precompressed = P3APackFilePrecompressed::No,
                uint64_t decompressedFilesize = 0, // ignored if not precompressed
                uint64_t decompressedHash = 0      // ignored if not precompressed
    );
#endif

    const std::array<char, 0x100>& GetFilename() const;
    void SetFilename(const std::array<char, 0x100>& filename);
    P3ACompressionType GetDesiredCompressionType() const;
    bool IsPrecompressed() const;
    uint64_t GetDecompressedFilesizeForPrecompressed() const;
    uint64_t GetDecompressedHashForPrecompressed() const;

    bool HasVectorData() const;
    const std::vector<char>& GetVectorData() const;
    void SetVectorData(std::vector<char> data);

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    bool HasPathData() const;
    const std::filesystem::path& GetPathData() const;
#endif

    // This file may be a duplicate of another file that is already being packed.
    // If so, this can be set so that only one copy is stored.
    // For N copies of a file, there must be 1 that does have this set as std::nullopt,
    // and N-1 that all have it set to the index of the file that has the std::nullopt.
    std::optional<size_t> IsSameAsIndex() const;
    void SetSameAsIndex(std::optional<size_t> index);

private:
    struct Impl;
    std::unique_ptr<Impl> Data;
};

struct P3APackData {
    P3APackData();
    P3APackData(const P3APackData& other) = delete;
    P3APackData(P3APackData&& other);
    P3APackData& operator=(const P3APackData& other) = delete;
    P3APackData& operator=(P3APackData&& other);
    ~P3APackData();

    uint32_t GetVersion() const;
    void SetVersion(uint32_t version);
    size_t GetAlignment() const;
    void SetAlignment(size_t alignment);
    const std::vector<P3APackFile>& GetFiles() const;
    std::vector<P3APackFile>& GetMutableFiles();

    void ClearZStdDictionaryData();

    bool HasZStdDictionaryVectorData() const;
    const std::vector<char>& GetZStdDictionaryVectorData() const;
    void SetZStdDictionaryVectorData(std::vector<char> data);

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    bool HasZStdDictionaryPathData() const;
    const std::filesystem::path& GetZStdDictionaryPathData() const;
    void SetZStdDictionaryPathData(std::filesystem::path path);
#endif

    bool IsAllowUppercaseInFilenames() const;
    void SetAllowUppercaseInFilenames(bool allowUppercase);

private:
    struct Impl;
    std::unique_ptr<Impl> Data;
};

// Replaces uppercase with lowercase, replaces backslash with forward slash for path separator, and
// strips data past the first null byte.
std::array<char, 0x100> NormalizeP3AFilename(const std::array<char, 0x100>& filename,
                                             bool allowUppercase = false);

struct P3ACompressionResult {
    std::unique_ptr<char[]> Buffer;
    size_t DataLength;
    size_t BufferLength;
    P3ACompressionType CompressionType;

    P3ACompressionResult() = default;
    P3ACompressionResult(const P3ACompressionResult& other) = delete;
    P3ACompressionResult(P3ACompressionResult&& other) = default;
    P3ACompressionResult& operator=(const P3ACompressionResult& other) = delete;
    P3ACompressionResult& operator=(P3ACompressionResult&& other) = default;
    ~P3ACompressionResult() = default;
};
enum class P3ACompressionError {
    InvalidCompressionType,
    InvalidUncompressedSize,
    CompressionError,
    ArgumentError,
    MemoryAllocationFailure,
};
HyoutaUtils::Result<P3ACompressionResult, P3ACompressionError>
    CompressForP3A(P3ACompressionType desiredCompressionType,
                   const char* filedata,
                   uint64_t uncompressedSize,
                   const void* zstdCDict = nullptr);

bool DeduplicateP3APackFiles(P3APackData& packData);

bool PackP3A(HyoutaUtils::IO::File& file, const P3APackData& packData, size_t desiredThreadCount);
} // namespace SenPatcher
