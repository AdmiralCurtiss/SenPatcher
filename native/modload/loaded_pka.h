#pragma once

#include <array>
#include <cstdint>

#include "sen/pka.h"

namespace SenLib::ModLoad {
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
                          uint32_t* out_filesize);
} // namespace SenLib::ModLoad
