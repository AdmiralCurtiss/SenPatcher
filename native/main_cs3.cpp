#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "lz4/lz4.h"
#include "zstd/common/xxhash.h"
#include "zstd/zdict.h"
#include "zstd/zstd.h"

#include "crc32.h"

#include "file.h"
#include "logger.h"

#include "sen3/file_fixes.h"

#include "p3a/p3a.h"
#include "p3a/structs.h"

#include "x64/emitter.h"

namespace {
struct ZSTD_DCtx_Deleter {
    void operator()(ZSTD_DCtx* ptr) {
        if (ptr) {
            ZSTD_freeDCtx(ptr);
        }
    }
};
using ZSTD_DCtx_UniquePtr = std::unique_ptr<ZSTD_DCtx, ZSTD_DCtx_Deleter>;

enum class GameVersion {
    Unknown,
    English,
    Japanese,
};

struct PageUnprotect {
    SenPatcher::Logger& Log;
    void* Address;
    size_t Length;
    DWORD Attributes;

    PageUnprotect(SenPatcher::Logger& logger, void* addr, size_t length) : Log(logger) {
        // FIXME: check length/alignment, this might span multiple pages!
        Length = 0x1000;
        Address = reinterpret_cast<void*>(reinterpret_cast<uint64_t>(addr) & (~(Length - 1)));
        Log.Log("Unprotecting ").LogHex(Length).Log(" bytes at ").LogPtr(Address);
        if (VirtualProtect(Address, Length, PAGE_READWRITE, &Attributes)) {
            Log.Log(" -> Success, previous attributes were ").LogHex(Attributes).Log(".\n");
        } else {
            Log.Log(" -> Failed.\n");
        }
    }

    ~PageUnprotect() {
        DWORD tmp;
        Log.Log("Reprotecting ").LogHex(Length).Log(" bytes at ").LogPtr(Address);
        Log.Log(" to attributes ").LogHex(Attributes);
        if (VirtualProtect(Address, Length, Attributes, &tmp)) {
            Log.Log(" -> Success.\n");
        } else {
            Log.Log(" -> Failed.\n");
        }
    }
};
} // namespace

static int SelectOffset(GameVersion version, int en, int jp) {
    switch (version) {
        case GameVersion::English: return en;
        case GameVersion::Japanese: return jp;
        default: return 0;
    }
}

using PDirectInput8Create =
    HRESULT (*)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, void* punkOuter);
static PDirectInput8Create LoadForwarderAddress(SenPatcher::Logger& logger) {
    constexpr int total = 5000;
    WCHAR tmp[total];
    UINT count = GetSystemDirectoryW(tmp, sizeof(tmp) / sizeof(WCHAR));
    if (count == 0 || count > total - 16) {
        logger.Log("Failed constructing path for system dinput8.dll.\n");
        return nullptr;
    }
    memcpy(tmp + count, L"\\dinput8.dll\0", sizeof(L"\\dinput8.dll\0"));

    HMODULE dll = ::LoadLibraryW(tmp);
    if (!dll) {
        logger.Log("Failed loading system dinput8.dll.\n");
        return nullptr;
    }
    void* addr = ::GetProcAddress(dll, "DirectInput8Create");
    if (!addr) {
        logger.Log("Failed finding system DirectInput8Create.\n");
    } else {
        logger.Log("Found system DirectInput8Create at ").LogPtr(addr).Log(".\n");
    }
    return (PDirectInput8Create)addr;
}

static void WriteFloat(SenPatcher::Logger& logger, void* addr, float value) {
    logger.Log("Writing float ").LogFloat(value).Log(" to ").LogPtr(addr).Log(".\n");
    PageUnprotect unprotect(logger, addr, 4);
    memcpy(addr, &value, 4);
}

static void WriteInt(SenPatcher::Logger& logger, void* addr, int value) {
    logger.Log("Writing int ").LogInt(value).Log(" to ").LogPtr(addr).Log(".\n");
    PageUnprotect unprotect(logger, addr, 4);
    memcpy(addr, &value, 4);
}

static void WriteByte(SenPatcher::Logger& logger, void* addr, char value) {
    logger.Log("Writing byte ").LogHex(value).Log(" to ").LogPtr(addr).Log(".\n");
    PageUnprotect unprotect(logger, addr, 1);
    memcpy(addr, &value, 1);
}

static char* Align16CodePage(SenPatcher::Logger& logger, void* new_page) {
    logger.Log("Aligning ").LogPtr(new_page).Log(" to 16 bytes.\n");
    char* p = reinterpret_cast<char*>(new_page);
    *p++ = 0xcc;
    while ((reinterpret_cast<unsigned long long>(p) & 0xf) != 0) {
        *p++ = 0xcc;
    }
    return p;
}

