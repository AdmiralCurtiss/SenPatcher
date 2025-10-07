#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

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
enum class ExistsResult {
    DoesExist,
    DoesNotExist,
    AccessDenied,
    UnspecifiedError,
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

#ifdef BUILD_FOR_WINDOWS
    void* ReleaseHandle() noexcept;
#else
    int ReleaseHandle() noexcept;
#endif

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

#ifdef BUILD_FOR_WINDOWS
    void* Filehandle;
#else
    int Filehandle;
    bool IsWritable = false;
    // true if the file was created with O_TMPFILE and does not have a path on disk yet
    bool IsUnlinked = false;
    std::string Path;
#endif
};

ExistsResult Exists(std::string_view p) noexcept;
ExistsResult FileExists(std::string_view p) noexcept;
std::optional<uint64_t> GetFilesize(std::string_view p) noexcept;
ExistsResult DirectoryExists(std::string_view p) noexcept;
bool CreateDirectory(std::string_view p) noexcept;
bool CopyFile(std::string_view source, std::string_view target, bool overwrite = true) noexcept;
bool Move(std::string_view source, std::string_view target, bool overwrite = true) noexcept;
bool DeleteFile(std::string_view path) noexcept;
bool DeleteDirectory(std::string_view path) noexcept; // must be empty

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
ExistsResult Exists(const std::filesystem::path& p) noexcept;
ExistsResult FileExists(const std::filesystem::path& p) noexcept;
std::optional<uint64_t> GetFilesize(const std::filesystem::path& p) noexcept;
ExistsResult DirectoryExists(const std::filesystem::path& p) noexcept;
bool CreateDirectory(const std::filesystem::path& p) noexcept;
bool DeleteFile(const std::filesystem::path& p) noexcept;
bool DeleteDirectory(const std::filesystem::path& p) noexcept; // must be empty

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
SplitPathData SplitPath(std::string_view path) noexcept;

// Inverse of the above: Given a path and a filename, appends the filename to the path.
// This does *not* resolve absolute paths. 'filename' should be a single element, not a full path.
// Effectively a fancy "path += '/' + filename" that works correctly if 'path' already ends with a
// separator and uses backslash on Windows.
void AppendPathElement(std::string& path, std::string_view filename);

// Returns everything but the filename of the given path. Effectively SplitPath(path).Directory
std::string_view GetDirectoryName(std::string_view path) noexcept;

// Returns filename of the given path. Effectively SplitPath(path).Filename
std::string_view GetFileName(std::string_view path) noexcept;

// Returns filename of the given path without extension.
std::string_view GetFileNameWithoutExtension(std::string_view path) noexcept;

// Returns the extension of the given path. This will include the dot.
std::string_view GetExtension(std::string_view path) noexcept;

// Makes the given path absolute.
// This will not resolve links or check whether the path exists at all, it's purely string-based.
// Returns an empty string on failure (broken input path or failure to get working dir).
std::string GetAbsolutePath(std::string_view path);

// Simulates an atomic file write.
// More specifically, opens a temp file, writes to it, then renames that temp file to the given path
// if the write succeeds. The temp file is deleted if the write fails.
// On success, the data will be written to the file at 'path'.
// On failure, the file at 'path' will not have been touched, retaining its previous data.
bool WriteFileAtomic(std::string_view path, const void* data, size_t length) noexcept;

#ifdef BUILD_FOR_WINDOWS
// Gets the logical drives on this computer; that is, stuff like "C:\" and "D:\".
// This concept only exists on Windows, so no Linux implementation is provided.
std::vector<std::string> GetLogicalDrives();
#endif
} // namespace HyoutaUtils::IO
