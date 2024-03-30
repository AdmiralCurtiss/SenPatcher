#include "loaded_mods.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
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
void FilterP3APath(char* path, size_t length) {
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
                path[out] = static_cast<char>(c + ('a' - 'A'));
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

bool FilterGamePath(char* out_path, const char* in_path, size_t length) {
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
                out_path[out] = static_cast<char>(c + ('a' - 'A'));
            } else {
                out_path[out] = c;
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

    // check for path starting with C: or similar
    if (path.size() >= 2) {
        if (path[1] == ':') {
            return false;
        }
    }

    // the following loop will reject all paths starting with '\' so we don't need to check things
    // like UNC paths or root-relative paths separately

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

static bool LoadOrderTxt(std::vector<std::string>& output,
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
        output.emplace_back(line.data(), line.size());
    }

    return true;
}

static bool WriteOrderTxt(const std::vector<std::string>& paths,
                          SenPatcher::Logger& logger,
                          SenPatcher::IO::File& file) {
    if (!file.IsOpen()) {
        return false;
    }

    for (const auto& utf8 : paths) {
        if (file.Write(utf8.data(), utf8.size()) != utf8.size()) {
            return false;
        }
        if (file.Write("\r\n", 2) != 2) {
            return false;
        }
    }

    return true;
}

static bool IsEquivalentPath(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        const char cl = static_cast<char>(
            lhs[i] == '\\' ? '/'
                           : ((lhs[i] >= 'A' && lhs[i] <= 'Z') ? (lhs[i] + ('a' - 'A')) : lhs[i]));
        const char cr = static_cast<char>(
            rhs[i] == '\\' ? '/'
                           : ((rhs[i] >= 'A' && rhs[i] <= 'Z') ? (rhs[i] + ('a' - 'A')) : rhs[i]));
        if (cl != cr) {
            return false;
        }
    }
    return true;
}

static bool ContainsPath(const std::vector<std::string>& paths, const std::string& searchpath) {
    for (const auto& path : paths) {
        if (IsEquivalentPath(path, searchpath)) {
            return true;
        }
    }
    return false;
}

static void MoveSenpatcherAssetsToEnd(std::vector<std::string>& paths) {
    static constexpr std::string_view prefix = "zzz_senpatcher_";
    const auto is_senpatcher_asset = [&](const std::string& str) {
        if (str.size() < prefix.size()) {
            return false;
        }
        return IsEquivalentPath(prefix, std::string_view(str).substr(0, prefix.size()));
    };
    std::stable_sort(
        paths.begin(), paths.end(), [&](const std::string& lhs, const std::string& rhs) {
            int lhsIsSenpatcherAsset = is_senpatcher_asset(lhs) ? 1 : 0;
            int rhsIsSenpatcherAsset = is_senpatcher_asset(rhs) ? 1 : 0;
            return lhsIsSenpatcherAsset < rhsIsSenpatcherAsset;
        });
}

static std::vector<std::string> CollectModPaths(SenPatcher::Logger& logger,
                                                const std::string& modsDir) {
    std::vector<std::string> paths;
    std::string orderPath;
    orderPath.reserve(modsDir.size() + 10);
    orderPath.append(modsDir);
    orderPath.push_back('/');
    orderPath.append("order.txt");

    // check for order.txt, which may define an order (priority) for the mods
    {
        SenPatcher::IO::File orderfile(std::string_view(orderPath), SenPatcher::IO::OpenMode::Read);
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
            auto filenamepath = path.filename();
            std::string filename = HyoutaUtils::TextUtils::WStringToUtf8(
                filenamepath.native().data(), filenamepath.native().size());
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
        std::string orderPathTmp;
        orderPathTmp.reserve(orderPath.size() + 4);
        orderPathTmp.append(orderPath);
        orderPathTmp.append(".tmp");
        SenPatcher::IO::File orderfile(std::string_view(orderPathTmp),
                                       SenPatcher::IO::OpenMode::Write);
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

void CreateModDirectory(std::string_view baseDir) {
    std::string modsDir;
    modsDir.reserve(baseDir.size() + 5);
    modsDir.append(baseDir);
    modsDir.push_back('/');
    modsDir.append("mods");
    SenPatcher::IO::CreateDirectory(std::string_view(modsDir));
}

void LoadModP3As(SenPatcher::Logger& logger,
                 LoadedModsData& loadedModsData,
                 std::string_view baseDir) {
    loadedModsData.CombinedFileInfoCount = 0;
    loadedModsData.CombinedFileInfos.reset();
    loadedModsData.P3As.reset();
    loadedModsData.CheckDevFolderForAssets = false;

    {
        std::string devPath;
        devPath.reserve(baseDir.size() + 4);
        devPath.append(baseDir);
        devPath.push_back('/');
        devPath.append("dev");
        loadedModsData.CheckDevFolderForAssets =
            SenPatcher::IO::DirectoryExists(std::string_view(devPath));
    }

    size_t p3acount = 0;
    std::unique_ptr<P3AData[]> p3as;
    {
        std::string modsDir;
        modsDir.reserve(baseDir.size() + 5);
        modsDir.append(baseDir);
        modsDir.push_back('/');
        modsDir.append("mods");

        const auto modPaths = CollectModPaths(logger, modsDir);
        std::vector<SenPatcher::P3A> p3avector;
        p3avector.reserve(modPaths.size());
        for (auto const& path : modPaths) {
            SenPatcher::P3A& p3a = p3avector.emplace_back();

            std::string filepath;
            filepath.reserve(modsDir.size() + path.size() + 1);
            filepath.append(modsDir);
            filepath.push_back('/');
            filepath.append(path);
            if (!p3a.Load(std::string_view(filepath))) {
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
                             return strncmp(lhs.FileInfo->Filename.data(),
                                            rhs.FileInfo->Filename.data(),
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
                if (strncmp(last.FileInfo->Filename.data(),
                            next.FileInfo->Filename.data(),
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
                                 const std::array<char, 0x100>& filteredPath) {
    const size_t count = loadedModsData.CombinedFileInfoCount;
    const P3AFileRef* const infos = loadedModsData.CombinedFileInfos.get();
    auto bound = std::lower_bound(
        infos,
        infos + count,
        filteredPath,
        [](const P3AFileRef& lhs, const std::array<char, 0x100>& rhs) {
            return strncmp(lhs.FileInfo->Filename.data(), rhs.data(), rhs.size()) < 0;
        });

    if (bound != (infos + count)
        && strncmp(bound->FileInfo->Filename.data(), filteredPath.data(), filteredPath.size())
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