static GameVersion FindImageBase(SenPatcher::Logger& logger, void** code) {
    GameVersion gameVersion = GameVersion::Unknown;
    MEMORY_BASIC_INFORMATION info;
    memset(&info, 0, sizeof(info));
    *code = nullptr;
    for (unsigned long long address = 0; address < 0x80000000000; address += info.RegionSize) {
        if (VirtualQuery(reinterpret_cast<void*>(address), &info, sizeof(info)) == 0) {
            break;
        }

        if (info.State == MEM_COMMIT && info.Type == MEM_IMAGE) {
            logger.Log("Allocation at ")
                .LogPtr(info.AllocationBase)
                .Log(", base ptr ")
                .LogPtr(info.BaseAddress)
                .Log(", size ")
                .LogHex(info.RegionSize)
                .Log(", protection ")
                .LogHex(info.Protect)
                .Log(".\n");
            if ((*code == 0) && info.RegionSize == 0x7e9000 && info.Protect == PAGE_EXECUTE_READ) {
                // crc_t crc = crc_init();
                // crc = crc_update(crc, info.BaseAddress, info.RegionSize);
                // crc = crc_finalize(crc);
                // logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                logger.Log("Appears to be the EN version.\n");
                *code = info.BaseAddress;
                gameVersion = GameVersion::English;
            } else if ((*code == 0) && info.RegionSize == 0x7dc000
                       && info.Protect == PAGE_EXECUTE_READ) {
                // crc_t crc = crc_init();
                // crc = crc_update(crc, info.BaseAddress, info.RegionSize);
                // crc = crc_finalize(crc);
                // logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                logger.Log("Appears to be the JP version.\n");
                *code = info.BaseAddress;
                gameVersion = GameVersion::Japanese;
            }

            // logger.Log("First 64 bytes are:");
            // for (int i = 0; i < (info.RegionSize < 64 ? info.RegionSize : 64); ++i) {
            //    logger.Log(" ").LogHex(*(reinterpret_cast<unsigned char*>(info.BaseAddress) + i));
            // }
            // logger.Log("\n");
        }
    }

    return gameVersion;
}

struct FFile {
    void* vfuncptr;
    uint32_t Filesize;
    uint32_t Unknown; // padding between members?
    void* NativeFileHandle;
    void* MemoryPointer;

    // if nonzero it will be free'd when the struct is released
    // should be allocated with the tracking malloc at 0x1405e03f0 (size, 4, 0, 0, 0)
    void* MemoryPointerForFreeing;

    uint32_t MemoryPosition;
};

using PTrackedMalloc = void*(__fastcall*)(uint64_t size,
                                          uint64_t alignment,
                                          const char* file,
                                          uint64_t line,
                                          uint64_t unknown);
using PTrackedFree = void(__fastcall*)(void* memory);

using PMalloc = void* (*)(uint64_t size);
using PFree = void (*)(void* memory);

struct P3AData {
    SenPatcher::P3A Archive;
    std::recursive_mutex Mutex;
};

struct P3AFileRef {
    P3AData* ArchiveData;
    SenPatcher::P3AFileInfo* FileInfo;
};

static PTrackedMalloc s_TrackedMalloc = nullptr;
static PTrackedFree s_TrackedFree = nullptr;
static bool s_CheckDevFolderForAssets = false;
static std::unique_ptr<P3AData[]> s_P3As;
static size_t s_CombinedFileInfoCount = 0;
static std::unique_ptr<P3AFileRef[]> s_CombinedFileInfos;

// so we have a consistent representation: lowercase, singular forward slash as separator
static void FilterP3APath(char8_t* path, size_t length) {
    size_t in = 0;
    size_t out = 0;

    while (in < length) { // in is always >= out so no need to check out
        const char c = path[in];
        if (c == '\0') {
            break;
        }
        if (c == '\\' || c == '/') {
            path[out] = '/';
            ++in;
            ++out;
            while (in < length && (path[in] == '\\' || path[in] == '/')) {
                ++in;
            }
        } else {
            if (c >= 'A' && c <= 'Z') {
                path[out] = c + ('a' - 'A');
            } else {
                path[out] = c;
            }
            ++in;
            ++out;
        }
    }
    if (out < length) {
        path[out] = '\0';
    }
}

// same as above, but slightly different because instead of writing in-place we write to a separate
// array, and also the input is only bounded by nulltermination and not length.
// returns true if the entire input string fit into out_path, false if not
static bool FilterGamePath(char8_t* out_path, const char* in_path, size_t length) {
    size_t in = 0;
    size_t out = 0;
    while (out < length) {
        const char c = in_path[in];
        if (c == '\0') {
            out_path[out] = '\0';
            return true;
        }
        if (c == '\\' || c == '/') {
            out_path[out] = '/';
            ++in;
            ++out;
            while (in_path[in] == '\\' || in_path[in] == '/') {
                ++in;
            }
        } else {
            if (c >= 'A' && c <= 'Z') {
                out_path[out] = c + ('a' - 'A');
            } else {
                out_path[out] = c;
            }
            ++in;
            ++out;
        }
    }
    return in_path[in] == '\0';
}

