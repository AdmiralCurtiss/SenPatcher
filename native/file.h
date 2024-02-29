#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

namespace SenPatcher::IO {
enum class OpenMode {
    Read,
    Write,
};

struct File {
public:
    File(const std::filesystem::path& p, OpenMode mode) noexcept;
    File(const File& other) = delete;
    File(File&& other) noexcept;
    File& operator=(const File& other) = delete;
    File& operator=(File&& other) noexcept;
    ~File() noexcept;

    bool IsOpen() const noexcept;
    void Close() noexcept;
    bool SetPosition(uint64_t position) noexcept;
    std::optional<uint64_t> GetLength() noexcept;
    size_t Read(void* data, size_t length);
    size_t Write(const void* data, size_t length);

    void* ReleaseHandle() noexcept;

private:
    void* Filehandle;
};
} // namespace SenPatcher::IO
