#ifndef BUILD_FOR_WINDOWS
#define _FILE_OFFSET_BITS 64
#endif

#include "file.h"

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
    auto oldPos = GetPosition();
    if (!oldPos) {
        return std::nullopt;
    }
    if (!SetPosition(0, SetPositionMode::End)) {
        return std::nullopt;
    }
    auto length = GetPosition();
    if (!length) {
        return std::nullopt;
    }
    SetPosition(*oldPos, SetPositionMode::Begin);
    return length;
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
    std::memset(alignedBuffer, 0, allocationLength);
    FILE_RENAME_INFO* info = new (alignedBuffer) FILE_RENAME_INFO;
    auto infoGuard = HyoutaUtils::MakeScopeGuard([&info]() { info->~FILE_RENAME_INFO(); });
    info->ReplaceIfExists = TRUE;
    info->RootDirectory = nullptr;
    info->FileNameLength = static_cast<DWORD>(wstr_len * sizeof(wchar_t));
    std::memcpy(info->FileName, wstr_data, wstr_len * sizeof(wchar_t));
    if (SetFileInformationByHandle(
            filehandle, FileRenameInfo, info, static_cast<DWORD>(allocationLength))
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
static bool FileExistsWindows(const wchar_t* path) {
    const auto attributes = GetFileAttributesW(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}
#else
static bool FileExistsLinux(const char* path) {
    struct stat buf {};
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
static bool DirectoryExistsWindows(const wchar_t* path) {
    const auto attributes = GetFileAttributesW(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
#else
static bool DirectoryExistsLinux(const char* path) {
    struct stat buf {};
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
    return CreateDirectoryW(path, nullptr);
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

#ifdef FILE_WRAPPER_WITH_STD_FILESYSTEM
std::filesystem::path FilesystemPathFromUtf8(std::string_view path) {
    return std::filesystem::path((const char8_t*)path.data(),
                                 (const char8_t*)path.data() + path.size());
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
} // namespace HyoutaUtils::IO