// ignore any path that doesn't begin with the 'data' directory
static bool IsValidReroutablePath(const char* path) {
    return (path[0] == 'D' || path[0] == 'd') && (path[1] == 'A' || path[1] == 'a')
           && (path[2] == 'T' || path[2] == 't') && (path[3] == 'A' || path[3] == 'a')
           && (path[4] == '/' || path[4] == '\\');
}

static void LoadModP3As(SenPatcher::Logger& logger, const std::filesystem::path& baseDir) {
    s_CombinedFileInfoCount = 0;
    s_CombinedFileInfos.reset();
    s_P3As.reset();

    size_t p3acount = 0;
    std::unique_ptr<P3AData[]> p3as;
    {
        std::vector<SenPatcher::P3A> p3avector;
        std::error_code ec;
        s_CheckDevFolderForAssets = std::filesystem::is_directory(baseDir / L"dev", ec);
        std::filesystem::directory_iterator iterator(baseDir / L"mods", ec);
        if (ec) {
            return;
        }
        for (auto const& entry : iterator) {
            if (entry.is_directory()) {
                continue;
            }

            SenPatcher::P3A& p3a = p3avector.emplace_back();
            if (!p3a.Load(entry.path())) {
                p3avector.pop_back();
            }
        }

        p3acount = p3avector.size();
        p3as = std::make_unique<P3AData[]>(p3acount);
        for (size_t i = 0; i < p3acount; ++i) {
            p3as[i].Archive = std::move(p3avector[i]);
        }
    }

    size_t totalFileInfoCount = 0;
    for (size_t i = 0; i < p3acount; ++i) {
        totalFileInfoCount += p3as[i].Archive.FileCount;
    }

    // now to make a binary tree from all files from all archives
    std::unique_ptr<P3AFileRef[]> combinedFileInfos;
    if (totalFileInfoCount > 0) {
        // first just stuff them all into a long array
        combinedFileInfos = std::make_unique<P3AFileRef[]>(totalFileInfoCount);
        size_t index = 0;
        for (size_t i = 0; i < p3acount; ++i) {
            auto& p3a = p3as[i];
            const size_t localFileInfoCount = p3a.Archive.FileCount;
            for (size_t j = 0; j < localFileInfoCount; ++j) {
                auto& fileinfo = p3a.Archive.FileInfo[j];
                combinedFileInfos[index].ArchiveData = &p3a;
                combinedFileInfos[index].FileInfo = &fileinfo;
                FilterP3APath(fileinfo.Filename.data(), fileinfo.Filename.size());
                ++index;
            }
        }

        // sort by filename
        std::stable_sort(combinedFileInfos.get(),
                         combinedFileInfos.get() + totalFileInfoCount,
                         [](const P3AFileRef& lhs, const P3AFileRef& rhs) {
                             return strncmp((const char*)lhs.FileInfo->Filename.data(),
                                            (const char*)rhs.FileInfo->Filename.data(),
                                            lhs.FileInfo->Filename.size())
                                    < 0;
                         });

        // drop identical filenames, preferring the first instance
        {
            size_t remainingFileInfoCount = totalFileInfoCount;
            size_t in = 1;
            size_t out = 0;
            while (in < totalFileInfoCount) {
                const P3AFileRef& last = combinedFileInfos[out];
                const P3AFileRef& next = combinedFileInfos[in];
                if (strncmp((const char*)last.FileInfo->Filename.data(),
                            (const char*)next.FileInfo->Filename.data(),
                            last.FileInfo->Filename.size())
                    == 0) {
                    --remainingFileInfoCount;
                } else {
                    const P3AFileRef tmp = next;
                    ++out;
                    combinedFileInfos[out] = tmp;
                }
                ++in;
            }
            totalFileInfoCount = remainingFileInfoCount;
        }
    }

    s_P3As = std::move(p3as);
    s_CombinedFileInfoCount = totalFileInfoCount;
    s_CombinedFileInfos = std::move(combinedFileInfos);

    return;
}

static const P3AFileRef* FindP3AFileRef(const std::array<char8_t, 0x100>& filteredPath) {
    const size_t count = s_CombinedFileInfoCount;
    const P3AFileRef* const infos = s_CombinedFileInfos.get();
    auto bound = std::lower_bound(infos,
                                  infos + count,
                                  filteredPath,
                                  [](const P3AFileRef& lhs, const std::array<char8_t, 0x100>& rhs) {
                                      return strncmp((const char*)lhs.FileInfo->Filename.data(),
                                                     (const char*)rhs.data(),
                                                     rhs.size())
                                             < 0;
                                  });

    if (bound != (infos + count)
        && strncmp((const char*)bound->FileInfo->Filename.data(),
                   (const char*)filteredPath.data(),
                   filteredPath.size())
               == 0) {
        return bound;
    }

    return nullptr;
}

