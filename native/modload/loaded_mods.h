#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>

#include "logger.h"
#include "p3a/p3a.h"

namespace SenLib::ModLoad {
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

struct LoadedModsData {
    std::unique_ptr<P3AData[]> P3As{};
    size_t CombinedFileInfoCount = 0;
    std::unique_ptr<P3AFileRef[]> CombinedFileInfos{};
    bool CheckDevFolderForAssets = false;
};

void LoadModP3As(SenPatcher::Logger& logger,
                 LoadedModsData& loadedModsData,
                 const std::filesystem::path& baseDir);
const P3AFileRef* FindP3AFileRef(const LoadedModsData& loadedModsData,
                                 const std::array<char8_t, 0x100>& filteredPath);
bool ExtractP3AFileToMemory(const P3AFileRef& ref,
                            uint64_t filesizeLimit,
                            void*& out_memory,
                            uint64_t& out_filesize,
                            PMalloc malloc_func,
                            PFree free_func);

// so we have a consistent representation: lowercase, singular forward slash as separator
void FilterP3APath(char8_t* path, size_t length);

// same as above, but slightly different because instead of writing in-place we write to a separate
// array, and also the input is only bounded by nulltermination and not length.
// returns true if the entire input string fit into out_path, false if not
bool FilterGamePath(char8_t* out_path, const char* in_path, size_t length);
} // namespace SenLib::ModLoad
