#ifndef _MSC_VER
#define _FILE_OFFSET_BITS 64
#endif

#include "file.h"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <memory>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <cstdio>

#define INVALID_HANDLE_VALUE nullptr
#define DWORD unsigned int
#endif

namespace SenPatcher::IO {
File::File() noexcept : Filehandle(INVALID_HANDLE_VALUE) {}

File::File(const std::filesystem::path& p, OpenMode mode) noexcept
  : Filehandle(INVALID_HANDLE_VALUE) {
    Open(p, mode);
}

File::File(File&& other) noexcept : Filehandle(other.Filehandle) {
    other.Filehandle = INVALID_HANDLE_VALUE;
}

File& File::operator=(File&& other) noexcept {
    Close();
    this->Filehandle = other.Filehandle;
    other.Filehandle = INVALID_HANDLE_VALUE;
    return *this;
}

File::~File() noexcept {
    Close();
}

bool File::Open(const std::filesystem::path& p, OpenMode mode) noexcept {
    Close();
    switch (mode) {
        case OpenMode::Read:
#ifdef _MSC_VER
            Filehandle = CreateFileW(p.c_str(),
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     nullptr,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);
#else
            Filehandle = fopen(p.c_str(), "rb");
#endif
            return Filehandle != INVALID_HANDLE_VALUE;
        case OpenMode::Write:
#ifdef _MSC_VER
            Filehandle = CreateFileW(p.c_str(),
                                     GENERIC_WRITE | DELETE,
                                     0,
                                     nullptr,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);
#else
            Filehandle = fopen(p.c_str(), "wb");
#endif
            return Filehandle != INVALID_HANDLE_VALUE;
        default: Filehandle = INVALID_HANDLE_VALUE; return false;
    }
}

bool File::IsOpen() const noexcept {
    return Filehandle != INVALID_HANDLE_VALUE;
}

void File::Close() noexcept {
    if (IsOpen()) {
#ifdef _MSC_VER
        CloseHandle(Filehandle);
#else
        fclose((FILE*)Filehandle);
#endif
        Filehandle = INVALID_HANDLE_VALUE;
    }
}

std::optional<uint64_t> File::GetPosition() noexcept {
    assert(IsOpen());
#ifdef _MSC_VER
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
#ifdef _MSC_VER
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
#ifdef _MSC_VER
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
#ifdef _MSC_VER
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
#ifdef _MSC_VER
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

#ifdef _MSC_VER
    FILE_DISPOSITION_INFO info{};
    info.DeleteFile = TRUE;
    if (SetFileInformationByHandle(Filehandle, FileDispositionInfo, &info, sizeof(info)) != 0) {
        return true;
    }
#else
    // TODO: How do you delete a file by handle in POSIX?
#endif

    return false;
}

bool File::Rename(const std::filesystem::path& p) noexcept {
    assert(IsOpen());

#ifdef _MSC_VER
    // This struct has a very odd definition, because its size is dynamic, so we must do something
    // like this...
    const auto& wstr = p.native();
    size_t allocationLength = sizeof(FILE_RENAME_INFO) + (wstr.size() * sizeof(char16_t));
    auto buffer = std::make_unique<char[]>(allocationLength);
    FILE_RENAME_INFO* info = reinterpret_cast<FILE_RENAME_INFO*>(buffer.get());
    info->ReplaceIfExists = TRUE;
    info->RootDirectory = nullptr;
    info->FileNameLength = wstr.size() * 2;
    std::memcpy(info->FileName, wstr.data(), wstr.size() * sizeof(char16_t));
    if (SetFileInformationByHandle(Filehandle, FileRenameInfo, info, allocationLength) != 0) {
        return true;
    }
#else
    // TODO: How do you rename a file by handle in POSIX?
#endif

    return false;
}

void* File::ReleaseHandle() noexcept {
    void* h = Filehandle;
    Filehandle = INVALID_HANDLE_VALUE;
    return h;
}

} // namespace SenPatcher::IO