static bool ExtractP3AFileToMemory(const P3AFileRef& ref,
                                   uint64_t filesizeLimit,
                                   void*& out_memory,
                                   uint64_t& out_filesize,
                                   PMalloc malloc_func,
                                   PFree free_func) {
    const SenPatcher::P3AFileInfo& fi = *ref.FileInfo;
    if (fi.UncompressedSize >= filesizeLimit) {
        return false;
    }

    switch (fi.CompressionType) {
        case SenPatcher::P3ACompressionType::None: {
            auto& archiveData = *ref.ArchiveData;
            void* memory = malloc_func(fi.UncompressedSize);
            if (!memory) {
                return false;
            }

            {
                std::lock_guard guard(archiveData.Mutex);
                auto& file = archiveData.Archive.FileHandle;
                if (!file.SetPosition(fi.Offset)) {
                    free_func(memory);
                    return false;
                }
                if (file.Read(memory, fi.UncompressedSize) != fi.UncompressedSize) {
                    free_func(memory);
                    return false;
                }
            }

            if (fi.Hash != XXH64(memory, fi.UncompressedSize, 0)) {
                free_func(memory);
                return false;
            }

            out_memory = memory;
            out_filesize = fi.UncompressedSize;
            return true;
        }
        case SenPatcher::P3ACompressionType::LZ4: {
            auto& archiveData = *ref.ArchiveData;
            void* memory = malloc_func(fi.UncompressedSize);
            if (!memory) {
                return false;
            }
            auto compressedMemory = std::make_unique_for_overwrite<char[]>(fi.CompressedSize);
            if (!compressedMemory) {
                free_func(memory);
                return false;
            }

            {
                std::lock_guard guard(archiveData.Mutex);
                auto& file = archiveData.Archive.FileHandle;
                if (!file.SetPosition(fi.Offset)) {
                    compressedMemory.reset();
                    free_func(memory);
                    return false;
                }
                if (file.Read(compressedMemory.get(), fi.CompressedSize) != fi.CompressedSize) {
                    compressedMemory.reset();
                    free_func(memory);
                    return false;
                }
            }

            if (fi.Hash != XXH64(compressedMemory.get(), fi.CompressedSize, 0)) {
                compressedMemory.reset();
                free_func(memory);
                return false;
            }

            if (LZ4_decompress_safe(compressedMemory.get(),
                                    static_cast<char*>(memory),
                                    fi.CompressedSize,
                                    fi.UncompressedSize)
                != fi.UncompressedSize) {
                compressedMemory.reset();
                free_func(memory);
                return false;
            }
            compressedMemory.reset();

            out_memory = memory;
            out_filesize = fi.UncompressedSize;
            return true;
        }
        case SenPatcher::P3ACompressionType::ZSTD: {
            auto& archiveData = *ref.ArchiveData;
            void* memory = malloc_func(fi.UncompressedSize);
            if (!memory) {
                return false;
            }
            auto compressedMemory = std::make_unique_for_overwrite<char[]>(fi.CompressedSize);
            if (!compressedMemory) {
                free_func(memory);
                return false;
            }

            {
                std::lock_guard guard(archiveData.Mutex);
                auto& file = archiveData.Archive.FileHandle;
                if (!file.SetPosition(fi.Offset)) {
                    compressedMemory.reset();
                    free_func(memory);
                    return false;
                }
                if (file.Read(compressedMemory.get(), fi.CompressedSize) != fi.CompressedSize) {
                    compressedMemory.reset();
                    free_func(memory);
                    return false;
                }
            }

            if (fi.Hash != XXH64(compressedMemory.get(), fi.CompressedSize, 0)) {
                compressedMemory.reset();
                free_func(memory);
                return false;
            }

            if (ZSTD_decompress(static_cast<char*>(memory),
                                fi.UncompressedSize,
                                compressedMemory.get(),
                                fi.CompressedSize)
                != fi.UncompressedSize) {
                compressedMemory.reset();
                free_func(memory);
                return false;
            }
            compressedMemory.reset();

            out_memory = memory;
            out_filesize = fi.UncompressedSize;
            return true;
        }
        case SenPatcher::P3ACompressionType::ZSTD_DICT: {
            auto& archiveData = *ref.ArchiveData;
            if (!archiveData.Archive.Dict) {
                return false;
            }
            void* memory = malloc_func(fi.UncompressedSize);
            if (!memory) {
                return false;
            }
            auto compressedMemory = std::make_unique_for_overwrite<char[]>(fi.CompressedSize);
            if (!compressedMemory) {
                free_func(memory);
                return false;
            }

            {
                std::lock_guard guard(archiveData.Mutex);
                auto& file = archiveData.Archive.FileHandle;
                if (!file.SetPosition(fi.Offset)) {
                    compressedMemory.reset();
                    free_func(memory);
                    return false;
                }
                if (file.Read(compressedMemory.get(), fi.CompressedSize) != fi.CompressedSize) {
                    compressedMemory.reset();
                    free_func(memory);
                    return false;
                }
            }

            if (fi.Hash != XXH64(compressedMemory.get(), fi.CompressedSize, 0)) {
                compressedMemory.reset();
                free_func(memory);
                return false;
            }

            ZSTD_DCtx_UniquePtr dctx = ZSTD_DCtx_UniquePtr(ZSTD_createDCtx());
            if (!dctx) {
                compressedMemory.reset();
                free_func(memory);
                return false;
            }
            if (ZSTD_decompress_usingDDict(dctx.get(),
                                           static_cast<char*>(memory),
                                           fi.UncompressedSize,
                                           compressedMemory.get(),
                                           fi.CompressedSize,
                                           archiveData.Archive.Dict)
                != fi.UncompressedSize) {
                dctx.reset();
                compressedMemory.reset();
                free_func(memory);
                return false;
            }
            dctx.reset();
            compressedMemory.reset();

            out_memory = memory;
            out_filesize = fi.UncompressedSize;
            return true;
        }
        default: return false;
    }
}

