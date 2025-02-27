#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <string_view>

#include "sen/pka.h"
#include "util/file.h"
#include "util/logger.h"

namespace SenLib::ModLoad {
struct LoadedPkaSingleData {
    HyoutaUtils::IO::File Handle;
    std::recursive_mutex Mutex;
};
struct LoadedPkaGroupData {
    std::unique_ptr<SenLib::PkaPkgToHashData[]> Pkgs;
    size_t PkgCount = 0;
    std::unique_ptr<SenLib::PkaFileHashData[]> PkgFiles;
    size_t PkgFileCount = 0;
};
struct LoadedPkaAllData {
    std::unique_ptr<SenLib::PkaHashToFileData[]> Files;
    size_t FilesCount = 0;
};
struct LoadedPkaData {
    std::unique_ptr<LoadedPkaGroupData[]> Groups;
    std::unique_ptr<LoadedPkaSingleData[]> Handles;
    size_t HandleCount = 0;
    LoadedPkaAllData Hashes;
};

// Loads all PKA files in baseDir/prefixes[i]/names[i][n].pka into loadedPkaData.
// ignorePkgsOfPrefix0File0 = true should be set for the games that have native .pka support,
// so we let the game handle the pkgs of that specific file on its own while still having it
// accessible for the hashes/files in it
void LoadPkas(HyoutaUtils::Logger& logger,
              LoadedPkaData& loadedPkaData,
              std::string_view baseDir,
              std::span<const std::string_view> prefixes,
              std::span<const std::string_view> names,
              bool ignorePkgsOfPrefix0File0);

// Build a 'fake' PKG where every file is just a reference with hash to the PKA.
// This is a nonstandard extension that the vanilla game does not understand!
// Preconditions:
// - memory be at least (8 + (pkaPkg->FileCount * (0x50 + 0x20))) bytes long
// - the array pointed at by pkgFiles must contain data for the range
//   [pkaPkg->FileOffset, pkaPkg->FileOffset + pkaPkg->FileCount)
bool BuildFakePkaPkg(char* memory,
                     const SenLib::PkaPkgToHashData* pkaPkg,
                     SenLib::PkaFileHashData* pkgFiles,
                     SenLib::PkaHashToFileData* files,
                     size_t filesCount);

// Checks if a given pkg file exists (on disk or in the PKA) and optionally returns its filesize.
// Returns: (compatible with the injected function return value)
// 0 if we should report that the file does not exist
// 1 if we should report that the file does exist (in only this case *out_filesize will be set)
// -1 if the given path is not a PKG that could be in the given PKA
int32_t GetPkaPkgFilesize(SenLib::PkaPkgToHashData* pkgs,
                          size_t pkgCount,
                          const char* pkgPrefix,
                          size_t pkgPrefixLength,
                          const std::array<char, 0x100>& filteredPath,
                          const char* path,
                          uint32_t* out_filesize,
                          bool checkFilesystemFirst);
} // namespace SenLib::ModLoad
