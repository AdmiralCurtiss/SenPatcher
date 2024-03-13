#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
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
#include "ini.h"
#include "logger.h"
#include "util/text.h"

#include "sen3/file_fixes.h"

#include "p3a/p3a.h"
#include "p3a/structs.h"

#include "x64/emitter.h"

#include "senpatcher_version.h"

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
    size_t address = 0;
    while (true) {
        if (VirtualQuery(reinterpret_cast<void*>(address), &info, sizeof(info)) == 0) {
            logger.Log("Querying address ").LogHex(address).Log(" failed.\n");
            break;
        }
        if (info.RegionSize == 0) {
            logger.Log("Querying address ").LogHex(address).Log(" returned zero-sized region.\n");
            break;
        }

        logger.Log("Querying address ")
            .LogHex(address)
            .Log(", got allocation at ")
            .LogPtr(info.AllocationBase)
            .Log(", base ptr ")
            .LogPtr(info.BaseAddress)
            .Log(", size ")
            .LogHex(info.RegionSize)
            .Log(", protection ")
            .LogHex(info.Protect)
            .Log(".\n");
        if (gameVersion == GameVersion::Unknown && info.State == MEM_COMMIT
            && info.Type == MEM_IMAGE) {
            if (info.RegionSize == 0x7e9000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(crc, static_cast<char*>(info.BaseAddress) + 0xf48f0, 0x3d);
                crc = crc_update(crc, static_cast<char*>(info.BaseAddress) + 0x7c51d8, 0x24);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x19068487) {
                    logger.Log("Appears to be the EN version.\n");
                    *code = info.BaseAddress;
                    gameVersion = GameVersion::English;
                }
            } else if (info.RegionSize == 0x7dc000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(crc, static_cast<char*>(info.BaseAddress) + 0xf4270, 0x3d);
                crc = crc_update(crc, static_cast<char*>(info.BaseAddress) + 0x7b82bc, 0x24);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x19068487) {
                    logger.Log("Appears to be the JP version.\n");
                    *code = info.BaseAddress;
                    gameVersion = GameVersion::Japanese;
                }
            }

            // logger.Log("Memory is: ");
            // for (int i = 0; i < info.RegionSize; ++i) {
            //  logger.Log(" ").LogHex(*(reinterpret_cast<unsigned char*>(info.BaseAddress) + i));
            // }
            // logger.Log("\n");
        }

        if (address >= (size_t(0) - info.RegionSize)) {
            // would wrap around
            break;
        }

        address += info.RegionSize;
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

static bool LoadOrderTxt(std::vector<std::filesystem::path>& output,
                         SenPatcher::Logger& logger,
                         SenPatcher::IO::File& file) {
    using HyoutaUtils::TextUtils::Trim;
    using HyoutaUtils::TextUtils::Utf8ToUtf16;

    if (!file.IsOpen()) {
        return false;
    }
    const auto length = file.GetLength();
    if (!length) {
        return false;
    }
    auto buffer = std::make_unique_for_overwrite<char[]>(*length);
    if (file.Read(buffer.get(), *length) != *length) {
        return false;
    }
    std::string_view remaining(buffer.get(), *length);
    // skip UTF8 BOM if it's there
    if (remaining.starts_with("\xef\xbb\xbf")) {
        remaining = remaining.substr(3);
    }

    while (!remaining.empty()) {
        const size_t nextLineSeparator = remaining.find_first_of("\r\n");
        std::string_view line = Trim(remaining.substr(0, nextLineSeparator));
        remaining = nextLineSeparator != std::string_view::npos
                        ? remaining.substr(nextLineSeparator + 1)
                        : std::string_view();
        if (line.empty()) {
            continue;
        }
        auto path = std::filesystem::path(Utf8ToUtf16(line.data(), line.size()));

        // TODO: sanitize path here so it doesn't go outside the mods dir

        output.emplace_back(path);
    }

    return true;
}

static bool WriteOrderTxt(const std::vector<std::filesystem::path>& paths,
                          SenPatcher::Logger& logger,
                          SenPatcher::IO::File& file) {
    if (!file.IsOpen()) {
        return false;
    }

    for (const auto& path : paths) {
        const auto& string = path.native();
        const auto utf8 =
            HyoutaUtils::TextUtils::Utf16ToUtf8((const char16_t*)string.data(), string.size());
        if (file.Write(utf8.data(), utf8.size()) != utf8.size()) {
            return false;
        }
        if (file.Write("\r\n", 2) != 2) {
            return false;
        }
    }

    return true;
}

static std::vector<std::filesystem::path> CollectModPaths(SenPatcher::Logger& logger,
                                                          const std::filesystem::path& modsDir) {
    std::vector<std::filesystem::path> paths;
    const auto orderPath = modsDir / L"order.txt";

    // check for order.txt, which may define an order (priority) for the mods
    {
        SenPatcher::IO::File orderfile(orderPath, SenPatcher::IO::OpenMode::Read);
        LoadOrderTxt(paths, logger, orderfile);
    }

    // the rest we just load in whatever the order the filesystem gives us, which may be
    // arbitrary...
    std::error_code ec;
    std::filesystem::directory_iterator iterator(modsDir, ec);
    if (ec) {
        return paths;
    }

    bool modified = false;
    for (auto const& entry : iterator) {
        if (entry.is_directory()) {
            continue;
        }

        auto& path = entry.path();
        auto& string = path.native();
        if (string.size() >= 4 && string[string.size() - 4] == L'.'
            && (string[string.size() - 3] == L'P' || string[string.size() - 3] == L'p')
            && string[string.size() - 2] == L'3'
            && (string[string.size() - 1] == L'A' || string[string.size() - 1] == L'a')) {
            auto filename = path.filename();
            if (std::find(paths.begin(), paths.end(), filename) == paths.end()) {
                paths.emplace_back(std::move(filename));
                modified = true;
            }
        }
    }

    if (modified) {
        // write out the modified file so that the user has an easier time editing the priorities
        auto orderPathTmp = orderPath;
        orderPathTmp += L".tmp";
        SenPatcher::IO::File orderfile(orderPathTmp, SenPatcher::IO::OpenMode::Write);
        if (WriteOrderTxt(paths, logger, orderfile)) {
            if (!orderfile.Rename(orderPath)) {
                orderfile.Delete();
            }
        } else {
            orderfile.Delete();
        }
    }

    return paths;
}