static bool OpenModFile(FFile* ffile, const char* path) {
    if (!IsValidReroutablePath(path)) {
        return false;
    }

    if (s_CheckDevFolderForAssets) {
        std::u8string tmp = u8"dev/";
        tmp += (char8_t*)path;

        SenPatcher::IO::File file(std::filesystem::path(tmp), SenPatcher::IO::OpenMode::Read);
        if (file.IsOpen()) {
            auto length = file.GetLength();
            if (length && *length < 0x8000'0000) {
                ffile->NativeFileHandle = file.ReleaseHandle();
                ffile->Filesize = static_cast<uint32_t>(*length);
                return true;
            }
        }
    }

    std::array<char8_t, 0x100> filteredPath;
    if (!FilterGamePath(filteredPath.data(), path, filteredPath.size())) {
        return false;
    }

    const P3AFileRef* refptr = FindP3AFileRef(filteredPath);
    if (refptr != nullptr) {
        void* memory = nullptr;
        uint64_t filesize = 0;
        if (ExtractP3AFileToMemory(
                *refptr,
                0x8000'0000,
                memory,
                filesize,
                [](size_t length) { return s_TrackedMalloc(length, 8, nullptr, 0, 0); },
                [](void* memory) { s_TrackedFree(memory); })) {
            ffile->Filesize = static_cast<uint32_t>(filesize);
            ffile->MemoryPointer = memory;
            ffile->MemoryPointerForFreeing = memory;
            ffile->MemoryPosition = 0;
            return true;
        }
    }

    return false;
}

static std::optional<uint64_t> GetFilesizeOfModFile(const char* path) {
    if (!IsValidReroutablePath(path)) {
        return std::nullopt;
    }

    if (s_CheckDevFolderForAssets) {
        std::u8string tmp = u8"dev/";
        tmp += (char8_t*)path;

        SenPatcher::IO::File file(std::filesystem::path(tmp), SenPatcher::IO::OpenMode::Read);
        if (file.IsOpen()) {
            auto length = file.GetLength();
            if (length && *length < 0x8000'0000) {
                return length;
            }
        }
    }

    std::array<char8_t, 0x100> filteredPath;
    if (!FilterGamePath(filteredPath.data(), path, filteredPath.size())) {
        return std::nullopt;
    }

    const P3AFileRef* refptr = FindP3AFileRef(filteredPath);
    if (refptr != nullptr) {
        const P3AFileRef& ref = *refptr;
        const SenPatcher::P3AFileInfo& fi = *ref.FileInfo;
        if (fi.UncompressedSize < 0x8000'0000) {
            return fi.UncompressedSize;
        }
    }

    return std::nullopt;
}

static int64_t __fastcall FFileOpenForwarder(FFile* ffile,
                                             const char* path,
                                             int unknownThirdParameter) {
    if (OpenModFile(ffile, path)) {
        return 1;
    }
    return 0;
}

static int64_t __fastcall FFileGetFilesizeForwarder(FFile* ffile,
                                                    const char* path,
                                                    int unknownThirdParameter,
                                                    uint32_t* out_filesize) {
    auto result = GetFilesizeOfModFile(path);
    if (result) {
        if (out_filesize) {
            *out_filesize = static_cast<uint32_t>(*result);
        }
        return 1;
    }
    return 0;
}

static int64_t __fastcall FreestandingGetFilesizeForwarder(const char* path,
                                                           int unknownThirdParameter,
                                                           uint32_t* out_filesize) {
    auto result = GetFilesizeOfModFile(path);
    if (result) {
        if (out_filesize) {
            *out_filesize = static_cast<uint32_t>(*result);
        }
        return 1;
    }
    return 0;
}

struct FSoundFile {
    void* FRead;
    void* FSeek;
    void* FTell;
    void* FClose;
};

static int32_t __fastcall SenPatcherFile_FSoundFileRead(SenPatcher::IO::File* file,
                                                        void* memory,
                                                        int32_t length) {
    if (length < 1) {
        return 0;
    }

    size_t bytesRead = file->Read(memory, static_cast<size_t>(length));
    if (bytesRead == 0) {
        return -128; // indicates end of file, I think?
    }
    return static_cast<int32_t>(bytesRead);
}

