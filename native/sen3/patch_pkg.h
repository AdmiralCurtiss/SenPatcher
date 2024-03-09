#pragma once

#include <vector>

namespace SenLib::Sen3 {
std::vector<char> PatchSingleTexturePkg(const char* file,
                                        size_t fileLength,
                                        const char* patch,
                                        size_t patchLength);
}
