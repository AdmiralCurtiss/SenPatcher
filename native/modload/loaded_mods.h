#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <span>
#include <string_view>

#include "p3a/p3a.h"
#include "util/logger.h"

namespace HyoutaUtils::Ini {
struct IniFile;
}

namespace SenLib::ModLoad {
using PMalloc = void* (*)(size_t size);
using PFree = void (*)(void* memory);

// use an unused bit in the p3a flags to store this
static constexpr uint32_t P3AFlag_IsSenPatcherAssetMod = 2;
static_assert((SenPatcher::P3AFlagMask & P3AFlag_IsSenPatcherAssetMod) == 0);

struct P3AData {
    SenPatcher::P3A Archive;
    uint32_t Flags = 0;
    std::recursive_mutex Mutex;

    P3AData() = default;
    P3AData(const P3AData& other) = delete;
    P3AData(P3AData&& other) = delete;
    P3AData& operator=(const P3AData& other) = delete;
    P3AData& operator=(P3AData&& other) = delete;
    ~P3AData() = default;
};

struct P3AFileRef {
    P3AData* ArchiveData;
    SenPatcher::P3AFileInfo* FileInfo;

    P3AFileRef() = default;
    P3AFileRef(const P3AFileRef& other) = default;
    P3AFileRef(P3AFileRef&& other) = default;
    P3AFileRef& operator=(const P3AFileRef& other) = default;
    P3AFileRef& operator=(P3AFileRef&& other) = default;
    ~P3AFileRef() = default;
};

struct LoadedP3AData {
    std::unique_ptr<P3AData[]> P3As{};
    size_t CombinedFileInfoCount = 0;
    std::unique_ptr<P3AFileRef[]> CombinedFileInfos{};

    LoadedP3AData() = default;
    LoadedP3AData(const LoadedP3AData& other) = delete;
    LoadedP3AData(LoadedP3AData&& other) = delete;
    LoadedP3AData& operator=(const LoadedP3AData& other) = delete;
    LoadedP3AData& operator=(LoadedP3AData&& other) = delete;
    ~LoadedP3AData() = default;
};

struct LoadedModsData {
    LoadedP3AData LoadedP3As;
    bool CheckDevFolderForAssets = false;

    LoadedModsData() = default;
    LoadedModsData(const LoadedModsData& other) = delete;
    LoadedModsData(LoadedModsData&& other) = delete;
    LoadedModsData& operator=(const LoadedModsData& other) = delete;
    LoadedModsData& operator=(LoadedModsData&& other) = delete;
    ~LoadedModsData() = default;
};

void CreateModDirectory(std::string_view baseDir);
void LoadP3As(HyoutaUtils::Logger& logger,
              LoadedP3AData& loadedP3AData,
              std::string_view baseDir,
              std::span<const std::string_view> files);
void LoadModP3As(HyoutaUtils::Logger& logger,
                 LoadedModsData& loadedModsData,
                 std::string_view baseDir,
                 bool shouldLoadAssetFixes,
                 std::string_view iniCategory,
                 bool isRunningInJapaneseLanguage,
                 const std::function<void(const HyoutaUtils::Ini::IniFile& ini)>& iniHandler);

// Searches a file with name filteredPath (assumed to be pre-filtered) in
// [CombinedFileInfos, CombinedFileInfos + CombinedFileInfoCount)
// which is assumed to be sorted and without duplicates.
const P3AFileRef* FindP3AFileRef(const LoadedP3AData& loadedModsData,
                                 const std::array<char, 0x100>& filteredPath);

bool ExtractP3AFileToMemory(const P3AFileRef& ref,
                            uint32_t p3aFlags,
                            uint64_t filesizeLimit,
                            void*& out_memory,
                            uint64_t& out_filesize,
                            PMalloc malloc_func,
                            PFree free_func);

bool ExtractP3AFileToMemory(const SenPatcher::P3AFileInfo& fi,
                            SenPatcher::P3A& archive,
                            uint32_t p3aFlags,
                            std::recursive_mutex* optional_mutex,
                            uint64_t filesizeLimit,
                            void*& out_memory,
                            uint64_t& out_filesize,
                            PMalloc malloc_func,
                            PFree free_func);

// so we have a consistent representation: lowercase, singular forward slash as separator
void FilterP3APath(char* path, size_t length);

// same as above, but slightly different because instead of writing in-place we write to a separate
// array, and also the input is only bounded by nulltermination and not length.
// returns true if the entire input string fit into out_path, false if not
bool FilterGamePath(char* out_path, const char* in_path, size_t length);

void AppendLoadedModInfo(
    char*& string,
    const LoadedModsData& loadedModsData,
    const std::function<bool(const SenPatcher::P3AFileInfo& fi)>& shouldDisplayCallback,
    bool assetFixCreatingFailed);
} // namespace SenLib::ModLoad