static int32_t __fastcall SenPatcherFile_FSoundFileSeek(SenPatcher::IO::File* file,
                                                        int64_t position,
                                                        int mode) {
    if (mode >= 0 && mode <= 2) {
        if (file->SetPosition(position, static_cast<SenPatcher::IO::SetPositionMode>(mode))) {
            return 0;
        }
    }
    return -1;
}

static int64_t __fastcall SenPatcherFile_FSoundFileTell(SenPatcher::IO::File* file) {
    auto position = file->GetPosition();
    if (position) {
        return *position;
    }
    return -1;
}

static void __fastcall SenPatcherFile_FSoundFileClose(SenPatcher::IO::File* file) {
    std::unique_ptr<SenPatcher::IO::File> tmp(file);
    tmp.reset();
}

struct MemoryFile {
    char* Memory;
    size_t Length;
    size_t Position;
};

static int32_t __fastcall MemoryFile_FSoundFileRead(MemoryFile* file,
                                                    void* memory,
                                                    int32_t length) {
    if (length < 1) {
        return 0;
    }

    char* current = file->Memory + file->Position;
    char* end = file->Memory + file->Length;
    size_t toRead = end - current;
    if (static_cast<size_t>(length) < toRead) {
        toRead = static_cast<size_t>(length);
    }
    if (toRead == 0) {
        return -128; // indicates end of file, I think?
    }

    std::memcpy(memory, current, toRead);
    file->Position += toRead;
    return static_cast<int32_t>(toRead);
}

static int32_t __fastcall MemoryFile_FSoundFileSeek(MemoryFile* file, int64_t position, int mode) {
    int64_t targetPosition;
    if (mode == 0) {
        targetPosition = position;
    } else if (mode == 1) {
        targetPosition = static_cast<int64_t>(file->Position) + position;
    } else if (mode == 2) {
        targetPosition = static_cast<int64_t>(file->Length) + position;
    } else {
        return -1;
    }

    if (targetPosition < 0 || static_cast<uint64_t>(targetPosition) > file->Length) {
        return -1;
    }

    file->Position = targetPosition;
    return 0;
}

static int64_t __fastcall MemoryFile_FSoundFileTell(MemoryFile* file) {
    return file->Position;
}

static void __fastcall MemoryFile_FSoundFileClose(MemoryFile* file) {
    free(file->Memory);
    delete file;
}

static void* __fastcall FSoundOpenForwarder(FSoundFile* soundFile, const char* path) {
    if (!IsValidReroutablePath(path)) {
        return nullptr;
    }

    if (s_CheckDevFolderForAssets) {
        std::u8string tmp = u8"dev/";
        tmp += (char8_t*)path;

        auto file = std::make_unique<SenPatcher::IO::File>(std::filesystem::path(tmp),
                                                           SenPatcher::IO::OpenMode::Read);
        if (file && file->IsOpen()) {
            void* handle = file.release();
            soundFile->FRead = &SenPatcherFile_FSoundFileRead;
            soundFile->FSeek = &SenPatcherFile_FSoundFileSeek;
            soundFile->FTell = &SenPatcherFile_FSoundFileTell;
            soundFile->FClose = &SenPatcherFile_FSoundFileClose;
            return handle;
        }
    }

    std::array<char8_t, 0x100> filteredPath;
    if (!FilterGamePath(filteredPath.data(), path, filteredPath.size())) {
        return nullptr;
    }

    const P3AFileRef* refptr = FindP3AFileRef(filteredPath);
    if (refptr != nullptr) {
        void* memory = nullptr;
        uint64_t filesize = 0;
        if (ExtractP3AFileToMemory(
                *refptr,
                0x8000'0000,
                memory,
                filesize,
                [](size_t length) { return malloc(length); },
                [](void* memory) { free(memory); })) {
            MemoryFile* handle = new MemoryFile();
            handle->Memory = static_cast<char*>(memory);
            handle->Length = filesize;
            handle->Position = 0;
            soundFile->FRead = &MemoryFile_FSoundFileRead;
            soundFile->FSeek = &MemoryFile_FSoundFileSeek;
            soundFile->FTell = &MemoryFile_FSoundFileTell;
            soundFile->FClose = &MemoryFile_FSoundFileClose;
            return handle;
        }
    }

    return nullptr;
}

