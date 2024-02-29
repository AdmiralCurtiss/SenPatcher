#include "file.h"

#include <cassert>
#include <cstdint>
#include <filesystem>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace SenPatcher::IO {
File::File(const std::filesystem::path& p, OpenMode mode) noexcept {
    switch (mode) {
        case OpenMode::Read:
            Filehandle = CreateFileW(p.c_str(),
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     nullptr,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);
            break;
        case OpenMode::Write:
            Filehandle = CreateFileW(p.c_str(),
                                     GENERIC_WRITE,
                                     FILE_SHARE_DELETE,
                                     nullptr,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);
            break;
        default: Filehandle = INVALID_HANDLE_VALUE; break;
    }
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

bool File::IsOpen() const noexcept {
    return Filehandle != INVALID_HANDLE_VALUE;
}

void File::Close() noexcept {
    if (IsOpen()) {
        CloseHandle(Filehandle);
        Filehandle = INVALID_HANDLE_VALUE;
    }
}

bool File::SetPosition(uint64_t position) noexcept {
    assert(IsOpen());
    LARGE_INTEGER offset;
    offset.QuadPart = static_cast<LONGLONG>(position);
    if (SetFilePointerEx(Filehandle, offset, nullptr, FILE_BEGIN) != 0) {
        return true;
    }
    return false;
}

std::optional<uint64_t> File::GetLength() noexcept {
    assert(IsOpen());
    LARGE_INTEGER size;
    if (GetFileSizeEx(Filehandle, &size) != 0) {
        return static_cast<uint64_t>(size.QuadPart);
    }
    return std::nullopt;
}

size_t File::Read(void* data, size_t length) {
    assert(IsOpen());

    char* buffer = static_cast<char*>(data);
    size_t totalRead = 0;
    size_t rest = length;
    while (rest > 0) {
        DWORD blockSize = rest > 0xffff'0000 ? 0xffff'0000 : static_cast<DWORD>(rest);
        DWORD blockRead = 0;
        if (ReadFile(Filehandle, buffer, blockSize, &blockRead, nullptr) == 0) {
            return totalRead;
        }
        if (blockRead == 0) {
            return totalRead;
        }

        rest -= blockRead;
        totalRead += blockRead;
        buffer += blockRead;
    }
    return totalRead;
}

size_t File::Write(const void* data, size_t length) {
    assert(IsOpen());

    const char* buffer = static_cast<const char*>(data);
    size_t totalWritten = 0;
    size_t rest = length;
    while (rest > 0) {
        DWORD blockSize = rest > 0xffff'0000 ? 0xffff'0000 : static_cast<DWORD>(rest);
        DWORD blockWritten = 0;
        if (WriteFile(Filehandle, buffer, blockSize, &blockWritten, nullptr) == 0) {
            return totalWritten;
        }
        if (blockWritten == 0) {
            return totalWritten;
        }

        rest -= blockWritten;
        totalWritten += blockWritten;
        buffer += blockWritten;
    }
    return totalWritten;
}

void* File::ReleaseHandle() noexcept {
    void* h = Filehandle;
    Filehandle = INVALID_HANDLE_VALUE;
    return h;
}

} // namespace SenPatcher::IO
