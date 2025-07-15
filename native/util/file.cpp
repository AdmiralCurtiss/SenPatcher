#ifndef BUILD_FOR_WINDOWS
#define _FILE_OFFSET_BITS 64
#endif

#include "file.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
#include <filesystem>
#endif

#include "util/scope.h"
#include "util/text.h"

#ifdef BUILD_FOR_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#undef CreateDirectory
#else
#include <cstdio>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define INVALID_HANDLE_VALUE nullptr
#define DWORD unsigned int
#endif

namespace HyoutaUtils::IO {
File::File() noexcept : Filehandle(INVALID_HANDLE_VALUE) {}

File::File(std::string_view p, OpenMode mode) noexcept : Filehandle(INVALID_HANDLE_VALUE) {
    Open(p, mode);
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
File::File(const std::filesystem::path& p, OpenMode mode) noexcept
  : Filehandle(INVALID_HANDLE_VALUE) {
    Open(p, mode);
}
#endif

#ifdef BUILD_FOR_WINDOWS
File::File(void* handle) noexcept : Filehandle(handle) {}

File File::FromHandle(void* handle) noexcept {
    return File(handle);
}
#endif

File::File(File&& other) noexcept
  : Filehandle(other.Filehandle)
#ifndef BUILD_FOR_WINDOWS
  , Path(std::move(other.Path))
#endif
{
    other.Filehandle = INVALID_HANDLE_VALUE;
}

File& File::operator=(File&& other) noexcept {
    Close();
    this->Filehandle = other.Filehandle;
    other.Filehandle = INVALID_HANDLE_VALUE;
#ifndef BUILD_FOR_WINDOWS
    this->Path = std::move(other.Path);
#endif
    return *this;
}

File::~File() noexcept {
    Close();
}

bool File::Open(std::string_view p, OpenMode mode) noexcept {
    Close();
    switch (mode) {
        case OpenMode::Read: {
#ifdef BUILD_FOR_WINDOWS
            auto s = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
            if (!s) {
                return false;
            }
            Filehandle = CreateFileW(s->c_str(),
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     nullptr,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);
#else
            std::string s(p);
            Filehandle = fopen(s.c_str(), "rb");
            if (Filehandle != INVALID_HANDLE_VALUE) {
                Path = std::move(s);
            }
#endif
            return Filehandle != INVALID_HANDLE_VALUE;
        }
        case OpenMode::Write: {
#ifdef BUILD_FOR_WINDOWS
            auto s = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
            if (!s) {
                return false;
            }
            Filehandle = CreateFileW(s->c_str(),
                                     GENERIC_WRITE | DELETE,
                                     0,
                                     nullptr,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);
#else
            std::string s(p);
            Filehandle = fopen(s.c_str(), "wb");
            if (Filehandle != INVALID_HANDLE_VALUE) {
                Path = std::move(s);
            }
#endif
            return Filehandle != INVALID_HANDLE_VALUE;
        }
        default: Filehandle = INVALID_HANDLE_VALUE; return false;
    }
}

bool File::OpenWithTempFilename(std::string_view p, OpenMode mode) noexcept {
    Close();
    switch (mode) {
        case OpenMode::Read: {
            // This cannot work, we'll never successfully open a not-yet-existing file.
            Filehandle = INVALID_HANDLE_VALUE;
            return false;
        }
        case OpenMode::Write: {
#ifdef BUILD_FOR_WINDOWS
            auto s = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
            if (!s) {
                return false;
            }
            s->append(L".tmp");
            size_t extraCharsAppended = 0;
            size_t loopIndex = 0;
            do {
                Filehandle = CreateFileW(s->c_str(),
                                         GENERIC_WRITE | DELETE,
                                         0,
                                         nullptr,
                                         CREATE_NEW,
                                         FILE_ATTRIBUTE_NORMAL,
                                         nullptr);
                if (Filehandle != INVALID_HANDLE_VALUE) {
                    break;
                }
                if (GetLastError() != ERROR_FILE_EXISTS) {
                    return false;
                }

                // file exists, try next
                // TODO: might be better to RNG something here, or use a timestamp or something?
                for (size_t i = 0; i < extraCharsAppended; ++i) {
                    s->pop_back();
                }
                extraCharsAppended = 0;
                size_t x = loopIndex;
                do {
                    // this counts from the wrong side but whatever...
                    s->push_back(L'0' + (x % 10));
                    x = (x / 10);
                    ++extraCharsAppended;
                } while (x > 0);
                ++loopIndex;
            } while (true);
            return true;
#else
            // TODO: Apparently on modern Linux you can do this much neater with
            // open(O_TMPFILE) + linkat(), but we'd need to use fd instead of a FILE*
            std::string s(p);
            s.append(".tmp");
            size_t extraCharsAppended = 0;
            size_t loopIndex = 0;
            do {
                errno = 0;
                int fd = open(s.c_str(),
                              O_CREAT | O_EXCL | O_WRONLY,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                if (fd != -1) {
                    Filehandle = fdopen(fd, "w");
                    if (Filehandle == INVALID_HANDLE_VALUE) {
                        close(fd);
                        return false;
                    }
                    break;
                }
                if (errno != EEXIST) {
                    return false;
                }

                // file exists, try next
                // TODO: might be better to RNG something here, or use a timestamp or something?
                for (size_t i = 0; i < extraCharsAppended; ++i) {
                    s.pop_back();
                }
                extraCharsAppended = 0;
                size_t x = loopIndex;
                do {
                    // this counts from the wrong side but whatever...
                    s.push_back('0' + (x % 10));
                    x = (x / 10);
                    ++extraCharsAppended;
                } while (x > 0);
                ++loopIndex;
            } while (true);

            Path = std::move(s);
            return true;
#endif
        }
        default: Filehandle = INVALID_HANDLE_VALUE; return false;
    }
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
bool File::Open(const std::filesystem::path& p, OpenMode mode) noexcept {
    Close();
    switch (mode) {
        case OpenMode::Read: {
#ifdef BUILD_FOR_WINDOWS
            Filehandle = CreateFileW(p.c_str(),
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     nullptr,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);
#else
            std::string s(p);
            Filehandle = fopen(s.c_str(), "rb");
            if (Filehandle != INVALID_HANDLE_VALUE) {
                Path = std::move(s);
            }
#endif
            return Filehandle != INVALID_HANDLE_VALUE;
        }
        case OpenMode::Write: {
#ifdef BUILD_FOR_WINDOWS
            Filehandle = CreateFileW(p.c_str(),
                                     GENERIC_WRITE | DELETE,
                                     0,
                                     nullptr,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);
#else
            std::string s(p);
            Filehandle = fopen(s.c_str(), "wb");
            if (Filehandle != INVALID_HANDLE_VALUE) {
                Path = std::move(s);
            }
#endif
            return Filehandle != INVALID_HANDLE_VALUE;
        }
        default: Filehandle = INVALID_HANDLE_VALUE; return false;
    }
}
#endif

bool File::IsOpen() const noexcept {
    return Filehandle != INVALID_HANDLE_VALUE;
}

void File::Close() noexcept {
    if (IsOpen()) {
#ifdef BUILD_FOR_WINDOWS
        CloseHandle(Filehandle);
#else
        fclose((FILE*)Filehandle);
#endif
        Filehandle = INVALID_HANDLE_VALUE;
#ifndef BUILD_FOR_WINDOWS
        Path.clear();
#endif
    }
}

std::optional<uint64_t> File::GetPosition() noexcept {
    assert(IsOpen());
#ifdef BUILD_FOR_WINDOWS
    LARGE_INTEGER offset;
    offset.QuadPart = static_cast<LONGLONG>(0);
    LARGE_INTEGER position;
    if (SetFilePointerEx(Filehandle, offset, &position, FILE_CURRENT) != 0) {
        return static_cast<uint64_t>(position.QuadPart);
    }
#else
    off_t offset = ftello((FILE*)Filehandle);
    if (offset >= 0) {
        return static_cast<uint64_t>(offset);
    }
#endif
    return std::nullopt;
}

bool File::SetPosition(uint64_t position) noexcept {
    if (position > static_cast<uint64_t>(INT64_MAX)) {
        return false;
    }
    return SetPosition(static_cast<int64_t>(position), SetPositionMode::Begin);
}

bool File::SetPosition(int64_t position, SetPositionMode mode) noexcept {
    assert(IsOpen());
#ifdef BUILD_FOR_WINDOWS
    DWORD moveMethod;
    switch (mode) {
        case SetPositionMode::Begin: moveMethod = FILE_BEGIN; break;
        case SetPositionMode::Current: moveMethod = FILE_CURRENT; break;
        case SetPositionMode::End: moveMethod = FILE_END; break;
        default: return false;
    }
    LARGE_INTEGER offset;
    offset.QuadPart = static_cast<LONGLONG>(position);
    if (SetFilePointerEx(Filehandle, offset, nullptr, moveMethod) != 0) {
        return true;
    }
#else
    int origin;
    switch (mode) {
        case SetPositionMode::Begin: origin = SEEK_SET; break;
        case SetPositionMode::Current: origin = SEEK_CUR; break;
        case SetPositionMode::End: origin = SEEK_END; break;
        default: return false;
    }
    int result = fseeko((FILE*)Filehandle, (off_t)position, origin);
    if (result == 0) {
        return true;
    }
#endif
    return false;
}

std::optional<uint64_t> File::GetLength() noexcept {
    assert(IsOpen());
#ifdef BUILD_FOR_WINDOWS
    LARGE_INTEGER size;
    if (GetFileSizeEx(Filehandle, &size) != 0) {
        return static_cast<uint64_t>(size.QuadPart);
    }
#else
    int fd = fileno((FILE*)Filehandle);
    if (fd == -1) {
        return std::nullopt;
    }
    struct stat buf{};
    if (fstat(fd, &buf) != 0) {
        return std::nullopt;
    }
    if (S_ISREG(buf.st_mode)) {
        return static_cast<uint64_t>(buf.st_size);
    }
#endif
    return std::nullopt;
}

size_t File::Read(void* data, size_t length) noexcept {
    assert(IsOpen());

    char* buffer = static_cast<char*>(data);
    size_t totalRead = 0;
    size_t rest = length;
    while (rest > 0) {
        DWORD blockSize = rest > 0xffff'0000 ? 0xffff'0000 : static_cast<DWORD>(rest);
        DWORD blockRead = 0;
#ifdef BUILD_FOR_WINDOWS
        if (ReadFile(Filehandle, buffer, blockSize, &blockRead, nullptr) == 0) {
            return totalRead;
        }
#else
        blockRead = (DWORD)fread(buffer, 1, blockSize, (FILE*)Filehandle);
#endif
        if (blockRead == 0) {
            return totalRead;
        }

        rest -= blockRead;
        totalRead += blockRead;
        buffer += blockRead;
    }
    return totalRead;
}

size_t File::Write(const void* data, size_t length) noexcept {
    assert(IsOpen());

    const char* buffer = static_cast<const char*>(data);
    size_t totalWritten = 0;
    size_t rest = length;
    while (rest > 0) {
        DWORD blockSize = rest > 0xffff'0000 ? 0xffff'0000 : static_cast<DWORD>(rest);
        DWORD blockWritten = 0;
#ifdef BUILD_FOR_WINDOWS
        if (WriteFile(Filehandle, buffer, blockSize, &blockWritten, nullptr) == 0) {
            return totalWritten;
        }
#else
        blockWritten = (DWORD)fwrite(buffer, 1, blockSize, (FILE*)Filehandle);
#endif
        if (blockWritten == 0) {
            return totalWritten;
        }

        rest -= blockWritten;
        totalWritten += blockWritten;
        buffer += blockWritten;
    }
    return totalWritten;
}

uint64_t File::Write(File& source, uint64_t length) noexcept {
    assert(IsOpen());
    assert(source.IsOpen());

    static constexpr size_t bufferSize = 4096;
    std::array<char, bufferSize> buffer;
    uint64_t totalWritten = 0;
    uint64_t rest = length;
    while (rest > 0) {
        DWORD blockSize = (rest > bufferSize) ? bufferSize : static_cast<DWORD>(rest);

        DWORD blockRead = 0;
#ifdef BUILD_FOR_WINDOWS
        if (ReadFile(source.Filehandle, buffer.data(), blockSize, &blockRead, nullptr) == 0) {
            return totalWritten;
        }
#else
        blockRead = (DWORD)fread(buffer.data(), 1, blockSize, (FILE*)source.Filehandle);
#endif
        if (blockRead == 0) {
            return totalWritten;
        }

        DWORD blockWritten = 0;
#ifdef BUILD_FOR_WINDOWS
        if (WriteFile(Filehandle, buffer.data(), blockRead, &blockWritten, nullptr) == 0) {
            return totalWritten;
        }
#else
        blockWritten = (DWORD)fwrite(buffer.data(), 1, blockRead, (FILE*)Filehandle);
#endif
        if (blockWritten != blockRead) {
            return (totalWritten + blockWritten);
        }

        rest -= blockWritten;
        totalWritten += blockWritten;
    }
    return totalWritten;
}

bool File::Delete() noexcept {
    assert(IsOpen());

#ifdef BUILD_FOR_WINDOWS
    FILE_DISPOSITION_INFO info{};
    info.DeleteFile = TRUE;
    if (SetFileInformationByHandle(Filehandle, FileDispositionInfo, &info, sizeof(info)) != 0) {
        return true;
    }
#else
    int result = unlink(Path.c_str());
    if (result == 0) {
        return true;
    }
#endif

    return false;
}

#ifdef BUILD_FOR_WINDOWS
bool RenameInternalWindows(void* filehandle, const wchar_t* wstr_data, size_t wstr_len) {
    // This struct has a very odd definition, because its size is dynamic, so we must do something
    // like this...
    if (wstr_len > 32771) {
        // not sure what the actual limit is, but this is max path length 32767 + '\\?\' prefix
        return false;
    }

    // note that this looks like it has an off-by-one error, but it doesn't, because
    // sizeof(FILE_RENAME_INFO) includes a single WCHAR which accounts for the nullterminator
    size_t structLength = sizeof(FILE_RENAME_INFO) + (wstr_len * sizeof(wchar_t));
    size_t allocationLength = structLength + alignof(FILE_RENAME_INFO);
    void* buffer = std::malloc(allocationLength);
    if (!buffer) {
        return false;
    }
    auto bufferGuard = HyoutaUtils::MakeScopeGuard([&buffer]() { std::free(buffer); });
    void* alignedBuffer = buffer;
    if (std::align(alignof(FILE_RENAME_INFO), structLength, alignedBuffer, allocationLength)
        == nullptr) {
        return false;
    }
    std::memset(alignedBuffer, 0, structLength);
    FILE_RENAME_INFO* info = new (alignedBuffer) FILE_RENAME_INFO;
    auto infoGuard = HyoutaUtils::MakeScopeGuard([&info]() { info->~FILE_RENAME_INFO(); });
    info->ReplaceIfExists = TRUE;
    info->RootDirectory = nullptr;
    info->FileNameLength = static_cast<DWORD>(wstr_len * sizeof(wchar_t));
    std::memcpy(info->FileName, wstr_data, wstr_len * sizeof(wchar_t));
    if (SetFileInformationByHandle(
            filehandle, FileRenameInfo, info, static_cast<DWORD>(structLength))
        != 0) {
        return true;
    }
    return false;
}
#endif

bool File::Rename(const std::string_view p) noexcept {
    assert(IsOpen());

#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
    if (!wstr) {
        return false;
    }
    return RenameInternalWindows(Filehandle, wstr->data(), wstr->size());
#else
    std::string newName(p);
    int result = rename(Path.c_str(), newName.c_str());
    return result == 0;
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
bool File::Rename(const std::filesystem::path& p) noexcept {
    assert(IsOpen());

#ifdef BUILD_FOR_WINDOWS
    const auto& wstr = p.native();
    return RenameInternalWindows(Filehandle, wstr.data(), wstr.size());
#else
    int result = rename(Path.c_str(), p.c_str());
    return result == 0;
#endif
}
#endif

void* File::ReleaseHandle() noexcept {
    void* h = Filehandle;
    Filehandle = INVALID_HANDLE_VALUE;
#ifndef BUILD_FOR_WINDOWS
    Path.clear();
#endif
    return h;
}

#ifdef BUILD_FOR_WINDOWS
static bool AnyExistsWindows(const wchar_t* path) {
    const auto attributes = GetFileAttributesW(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return true;
}
#else
static bool AnyExistsLinux(const char* path) {
    struct stat buf{};
    if (stat(path, &buf) != 0) {
        return false;
    }
    return true;
}
#endif

bool Exists(std::string_view p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
    if (!wstr) {
        return false;
    }
    return AnyExistsWindows(wstr->data());
#else
    std::string s(p);
    return AnyExistsLinux(s.c_str());
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
bool Exists(const std::filesystem::path& p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return AnyExistsWindows(p.native().data());
#else
    return AnyExistsLinux(p.c_str());
#endif
}
#endif

#ifdef BUILD_FOR_WINDOWS
static bool FileExistsWindows(const wchar_t* path) {
    const auto attributes = GetFileAttributesW(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}
#else
static bool FileExistsLinux(const char* path) {
    struct stat buf{};
    if (stat(path, &buf) != 0) {
        return false;
    }
    if (S_ISREG(buf.st_mode)) {
        return true;
    }
    return false;
}
#endif

bool FileExists(std::string_view p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
    if (!wstr) {
        return false;
    }
    return FileExistsWindows(wstr->data());
#else
    std::string s(p);
    return FileExistsLinux(s.c_str());
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
bool FileExists(const std::filesystem::path& p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return FileExistsWindows(p.native().data());
#else
    return FileExistsLinux(p.c_str());
#endif
}
#endif

#ifdef BUILD_FOR_WINDOWS
static std::optional<uint64_t> GetFilesizeWindows(const wchar_t* path) {
    WIN32_FILE_ATTRIBUTE_DATA data{};
    const auto rv = GetFileAttributesExW(path, GetFileExInfoStandard, &data);
    if (rv == 0) {
        return std::nullopt;
    }
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        return (static_cast<uint64_t>(data.nFileSizeHigh) << 32)
               | static_cast<uint64_t>(data.nFileSizeLow);
    }
    // is a directory
    return std::nullopt;
}
#else
static std::optional<uint64_t> GetFilesizeLinux(const char* path) {
    struct stat buf{};
    if (stat(path, &buf) != 0) {
        return std::nullopt;
    }
    if (S_ISREG(buf.st_mode)) {
        return static_cast<uint64_t>(buf.st_size);
    }
    return std::nullopt;
}
#endif

std::optional<uint64_t> GetFilesize(std::string_view p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
    if (!wstr) {
        return std::nullopt;
    }
    return GetFilesizeWindows(wstr->data());
#else
    std::string s(p);
    return GetFilesizeLinux(s.c_str());
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
std::optional<uint64_t> GetFilesize(const std::filesystem::path& p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return GetFilesizeWindows(p.native().data());
#else
    return GetFilesizeLinux(p.c_str());
#endif
}
#endif

#ifdef BUILD_FOR_WINDOWS
static bool DirectoryExistsWindows(const wchar_t* path) {
    const auto attributes = GetFileAttributesW(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
#else
static bool DirectoryExistsLinux(const char* path) {
    struct stat buf{};
    if (stat(path, &buf) != 0) {
        return false;
    }
    if (S_ISDIR(buf.st_mode)) {
        return true;
    }
    return false;
}
#endif

bool DirectoryExists(std::string_view p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
    if (!wstr) {
        return false;
    }
    return DirectoryExistsWindows(wstr->data());
#else
    std::string s(p);
    return DirectoryExistsLinux(s.c_str());
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
bool DirectoryExists(const std::filesystem::path& p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return DirectoryExistsWindows(p.native().data());
#else
    return DirectoryExistsLinux(p.c_str());
#endif
}
#endif

#ifdef BUILD_FOR_WINDOWS
static bool CreateDirectoryWindows(const wchar_t* path) {
    if (CreateDirectoryW(path, nullptr)) {
        return true;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return true;
    }
    return false;
}
#else
static bool CreateDirectoryLinux(const char* path) {
    if (DirectoryExistsLinux(path)) {
        return true;
    }
    int result = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
    if (result == 0) {
        return true;
    }
    return false;
}
#endif

bool CreateDirectory(std::string_view p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
    if (!wstr) {
        return false;
    }
    return CreateDirectoryWindows(wstr->data());
#else
    std::string s(p);
    return CreateDirectoryLinux(s.c_str());
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
bool CreateDirectory(const std::filesystem::path& p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return CreateDirectoryWindows(p.native().data());
#else
    return CreateDirectoryLinux(p.c_str());
#endif
}
#endif

#if defined(BUILD_FOR_WINDOWS) && defined(CopyFile)
#undef CopyFile
#endif
bool CopyFile(std::string_view source, std::string_view target, bool overwrite) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wsource = HyoutaUtils::TextUtils::Utf8ToWString(source.data(), source.size());
    if (!wsource) {
        return false;
    }
    auto wtarget = HyoutaUtils::TextUtils::Utf8ToWString(target.data(), target.size());
    if (!wtarget) {
        return false;
    }
    return CopyFileExW(wsource->c_str(),
                       wtarget->c_str(),
                       nullptr,
                       nullptr,
                       nullptr,
                       static_cast<DWORD>(overwrite ? 0 : COPY_FILE_FAIL_IF_EXISTS))
           != 0;
#else
    std::string sourcePath(source);
    std::string targetPath(target);

    // try to link() first, that will succeed in many cases
    errno = 0;
    if (link(sourcePath.c_str(), targetPath.c_str()) == 0) {
        return true;
    }
    if (errno == EEXIST) {
        if (!overwrite) {
            return false;
        }
        // we do want to overwrite, so delete the file and try again
        if (unlink(targetPath.c_str()) != 0) {
            return false;
        }
        if (link(sourcePath.c_str(), targetPath.c_str()) == 0) {
            return true;
        }
    }

    // link() failed, do a naive file copy instead
    HyoutaUtils::IO::File sourceFile(source, HyoutaUtils::IO::OpenMode::Read);
    if (!sourceFile.IsOpen()) {
        return false;
    }
    auto length = sourceFile.GetLength();
    if (!length) {
        return false;
    }
    HyoutaUtils::IO::File targetFile(target, HyoutaUtils::IO::OpenMode::Write);
    if (!targetFile.IsOpen()) {
        return false;
    }
    if (targetFile.Write(sourceFile, *length) != *length) {
        targetFile.Delete();
        return false;
    }
    return true;
#endif
}

#if defined(BUILD_FOR_WINDOWS) && defined(MoveFile)
#undef MoveFile
#endif
bool Move(std::string_view source, std::string_view target, bool overwrite) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wsource = HyoutaUtils::TextUtils::Utf8ToWString(source.data(), source.size());
    if (!wsource) {
        return false;
    }
    auto wtarget = HyoutaUtils::TextUtils::Utf8ToWString(target.data(), target.size());
    if (!wtarget) {
        return false;
    }
    return MoveFileExW(wsource->c_str(),
                       wtarget->c_str(),
                       static_cast<DWORD>(MOVEFILE_COPY_ALLOWED
                                          | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0)))
           != 0;
#else
    std::string sourcePath(source);
    std::string targetPath(target);
    int result = renameat2(AT_FDCWD,
                           sourcePath.c_str(),
                           AT_FDCWD,
                           targetPath.c_str(),
                           overwrite ? 0u : RENAME_NOREPLACE);
    return result == 0;
#endif
}

#if defined(BUILD_FOR_WINDOWS) && defined(DeleteFile)
#undef DeleteFile
#endif

#if defined(BUILD_FOR_WINDOWS)
bool DeleteFileWindows(const wchar_t* path) noexcept {
    if (DeleteFileW(path) != 0) {
        return true;
    }
    if (GetLastError() != ERROR_ACCESS_DENIED) {
        return false;
    }

    // DeleteFileW() fails on read-only files.
    // Check if this is the case, remove the attribute, and retry.
    WIN32_FILE_ATTRIBUTE_DATA data{};
    const auto rv = GetFileAttributesExW(path, GetFileExInfoStandard, &data);
    if (rv == 0) {
        return false;
    }
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        // is a directory
        return false;
    }
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == 0) {
        // not read-only
        return false;
    }
    if (SetFileAttributesW(path, data.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY) == 0) {
        return false;
    }
    return DeleteFileW(path) != 0;
}
#else
bool DeleteFileLinux(const char* path) noexcept {
    int result = unlink(path);
    return result == 0;
}
#endif

bool DeleteFile(std::string_view path) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(path.data(), path.size());
    if (!wstr) {
        return false;
    }
    return DeleteFileWindows(wstr->c_str());
#else
    std::string p(path);
    return DeleteFileLinux(p.c_str());
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
bool DeleteFile(const std::filesystem::path& p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return DeleteFileWindows(p.c_str());
#else
    return DeleteFileLinux(p.c_str());
#endif
}
#endif

bool DeleteDirectory(std::string_view path) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(path.data(), path.size());
    if (!wstr) {
        return false;
    }
    return RemoveDirectoryW(wstr->c_str()) != 0;
#else
    std::string p(path);
    int result = rmdir(p.c_str());
    return result == 0;
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
bool DeleteDirectory(const std::filesystem::path& p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return RemoveDirectoryW(p.c_str()) != 0;
#else
    int result = rmdir(p.c_str());
    return result == 0;
#endif
}
#endif

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
std::filesystem::path FilesystemPathFromUtf8(std::string_view path) {
#ifdef BUILD_FOR_WINDOWS
    // convert forward slashes to backslashes, some Windows functions need that
    std::array<char8_t, 1024> stackBuffer;
    std::unique_ptr<char8_t[]> heapBuffer;
    char8_t* buffer;
    if (path.size() <= stackBuffer.size()) {
        buffer = stackBuffer.data();
    } else {
        heapBuffer = std::make_unique_for_overwrite<char8_t[]>(path.size());
        buffer = heapBuffer.get();
        if (!buffer) {
            assert(0); // uuuh...
        }
    }
    for (size_t i = 0; i < path.size(); ++i) {
        char c = path[i];
        buffer[i] = (c == '/') ? u8'\\' : static_cast<char8_t>(c);
    }
    return std::filesystem::path(buffer, buffer + path.size());
#else
    return std::filesystem::path((const char8_t*)path.data(),
                                 (const char8_t*)path.data() + path.size());
#endif
}

std::string FilesystemPathToUtf8(const std::filesystem::path& p) {
#ifdef BUILD_FOR_WINDOWS
    const auto& w = p.native();
    auto utf8 = HyoutaUtils::TextUtils::WStringToUtf8(w.data(), w.size());
    if (utf8) {
        return *utf8;
    }
    assert(0); // this should never happen
    return std::string();
#else
    return p.native();
#endif
}
#endif

SplitPathData SplitPath(std::string_view path) {
    size_t lastPathSep = path.find_last_of(
#ifdef BUILD_FOR_WINDOWS
        "\\/"
#else
        '/'
#endif
    );

    SplitPathData result;
    if (lastPathSep != std::string_view::npos) {
        result.Directory = path.substr(0, lastPathSep);
        result.Filename = path.substr(lastPathSep + 1);
    } else {
        result.Filename = path;
    }
    return result;
}

void AppendPathElement(std::string& path, std::string_view filename) {
    const bool alreadyEndsInSeparator = !path.empty()
                                        && (path.back() == '/'
#ifdef BUILD_FOR_WINDOWS
                                            || path.back() == '\\'
#endif
                                        );
    if (!alreadyEndsInSeparator) {
#ifdef BUILD_FOR_WINDOWS
        path.push_back('\\');
#else
        path.push_back('/');
#endif
    }
    path.append(filename);
}

std::string_view GetDirectoryName(std::string_view path) {
    return SplitPath(path).Directory;
}

std::string_view GetFileName(std::string_view path) {
    return SplitPath(path).Filename;
}

std::string_view GetFileNameWithoutExtension(std::string_view path) {
    std::string_view name = GetFileName(path);
    size_t pos = name.rfind('.');
    if (pos == std::string_view::npos) {
        return name;
    }
    return name.substr(0, pos);
}

std::string_view GetExtension(std::string_view path) {
    std::string_view name = GetFileName(path);
    size_t pos = name.rfind('.');
    if (pos == std::string_view::npos) {
        return std::string_view();
    }
    return name.substr(pos);
}

std::string GetAbsolutePath(std::string_view path) {
    std::string result;
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(path.data(), path.size());
    if (!wstr) {
        return result;
    }
    DWORD length = GetFullPathNameW(wstr->c_str(), 0, nullptr, nullptr);
    if (length <= 1) {
        return result;
    }
    DWORD length2;
    std::unique_ptr<wchar_t[]> wresult;
    do {
        length2 = length;
        wresult = std::make_unique_for_overwrite<wchar_t[]>(length + 1);
        length = GetFullPathNameW(wstr->c_str(), length + 1, wresult.get(), nullptr);
    } while (length > length2);
    auto utf8 = HyoutaUtils::TextUtils::WStringToUtf8(wresult.get(), length);
    if (!utf8) {
        return result;
    }
    result = std::move(*utf8);
#endif
    return result;
}

bool WriteFileAtomic(std::string_view path, const void* data, size_t length) noexcept {
    HyoutaUtils::IO::File outfile;
    if (!outfile.OpenWithTempFilename(path, HyoutaUtils::IO::OpenMode::Write)) {
        return false;
    }
    auto outfileScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { outfile.Delete(); });
    if (outfile.Write(data, length) != length) {
        return false;
    }
    if (!outfile.Rename(path)) {
        return false;
    }
    outfileScope.Dispose();
    return true;
}

#ifdef BUILD_FOR_WINDOWS
std::vector<std::string> GetLogicalDrives() noexcept {
    std::vector<std::string> result;
    DWORD bufferSize = GetLogicalDriveStringsW(0, nullptr);
    if (bufferSize == 0) {
        return result;
    }
    std::array<wchar_t, 1024> stackBuffer;
    std::unique_ptr<wchar_t[]> heapBuffer;
    wchar_t* buffer;
    if (bufferSize <= stackBuffer.size()) {
        buffer = stackBuffer.data();
        bufferSize = static_cast<DWORD>(stackBuffer.size());
    } else {
        heapBuffer = std::make_unique_for_overwrite<wchar_t[]>(bufferSize);
        buffer = heapBuffer.get();
        if (!buffer) {
            assert(0);
            return result;
        }
    }
    const DWORD bufferSize2 = GetLogicalDriveStringsW(bufferSize, buffer);
    if (bufferSize2 > bufferSize) {
        // can this happen?
        return result;
    }

    DWORD pos = 0;
    for (DWORD i = 0; i < bufferSize2; ++i) {
        if (buffer[i] == L'\0') {
            auto utf8 = HyoutaUtils::TextUtils::WStringToUtf8(&buffer[pos], i - pos);
            if (utf8 && !utf8->empty()) {
                result.emplace_back(std::move(*utf8));
            }
            pos = i + 1;
        }
    }

    return result;
}
#endif
} // namespace HyoutaUtils::IO
