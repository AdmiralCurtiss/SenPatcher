#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#ifndef _MSC_VER
#include <string>
#endif

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
#include <filesystem>
#endif

namespace HyoutaUtils::IO {
enum class OpenMode {
    Read,
    Write,
};
enum class SetPositionMode {
    Begin = 0,
    Current = 1,
    End = 2,
};

struct File {
public:
    File() noexcept;
    File(std::string_view p, OpenMode mode) noexcept;
    File(const File& other) = delete;
    File(File&& other) noexcept;
    File& operator=(const File& other) = delete;
    File& operator=(File&& other) noexcept;
    ~File() noexcept;

    bool Open(std::string_view p, OpenMode mode) noexcept;
    bool IsOpen() const noexcept;
    void Close() noexcept;
    std::optional<uint64_t> GetPosition() noexcept;
    bool SetPosition(uint64_t position) noexcept;
    bool SetPosition(int64_t position, SetPositionMode mode) noexcept;
    std::optional<uint64_t> GetLength() noexcept;
    size_t Read(void* data, size_t length) noexcept;
    size_t Write(const void* data, size_t length) noexcept;
    bool Delete() noexcept;
    bool Rename(std::string_view p) noexcept;

    void* ReleaseHandle() noexcept;

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
    File(const std::filesystem::path& p, OpenMode mode) noexcept;
    bool Open(const std::filesystem::path& p, OpenMode mode) noexcept;
    bool Rename(const std::filesystem::path& p) noexcept;
#endif

private:
    void* Filehandle;

#ifndef _MSC_VER
    std::string Path;
#endif
};

bool FileExists(std::string_view p) noexcept;
bool DirectoryExists(std::string_view p) noexcept;
bool CreateDirectory(std::string_view p) noexcept;

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
bool FileExists(const std::filesystem::path& p) noexcept;
bool DirectoryExists(const std::filesystem::path& p) noexcept;
bool CreateDirectory(std::string_view p) noexcept;
#endif
} // namespace HyoutaUtils::IO
