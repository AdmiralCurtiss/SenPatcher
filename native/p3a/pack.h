#pragma once

#define P3A_PACKER_WITH_STD_FILESYSTEM

#include <array>
#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
#include <filesystem>
#endif

namespace SenPatcher::IO {
struct File;
}

namespace SenPatcher {
enum class P3ACompressionType : uint64_t;

struct P3APackFile {
    P3APackFile(std::vector<char> data,
                const std::array<char, 0x100>& filename,
                P3ACompressionType desiredCompressionType);
    P3APackFile(const P3APackFile& other) = delete;
    P3APackFile(P3APackFile&& other);
    P3APackFile& operator=(const P3APackFile& other) = delete;
    P3APackFile& operator=(P3APackFile&& other);
    ~P3APackFile();

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    P3APackFile(std::filesystem::path path,
                const std::array<char, 0x100>& filename,
                P3ACompressionType desiredCompressionType);
#endif

    const std::array<char, 0x100>& GetFilename() const;
    P3ACompressionType GetDesiredCompressionType() const;

    bool HasVectorData() const;
    const std::vector<char>& GetVectorData() const;

#ifdef P3A_PACKER_WITH_STD_FILESYSTEM
    bool HasPathData() const;
    const std::filesystem::path& GetPathData() const;
#endif

private:
    struct Impl;
    std::unique_ptr<Impl> Data;
};

struct P3APackData {
    size_t Alignment = 0;
    std::variant<std::monostate, std::vector<char>, std::filesystem::path> ZStdDictionary =
        std::monostate();
    std::vector<P3APackFile> Files;

    P3APackData();
    P3APackData(const P3APackData& other) = delete;
    P3APackData(P3APackData&& other);
    P3APackData& operator=(const P3APackData& other) = delete;
    P3APackData& operator=(P3APackData&& other);
    ~P3APackData();
};

bool PackP3A(SenPatcher::IO::File& file, const P3APackData& packData);
} // namespace SenPatcher
