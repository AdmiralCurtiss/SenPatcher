#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

namespace SenLib::ModLoad {
struct LoadedPkaData;
}

namespace SenLib::Sen3 {
struct SingleFilePatchInfo {
    const char* PatchData;
    size_t PatchLength;
    size_t FileIndexToPatch;
};

std::vector<char> PatchSingleFileInPkg(const char* file,
                                       size_t fileLength,
                                       const char* patch,
                                       size_t patchLength,
                                       size_t expectedFileCount,
                                       size_t fileToPatch,
                                       SenLib::ModLoad::LoadedPkaData* vanillaPKAs = nullptr);
bool TryApplyPkgPatches(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
                        std::vector<SenPatcher::P3APackFile>& result,
                        SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                        const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub,
                        std::string_view inpath,
                        std::string_view outpath,
                        size_t stubsize,
                        const HyoutaUtils::Hash::SHA1& stubhash,
                        size_t realsize,
                        const HyoutaUtils::Hash::SHA1& realhash,
                        size_t expectedFileCount,
                        std::span<const SingleFilePatchInfo> patches);
} // namespace SenLib::Sen3
