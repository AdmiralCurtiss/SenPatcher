#pragma once

#include <cstddef>
#include <vector>

namespace SenLib::ModLoad {
struct LoadedPkaData;
}

namespace SenLib::Sen3 {
std::vector<char> PatchSingleFileInPkg(const char* file,
                                       size_t fileLength,
                                       const char* patch,
                                       size_t patchLength,
                                       size_t expectedFileCount,
                                       size_t fileToPatch,
                                       SenLib::ModLoad::LoadedPkaData* vanillaPKAs = nullptr);
}
