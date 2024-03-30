#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <variant>
#include <vector>

namespace SenPatcher::IO {
struct File;
}

namespace SenPatcher {
enum class P3ACompressionType : uint64_t;

struct P3APackFile {
    std::variant<std::vector<char>, std::filesystem::path> Data = std::vector<char>();
    std::array<char, 0x100> Filename{};
    P3ACompressionType DesiredCompressionType{};

    P3APackFile(std::vector<char> data,
                const std::array<char, 0x100>& filename,
                P3ACompressionType desiredCompressionType);
    P3APackFile(std::filesystem::path path,
                const std::array<char, 0x100>& filename,
                P3ACompressionType desiredCompressionType);
    P3APackFile(const P3APackFile& other);
    P3APackFile(P3APackFile&& other);
    P3APackFile& operator=(const P3APackFile& other);
    P3APackFile& operator=(P3APackFile&& other);
    ~P3APackFile();
};

struct P3APackData {
    size_t Alignment = 0;
    std::variant<std::monostate, std::vector<char>, std::filesystem::path> ZStdDictionary =
        std::monostate();
    std::vector<P3APackFile> Files;

    P3APackData();
    P3APackData(const P3APackData& other);
    P3APackData(P3APackData&& other);
    P3APackData& operator=(const P3APackData& other);
    P3APackData& operator=(P3APackData&& other);
    ~P3APackData();
};

bool PackP3AFromDirectory(const std::filesystem::path& directoryPath,
                          const std::filesystem::path& archivePath,
                          P3ACompressionType desiredCompressionType,
                          const std::filesystem::path& dictPath = std::filesystem::path());
bool PackP3A(const std::filesystem::path& archivePath, const P3APackData& packData);
bool PackP3A(SenPatcher::IO::File& file, const P3APackData& packData);
} // namespace SenPatcher
