#include "loaded_pka.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string_view>

#include "loaded_mods.h"

#include "sen/pka.h"
#include "util/file.h"
#include "util/logger.h"

namespace {
struct PkaHeaderWithHandle {
    SenLib::PkaHeader Header;
    HyoutaUtils::IO::File FileHandle;
};
} // namespace

namespace SenLib::ModLoad {
static std::optional<HyoutaUtils::IO::File>
    LoadPka(HyoutaUtils::Logger& logger, SenLib::PkaHeader& pka, std::string_view path) {
    logger.Log("Trying to open PKA at ").Log(path).Log("\n");
    HyoutaUtils::IO::File f(path, HyoutaUtils::IO::OpenMode::Read);
    bool success = f.IsOpen() && SenLib::ReadPkaFromFile(pka, f);
    if (success) {
        // make pkg names consistent
        for (size_t i = 0; i < pka.PkgCount; ++i) {
            auto& pkgName = pka.Pkgs[i].PkgName;
            SenLib::ModLoad::FilterP3APath(pkgName.data(), pkgName.size());
        }

        // sort the pkgs for binary lookup
        std::stable_sort(
            pka.Pkgs.get(),
            pka.Pkgs.get() + pka.PkgCount,
            [](const SenLib::PkaPkgToHashData& lhs, const SenLib::PkaPkgToHashData& rhs) {
                return strncmp(lhs.PkgName.data(), rhs.PkgName.data(), lhs.PkgName.size()) < 0;
            });

        // also need to sort the files but we do that later...
        logger.Log("Opened PKA.\n");
        return f;
    }
    return std::nullopt;
}

static void CombinePkaGroup(LoadedPkaGroupData& group,
                            std::vector<PkaHeaderWithHandle>& pkaHeaders,
                            size_t prefixGroupStart,
                            size_t prefixGroupEnd) {
    // count the number of PKGs we have
    size_t numberOfPkgs = 0;
    for (size_t j = prefixGroupStart; j < prefixGroupEnd; ++j) {
        numberOfPkgs += pkaHeaders[j].Header.PkgCount;
    }
    if (numberOfPkgs == 0) {
        group.Pkgs = nullptr;
        group.PkgCount = 0;
        group.PkgFiles = nullptr;
        group.PkgFileCount = 0;
        return;
    }

    // sort these via a proxy
    struct PkgIndexProxy {
        size_t PkgIndex;
        size_t PkaIndex;
    };
    auto pkgArray = std::make_unique<PkgIndexProxy[]>(numberOfPkgs);
    {
        size_t idx = 0;
        for (size_t j = prefixGroupStart; j < prefixGroupEnd; ++j) {
            auto& hdr = pkaHeaders[j].Header;
            for (size_t k = 0; k < hdr.PkgCount; ++k) {
                pkgArray[idx].PkgIndex = k;
                pkgArray[idx].PkaIndex = j;
                ++idx;
            }
        }
    }
    std::stable_sort(pkgArray.get(),
                     pkgArray.get() + numberOfPkgs,
                     [&](const PkgIndexProxy& lhs, const PkgIndexProxy& rhs) {
                         // we sort first by pkg name, second by pka index (biggest first)
                         auto& lhsName = pkaHeaders[lhs.PkaIndex].Header.Pkgs[lhs.PkgIndex].PkgName;
                         auto& rhsName = pkaHeaders[rhs.PkaIndex].Header.Pkgs[rhs.PkgIndex].PkgName;
                         int nameResult = strncmp(lhsName.data(), rhsName.data(), lhsName.size());
                         if (nameResult != 0) {
                             return nameResult < 0;
                         }
                         return lhs.PkaIndex > rhs.PkaIndex;
                     });

    // now drop all duplicate pkg names
    size_t remainingPkgCount = numberOfPkgs;
    {
        size_t in = 1;
        size_t out = 0;
        while (in < numberOfPkgs) {
            const PkgIndexProxy& last = pkgArray[out];
            const PkgIndexProxy& next = pkgArray[in];
            auto& lastName = pkaHeaders[last.PkaIndex].Header.Pkgs[last.PkgIndex].PkgName;
            auto& nextName = pkaHeaders[next.PkaIndex].Header.Pkgs[next.PkgIndex].PkgName;
            if (strncmp(lastName.data(), nextName.data(), lastName.size()) == 0) {
                --remainingPkgCount;
            } else {
                const PkgIndexProxy tmp = next;
                ++out;
                pkgArray[out] = tmp;
            }
            ++in;
        }
    }

    // combine the remaining pkgs into one large array
    size_t remainingFileCount = 0;
    for (size_t j = 0; j < remainingPkgCount; ++j) {
        const PkgIndexProxy& proxy = pkgArray[j];
        remainingFileCount += pkaHeaders[proxy.PkaIndex].Header.Pkgs[proxy.PkgIndex].FileCount;
    }
    auto restPkgs = remainingPkgCount > 0
                        ? std::make_unique<SenLib::PkaPkgToHashData[]>(remainingPkgCount)
                        : nullptr;
    auto restPkgFiles = remainingFileCount > 0
                            ? std::make_unique<SenLib::PkaFileHashData[]>(remainingFileCount)
                            : nullptr;
    uint32_t newOffset = 0;
    for (size_t j = 0; j < remainingPkgCount; ++j) {
        const PkgIndexProxy& proxy = pkgArray[j];
        const auto& oldHdr = pkaHeaders[proxy.PkaIndex].Header;
        const auto& oldPkg = oldHdr.Pkgs[proxy.PkgIndex];
        auto& newPkg = restPkgs[j];
        newPkg.PkgName = oldPkg.PkgName;
        newPkg.FileOffset = newOffset;
        newPkg.FileCount = oldPkg.FileCount;
        for (size_t k = 0; k < oldPkg.FileCount; ++k) {
            restPkgFiles[newOffset] = oldHdr.PkgFiles[oldPkg.FileOffset + k];
            ++newOffset;
        }
    }

    group.Pkgs = std::move(restPkgs);
    group.PkgCount = remainingPkgCount;
    group.PkgFiles = std::move(restPkgFiles);
    group.PkgFileCount = remainingFileCount;
}

void LoadPkas(HyoutaUtils::Logger& logger,
              LoadedPkaData& loadedPkaData,
              std::string_view baseDir,
              std::span<const std::string_view> prefixes,
              std::span<const std::string_view> names,
              LoadedPkaGroupData* pkgsOfPrefix0File0) {
    // prefixes contains a list of directories that may contain .pka files
    // names contains a list of names for the pka file
    // eg. if prefix is "data/asset/d3d11/" and name is "assets", then
    // the .pka files will be at
    // "data/asset/d3d11/assets.pka"
    // "data/asset/d3d11/assets1.pka"
    // "data/asset/d3d11/assets2.pka"
    // "data/asset/d3d11/assets3.pka"
    // "data/asset/d3d11/assets4.pka"
    // in sequential order until one file no longer exists
    // we bound this at 2 digits, ie assets99.pka
    // if a .pkg name exists in multiple .pkas at the same prefix, the last entry wins
    // this way we can just add a new .pka without modifying the old ones to patch files

    size_t longestPrefix = 0;
    size_t longestName = 0;
    for (const std::string_view& prefix : prefixes) {
        longestPrefix = std::max(longestPrefix, prefix.size());
    }
    if (names.empty()) {
        longestName = 6;
    } else {
        for (const std::string_view& name : names) {
            longestName = std::max(longestName, name.size());
        }
    }
    std::string pkaFilePath;
    pkaFilePath.reserve(baseDir.size() + longestPrefix + longestName + 8);

    auto groups = std::make_unique<LoadedPkaGroupData[]>(prefixes.size());

    std::vector<PkaHeaderWithHandle> pkaHeaders;
    // likely way too much, but we want to avoid a reallocation,
    // this will be free'd at the end of the function anyway
    pkaHeaders.reserve(prefixes.size() * 100);
    size_t prefixGroupStart = 0;
    for (size_t i = 0; i < prefixes.size(); ++i) {
        std::string_view prefix = prefixes[i];
        std::string_view name;
        if (i < names.size()) {
            name = names[i];
        } else if (!names.empty()) {
            name = names.back();
        } else {
            name = "assets";
        }

        // find all the .pka files in this prefix
        for (size_t j = 0; j < 100; ++j) {
            pkaFilePath.clear();
            pkaFilePath.append(baseDir);
            pkaFilePath.push_back('/');
            pkaFilePath.append(prefix);
            pkaFilePath.append(name);
            if (j != 0) {
                pkaFilePath.append(std::to_string(j));
            }
            pkaFilePath.append(".pka");

            auto& h = pkaHeaders.emplace_back();
            auto file = LoadPka(logger, h.Header, pkaFilePath);
            if (!file) {
                pkaHeaders.pop_back();
                break;
            }
            h.FileHandle = std::move(*file);
        }

        // combine the group into one set of .pkg files
        if (pkgsOfPrefix0File0 != nullptr && i == 0) {
            // the caller requested the first file here to be returned separately, so do that
            LoadedPkaGroupData& file0Group = *pkgsOfPrefix0File0;
            if (prefixGroupStart != pkaHeaders.size()) {
                CombinePkaGroup(file0Group, pkaHeaders, prefixGroupStart, prefixGroupStart + 1);
                CombinePkaGroup(groups[i], pkaHeaders, prefixGroupStart + 1, pkaHeaders.size());
            } else {
                // this is the '0 pkas found' case, this will clear out the result just in case
                CombinePkaGroup(file0Group, pkaHeaders, prefixGroupStart, pkaHeaders.size());
                CombinePkaGroup(groups[i], pkaHeaders, prefixGroupStart, pkaHeaders.size());
            }
        } else {
            CombinePkaGroup(groups[i], pkaHeaders, prefixGroupStart, pkaHeaders.size());
        }

        // free the data for this group (except the actual file references, we need that later)
        for (size_t j = prefixGroupStart; j < pkaHeaders.size(); ++j) {
            auto& hdr = pkaHeaders[j].Header;
            hdr.Pkgs.reset();
            hdr.PkgFiles.reset();
        }

        prefixGroupStart = pkaHeaders.size();
    }

    // now all files are known, combine them into one large file array

    // use the high 16 bits of the offset to store the pka index. this is practically safe, you'd
    // need a pka of over 256 TiB for there to be meaningful data in those high bits
    for (size_t i = 0; i < pkaHeaders.size(); ++i) {
        auto& h = pkaHeaders[i].Header;
        for (size_t j = 0; j < h.FilesCount; ++j) {
            h.Files[j].Offset = ((static_cast<uint64_t>(i) << 48)
                                 | (h.Files[j].Offset & static_cast<uint64_t>(0xffff'ffff'ffff)));
        }
    }

    // build a combined files list
    size_t filesCount = 0;
    for (size_t i = 0; i < pkaHeaders.size(); ++i) {
        filesCount += pkaHeaders[i].Header.FilesCount;
    }
    auto files = std::make_unique<SenLib::PkaHashToFileData[]>(filesCount);
    size_t fileIndex = 0;
    for (size_t i = 0; i < pkaHeaders.size(); ++i) {
        auto& h = pkaHeaders[i].Header;
        for (size_t j = 0; j < h.FilesCount; ++j) {
            files[fileIndex] = h.Files[j];
            ++fileIndex;
        }
    }

    // sort the file hashes for binary lookup
    std::stable_sort(
        files.get(),
        files.get() + filesCount,
        [](const SenLib::PkaHashToFileData& lhs, const SenLib::PkaHashToFileData& rhs) {
            return memcmp(lhs.Hash.data(), rhs.Hash.data(), lhs.Hash.size()) < 0;
        });

    // keep the file handles
    auto pkaSingles = std::make_unique<LoadedPkaSingleData[]>(pkaHeaders.size());
    size_t pkaSingleCount = pkaHeaders.size();
    for (size_t i = 0; i < pkaSingleCount; ++i) {
        pkaSingles[i].Handle = std::move(pkaHeaders[i].FileHandle);
    }

    // store the collected data
    loadedPkaData.Handles = std::move(pkaSingles);
    loadedPkaData.HandleCount = pkaSingleCount;
    loadedPkaData.Groups = std::move(groups);
    loadedPkaData.Hashes.Files = std::move(files);
    loadedPkaData.Hashes.FilesCount = filesCount;
}

bool BuildFakePkaPkg(char* memory,
                     const SenLib::PkaPkgToHashData* pkaPkg,
                     SenLib::PkaFileHashData* pkgFiles,
                     SenLib::PkaHashToFileData* files,
                     size_t filesCount) {
    char* header = memory;
    std::memcpy(header, &pkaPkg->PkgName[pkaPkg->PkgName.size() - 4], 4);
    std::memcpy(header + 4, &pkaPkg->FileCount, 4);
    header += 8;
    char* data = header + (pkaPkg->FileCount * 0x50);
    uint32_t dataPosition = 8 + (pkaPkg->FileCount * 0x50);
    for (size_t i = 0; i < pkaPkg->FileCount; ++i) {
        const SenLib::PkaFileHashData& fileHashData = pkgFiles[pkaPkg->FileOffset + i];
        const SenLib::PkaHashToFileData* fileData =
            SenLib::FindFileInPkaByHash(files, filesCount, fileHashData.Hash);
        if (!fileData) {
            return false;
        }
        std::array<uint32_t, 4> headerData;
        headerData[0] = fileData->UncompressedSize;
        headerData[1] = 0x20; // compressed size, always the SHA256 hash
        headerData[2] = dataPosition;
        headerData[3] = 0x80; // fake flags to indicate that it should look in the PKA

        assert(fileHashData.Filename.size() == 0x40);
        assert(fileHashData.Hash.size() == 0x20);
        std::memcpy(header, fileHashData.Filename.data(), fileHashData.Filename.size());
        std::memcpy(header + 0x40, headerData.data(), 0x10);
        std::memcpy(data, fileHashData.Hash.data(), fileHashData.Hash.size());

        header += 0x50;
        data += 0x20;
        dataPosition += 0x20;
    }
    return true;
}

int32_t GetPkaPkgFilesize(SenLib::PkaPkgToHashData* pkgs,
                          size_t pkgCount,
                          const char* pkgPrefix,
                          size_t pkgPrefixLength,
                          const std::array<char, 0x100>& filteredPath,
                          const char* path,
                          uint32_t* out_filesize,
                          bool checkFilesystemFirst) {
    if (pkgCount > 0 && memcmp(pkgPrefix, filteredPath.data(), pkgPrefixLength) == 0) {
        if (checkFilesystemFirst) {
            // first check for the real PKG
            HyoutaUtils::IO::File file(std::string_view(path), HyoutaUtils::IO::OpenMode::Read);
            if (file.IsOpen()) {
                auto length = file.GetLength();
                if (!length) {
                    return 0;
                }
                if (out_filesize) {
                    *out_filesize = static_cast<uint32_t>(*length);
                }
                return 1;
            }
        }

        // then check for data in the PKA
        const size_t start = pkgPrefixLength;
        assert(filteredPath.size() - start >= pkgs[0].PkgName.size());
        const SenLib::PkaPkgToHashData* pkaPkg =
            SenLib::FindPkgInPkaByName(pkgs, pkgCount, &filteredPath[start]);
        if (pkaPkg) {
            if (out_filesize) {
                // this pkg isn't actually real, but its size when crafted is going to be:
                // 8 bytes fixed header
                // 0x50 bytes header per file
                // 0x20 bytes data per file (the SHA256 hash)
                *out_filesize = 8 + (pkaPkg->FileCount * (0x50 + 0x20));
            }
            return 1;
        }
    }
    return -1;
}
} // namespace SenLib::ModLoad