static void InjectAtFFileOpen(SenPatcher::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd) {
    using namespace SenPatcher::x64;

    char* const entryPoint = textRegion
                             + (version == GameVersion::Japanese ? (0x1400f5270 - 0x140001000)
                                                                 : (0x1400f58f0 - 0x140001000));
    char* const exitPoint = textRegion
                            + (version == GameVersion::Japanese ? (0x1400f538d - 0x140001000)
                                                                : (0x1400f5a0d - 0x140001000));


    char* codespaceBegin = codespace;
    char* inject = entryPoint;
    std::array<char, 12> overwrittenInstructions;

    {
        PageUnprotect page(logger, inject, overwrittenInstructions.size());
        std::memcpy(overwrittenInstructions.data(), inject, overwrittenInstructions.size());

        Emit_MOV_R64_IMM64(inject, R64::RAX, std::bit_cast<uint64_t>(codespaceBegin), true);
        Emit_JMP_R64(inject, R64::RAX);
    }

    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();

    Emit_PUSH_R64(codespace, R64::RAX);
    Emit_PUSH_R64(codespace, R64::RCX);
    Emit_PUSH_R64(codespace, R64::RDX);
    Emit_PUSH_R64(codespace, R64::R8);

    // for some reason I cannot figure out, the caller has to reserve stack space for the called
    // function's parameters in x64???
    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(&FFileOpenForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);
    Emit_POP_R64(codespace, R64::RAX);

    // if successful, skip the following jmp r9
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // go back to function and pretend nothing happened
    Emit_MOV_R64_IMM64(codespace, R64::R9, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::R9);

    // return that we succeeded
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, 1);

    // jump to exit point
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPoint));
    Emit_JMP_R64(codespace, R64::RCX);
}

static void InjectAtFFileGetFilesize(SenPatcher::Logger& logger,
                                     char* textRegion,
                                     GameVersion version,
                                     char*& codespace,
                                     char* codespaceEnd) {
    using namespace SenPatcher::x64;

    char* const entryPoint = textRegion
                             + (version == GameVersion::Japanese ? (0x1400f5120 - 0x140001000)
                                                                 : (0x1400f57a0 - 0x140001000));
    char* const exitPoint = textRegion
                            + (version == GameVersion::Japanese ? (0x1400f5224 - 0x140001000)
                                                                : (0x1400f58a4 - 0x140001000));


    char* codespaceBegin = codespace;
    char* inject = entryPoint;
    std::array<char, 12> overwrittenInstructions;

    {
        PageUnprotect page(logger, inject, overwrittenInstructions.size());
        std::memcpy(overwrittenInstructions.data(), inject, overwrittenInstructions.size());

        Emit_MOV_R64_IMM64(inject, R64::RAX, std::bit_cast<uint64_t>(codespaceBegin), true);
        Emit_JMP_R64(inject, R64::RAX);
    }

    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();

    Emit_PUSH_R64(codespace, R64::RCX);
    Emit_PUSH_R64(codespace, R64::RDX);
    Emit_PUSH_R64(codespace, R64::R8);
    Emit_PUSH_R64(codespace, R64::R9);
    Emit_PUSH_R64(codespace, R64::R10);
    Emit_PUSH_R64(codespace, R64::R11);
    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(&FFileGetFilesizeForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);
    Emit_POP_R64(codespace, R64::R11);
    Emit_POP_R64(codespace, R64::R10);
    Emit_POP_R64(codespace, R64::R9);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);

    // check for success
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // on fail, go back to function and pretend nothing happened
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RAX);

    // on success, return that we succeeded
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, 1);

    // jump to exit point
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPoint));
    Emit_JMP_R64(codespace, R64::RCX);
}

static void InjectAtFreestandingGetFilesize(SenPatcher::Logger& logger,
                                            char* textRegion,
                                            GameVersion version,
                                            char*& codespace,
                                            char* codespaceEnd) {
    using namespace SenPatcher::x64;

    char* const entryPoint = textRegion
                             + (version == GameVersion::Japanese ? (0x1400f4905 - 0x140001000)
                                                                 : (0x1400f4ea5 - 0x140001000));
    char* const exitPoint = textRegion
                            + (version == GameVersion::Japanese ? (0x1400f4a2e - 0x140001000)
                                                                : (0x1400f4fce - 0x140001000));


    char* codespaceBegin = codespace;
    char* inject = entryPoint;
    std::array<char, 12> overwrittenInstructions;

    {
        PageUnprotect page(logger, inject, overwrittenInstructions.size());
        std::memcpy(overwrittenInstructions.data(), inject, overwrittenInstructions.size());

        Emit_MOV_R64_IMM64(inject, R64::RAX, std::bit_cast<uint64_t>(codespaceBegin), true);
        Emit_JMP_R64(inject, R64::RAX);
    }

    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();

    Emit_PUSH_R64(codespace, R64::RCX);
    Emit_PUSH_R64(codespace, R64::RDX);
    Emit_PUSH_R64(codespace, R64::R8);
    Emit_PUSH_R64(codespace, R64::R9);
    Emit_PUSH_R64(codespace, R64::R10);
    Emit_PUSH_R64(codespace, R64::R11);
    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_MOV_R64_IMM64(
        codespace, R64::RAX, std::bit_cast<uint64_t>(&FreestandingGetFilesizeForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);
    Emit_POP_R64(codespace, R64::R11);
    Emit_POP_R64(codespace, R64::R10);
    Emit_POP_R64(codespace, R64::R9);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);

    // if successful, skip the following jmp r9
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // go back to function and pretend nothing happened
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RAX);

    // return that we succeeded
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, 1);

    // jump to exit point
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPoint));
    Emit_JMP_R64(codespace, R64::RCX);
}

