#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#ifndef BUILD_FOR_WINDOWS
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
    uint64_t Write(File& source, uint64_t length) noexcept; // copies up to length bytes from source
    bool Delete() noexcept;
    bool Rename(std::string_view p) noexcept;

    // Like Open(), except that we will not open the file at the given filename, but instead try to
    // find a unused filename in the same directory so that we can later rename the file to the
    // given filename after writes are done. This makes the target file replacement atomic.
    bool OpenWithTempFilename(std::string_view p, OpenMode mode) noexcept;

    void* ReleaseHandle() noexcept;

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
    File(const std::filesystem::path& p, OpenMode mode) noexcept;
    bool Open(const std::filesystem::path& p, OpenMode mode) noexcept;
    bool Rename(const std::filesystem::path& p) noexcept;
#endif

#ifdef BUILD_FOR_WINDOWS
    // Construct a File from an existing Win32 HANDLE.
    // The File instance will take ownership of the handle.
    static File FromHandle(void* handle) noexcept;
#endif

private:
#ifdef BUILD_FOR_WINDOWS
    explicit File(void* handle) noexcept;
#endif

    void* Filehandle;

#ifndef BUILD_FOR_WINDOWS
    std::string Path;
#endif
};

bool FileExists(std::string_view p) noexcept;
std::optional<uint64_t> GetFilesize(std::string_view p) noexcept;
bool DirectoryExists(std::string_view p) noexcept;
bool CreateDirectory(std::string_view p) noexcept;
bool CopyFile(std::string_view source, std::string_view target, bool overwrite = true) noexcept;
bool DeleteFile(std::string_view path) noexcept;
bool DeleteDirectory(std::string_view path) noexcept; // must be empty

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
bool FileExists(const std::filesystem::path& p) noexcept;
std::optional<uint64_t> GetFilesize(const std::filesystem::path& p) noexcept;
bool DirectoryExists(const std::filesystem::path& p) noexcept;
bool CreateDirectory(const std::filesystem::path& p) noexcept;

std::filesystem::path FilesystemPathFromUtf8(std::string_view path);
std::string FilesystemPathToUtf8(const std::filesystem::path& p);
#endif

// Splits a path at the last path separator. No actual checks for anything on disk are performed.
// If there is no path separator, the returned Directory will be empty and the Filename will be the
// given path. If the path ends with a path separator, the Filename component will be empty.
struct SplitPathData {
    std::string_view Directory;
    std::string_view Filename;
};
SplitPathData SplitPath(std::string_view path);

// Simulates an atomic file write.
// More specifically, opens a temp file, writes to it, then renames that temp file to the given path
// if the write succeeds. The temp file is deleted if the write fails.
// On success, the data will be written to the file at 'path'.
// On failure, the file at 'path' will not have been touched, retaining its previous data.
bool WriteFileAtomic(std::string_view path, const void* data, size_t length) noexcept;

// Try to get the directory the currently running executable is in.
// This is NOT the same as the current working directory!
std::optional<std::string> GetCurrentExecutableDirectory() noexcept;
} // namespace HyoutaUtils::IO
