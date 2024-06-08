#pragma once

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
} // namespace SenLib::ModLoad
