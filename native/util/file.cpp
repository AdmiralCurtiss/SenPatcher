#ifndef BUILD_FOR_WINDOWS
#define _FILE_OFFSET_BITS 64
#endif

#include "file.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
#include <filesystem>
#include <stdexcept>
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

#define INVALID_HANDLE_VALUE (-1)
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
  , IsWritable(other.IsWritable)
  , IsUnlinked(other.IsUnlinked)
  , Path(std::move(other.Path))
#endif
{
    other.Filehandle = INVALID_HANDLE_VALUE;
#ifndef BUILD_FOR_WINDOWS
    other.IsWritable = false;
    other.IsUnlinked = false;
#endif
}

File& File::operator=(File&& other) noexcept {
    Close();
    this->Filehandle = other.Filehandle;
    other.Filehandle = INVALID_HANDLE_VALUE;
#ifndef BUILD_FOR_WINDOWS
    this->IsWritable = other.IsWritable;
    other.IsWritable = false;
    this->IsUnlinked = other.IsUnlinked;
    other.IsUnlinked = false;
    this->Path = std::move(other.Path);
#endif
    return *this;
}

File::~File() noexcept {
    Close();
}

#ifndef BUILD_FOR_WINDOWS
static bool IsFdRegularFile(int fd) noexcept {
    struct stat buf{};
    if (fstat(fd, &buf) != 0) {
        return false;
    }
    if (S_ISREG(buf.st_mode)) {
        return true;
    }
    return false;
}
#endif

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
            std::string s;
            try {
                s.assign(p);
            } catch (...) {
                return false;
            }
            Filehandle = open(s.c_str(), O_RDONLY | O_CLOEXEC);
            if (Filehandle != INVALID_HANDLE_VALUE && !IsFdRegularFile(Filehandle)) {
                close(Filehandle);
                Filehandle = INVALID_HANDLE_VALUE;
            }
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
            std::string s;
            try {
                s.assign(p);
            } catch (...) {
                return false;
            }
            Filehandle = open(s.c_str(),
                              O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            if (Filehandle != INVALID_HANDLE_VALUE) {
                IsWritable = true;
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
            try {
                s->append(L".tmp");
            } catch (...) {
                return false;
            }
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
                try {
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
                } catch (...) {
                    return false;
                }
            } while (true);
            return true;
#else
            // try O_TMPFILE first
            std::string s;
            try {
                s.assign(p);
                while (s.size() > 0 && s.back() != '/') {
                    s.pop_back();
                }
                if (s.ends_with('/')) {
                    s.pop_back();
                }
                if (s.empty()) {
                    s.push_back('.');
                }
            } catch (...) {
                return false;
            }
            {
                int fd = open(s.c_str(),
                              O_TMPFILE | O_WRONLY,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                if (fd != -1) {
                    Filehandle = fd;
                    IsWritable = true;
                    IsUnlinked = true;
                    return true;
                }
            }


            try {
                s.assign(p);
                s.append(".tmp");
            } catch (...) {
                return false;
            }
            size_t extraCharsAppended = 0;
            size_t loopIndex = 0;
            do {
                errno = 0;
                int fd = open(s.c_str(),
                              O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                if (fd != INVALID_HANDLE_VALUE) {
                    Filehandle = fd;
                    break;
                }
                if (errno != EEXIST) {
                    return false;
                }

                // file exists, try next
                // TODO: might be better to RNG something here, or use a timestamp or something?
                try {
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
                } catch (...) {
                    return false;
                }
            } while (true);

            Path = std::move(s);
            IsWritable = true;
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
            std::string s;
            try {
                s.assign(p);
            } catch (...) {
                return false;
            }
            Filehandle = open(s.c_str(), O_RDONLY | O_CLOEXEC);
            if (Filehandle != INVALID_HANDLE_VALUE && !IsFdRegularFile(Filehandle)) {
                close(Filehandle);
                Filehandle = INVALID_HANDLE_VALUE;
            }
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
            std::string s;
            try {
                s.assign(p);
            } catch (...) {
                return false;
            }
            Filehandle = open(s.c_str(),
                              O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            if (Filehandle != INVALID_HANDLE_VALUE) {
                IsWritable = true;
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
        close(Filehandle);
#endif
        Filehandle = INVALID_HANDLE_VALUE;
#ifndef BUILD_FOR_WINDOWS
        IsWritable = false;
        IsUnlinked = false;
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
    off64_t offset = lseek64(Filehandle, 0, SEEK_CUR);
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
    int whence;
    switch (mode) {
        case SetPositionMode::Begin: whence = SEEK_SET; break;
        case SetPositionMode::Current: whence = SEEK_CUR; break;
        case SetPositionMode::End: whence = SEEK_END; break;
        default: return false;
    }
    off64_t result = lseek64(Filehandle, static_cast<off64_t>(position), whence);
    if (result != static_cast<off64_t>(-1)) {
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
    struct stat buf{};
    if (fstat(Filehandle, &buf) != 0) {
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
        DWORD blockRead = 0;
#ifdef BUILD_FOR_WINDOWS
        DWORD blockSize = rest > 0xffff'0000 ? 0xffff'0000 : static_cast<DWORD>(rest);
        if (ReadFile(Filehandle, buffer, blockSize, &blockRead, nullptr) == 0) {
            return totalRead;
        }
#else
        size_t blockSize = rest > 0x7fff'f000 ? 0x7fff'f000 : rest;
        ssize_t readResult = read(Filehandle, buffer, blockSize);
        if (readResult < 0) {
            return totalRead;
        }
        blockRead = static_cast<DWORD>(readResult);
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
        DWORD blockWritten = 0;
#ifdef BUILD_FOR_WINDOWS
        DWORD blockSize = rest > 0xffff'0000 ? 0xffff'0000 : static_cast<DWORD>(rest);
        if (WriteFile(Filehandle, buffer, blockSize, &blockWritten, nullptr) == 0) {
            return totalWritten;
        }
#else
        size_t blockSize = rest > 0x7fff'f000 ? 0x7fff'f000 : rest;
        ssize_t writeResult = write(Filehandle, buffer, blockSize);
        if (writeResult < 0) {
            return totalWritten;
        }
        blockWritten = static_cast<DWORD>(writeResult);
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
        ssize_t readResult = read(source.Filehandle, buffer.data(), blockSize);
        if (readResult < 0) {
            return totalWritten;
        }
        blockRead = static_cast<DWORD>(readResult);
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
        ssize_t writeResult = write(Filehandle, buffer.data(), blockRead);
        if (writeResult < 0) {
            return totalWritten;
        }
        blockWritten = static_cast<DWORD>(writeResult);
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
    if (IsUnlinked) {
        return true;
    }
    if (!IsWritable) {
        return false;
    }
    int result = unlink(Path.c_str());
    if (result == 0) {
        Path.clear();
        return true;
    }
#endif

    return false;
}

#ifdef BUILD_FOR_WINDOWS
bool RenameInternalWindows(void* filehandle, const wchar_t* wstr_data, size_t wstr_len) noexcept {
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
    if (!IsWritable) {
        return false;
    }
    std::string newName;
    try {
        newName.assign(p);
    } catch (...) {
        return false;
    }
    bool success;
    if (IsUnlinked) {
        int result = linkat(Filehandle, "", AT_FDCWD, newName.c_str(), AT_EMPTY_PATH);
        success = (result == 0);
        if (success) {
            IsUnlinked = false;
        }
    } else {
        int result = rename(Path.c_str(), newName.c_str());
        success = (result == 0);
    }
    if (success) {
        Path = std::move(newName);
        return true;
    }
    return false;
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
bool File::Rename(const std::filesystem::path& p) noexcept {
    assert(IsOpen());

#ifdef BUILD_FOR_WINDOWS
    const auto& wstr = p.native();
    return RenameInternalWindows(Filehandle, wstr.data(), wstr.size());
#else
    return Rename(std::string_view(p.native()));
#endif
}
#endif

#ifdef BUILD_FOR_WINDOWS
void* File::ReleaseHandle() noexcept {
    void* h = Filehandle;
    Filehandle = INVALID_HANDLE_VALUE;
    return h;
}
#else
int File::ReleaseHandle() noexcept {
    int h = Filehandle;
    Filehandle = INVALID_HANDLE_VALUE;
    Path.clear();
    return h;
}
#endif

#ifdef BUILD_FOR_WINDOWS
static ExistsResult AnyExistsWindows(const wchar_t* path) noexcept {
    const auto attributes = GetFileAttributesW(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        DWORD lastError = GetLastError();
        switch (lastError) {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND: return ExistsResult::DoesNotExist;
            case ERROR_ACCESS_DENIED: return ExistsResult::AccessDenied;
            default: return ExistsResult::UnspecifiedError;
        }
    }
    return ExistsResult::DoesExist;
}
#else
static ExistsResult AnyExistsLinux(const char* path) noexcept {
    struct stat buf{};
    errno = 0;
    if (fstatat(AT_FDCWD, path, &buf, AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT) != 0) {
        switch (errno) {
            case ENOENT:
            case ENOTDIR: return ExistsResult::DoesNotExist;
            case EACCES: return ExistsResult::AccessDenied;
            default: return ExistsResult::UnspecifiedError;
        }
    }
    return ExistsResult::DoesExist;
}
#endif

ExistsResult Exists(std::string_view p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
    if (!wstr) {
        return ExistsResult::UnspecifiedError;
    }
    return AnyExistsWindows(wstr->data());
#else
    std::string s;
    try {
        s.assign(p);
    } catch (...) {
        return ExistsResult::UnspecifiedError;
    }
    return AnyExistsLinux(s.c_str());
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
ExistsResult Exists(const std::filesystem::path& p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return AnyExistsWindows(p.native().data());
#else
    return AnyExistsLinux(p.c_str());
#endif
}
#endif

#ifdef BUILD_FOR_WINDOWS
static ExistsResult FileExistsWindows(const wchar_t* path) noexcept {
    const auto attributes = GetFileAttributesW(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        DWORD lastError = GetLastError();
        switch (lastError) {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND: return ExistsResult::DoesNotExist;
            case ERROR_ACCESS_DENIED: return ExistsResult::AccessDenied;
            default: return ExistsResult::UnspecifiedError;
        }
    }
    return ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) ? ExistsResult::DoesExist
                                                          : ExistsResult::DoesNotExist;
}
#else
static ExistsResult FileExistsLinux(const char* path) noexcept {
    struct stat buf{};
    errno = 0;
    if (fstatat(AT_FDCWD, path, &buf, AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT) != 0) {
        switch (errno) {
            case ENOENT:
            case ENOTDIR: return ExistsResult::DoesNotExist;
            case EACCES: return ExistsResult::AccessDenied;
            default: return ExistsResult::UnspecifiedError;
        }
    }
    if (S_ISREG(buf.st_mode)) {
        return ExistsResult::DoesExist;
    }
    return ExistsResult::DoesNotExist;
}
#endif

ExistsResult FileExists(std::string_view p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
    if (!wstr) {
        return ExistsResult::UnspecifiedError;
    }
    return FileExistsWindows(wstr->data());
#else
    std::string s;
    try {
        s.assign(p);
    } catch (...) {
        return ExistsResult::UnspecifiedError;
    }
    return FileExistsLinux(s.c_str());
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
ExistsResult FileExists(const std::filesystem::path& p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return FileExistsWindows(p.native().data());
#else
    return FileExistsLinux(p.c_str());
#endif
}
#endif

#ifdef BUILD_FOR_WINDOWS
static std::optional<uint64_t> GetFilesizeWindows(const wchar_t* path) noexcept {
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
static std::optional<uint64_t> GetFilesizeLinux(const char* path) noexcept {
    struct stat buf{};
    if (fstatat(AT_FDCWD, path, &buf, AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT) != 0) {
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
    std::string s;
    try {
        s.assign(p);
    } catch (...) {
        return std::nullopt;
    }
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
static ExistsResult DirectoryExistsWindows(const wchar_t* path) noexcept {
    const auto attributes = GetFileAttributesW(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        DWORD lastError = GetLastError();
        switch (lastError) {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND: return ExistsResult::DoesNotExist;
            case ERROR_ACCESS_DENIED: return ExistsResult::AccessDenied;
            default: return ExistsResult::UnspecifiedError;
        }
    }
    return ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ? ExistsResult::DoesExist
                                                          : ExistsResult::DoesNotExist;
}
#else
static ExistsResult DirectoryExistsLinux(const char* path) noexcept {
    struct stat buf{};
    errno = 0;
    if (fstatat(AT_FDCWD, path, &buf, AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT) != 0) {
        switch (errno) {
            case ENOENT:
            case ENOTDIR: return ExistsResult::DoesNotExist;
            case EACCES: return ExistsResult::AccessDenied;
            default: return ExistsResult::UnspecifiedError;
        }
    }
    if (S_ISDIR(buf.st_mode)) {
        return ExistsResult::DoesExist;
    }
    return ExistsResult::DoesNotExist;
}
#endif

ExistsResult DirectoryExists(std::string_view p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(p.data(), p.size());
    if (!wstr) {
        return ExistsResult::UnspecifiedError;
    }
    return DirectoryExistsWindows(wstr->data());
#else
    std::string s;
    try {
        s.assign(p);
    } catch (...) {
        return ExistsResult::UnspecifiedError;
    }
    return DirectoryExistsLinux(s.c_str());
#endif
}

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
ExistsResult DirectoryExists(const std::filesystem::path& p) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return DirectoryExistsWindows(p.native().data());
#else
    return DirectoryExistsLinux(p.c_str());
#endif
}
#endif

#ifdef BUILD_FOR_WINDOWS
static bool CreateDirectoryWindows(const wchar_t* path) noexcept {
    if (CreateDirectoryW(path, nullptr)) {
        return true;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // this is returned even if the thing at path is a file,
        // so we need to check if it's really a directory
        // FIXME: is there a way to do this atomically?
        return DirectoryExistsWindows(path) == ExistsResult::DoesExist;
    }
    return false;
}
#else
static bool CreateDirectoryLinux(const char* path) noexcept {
    ExistsResult exists = DirectoryExistsLinux(path);
    if (exists == ExistsResult::DoesExist) {
        return true;
    }
    if (exists == ExistsResult::DoesNotExist) {
        int result = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
        if (result == 0) {
            return true;
        }
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
    std::string s;
    try {
        s.assign(p);
    } catch (...) {
        return false;
    }
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
    std::string sourcePath;
    std::string targetPath;
    try {
        sourcePath.assign(source);
        targetPath.assign(target);
    } catch (...) {
        return false;
    }

    // try to link() first, that will succeed in many cases
    errno = 0;
    if (link(sourcePath.c_str(), targetPath.c_str()) == 0) {
        return true;
    }
    if (errno == EEXIST && !overwrite) {
        return false;
    }

    // link() failed, now it gets complicated.
    // to prevent file loss we first need to check whether the two paths refer to the same file,
    // because if yes everything we'll try will have catastrophic results
    struct stat sourceBuf{};
    if (fstatat(AT_FDCWD, sourcePath.c_str(), &sourceBuf, AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT)
        != 0) {
        // source doesn't exist or is inaccessible
        return false;
    }
    if (!(S_ISREG(sourceBuf.st_mode) || S_ISLNK(sourceBuf.st_mode))) {
        // source is not a file
        return false;
    }
    struct stat targetBuf{};
    errno = 0;
    if (fstatat(AT_FDCWD, targetPath.c_str(), &targetBuf, AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT)
        == 0) {
        // target exists
        if (!overwrite) {
            return false;
        }

        if (!(S_ISREG(targetBuf.st_mode) || S_ISLNK(targetBuf.st_mode))) {
            // target is not a file
            return false;
        }

        // is this the same file as source?
        if (sourceBuf.st_dev == targetBuf.st_dev && sourceBuf.st_ino == targetBuf.st_ino) {
            // yes, bail
            return false;
        }

        // delete the old file and retry the link
        if (unlink(targetPath.c_str()) != 0) {
            return false;
        }
        if (link(sourcePath.c_str(), targetPath.c_str()) == 0) {
            return true;
        }
    } else {
        // only proceed in regular error cases
        switch (errno) {
            case ENOENT:
            case ENOTDIR: break;
            default: return false;
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
    std::string sourcePath;
    std::string targetPath;
    try {
        sourcePath.assign(source);
        targetPath.assign(target);
    } catch (...) {
        return false;
    }
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
    std::string s;
    try {
        s.assign(path);
    } catch (...) {
        return false;
    }
    return DeleteFileLinux(s.c_str());
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
    std::string s;
    try {
        s.assign(path);
    } catch (...) {
        return false;
    }
    int result = rmdir(s.c_str());
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
    // this can happen if the memory allocation failed, I guess we just throw?
    throw std::runtime_error("failed to convert path to UTF8");
#else
    return p.native();
#endif
}
#endif

SplitPathData SplitPath(std::string_view path) noexcept {
    size_t lastPathSep = path.find_last_of(
#ifdef BUILD_FOR_WINDOWS
        "\\/"
#else
        '/'
#endif
    );

    SplitPathData result;
    if (lastPathSep != std::string_view::npos) {
#ifdef BUILD_FOR_WINDOWS
        const bool isRootDir =
            (lastPathSep == 2
             && ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z'))
             && path[1] == ':');
#else
        const bool isRootDir = (lastPathSep == 0);
#endif
        result.Directory = path.substr(0, lastPathSep + (isRootDir ? 1 : 0));
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

std::string_view GetDirectoryName(std::string_view path) noexcept {
    return SplitPath(path).Directory;
}

std::string_view GetFileName(std::string_view path) noexcept {
    return SplitPath(path).Filename;
}

std::string_view GetFileNameWithoutExtension(std::string_view path) noexcept {
    std::string_view name = GetFileName(path);
    size_t pos = name.rfind('.');
    if (pos == std::string_view::npos) {
        return name;
    }
    return name.substr(0, pos);
}

std::string_view GetExtension(std::string_view path) noexcept {
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
#else
    if (path.empty()) {
        return result;
    }
    std::string_view p = path;
    if (!p.starts_with('/')) {
        // relative path in 'p', we need to get the working dir
        std::array<char, 1024> stackBuffer;
        char* cwd = getcwd(stackBuffer.data(), stackBuffer.size());
        if (cwd == nullptr) {
            // POSIX doesn't deign to tell us how large the buffer it actually wants is,
            // so just retry until it succeeds...
            std::vector<char> heapBuffer;
            heapBuffer.resize(stackBuffer.size() * 2);
            while (true) {
                cwd = getcwd(heapBuffer.data(), heapBuffer.size());
                if (cwd != nullptr) {
                    break;
                }
                if (heapBuffer.size() >= 0x1000'0000u) {
                    // sanity exit in case something goes really wrong...
                    return result;
                }
                heapBuffer.resize(heapBuffer.size() * 2);
            }
        }
        if (cwd[0] != '/') {
            // did not get a valid path
            return result;
        }
        result.assign(cwd);
        while (result.ends_with('/')) {
            result.pop_back();
        }
        if (result.empty()) {
            result.push_back('/');
        }
    } else {
        // absolute (but possibly not normalized) path in 'p'
        result.assign("/");
        while (p.starts_with('/')) {
            // drop consecutive path separators
            p = p.substr(1);
        }
    }

    // we now have the start of an absolute path in 'result' and a relative path in 'p'
    // now we just go through all path elements of 'p' and append them onto 'result'
    while (true) {
        assert(result.size() >= 1);
        assert(result.front() == '/');

        const size_t idx = p.find_first_of('/');
        std::string_view element;
        if (idx == std::string::npos) {
            // last path element
            element = p;
        } else {
            element = p.substr(0, idx);
            p = p.substr(idx + 1);
            while (p.starts_with('/')) {
                // drop consecutive path separators
                p = p.substr(1);
            }
        }

        if (element == ".") {
            // nop element, don't do anything
        } else if (element == "..") {
            // go up once, unless we're at root
            if (result.size() > 1) {
                const size_t next =
                    std::string_view(result).substr(0, result.size() - 1).find_last_of('/');
                result.resize(next == static_cast<size_t>(0) ? static_cast<size_t>(1) : next);
            }
        } else {
            // append the element
            if (result.back() != '/') {
                result.push_back('/');
            }
            result.append(element);
        }

        if (idx == std::string::npos) {
            break;
        }
    }
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
std::vector<std::string> GetLogicalDrives() {
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
