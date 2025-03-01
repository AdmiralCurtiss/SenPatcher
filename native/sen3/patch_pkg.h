#pragma once

#include <vector>

namespace SenLib::Sen3 {
std::vector<char> PatchSingleFileInPkg(const char* file,
                                       size_t fileLength,
                                       const char* patch,
                                       size_t patchLength,
                                       size_t expectedFileCount,
                                       size_t fileToPatch);
}