static void InjectAtOpenFSoundFile(SenPatcher::Logger& logger,
                                   char* textRegion,
                                   GameVersion version,
                                   char*& codespace,
                                   char* codespaceEnd) {
    using namespace SenPatcher::x64;

    char* const entryPoint = textRegion
                             + (version == GameVersion::Japanese ? (0x140086900 - 0x140001000)
                                                                 : (0x140086900 - 0x140001000));


    char* codespaceBegin = codespace;
    char* inject = entryPoint;
    std::array<char, 15> overwrittenInstructions;

    {
        PageUnprotect page(logger, inject, overwrittenInstructions.size());
        std::memcpy(overwrittenInstructions.data(), inject, overwrittenInstructions.size());

        Emit_MOV_R64_IMM64(inject, R64::RAX, std::bit_cast<uint64_t>(codespaceBegin), true);
        Emit_JMP_R64(inject, R64::RAX);
        *inject++ = 0xcc;
        *inject++ = 0xcc;
        *inject++ = 0xcc;
    }

    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();

    Emit_PUSH_R64(codespace, R64::RCX);
    Emit_PUSH_R64(codespace, R64::RDX);
    Emit_PUSH_R64(codespace, R64::R8);
    Emit_PUSH_R64(codespace, R64::R9);
    Emit_PUSH_R64(codespace, R64::R10);
    Emit_PUSH_R64(codespace, R64::R11);
    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(&FSoundOpenForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);
    Emit_POP_R64(codespace, R64::R11);
    Emit_POP_R64(codespace, R64::R10);
    Emit_POP_R64(codespace, R64::R9);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);

    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // on failure go back to function
    Emit_MOV_R64_IMM64(codespace, R64::R9, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::R9);

    // on success just return from the function, RAX already has the correct return value
    success.SetTarget(codespace);
    Emit_RET(codespace);
}

static PDirectInput8Create addr_PDirectInput8Create = 0;
static void* SetupHacks() {
    SenPatcher::Logger logger("senpatcher_inject_cs3.log");

    addr_PDirectInput8Create = LoadForwarderAddress(logger);

    void* codeBase = nullptr;
    GameVersion version = FindImageBase(logger, &codeBase);
    if (version == GameVersion::Unknown || !codeBase) {
        logger.Log("Failed finding executable in memory -- wrong game or version?\n");
        return nullptr;
    }

    s_TrackedMalloc = reinterpret_cast<PTrackedMalloc>(static_cast<char*>(codeBase)
                                                       + (version == GameVersion::Japanese
                                                              ? (0x1405d3fa0 - 0x140001000)
                                                              : (0x1405e03f0 - 0x140001000)));
    s_TrackedFree = reinterpret_cast<PTrackedFree>(static_cast<char*>(codeBase)
                                                   + (version == GameVersion::Japanese
                                                          ? (0x1405d3ed0 - 0x140001000)
                                                          : (0x1405e0320 - 0x140001000)));

    // allocate extra page for code
    const size_t newPageLength = 0x1000;
    char* newPage =
        static_cast<char*>(VirtualAlloc(nullptr, newPageLength, MEM_COMMIT, PAGE_READWRITE));
    if (!newPage) {
        logger.Log("VirtualAlloc failed, skipping remaining patches.\n");
        return nullptr;
    }
    std::memset(newPage, 0xcc, newPageLength);
    char* newPageStart = newPage;
    char* newPageEnd = newPage + newPageLength;

    // figure out whether we're running in the root game directory or in the bin/x64 subdirectory
    // and get a relative path to the root game directory
    std::filesystem::path baseDir;
    if (std::filesystem::exists(L"Sen3Launcher.exe")) {
        baseDir = L"./";
    } else if (std::filesystem::exists(L"../../Sen3Launcher.exe")) {
        baseDir = L"../../";
    } else {
        logger.Log("Failed finding root game directory.\n");
        return nullptr;
    }

    SenLib::Sen3::CreateAssetPatchIfNeeded(logger, baseDir);
    LoadModP3As(logger, baseDir);

    InjectAtFFileOpen(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    InjectAtFFileGetFilesize(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    InjectAtFreestandingGetFilesize(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    InjectAtOpenFSoundFile(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);

    // mark newly allocated page as executable
    {
        DWORD tmpdword;
        VirtualProtect(newPageStart, newPageLength, PAGE_EXECUTE_READ, &tmpdword);
        FlushInstructionCache(GetCurrentProcess(), newPageStart, newPageLength);
    }

    return newPageStart;
}
static void* dummy = SetupHacks();

extern "C" {
HRESULT DirectInput8Create(HINSTANCE hinst,
                           DWORD dwVersion,
                           REFIID riidltf,
                           LPVOID* ppvOut,
                           void* punkOuter) {
    PDirectInput8Create addr = addr_PDirectInput8Create;
    if (!addr) {
        return 0x8007000EL; // DIERR_OUTOFMEMORY
    }
    return addr(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}
}
