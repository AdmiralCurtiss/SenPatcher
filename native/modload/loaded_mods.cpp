#include "loaded_mods.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

#include "lz4/lz4.h"
#include "zstd/common/xxhash.h"
#include "zstd/zdict.h"
#include "zstd/zstd.h"

#include "file.h"
#include "util/text.h"

namespace {
struct ZSTD_DCtx_Deleter {
    void operator()(ZSTD_DCtx* ptr) {
        if (ptr) {
            ZSTD_freeDCtx(ptr);
        }
    }
};
using ZSTD_DCtx_UniquePtr = std::unique_ptr<ZSTD_DCtx, ZSTD_DCtx_Deleter>;
} // namespace

namespace SenLib::ModLoad {
void FilterP3APath(char8_t* path, size_t length) {
    size_t in = 0;
    size_t out = 0;

    while (in < length) { // in is always >= out so no need to check out
        const char8_t c = path[in];
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
                path[out] = static_cast<char8_t>(c + ('a' - 'A'));
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

bool FilterGamePath(char8_t* out_path, const char* in_path, size_t length) {
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
                out_path[out] = static_cast<char8_t>(c + ('a' - 'A'));
            } else {
                out_path[out] = static_cast<char8_t>(c);
            }
            ++in;
            ++out;
        }
    }
    return in_path[in] == '\0';
}

static bool PathIsAllowed(std::string_view path) {
    if (path.empty()) {
        return false;
    }

    std::string_view rest = path;
    while (true) {
        size_t nextPathSeparator = rest.find_first_of("/\\");
        std::string_view currentElement;
        if (nextPathSeparator == std::string_view::npos) {
            currentElement = rest;
            rest = std::string_view();
        } else {
            currentElement = rest.substr(0, nextPathSeparator);
            rest = rest.substr(nextPathSeparator + 1);
        }

        if (currentElement.empty()) {
            // empty path element is not allowed
            return false;
        }

        if (std::all_of(
                currentElement.begin(), currentElement.end(), [](char c) { return c == '.'; })) {
            // all dot path element is not allowed
            return false;
        }

        if (rest.empty()) {
            return true;
        }
    }
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
        if (!PathIsAllowed(line)) {
            continue;
        }
        auto& p = output.emplace_back(Utf8ToUtf16(line.data(), line.size()));
        if (p.is_absolute()) {
            // don't allow absolute paths either
            output.pop_back();
        }
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

static bool IsEquivalentPath(std::wstring_view lhs, std::wstring_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        const char cl =
            (lhs[i] == L'\\'
                 ? L'/'
                 : ((lhs[i] >= L'A' && lhs[i] <= L'Z') ? (lhs[i] + (L'a' - L'A')) : lhs[i]));
        const char cr =
            (rhs[i] == L'\\'
                 ? L'/'
                 : ((rhs[i] >= L'A' && rhs[i] <= L'Z') ? (rhs[i] + (L'a' - L'A')) : rhs[i]));
        if (cl != cr) {
            return false;
        }
    }
    return true;
}

static bool ContainsPath(const std::vector<std::filesystem::path>& paths,
                         const std::filesystem::path& searchpath) {
    for (const auto& path : paths) {
        if (IsEquivalentPath(path.native(), searchpath.native())) {
            return true;
        }
    }
    return false;
}

static void MoveSenpatcherAssetsToEnd(std::vector<std::filesystem::path>& paths) {
    std::wstring_view prefix = L"zzz_senpatcher_";
    const auto is_senpatcher_asset = [&](const std::filesystem::path& p) {
        const auto& str = p.native();
        if (str.size() < 15) {
            return false;
        }
        return IsEquivalentPath(prefix, std::wstring_view(str).substr(0, 15));
    };
    std::stable_sort(paths.begin(),
                     paths.end(),
                     [&](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
                         int lhsIsSenpatcherAsset = is_senpatcher_asset(lhs) ? 1 : 0;
                         int rhsIsSenpatcherAsset = is_senpatcher_asset(rhs) ? 1 : 0;
                         return lhsIsSenpatcherAsset < rhsIsSenpatcherAsset;
                     });
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
            if (!ContainsPath(paths, filename)) {
                paths.emplace_back(std::move(filename));
                modified = true;
            }
        }
    }

    if (modified) {
        // new files from the directory were added, so to avoid confusing behavior where the
        // built-in asset patches override a mod, move them to the end of the list
        MoveSenpatcherAssetsToEnd(paths);

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

void LoadModP3As(SenPatcher::Logger& logger,
                 LoadedModsData& loadedModsData,
                 std::string_view baseDir) {
    loadedModsData.CombinedFileInfoCount = 0;
    loadedModsData.CombinedFileInfos.reset();
    loadedModsData.P3As.reset();
    loadedModsData.CheckDevFolderForAssets = false;

    std::filesystem::path baseDirStdFs =
        HyoutaUtils::TextUtils::Utf8ToWString(baseDir.data(), baseDir.size());

    {
        std::error_code ec;
        loadedModsData.CheckDevFolderForAssets =
            std::filesystem::is_directory(baseDirStdFs / L"dev", ec);
    }

    size_t p3acount = 0;
    std::unique_ptr<P3AData[]> p3as;
    {
        const auto modsDir = baseDirStdFs / L"mods";
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

    loadedModsData.P3As = std::move(p3as);
    loadedModsData.CombinedFileInfoCount = totalFileInfoCount;
    loadedModsData.CombinedFileInfos = std::move(combinedFileInfos);

    return;
}

const P3AFileRef* FindP3AFileRef(const LoadedModsData& loadedModsData,
                                 const std::array<char8_t, 0x100>& filteredPath) {
    const size_t count = loadedModsData.CombinedFileInfoCount;
    const P3AFileRef* const infos = loadedModsData.CombinedFileInfos.get();
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

bool ExtractP3AFileToMemory(const P3AFileRef& ref,
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
            if (fi.CompressedSize != fi.UncompressedSize) {
                return false;
            }

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
            if (fi.UncompressedSize > static_cast<uint32_t>(LZ4_MAX_INPUT_SIZE)
                || fi.CompressedSize
                       > static_cast<uint32_t>(LZ4_COMPRESSBOUND(LZ4_MAX_INPUT_SIZE))) {
                return false;
            }

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
                                    static_cast<int>(fi.CompressedSize),
                                    static_cast<int>(fi.UncompressedSize))
                != static_cast<int>(fi.UncompressedSize)) {
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
} // namespace SenLib::ModLoad