static void LoadModP3As(SenPatcher::Logger& logger, const std::filesystem::path& baseDir) {
    s_CombinedFileInfoCount = 0;
    s_CombinedFileInfos.reset();
    s_P3As.reset();
    {
        std::error_code ec;
        s_CheckDevFolderForAssets = std::filesystem::is_directory(baseDir / L"dev", ec);
    }

    size_t p3acount = 0;
    std::unique_ptr<P3AData[]> p3as;
    {
        const auto modsDir = baseDir / L"mods";
        const auto modPaths = CollectModPaths(logger, modsDir);
        std::vector<SenPatcher::P3A> p3avector;
        p3avector.reserve(modPaths.size());
        for (auto const& path : modPaths) {
            SenPatcher::P3A& p3a = p3avector.emplace_back();
            if (!p3a.Load(modsDir / path)) {
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

    if (totalFileInfoCount == 0) {
        return;
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

namespace {
template<size_t length>
struct InjectJumpIntoCodeResult {
    char* JumpBackAddress;
    std::array<char, length> OverwrittenInstructions;
};
} // namespace

template<size_t length>
static InjectJumpIntoCodeResult<length> InjectJumpIntoCode(SenPatcher::Logger& logger,
                                                           char* injectAt,
                                                           SenPatcher::x64::R64 reg,
                                                           char* jumpTarget) {
    static_assert(length >= 12);

    InjectJumpIntoCodeResult<length> rv;
    char* inject = injectAt;
    {
        PageUnprotect page(logger, inject, rv.OverwrittenInstructions.size());
        std::memcpy(rv.OverwrittenInstructions.data(), inject, rv.OverwrittenInstructions.size());

        Emit_MOV_R64_IMM64(inject, reg, std::bit_cast<uint64_t>(jumpTarget), 10);
        Emit_JMP_R64(inject, reg);
        if constexpr (length > 12) {
            for (size_t i = 12; i < length; ++i) {
                *inject++ = 0xcc;
            }
        }
    }
    rv.JumpBackAddress = inject;
    return rv;
}

static char* GetCodeAddressJpEn(GameVersion version,
                                char* textRegion,
                                uint64_t addressJp,
                                uint64_t addressEn) {
    return textRegion
           + (version == GameVersion::Japanese ? (addressJp - 0x140001000u)
                                               : (addressEn - 0x140001000u));
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
    auto injectResult = InjectJumpIntoCode<12>(logger, entryPoint, R64::RAX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

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
    auto injectResult = InjectJumpIntoCode<12>(logger, entryPoint, R64::RAX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

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
    auto injectResult = InjectJumpIntoCode<12>(logger, entryPoint, R64::RAX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

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
    auto injectResult = InjectJumpIntoCode<15>(logger, entryPoint, R64::RAX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

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

static void DeglobalizeMutexes(SenPatcher::Logger& logger,
                               char* textRegion,
                               GameVersion version,
                               char*& codespace,
                               char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* createMutexString1 = GetCodeAddressJpEn(version, textRegion, 0x14054fb09, 0x14055bc99);
    char* createMutexString2 = GetCodeAddressJpEn(version, textRegion, 0x14011c611, 0x14011cb11);
    {
        PageUnprotect page(logger, createMutexString1, 3);
        Emit_XOR_R64_R64(createMutexString1, R64::R8, R64::R8);
    }
    {
        PageUnprotect page(logger, createMutexString2, 7);
        Emit_MOV_R64_IMM64(createMutexString2, R64::R8, 0, 7);
    }
}

static void AddSenPatcherVersionToTitle(SenPatcher::Logger& logger,
                                        char* textRegion,
                                        GameVersion version,
                                        char*& codespace,
                                        char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* entryPoint = GetCodeAddressJpEn(version, textRegion, 0x140421caf, 0x14042d2a2);
    char* rdxLoad = GetCodeAddressJpEn(version, textRegion, 0x140421cc6, 0x14042d2b9);
    char* codespaceBegin = codespace;

    // safe to clobber: RCX, RDX (needs to hold string result), R9

    // get the current title screen string
    uint32_t offset;
    std::memcpy(&offset, rdxLoad + 3, 4);
    const char* originalTitleString = rdxLoad + 7 + offset;

    // copy the title screen string into our codespace and expand with senpatcher version
    {
        const char* tmp = originalTitleString;
        while (*tmp != 0) {
            *codespace = *tmp;
            ++codespace;
            ++tmp;
        }
    }
    constexpr char senpatcherVersionString[] = "  SenPatcher " SENPATCHER_VERSION;
    std::strcpy(codespace, senpatcherVersionString);
    codespace += sizeof(senpatcherVersionString);

    // inject a jump to codespace
    auto injectResult = InjectJumpIntoCode<12>(logger, entryPoint, R64::RDX, codespace);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();

    // load the title screen string into RDX
    Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(codespaceBegin));

    // go back
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // remove the old load of rdx
    {
        char* tmp = rdxLoad;
        PageUnprotect page(logger, rdxLoad, 7);
        for (size_t i = 0; i < 7; ++i) {
            *tmp++ = 0x90; // nop
        }
    }
}

static void FixInGameButtonMappingValidity(SenPatcher::Logger& logger,
                                           char* textRegion,
                                           GameVersion version,
                                           char*& codespace,
                                           char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* entryPoint = GetCodeAddressJpEn(version, textRegion, 0x140447427, 0x140453257);
    size_t length = 0x44;
    PageUnprotect page(logger, entryPoint, length);

    // remove buggy code
    {
        char* tmp = entryPoint;
        for (size_t i = 0; i < length; ++i) {
            *tmp++ = 0xcc; // int 3
        }
    }

    BranchHelper1Byte done;
    done.SetTarget(entryPoint + length);

    // input:
    //   ax as the button, same order as in the config menu (0-9)
    //   bx as the action, same order as in the config menu (0-12)
    //   rax, rcx, rdx may be clobbered
    // output:
    //   esi = 0 if option should be enabled, esi = 3 if option should be disabled

    char* inject = entryPoint;
    const auto write_instruction_32 = [&](uint32_t instr) {
        *inject++ = (char)((instr >> 24) & 0xff);
        *inject++ = (char)((instr >> 16) & 0xff);
        *inject++ = (char)((instr >> 8) & 0xff);
        *inject++ = (char)(instr & 0xff);
    };
    const auto write_instruction_24 = [&](uint32_t instr) {
        *inject++ = (char)((instr >> 16) & 0xff);
        *inject++ = (char)((instr >> 8) & 0xff);
        *inject++ = (char)(instr & 0xff);
    };
    const auto write_integer_16 = [&](uint32_t value) {
        *inject++ = (char)(value & 0xff);
        *inject++ = (char)((value >> 8) & 0xff);
    };

    write_instruction_24(0x488d0d); // lea rcx,[lookup_table]
    write_instruction_32(0);        // ^ relative address of lookup table
    char* lookup_table_inject_address = inject;
    write_instruction_32(0x480fb7c0);           // movzx rax,ax
    write_instruction_32(0x488d0c41);           // lea rcx,[rcx+rax*2]
    write_instruction_24(0x668b01);             // mov ax,word ptr[rcx]
    write_instruction_24(0x668bcb);             // mov cx,bx
    write_instruction_24(0x66d3e8);             // shr ax,cl
    write_instruction_32(0x6683e001);           // and ax,1
    write_instruction_24(0x668bc8);             // mov cx,ax
    write_instruction_24(0x66d1e0);             // shl ax,1
    write_instruction_24(0x660bc1);             // or ax,cx
    write_instruction_24(0x0fb7f0);             // movzx esi,ax
    done.WriteJump(inject, JumpCondition::JMP); // jmp done

    while (std::bit_cast<uint64_t>(inject) % 4 != 0) {
        *inject++ = 0xcc; // int 3
    }

    char* lookup_table_address = inject;
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad Up
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad Left
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad Right
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad L1
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad R1
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad R3
    write_integer_16(0x0c42); // bitmask for disallowed options for D-Pad Circle
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad Select
    write_integer_16(0x03ff); // bitmask for disallowed options for D-Pad Select (Battle)
    write_integer_16(0x03ff); // bitmask for disallowed options for D-Pad Start (Battle)

    const uint32_t relativeOffset =
        static_cast<uint32_t>(lookup_table_address - lookup_table_inject_address);
    std::memcpy(lookup_table_inject_address - 4, &relativeOffset, 4);
}

static void AllowSwitchToNightmare(SenPatcher::Logger& logger,
                                   char* textRegion,
                                   GameVersion version,
                                   char*& codespace,
                                   char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* setMaxDifficultyIndexPos =
        GetCodeAddressJpEn(version, textRegion, 0x14042a667, 0x140436347) + 1;
    char* switchFromNightmareWarning =
        GetCodeAddressJpEn(version, textRegion, 0x140236118, 0x14023c65d) + 1;

    // allow nightmare to be selected when difficulty is currently not nightmare
    {
        PageUnprotect page(logger, setMaxDifficultyIndexPos, 4);
        const uint32_t value = 5;
        std::memcpy(setMaxDifficultyIndexPos, &value, 4);
    }

    // don't show warning when trying to switch away from nightmare
    {
        PageUnprotect page(logger, switchFromNightmareWarning, 4);
        const uint32_t value = 0x1e4;
        std::memcpy(switchFromNightmareWarning, &value, 4);
    }
}

static void SwapBrokenMasterQuartzValuesForDisplay(SenPatcher::Logger& logger,
                                                   char* textRegion,
                                                   GameVersion version,
                                                   char*& codespace,
                                                   char* codespaceEnd) {
    using namespace SenPatcher::x64;
    if (version != GameVersion::English) {
        return; // bug only exists in the english version
    }

    // on function entry:
    // r8w -> 0, 1, or 2 depending on which part of the description to print
    // dx  -> master quartz ID
    // we need to swap when:
    // description is 1 (applies to all of these) and
    // ID is any of 0x0c85, 0x0c86, 0x0c8a, 0x0c8c, 0x0c91, 0x0c95, 0x0c98
    // additionally, on french text only when ID is 0x0c81
    // then swap xmm3 and xmm1 for the call to snprintf_s()

    // ...actually this kinda sucks to hardcode, so instead...
    // we insert a sentinel character at the start of the actual text description in the t_mstqrt
    // file. before the sprintf call, we check if this sentinel character is there. if yes we swap
    // xmm3 and xmm1, and advance string ptr by 1

    // stack from rsp+31h to rsp+37h looks unused, so stash our sentinel check flag in there...
    // rsp+34h looks good

    char* initSentinelCheckPos = textRegion + ((0x14027fd47u - 0x140001000u));
    constexpr size_t initSentinelCheckLen = 13;
    char* checkSentinelPos = textRegion + ((0x14028015bu - 0x140001000u));
    constexpr size_t checkSentinelLen = 13;
    char* fixParametersPos = textRegion + ((0x14028020eu - 0x140001000u));
    constexpr size_t fixParametersLen = 12;

    const auto write_instruction_40 = [&](char*& codepos, uint64_t instr) {
        *codepos++ = (char)((instr >> 32) & 0xff);
        *codepos++ = (char)((instr >> 24) & 0xff);
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_32 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 24) & 0xff);
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_24 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };

    // first initialize sentinel check flag on stack near start of function
    {
        auto injectResult = InjectJumpIntoCode<initSentinelCheckLen>(
            logger, initSentinelCheckPos, R64::RAX, codespace);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        write_instruction_40(codespace, 0xc644243400); // mov byte ptr [rsp+34h],0

        Emit_MOV_R64_IMM64(
            codespace, R64::RAX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RAX);
    }

    // on strncat() call that appends the description string, check for the sentinel;
    // if it's there skip it and set the flag on the stack
    {
        // we're a bit constrained for space here so this is somewhat creative
        auto injectResult =
            InjectJumpIntoCode<checkSentinelLen>(logger, checkSentinelPos, R64::RAX, codespace);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

        // there's a pending jz, handle that
        BranchHelper1Byte skip_strncpy;
        skip_strncpy.WriteJump(codespace, JumpCondition::JZ);

        // rdx contains the address of the string, r8d is free to use
        BranchHelper1Byte return_to_function_short;
        write_instruction_24(codespace, 0x448a02);   // mov r8b,byte ptr[rdx]
        write_instruction_32(codespace, 0x4180f824); // cmp r8b,24h
        return_to_function_short.WriteJump(codespace,
                                           JumpCondition::JNE); // jne return_to_function_short
        write_instruction_40(codespace, 0xc644243401);          // mov byte ptr [rsp+34h],1
        write_instruction_24(codespace, 0x48ffc2);              // inc rdx
        return_to_function_short.SetTarget(codespace);

        // don't copy back the jz at the start of the overwritten instructions
        std::memcpy(
            codespace, overwrittenInstructions.data() + 2, overwrittenInstructions.size() - 2);
        codespace += (overwrittenInstructions.size() - 2);
        Emit_MOV_R64_IMM64(
            codespace, R64::RAX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RAX);

        // skip the call strncpy
        skip_strncpy.SetTarget(codespace);
        Emit_MOV_R64_IMM64(
            codespace, R64::RAX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress + 6));
        Emit_JMP_R64(codespace, R64::RAX);
    }

    // check sentinel flag and fix up parameters if it's set
    {
        auto injectResult =
            InjectJumpIntoCode<fixParametersLen>(logger, fixParametersPos, R64::RAX, codespace);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        BranchHelper1Byte exit_without_swap;
        write_instruction_40(codespace, 0x448a4c2434);             // mov r9b,byte ptr[rsp+34h]
        write_instruction_24(codespace, 0x4584c9);                 // test r9b,r9b
        exit_without_swap.WriteJump(codespace, JumpCondition::JZ); // jz exit_without_swap
        write_instruction_32(codespace, 0xf30f7ed3);               // movq xmm2,xmm3
        write_instruction_32(codespace, 0xf30f7ed9);               // movq xmm3,xmm1
        write_instruction_32(codespace, 0xf30f7eca);               // movq xmm1,xmm2
        exit_without_swap.SetTarget(codespace);

        Emit_MOV_R64_IMM64(
            codespace, R64::RAX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RAX);
    }
}

static void PatchDisableMouseCapture(SenPatcher::Logger& logger,
                                     char* textRegion,
                                     GameVersion version,
                                     char*& codespace,
                                     char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* captureMouseCursorPos1 =
        GetCodeAddressJpEn(version, textRegion, 0x140320db8, 0x14032a2c8);
    char* captureMouseCursorPos2 =
        GetCodeAddressJpEn(version, textRegion, 0x140320f85, 0x14032a495);
    char* cameraMouseFuncPos1 = GetCodeAddressJpEn(version, textRegion, 0x1400f7be8, 0x1400f8268);
    char* cameraMouseFuncPos2 = GetCodeAddressJpEn(version, textRegion, 0x1400f7d61, 0x1400f83e1);
    char* processMouseFuncPos = GetCodeAddressJpEn(version, textRegion, 0x1400f88bd, 0x1400f8f3d);

    // change functions that capture the mouse cursor to not do that
    {
        PageUnprotect page(logger, captureMouseCursorPos1, 1);
        *captureMouseCursorPos1 = 0xeb; // jz -> jmp
    }
    {
        PageUnprotect page(logger, captureMouseCursorPos2, 1);
        *captureMouseCursorPos2 = 0xeb; // jz -> jmp
    }

    // change function that handles camera movement to not react to mouse movement
    // and not to fall back to WASD camera movement either (legacy code...?)
    {
        PageUnprotect page(logger, cameraMouseFuncPos1, 6);
        BranchHelper4Byte branch;
        branch.SetTarget(cameraMouseFuncPos2);
        char* tmp = cameraMouseFuncPos1;
        branch.WriteJump(tmp, JumpCondition::JMP);
        *tmp = 0x90; // nop
    }

    // skip mouse movement processing function or something like that?
    {
        char* tmp = processMouseFuncPos;
        PageUnprotect page(logger, tmp, 5);
        for (size_t i = 0; i < 5; ++i) {
            *tmp++ = 0x90; // nop
        }
    }
}

static void PatchShowMouseCursor(SenPatcher::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* showCursorPos = GetCodeAddressJpEn(version, textRegion, 0x140599734, 0x1405a5ae4);

    // remove call to ShowCursor(0)
    {
        char* tmp = showCursorPos;
        PageUnprotect page(logger, tmp, 8);
        for (size_t i = 0; i < 8; ++i) {
            *tmp++ = 0x90; // nop
        }
    }
}

static void PatchDisablePauseOnFocusLoss(SenPatcher::Logger& logger,
                                         char* textRegion,
                                         GameVersion version,
                                         char*& codespace_,
                                         char* codespaceEnd_) {
    using namespace SenPatcher::x64;
    char* silenceAudioIfUnfocusedPos1 =
        GetCodeAddressJpEn(version, textRegion, 0x1400f943b, 0x1400f9abb);
    char* silenceAudioIfUnfocusedPos2 =
        GetCodeAddressJpEn(version, textRegion, 0x1400f9474, 0x1400f9af4);
    char* runMainGameLoopIfUnfocusedPos =
        GetCodeAddressJpEn(version, textRegion, 0x1400f7024, 0x1400f76a4);
    char* skipMouseButtonsIfUnfocusedPos1 =
        GetCodeAddressJpEn(version, textRegion, 0x14012dfd2, 0x140131552);
    char* skipMouseButtonsIfUnfocusedPos2 =
        GetCodeAddressJpEn(version, textRegion, 0x141d43de0, 0x141d59f80);

    // don't silence audio output when unfocused
    int distance = silenceAudioIfUnfocusedPos2 - silenceAudioIfUnfocusedPos1;
    PageUnprotect page(logger, silenceAudioIfUnfocusedPos1, distance);
    {
        char* tmp = silenceAudioIfUnfocusedPos1;
        BranchHelper1Byte branch;
        branch.SetTarget(silenceAudioIfUnfocusedPos2);
        branch.WriteJump(tmp, JumpCondition::JMP);

        // the rest of this we reserve for the mouse click asm below
        // (there's not enough space there to write a 12byte absolute jmp)
        for (int i = 2; i < distance; ++i) {
            *tmp++ = 0xcc; // int 3
        }
    }

    // still run main game loop when unfocused
    {
        char* tmp = runMainGameLoopIfUnfocusedPos;
        PageUnprotect page(logger, tmp, 6);
        for (size_t i = 0; i < 6; ++i) {
            *tmp++ = 0x90; // nop
        }
    }

    const auto write_instruction_32 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 24) & 0xff);
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_24 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_16 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };

    // avoid processing mouse clicks when unfocused
    // (this previously happened only implicitly because the game didn't run...)
    // assemble some logic to skip mouse button processing when unfocused
    {
        char* codespaceStart = (silenceAudioIfUnfocusedPos1) + 2;
        char* codespaceEnd = silenceAudioIfUnfocusedPos2;
        char* codespace = codespaceStart;

        BranchHelper4Byte jump_to_codespace;
        jump_to_codespace.SetTarget(codespace);
        BranchHelper4Byte back_to_function;
        BranchHelper1Byte back_to_function_short;
        BranchHelper1Byte skip_processing;

        back_to_function.SetTarget(skipMouseButtonsIfUnfocusedPos1 + 6);
        int32_t relativeGetKeyStateAddress;
        std::memcpy(&relativeGetKeyStateAddress, skipMouseButtonsIfUnfocusedPos1 + 2, 4);
        char* GetKeyStateAddress =
            relativeGetKeyStateAddress + (skipMouseButtonsIfUnfocusedPos1 + 6);
        char* GameStateAddress = skipMouseButtonsIfUnfocusedPos2;

        using JC = JumpCondition;
        {
            char* tmp = skipMouseButtonsIfUnfocusedPos1;
            PageUnprotect page(logger, tmp, 6);
            jump_to_codespace.WriteJump(tmp, JC::JMP); // jmp jump_to_codespace
            *tmp = 0x90;                               // nop
        }

        const auto WriteRelativeAddress32 = [](char*& codepos, char* absoluteTarget) {
            char* absoluteHere = codepos + 4;
            int32_t a = static_cast<int32_t>(absoluteTarget - absoluteHere);
            std::memcpy(codepos, &a, 4);
            codepos += 4;
        };

        write_instruction_24(codespace, 0x488d05u); // lea rax,[address_that_holds_game_state]
        WriteRelativeAddress32(codespace, GameStateAddress);
        write_instruction_24(codespace, 0x488b00u);   // mov rax,[rax]
        write_instruction_24(codespace, 0x4885c0u);   // test rax,rax
        skip_processing.WriteJump(codespace, JC::JZ); // jz skip_processing
        write_instruction_24(codespace, 0x0fb680u);   // movzx eax,byte ptr[rax+1ce0h]
        write_instruction_32(codespace, 0xe01c0000u);
        write_instruction_16(codespace, 0x85c0);      // test eax,eax
        skip_processing.WriteJump(codespace, JC::JZ); // jz skip_processing
        write_instruction_24(codespace, 0x488d05u);   // lea rax,[GetKeyState]
        WriteRelativeAddress32(codespace, GetKeyStateAddress);
        write_instruction_24(codespace, 0x488b00u);           // mov rax,[rax]
        write_instruction_24(codespace, 0x4885c0u);           // test rax,rax
        skip_processing.WriteJump(codespace, JC::JZ);         // jz skip_processing
        write_instruction_16(codespace, 0xffd0);              // call rax
        back_to_function_short.WriteJump(codespace, JC::JMP); // jmp back_to_function_short
        skip_processing.SetTarget(codespace);
        write_instruction_24(codespace, 0x6633c0); // xor ax,ax
        back_to_function_short.SetTarget(codespace);
        back_to_function.WriteJump(codespace, JC::JMP); // jmp back_to_function
    }
}

static void PatchForceXInput(SenPatcher::Logger& logger,
                             char* textRegion,
                             GameVersion version,
                             char*& codespace,
                             char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* xinputCheckPos = GetCodeAddressJpEn(version, textRegion, 0x1406add65, 0x1406ba1b5);

    {
        char* tmp = xinputCheckPos;
        PageUnprotect page(logger, xinputCheckPos, 6);
        for (size_t i = 0; i < 6; ++i) {
            *tmp++ = 0x90; // nop
        }
    }
}

static void PatchFixControllerMappings(SenPatcher::Logger& logger,
                                       char* textRegion,
                                       GameVersion version,
                                       char*& codespace,
                                       char* codespaceEnd) {
    using namespace SenPatcher::x64;
    using JC = JumpCondition;
    const bool jp = version == GameVersion::Japanese;

    char* addressStructMemAlloc =
        GetCodeAddressJpEn(version, textRegion, 0x14012dd89, 0x140131309) + 2;
    char* addressInjectPos = GetCodeAddressJpEn(version, textRegion, 0x14012d88c, 0x140130e0e);
    char* addressMapLookupCode = GetCodeAddressJpEn(version, textRegion, 0x14012e086, 0x140131622);
    size_t lengthMapLookupCodeForDelete = (jp ? 0x45 : 0x3f);
    char* addressMapLookupSuccessForDelete =
        GetCodeAddressJpEn(version, textRegion, 0x14012e0d8, 0x14013166e);
    size_t lengthMapLookupSuccessForDelete = (jp ? 4 : 3);
    char* jpSwapActions45 = textRegion + (0x14012d785u - 0x140001000u); // only in JP build
    static constexpr size_t jpSwapActions45Len = 13;                    // only in JP build
    assert(addressMapLookupCode < addressMapLookupSuccessForDelete);

    const auto write_instruction_48 = [&](char*& codepos, uint64_t instr) {
        *codepos++ = (char)((instr >> 40) & 0xff);
        *codepos++ = (char)((instr >> 32) & 0xff);
        *codepos++ = (char)((instr >> 24) & 0xff);
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_40 = [&](char*& codepos, uint64_t instr) {
        *codepos++ = (char)((instr >> 32) & 0xff);
        *codepos++ = (char)((instr >> 24) & 0xff);
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_32 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 24) & 0xff);
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_24 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_16 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };

    // increase struct allocation by 0x20 bytes
    uint8_t allocLengthOld;
    {
        PageUnprotect page(logger, addressStructMemAlloc, 1);
        allocLengthOld = *addressStructMemAlloc;
        uint8_t allocLengthNew = allocLengthOld + 0x20;
        *addressStructMemAlloc = allocLengthNew;
    }

    {
        BranchHelper1Byte begin_loop_1;
        BranchHelper1Byte begin_loop_2;
        BranchHelper1Byte lookup_1;
        BranchHelper1Byte lookup_2;
        BranchHelper1Byte lookup_3;
        BranchHelper1Byte lookup_4;
        BranchHelper1Byte lookup_5;
        BranchHelper1Byte lookup_fail;

        auto injectResult = InjectJumpIntoCode<12>(logger, addressInjectPos, R64::RAX, codespace);

        // initialize the lookup table so every button points at itself
        write_instruction_32(
            codespace, 0x4d8d6500u | ((uint32_t)allocLengthOld)); // lea r12,[r13+allocLengthOld]
        write_instruction_24(codespace, 0x4d33c9);                // xor r9,r9
        begin_loop_1.SetTarget(codespace);
        write_instruction_32(codespace, 0x4f8d3c0c); // lea r15,[r12+r9]
        write_instruction_24(codespace, 0x45880f);   // mov byte ptr[r15],r9b
        write_instruction_24(codespace, 0x49ffc1);   // inc r9
        write_instruction_32(codespace, 0x4983f920); // cmp r9,20h
        begin_loop_1.WriteJump(codespace, JC::JB);   // jb begin_loop_1

        // look up each key in the (presumably) std::map<int, int>
        // and write it into the lookup table in the other direction
        write_instruction_24(codespace, 0x4d33c9); // xor r9,r9
        begin_loop_2.SetTarget(codespace);
        write_instruction_24(codespace, 0x498bdd);   // mov rbx,r13
        write_instruction_24(codespace, 0x488b03);   // mov rax,qword ptr [rbx]
        write_instruction_32(codespace, 0x488b5010); // mov rdx,qword ptr [rax + 0x10]
        write_instruction_24(codespace, 0x488bc2);   // mov rax,rdx
        write_instruction_32(codespace, 0x488b4a08); // mov rcx,qword ptr [rdx + 0x8]
        write_instruction_32(codespace, 0x80791900); // cmp byte ptr [rcx + 0x19],0x0
        lookup_4.WriteJump(codespace, JC::JNZ);      // jnz lookup_4
        lookup_1.SetTarget(codespace);
        write_instruction_32(codespace, 0x4439491c); // cmp dword ptr [rcx + 0x1c],r9d
        lookup_2.WriteJump(codespace, JC::JGE);      // jge lookup_2
        write_instruction_32(codespace, 0x488b4910); // mov rcx,qword ptr [rcx + 0x10]
        lookup_3.WriteJump(codespace, JC::JMP);      // jmp lookup_3
        lookup_2.SetTarget(codespace);
        write_instruction_24(codespace, 0x488bc1); // mov rax,rcx
        write_instruction_24(codespace, 0x488b09); // mov rcx,qword ptr [rcx]
        lookup_3.SetTarget(codespace);
        write_instruction_32(codespace, 0x80791900); // cmp byte ptr [rcx + 0x19],0x0
        lookup_1.WriteJump(codespace, JC::JZ);       // jz  lookup_1
        write_instruction_24(codespace, 0x483bc2);   // cmp rax,rdx
        lookup_4.WriteJump(codespace, JC::JZ);       // jz  lookup_4
        write_instruction_32(codespace, 0x443b481c); // cmp r9d,dword ptr [rax + 0x1c]
        lookup_5.WriteJump(codespace, JC::JGE);      // jge lookup_5
        lookup_4.SetTarget(codespace);
        write_instruction_24(codespace, 0x488bc2); // mov rax,rdx
        lookup_5.SetTarget(codespace);
        write_instruction_24(codespace, 0x483bc2); // cmp rax,rdx
        write_instruction_24(codespace, 0x418bc9); // mov ecx,r9d
        lookup_fail.WriteJump(codespace, JC::JZ);  // jz  lookup_fail
        write_instruction_24(codespace, 0x8b4820); // mov ecx,dword ptr[rax + 20h]
        lookup_fail.SetTarget(codespace);
        // r9d now contains the UNMAPPED value
        // ecx now contains the MAPPED value
        write_instruction_32(codespace, 0x45880c0c); // mov byte ptr[r12+rcx],r9b
        write_instruction_24(codespace, 0x49ffc1);   // inc r9
        write_instruction_32(codespace, 0x4983f920); // cmp r9,20h
        begin_loop_2.WriteJump(codespace, JC::JB);   // jb begin_loop_2

        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        write_instruction_24(codespace, 0x498bc5); // mov rax,r13 ; restore clobbered rax

        Emit_MOV_R64_IMM64(
            codespace, R64::RSI, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RSI);
    }

    {
        PageUnprotect page(logger,
                           addressMapLookupCode,
                           (addressMapLookupSuccessForDelete - addressMapLookupCode)
                               + lengthMapLookupSuccessForDelete);

        // clear out old lookup code
        char* tmp = addressMapLookupCode;
        for (size_t i = 0; i < lengthMapLookupCodeForDelete; ++i) {
            *tmp++ = 0x90; // nop
        }
        tmp = addressMapLookupSuccessForDelete;
        for (size_t i = 0; i < lengthMapLookupSuccessForDelete; ++i) {
            *tmp++ = 0x90; // nop
        }

        // replace with new lookup code
        BranchHelper1Byte lookup_success;
        BranchHelper1Byte lookup_fail;

        // on success: result should be in ecx (on US) or eax (on JP)
        // on fail: result doesn't matter, restores itself
        lookup_success.SetTarget(addressMapLookupSuccessForDelete
                                 + lengthMapLookupSuccessForDelete);
        lookup_fail.SetTarget(addressMapLookupCode + lengthMapLookupCodeForDelete);

        tmp = addressMapLookupCode;
        if (!jp) {
            write_instruction_24(tmp, 0x418bc1); // mov eax,r9d
        }
        write_instruction_24(tmp, 0x83f820); // cmp eax,20h
        lookup_fail.WriteJump(tmp, JC::JNB); // jnb lookup_fail
        if (!jp) {
            write_instruction_32(tmp, 0x0fb64c03u); // movzx ecx,byte ptr[rbx+rax+allocLengthOld]
            *tmp++ = allocLengthOld;
            write_instruction_24(tmp, 0x83f920); // cmp ecx,20h
        } else {
            write_instruction_32(tmp, 0x0fb64403u); // movzx eax,byte ptr[rbx+rax+allocLengthOld]
            *tmp++ = allocLengthOld;
            write_instruction_24(tmp, 0x83f820); // cmp eax,20h
        }
        lookup_fail.WriteJump(tmp, JC::JNB);    // jnb lookup_fail
        lookup_success.WriteJump(tmp, JC::JMP); // jmp lookup_success
    }

    if (jp) {
        // swap actions 4/5 on controller config readin so we get identical mapping behavior between
        // JP/US builds
        auto injectResult =
            InjectJumpIntoCode<jpSwapActions45Len>(logger, jpSwapActions45, R64::RDX, codespace);
        const auto& ow = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, ow.data(), ow.size() - 5);
        codespace += ow.size() - 5;

        // the last 5 bytes we overwrote was a call instruction with a relative address, fix that
        int32_t relativeAddress;
        std::memcpy(&relativeAddress, ow.data() + ow.size() - 4, 4);
        char* absoluteAddress = injectResult.JumpBackAddress + relativeAddress;
        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(absoluteAddress));
        Emit_CALL_R64(codespace, R64::RAX); // call (replaced function call)

        BranchHelper1Byte check5;
        BranchHelper1Byte checkdone;
        write_instruction_40(codespace, 0x488d4c2450);   // lea rcx,[rsp+50h]
        write_instruction_16(codespace, 0x8b11);         // mov edx,dword ptr[rcx]
        write_instruction_24(codespace, 0x83fa04);       // cmp edx,4
        check5.WriteJump(codespace, JC::JNE);            // jne check5
        write_instruction_48(codespace, 0xc70105000000); // mov dword ptr[rcx],5
        write_instruction_16(codespace, 0xeb0b);         // jmp checkdone
        check5.SetTarget(codespace);
        write_instruction_24(codespace, 0x83fa05);       // cmp edx,5
        checkdone.WriteJump(codespace, JC::JNE);         // jne checkdone
        write_instruction_48(codespace, 0xc70104000000); // mov dword ptr[rcx],4
        checkdone.SetTarget(codespace);

        Emit_MOV_R64_IMM64(
            codespace, R64::RCX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RCX);
    }
}

