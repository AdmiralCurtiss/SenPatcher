#include "loaded_mods.h"

#include <algorithm>
#include <charconv>
#include <cstring>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "lz4/lz4.h"
#include "zstd/common/xxhash.h"
#include "zstd/zdict.h"
#include "zstd/zstd.h"

#include "util/file.h"
#include "util/ini.h"
#include "util/text.h"

// Feature level for senpatcher_mod.ini. p3a files can set a feature level via 'MinFeatureLevel' and
// SenPatcher (since version 1.3) will not load it.
// Features by version:
// 0 -> Support for the 'Language' key, which allows 'English', 'Japanese', and 'All' values
// 1 -> Support for the 'StripPathPrefix' key, which strips a prefix from all paths matching it and
//      drops all files that don't match the prefix
#define SENPATCHER_MOD_MIN_FEATURE_LEVEL 1

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
                         HyoutaUtils::Logger& logger,
                         HyoutaUtils::IO::File& file) {
    using HyoutaUtils::TextUtils::Trim;

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
                          HyoutaUtils::Logger& logger,
                          HyoutaUtils::IO::File& file) {
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

static bool IsSenpatcherAsset(const std::string& str) {
    static constexpr std::string_view prefix = "zzz_senpatcher_";
    if (str.size() < prefix.size()) {
        return false;
    }
    return IsEquivalentPath(prefix, std::string_view(str).substr(0, prefix.size()));
}

static void MoveSenpatcherAssetsToEnd(std::vector<std::string>& paths) {
    std::stable_sort(
        paths.begin(), paths.end(), [&](const std::string& lhs, const std::string& rhs) {
            int lhsIsSenpatcherAsset = IsSenpatcherAsset(lhs) ? 1 : 0;
            int rhsIsSenpatcherAsset = IsSenpatcherAsset(rhs) ? 1 : 0;
            return lhsIsSenpatcherAsset < rhsIsSenpatcherAsset;
        });
}

static std::vector<std::string> CollectModPaths(HyoutaUtils::Logger& logger,
                                                const std::string& modsDir) {
    std::vector<std::string> paths;
    std::string orderPath;
    orderPath.reserve(modsDir.size() + 10);
    orderPath.append(modsDir);
    orderPath.push_back('/');
    orderPath.append("order.txt");

    // check for order.txt, which may define an order (priority) for the mods
    {
        HyoutaUtils::IO::File orderfile(std::string_view(orderPath),
                                        HyoutaUtils::IO::OpenMode::Read);
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
    while (iterator != std::filesystem::directory_iterator()) {
        if (!iterator->is_directory()) {
            auto& path = iterator->path();
            auto& string = path.native();
            if (string.size() >= 4 && string[string.size() - 4] == L'.'
                && (string[string.size() - 3] == L'P' || string[string.size() - 3] == L'p')
                && string[string.size() - 2] == L'3'
                && (string[string.size() - 1] == L'A' || string[string.size() - 1] == L'a')) {
                auto filenamepath = path.filename();
#ifdef BUILD_FOR_WINDOWS
                auto filename = HyoutaUtils::TextUtils::WStringToUtf8(filenamepath.native().data(),
                                                                      filenamepath.native().size());
#else
                std::optional<std::string> filename = filenamepath.native();
#endif
                if (filename && !ContainsPath(paths, *filename)) {
                    paths.emplace_back(std::move(*filename));
                    modified = true;
                }
            }
        }
        iterator.increment(ec);
        if (ec) {
            return paths;
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
        HyoutaUtils::IO::File orderfile(std::string_view(orderPathTmp),
                                        HyoutaUtils::IO::OpenMode::Write);
        if (WriteOrderTxt(paths, logger, orderfile)) {
            if (!orderfile.Rename(std::string_view(orderPath))) {
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
    HyoutaUtils::IO::CreateDirectory(std::string_view(modsDir));
}

static void MakeP3ABinarySearchable(std::unique_ptr<P3AFileRef[]>& combinedFileInfos,
                                    size_t& totalFileInfoCount,
                                    P3AData* p3as,
                                    size_t p3acount) {
    combinedFileInfos.reset();
    totalFileInfoCount = 0;
    for (size_t i = 0; i < p3acount; ++i) {
        totalFileInfoCount += p3as[i].Archive.FileCount;
    }
    if (totalFileInfoCount == 0) {
        return;
    }

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

void LoadP3As(HyoutaUtils::Logger& logger,
              LoadedP3AData& loadedP3AData,
              std::string_view baseDir,
              std::span<const std::string_view> files) {
    loadedP3AData.CombinedFileInfoCount = 0;
    loadedP3AData.CombinedFileInfos.reset();
    loadedP3AData.P3As.reset();
    if (files.empty()) {
        return;
    }

    size_t p3acount = 0;
    std::unique_ptr<P3AData[]> p3as;
    {
        std::vector<std::pair<SenPatcher::P3A, uint32_t>> p3avector;
        p3avector.reserve(files.size());
        for (std::string_view filename : files) {
            std::string filepath;
            filepath.reserve(baseDir.size() + filename.size() + 1);
            filepath.append(baseDir);
            filepath.push_back('/');
            filepath.append(filename);

            auto& p3apair = p3avector.emplace_back();
            SenPatcher::P3A& p3a = p3apair.first;

            uint32_t p3aflags = 0;
            if (!p3a.Load(std::string_view(filepath), &p3aflags)) {
                p3avector.pop_back();
                continue;
            }
            p3apair.second = p3aflags;

            for (size_t i = 0; i < p3a.FileCount; ++i) {
                auto& fileinfo = p3a.FileInfo[i];
                FilterP3APath(fileinfo.Filename.data(), fileinfo.Filename.size());
            }
        }

        p3acount = p3avector.size();
        p3as = std::make_unique<P3AData[]>(p3acount);
        for (size_t i = 0; i < p3acount; ++i) {
            p3as[i].Archive = std::move(p3avector[i].first);
            p3as[i].Flags = p3avector[i].second;
        }
    }

    std::unique_ptr<P3AFileRef[]> combinedFileInfos;
    size_t totalFileInfoCount = 0;
    MakeP3ABinarySearchable(combinedFileInfos, totalFileInfoCount, p3as.get(), p3acount);

    loadedP3AData.P3As = std::move(p3as);
    loadedP3AData.CombinedFileInfoCount = totalFileInfoCount;
    loadedP3AData.CombinedFileInfos = std::move(combinedFileInfos);
}

// Returns true if the p3a is acceptable, false if it should be skipped.
static bool HandleSenpatcherSettingsIni(
    HyoutaUtils::Logger& logger,
    SenPatcher::P3A& p3a,
    const char* iniBuffer,
    size_t iniBufferLength,
    std::string_view iniCategory,
    bool isRunningInJapaneseLanguage,
    const std::function<void(const HyoutaUtils::Ini::IniFile& ini)>& iniHandler) {
    HyoutaUtils::Ini::IniFile ini;
    if (!ini.ParseExternalMemory(iniBuffer, iniBufferLength)) {
        return false;
    }

    bool hasGameCategory = false;
    for (const auto& kvp : ini.GetValues()) {
        if (kvp.Section == iniCategory) {
            hasGameCategory = true;
            break;
        }
    }
    if (!hasGameCategory) {
        // if there's an ini but no section for this game, this p3a is for a different game
        logger.Log("Didn't find game section [");
        logger.Log(iniCategory);
        logger.Log("] in ini\n");
        return false;
    }

    auto* featureLevel = ini.FindValue(iniCategory, "MinFeatureLevel");
    if (featureLevel) {
        unsigned int intval = 0;
        const auto [_, ec] =
            std::from_chars(featureLevel->Value.data(),
                            featureLevel->Value.data() + featureLevel->Value.size(),
                            intval);
        if (ec == std::errc()) {
            if (intval > SENPATCHER_MOD_MIN_FEATURE_LEVEL) {
                // requested feature level exceeds the level this build implements
                logger.Log("Feature level ");
                logger.LogInt(intval);
                logger.Log(" > ");
                logger.LogInt(SENPATCHER_MOD_MIN_FEATURE_LEVEL);
                logger.Log("\n");
                return false;
            }
        } else {
            // not a valid integer, skip always
            return false;
        }
    }
    auto* lang = ini.FindValue(iniCategory, "Language");
    if (lang) {
        using HyoutaUtils::TextUtils::CaseInsensitiveEquals;
        if (isRunningInJapaneseLanguage) {
            if (!(CaseInsensitiveEquals("Japanese", lang->Value)
                  || CaseInsensitiveEquals("All", lang->Value))) {
                logger.Log("Language mismatch\n");
                return false;
            }
        } else {
            if (!(CaseInsensitiveEquals("English", lang->Value)
                  || CaseInsensitiveEquals("All", lang->Value))) {
                logger.Log("Language mismatch\n");
                return false;
            }
        }
    }

    auto* stripPrefix = ini.FindValue(iniCategory, "StripPathPrefix");
    if (stripPrefix && stripPrefix->Value.size() > 0) {
        std::string strip = HyoutaUtils::TextUtils::ToLower(stripPrefix->Value);
        uint64_t out = 0;
        for (uint64_t in = 0; in < p3a.FileCount; ++in) {
            auto& f = p3a.FileInfo[in];
            auto path = HyoutaUtils::TextUtils::StripToNull(f.Filename);
            if (path.starts_with(strip)) {
                std::array<char, 256> newPath{};
                for (size_t i = strip.size(), j = 0; i < path.size(); ++i, ++j) {
                    newPath[j] = path[i];
                }
                f.Filename = newPath;
                if (in != out) {
                    p3a.FileInfo[out] = f;
                }
                ++out;
            }
        }
        p3a.FileCount = out;
    }

    iniHandler(ini);
    return true;
}

void LoadModP3As(HyoutaUtils::Logger& logger,
                 LoadedModsData& loadedModsData,
                 std::string_view baseDir,
                 bool shouldLoadAssetFixes,
                 std::string_view iniCategory,
                 bool isRunningInJapaneseLanguage,
                 const std::function<void(const HyoutaUtils::Ini::IniFile& ini)>& iniHandler) {
    loadedModsData.LoadedP3As.CombinedFileInfoCount = 0;
    loadedModsData.LoadedP3As.CombinedFileInfos.reset();
    loadedModsData.LoadedP3As.P3As.reset();
    loadedModsData.CheckDevFolderForAssets = false;

    {
        std::string devPath;
        devPath.reserve(baseDir.size() + 4);
        devPath.append(baseDir);
        devPath.push_back('/');
        devPath.append("dev");
        loadedModsData.CheckDevFolderForAssets =
            HyoutaUtils::IO::DirectoryExists(std::string_view(devPath));
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
        std::vector<std::pair<SenPatcher::P3A, uint32_t>> p3avector;
        p3avector.reserve(modPaths.size());
        for (auto const& path : modPaths) {
            const bool isSenpatcherAsset = IsSenpatcherAsset(path);
            if (!shouldLoadAssetFixes && isSenpatcherAsset) {
                continue;
            }

            auto& p3apair = p3avector.emplace_back();
            SenPatcher::P3A& p3a = p3apair.first;

            std::string filepath;
            filepath.reserve(modsDir.size() + path.size() + 1);
            filepath.append(modsDir);
            filepath.push_back('/');
            filepath.append(path);
            logger.Log("Loading mod file at ").Log(filepath).Log("...\n");
            uint32_t p3aflags = 0;
            if (!p3a.Load(std::string_view(filepath), &p3aflags)) {
                logger.Log("Couldn't load, skipping.\n");
                p3avector.pop_back();
                continue;
            }

            for (size_t i = 0; i < p3a.FileCount; ++i) {
                auto& fileinfo = p3a.FileInfo[i];
                FilterP3APath(fileinfo.Filename.data(), fileinfo.Filename.size());
            }

            // check ini and skip this file if it's language-limited to a different language
            // TODO: can we do this in a cleaner way? this works but looks like a mess...
            const bool shouldSkip = [&]() -> bool {
                for (size_t i = 0; i < p3a.FileCount; ++i) {
                    auto& fileinfo = p3a.FileInfo[i];
                    if (strncmp(fileinfo.Filename.data(),
                                "senpatcher_mod.ini",
                                fileinfo.Filename.size())
                        == 0) {
                        void* out_memory = nullptr;
                        uint64_t out_filesize = 0;
                        if (ExtractP3AFileToMemory(
                                fileinfo,
                                p3a,
                                p3aflags,
                                nullptr,
                                0x8000'0000,
                                out_memory,
                                out_filesize,
                                [](size_t length) { return malloc(length); },
                                [](void* memory) { free(memory); })) {
                            bool skip = !HandleSenpatcherSettingsIni(logger,
                                                                     p3a,
                                                                     static_cast<char*>(out_memory),
                                                                     out_filesize,
                                                                     iniCategory,
                                                                     isRunningInJapaneseLanguage,
                                                                     iniHandler);
                            free(out_memory);
                            return skip;
                        } else {
                            // failed to read ini, assume this p3a is broken
                            return true;
                        }
                    }
                }

                return false;
            }();
            if (shouldSkip) {
                logger.Log("Skipped via senpatcher_mod.ini\n");
                p3avector.pop_back();
                continue;
            }

            p3apair.second = p3aflags;
            if (isSenpatcherAsset) {
                p3apair.second |= P3AFlag_IsSenPatcherAssetMod;
            }
        }

        p3acount = p3avector.size();
        p3as = std::make_unique<P3AData[]>(p3acount);
        for (size_t i = 0; i < p3acount; ++i) {
            p3as[i].Archive = std::move(p3avector[i].first);
            p3as[i].Flags = p3avector[i].second;
        }
    }

    std::unique_ptr<P3AFileRef[]> combinedFileInfos;
    size_t totalFileInfoCount = 0;
    MakeP3ABinarySearchable(combinedFileInfos, totalFileInfoCount, p3as.get(), p3acount);

    loadedModsData.LoadedP3As.P3As = std::move(p3as);
    loadedModsData.LoadedP3As.CombinedFileInfoCount = totalFileInfoCount;
    loadedModsData.LoadedP3As.CombinedFileInfos = std::move(combinedFileInfos);
}

const P3AFileRef* FindP3AFileRef(const LoadedP3AData& loadedModsData,
                                 const std::array<char, 0x100>& filteredPath) {
    const P3AFileRef* infos = loadedModsData.CombinedFileInfos.get();
    size_t count = loadedModsData.CombinedFileInfoCount;
    while (true) {
        if (count == 0) {
            return nullptr;
        }

        const size_t countHalf = count / 2;
        const P3AFileRef* middle = infos + countHalf;
        const int cmp =
            strncmp(middle->FileInfo->Filename.data(), filteredPath.data(), filteredPath.size());
        if (cmp < 0) {
            infos = middle + 1;
            count = count - (countHalf + 1);
        } else if (cmp > 0) {
            count = countHalf;
        } else {
            return middle;
        }
    }
}

bool ExtractP3AFileToMemory(const P3AFileRef& ref,
                            uint32_t p3aFlags,
                            uint64_t filesizeLimit,
                            void*& out_memory,
                            uint64_t& out_filesize,
                            PMalloc malloc_func,
                            PFree free_func) {
    return ExtractP3AFileToMemory(*ref.FileInfo,
                                  ref.ArchiveData->Archive,
                                  p3aFlags,
                                  &ref.ArchiveData->Mutex,
                                  filesizeLimit,
                                  out_memory,
                                  out_filesize,
                                  malloc_func,
                                  free_func);
}

bool ExtractP3AFileToMemory(const SenPatcher::P3AFileInfo& fi,
                            SenPatcher::P3A& archive,
                            uint32_t p3aFlags,
                            std::recursive_mutex* optional_mutex,
                            uint64_t filesizeLimit,
                            void*& out_memory,
                            uint64_t& out_filesize,
                            PMalloc malloc_func,
                            PFree free_func) {
    if (fi.UncompressedSize >= filesizeLimit) {
        return false;
    }

    switch (fi.CompressionType) {
        case SenPatcher::P3ACompressionType::None: {
            if (fi.CompressedSize != fi.UncompressedSize) {
                return false;
            }
            if ((p3aFlags & SenPatcher::P3AFlag_HasUncompressedHashes) != 0
                && fi.CompressedHash != fi.UncompressedHash) {
                return false;
            }

            void* memory = malloc_func(fi.UncompressedSize);
            if (!memory) {
                return false;
            }

            {
                std::optional<std::lock_guard<std::recursive_mutex>> guard;
                if (optional_mutex) {
                    guard.emplace(*optional_mutex);
                }
                auto& file = archive.FileHandle;
                if (!file.SetPosition(fi.Offset)) {
                    free_func(memory);
                    return false;
                }
                if (file.Read(memory, fi.UncompressedSize) != fi.UncompressedSize) {
                    free_func(memory);
                    return false;
                }
            }

            if (fi.CompressedHash != XXH64(memory, fi.UncompressedSize, 0)) {
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
                std::optional<std::lock_guard<std::recursive_mutex>> guard;
                if (optional_mutex) {
                    guard.emplace(*optional_mutex);
                }
                auto& file = archive.FileHandle;
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

            if (fi.CompressedHash != XXH64(compressedMemory.get(), fi.CompressedSize, 0)) {
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

            if ((p3aFlags & SenPatcher::P3AFlag_HasUncompressedHashes) != 0
                && XXH64(memory, fi.UncompressedSize, 0) != fi.UncompressedHash) {
                free_func(memory);
                return false;
            }

            out_memory = memory;
            out_filesize = fi.UncompressedSize;
            return true;
        }
        case SenPatcher::P3ACompressionType::ZSTD: {
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
                std::optional<std::lock_guard<std::recursive_mutex>> guard;
                if (optional_mutex) {
                    guard.emplace(*optional_mutex);
                }
                auto& file = archive.FileHandle;
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

            if (fi.CompressedHash != XXH64(compressedMemory.get(), fi.CompressedSize, 0)) {
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

            if ((p3aFlags & SenPatcher::P3AFlag_HasUncompressedHashes) != 0
                && XXH64(memory, fi.UncompressedSize, 0) != fi.UncompressedHash) {
                free_func(memory);
                return false;
            }

            out_memory = memory;
            out_filesize = fi.UncompressedSize;
            return true;
        }
        case SenPatcher::P3ACompressionType::ZSTD_DICT: {
            if (!archive.Dict) {
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
                std::optional<std::lock_guard<std::recursive_mutex>> guard;
                if (optional_mutex) {
                    guard.emplace(*optional_mutex);
                }
                auto& file = archive.FileHandle;
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

            if (fi.CompressedHash != XXH64(compressedMemory.get(), fi.CompressedSize, 0)) {
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
                                           archive.Dict)
                != fi.UncompressedSize) {
                dctx.reset();
                compressedMemory.reset();
                free_func(memory);
                return false;
            }
            dctx.reset();
            compressedMemory.reset();

            if ((p3aFlags & SenPatcher::P3AFlag_HasUncompressedHashes) != 0
                && XXH64(memory, fi.UncompressedSize, 0) != fi.UncompressedHash) {
                free_func(memory);
                return false;
            }

            out_memory = memory;
            out_filesize = fi.UncompressedSize;
            return true;
        }
        default: return false;
    }
}

void AppendLoadedModInfo(
    char*& string,
    const LoadedModsData& loadedModsData,
    const std::function<bool(const SenPatcher::P3AFileInfo& fi)>& shouldDisplayCallback,
    bool assetFixCreatingFailed) {
    size_t modfileCountExternal = 0;
    size_t modfileCountSenPatcher = 0;

    for (size_t i = 0; i < loadedModsData.LoadedP3As.CombinedFileInfoCount; ++i) {
        const auto& cfi = loadedModsData.LoadedP3As.CombinedFileInfos[i];
        if (!shouldDisplayCallback(*cfi.FileInfo)) {
            continue;
        }

        if (cfi.ArchiveData->Flags & P3AFlag_IsSenPatcherAssetMod) {
            ++modfileCountSenPatcher;
        } else {
            ++modfileCountExternal;
        }
    }

    char buffer[128];
    int len;
    if (modfileCountExternal > 0) {
        len = sprintf(buffer, "  %zu modded file", modfileCountExternal);
        strcpy(string, buffer);
        string += len;
        if (modfileCountExternal > 1) {
            *string++ = 's';
        }
    }
    if (modfileCountSenPatcher > 0) {
        len = sprintf(buffer, "  %zu asset fix", modfileCountSenPatcher);
        strcpy(string, buffer);
        string += len;
        if (modfileCountSenPatcher > 1) {
            *string++ = 'e';
            *string++ = 's';
        }
    }
    if (loadedModsData.CheckDevFolderForAssets) {
        constexpr char devFolderActive[] = "  'dev' folder active";
        memcpy(string, devFolderActive, sizeof(devFolderActive));
        string += (sizeof(devFolderActive) - 1);
    }
    if (assetFixCreatingFailed) {
        constexpr char failureNotice[] = "  FAILED creating asset fixes!";
        memcpy(string, failureNotice, sizeof(failureNotice));
        string += (sizeof(failureNotice) - 1);
    }
}
} // namespace SenLib::ModLoad