static bool CaseInsensitiveEquals(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        const char cl = (lhs[i] >= 'A' && lhs[i] <= 'Z') ? (lhs[i] + ('a' - 'A')) : lhs[i];
        const char cr = (rhs[i] >= 'A' && rhs[i] <= 'Z') ? (rhs[i] + ('a' - 'A')) : rhs[i];
        if (cl != cr) {
            return false;
        }
    }
    return true;
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

    bool assetFixes = true;
    bool fixInGameButtonMappingValidity = true;
    bool allowSwitchToNightmare = true;
    bool fixControllerMapping = true;
    bool disableMouseCapture = false;
    bool showMouseCursor = false;
    bool disablePauseOnFocusLoss = false;
    bool forceXInput = false;
    bool swapBrokenMasterQuartzValuesForDisplay = true;

    {
        SenPatcher::IO::File settingsFile(baseDir / L"senpatcher_settings.ini",
                                          SenPatcher::IO::OpenMode::Read);
        if (settingsFile.IsOpen()) {
            SenPatcher::IniFile ini;
            if (ini.ParseFile(settingsFile)) {
                const auto check_boolean =
                    [&](std::string_view section, std::string_view key, bool& b) {
                        const auto* kvp = ini.FindValue(section, key);
                        if (kvp) {
                            if (CaseInsensitiveEquals(kvp->Value, "true")) {
                                logger.Log("Patch ");
                                logger.Log(key);
                                logger.Log(" enabled via ini.\n");
                                b = true;
                            } else if (CaseInsensitiveEquals(kvp->Value, "false")) {
                                logger.Log("Patch ");
                                logger.Log(key);
                                logger.Log(" disabled via ini.\n");
                                b = false;
                            } else {
                                logger.Log("Patch ");
                                logger.Log(key);
                                logger.Log(" not found in ini, leaving default.\n");
                            }
                        }
                    };
                check_boolean("CS3", "AssetFixes", assetFixes);
                check_boolean("CS3", "FixInGameButtonRemapping", fixInGameButtonMappingValidity);
                check_boolean("CS3", "AllowSwitchToNightmare", allowSwitchToNightmare);
                check_boolean("CS3", "FixControllerMapping", fixControllerMapping);
                check_boolean("CS3", "DisableMouseCapture", disableMouseCapture);
                check_boolean("CS3", "ShowMouseCursor", showMouseCursor);
                check_boolean("CS3", "DisablePauseOnFocusLoss", disablePauseOnFocusLoss);
                check_boolean("CS3", "ForceXInput", forceXInput);
            }
        }
    }

    if (assetFixes) {
        SenLib::Sen3::CreateAssetPatchIfNeeded(logger, baseDir);
    }

    LoadModP3As(logger, baseDir);

    InjectAtFFileOpen(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    InjectAtFFileGetFilesize(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    InjectAtFreestandingGetFilesize(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    InjectAtOpenFSoundFile(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);

    DeglobalizeMutexes(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    AddSenPatcherVersionToTitle(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);

    if (fixInGameButtonMappingValidity) {
        FixInGameButtonMappingValidity(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    }
    if (allowSwitchToNightmare) {
        AllowSwitchToNightmare(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    }
    if (swapBrokenMasterQuartzValuesForDisplay) {
        SwapBrokenMasterQuartzValuesForDisplay(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    }
    if (disableMouseCapture) {
        PatchDisableMouseCapture(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    }
    if (showMouseCursor) {
        PatchShowMouseCursor(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    }
    if (disablePauseOnFocusLoss) {
        PatchDisablePauseOnFocusLoss(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    }
    if (forceXInput) {
        PatchForceXInput(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    }
    if (fixControllerMapping) {
        PatchFixControllerMappings(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    }


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
