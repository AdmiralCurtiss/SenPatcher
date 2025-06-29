#include "pka_pack.h"
#include "pka_pack_main.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <filesystem>
#include <format>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "sen/pka.h"
#include "sen/pka_to_pkg.h"
#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "sen/pkg_extract.h"
#include "util/args.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/hash/sha256.h"
#include "util/memread.h"
#include "util/memwrite.h"
#include "util/scope.h"
#include "util/text.h"

namespace SenTools {
// in the vanilla games the pkgs are always uppercase except for the extension which is
// lowercase, so make sure we pack like that too...
static void NormalizePkgName(std::array<char, 0x20>& pkgName) {
    const size_t length = [&]() {
        for (size_t i = 0; i < pkgName.size(); ++i) {
            if (pkgName[i] == '\0') {
                return i;
            }
        }
        return pkgName.size();
    }();
    const size_t extensionSeparator = [&]() {
        for (size_t i = 0; i < length; ++i) {
            if (pkgName[i] == '.') {
                return i;
            }
        }
        return length;
    }();
    for (size_t i = 0; i < extensionSeparator; ++i) {
        char& c = pkgName[i];
        if (c >= 'a' && c <= 'z') {
            c = c + ('A' - 'a');
        }
    }
    for (size_t i = extensionSeparator; i < length; ++i) {
        char& c = pkgName[i];
        if (c >= 'A' && c <= 'Z') {
            c = c + ('a' - 'A');
        }
    }
}

namespace {
struct PkgPackFile {
    std::array<char, 0x40> Filename;
    HyoutaUtils::Hash::SHA256 Hash;
    uint32_t OffsetInPkg;
    uint32_t CompressedSize;
    uint32_t UncompressedSize;
    uint32_t Flags;
    uint32_t Index;
    bool IsCorruptedFile = false;
};
struct PkgPackArchive {
    HyoutaUtils::IO::File FileHandle;
    std::array<char, 0x20> PkgName;
    size_t PkgNameLength;
    std::vector<PkgPackFile> Files;
    bool IncludeInPka = true;
};
struct FileReference {
    size_t ArchiveIndex;
    size_t FileIndex;
    char* PkaHeaderPtr = nullptr;
    bool AlreadyWritten = false;
    bool ShouldBeWritten = true;
};
struct SHA256Sorter {
    bool operator()(const HyoutaUtils::Hash::SHA256& lhs,
                    const HyoutaUtils::Hash::SHA256& rhs) const {
        const auto& l = lhs.Hash;
        const auto& r = rhs.Hash;
        return std::memcmp(l.data(), r.data(), l.size()) < 0;
    }
};
struct FileToWriteInfo {
    size_t ArchiveIndex;
    size_t FileIndex;

    // filled while writing/recompressing
    uint64_t FileOffset;
    uint32_t CompressedSize;
    uint32_t UncompressedSize;
    uint32_t PkgFlags;

    char* PkaHeaderPtr;
};
struct PkaConstructionData {
    std::unique_ptr<char[]> PkaHeader;
    size_t PkaHeaderLength;
    std::vector<FileToWriteInfo> FilesToWrite;
};
} // namespace

static PkgPackArchive InitPkgPackArchive(const std::vector<PkgPackArchive>& pkgPackFiles,
                                         const PackPkaPkgInfo& sourcePkg) {
    const char* filenameC = sourcePkg.NameInPka.c_str();

    std::array<char, 0x20> fn{};
    const size_t filenameLength = [&]() -> size_t {
        for (size_t i = 0; i < fn.size() - 1; ++i) {
            const char c = filenameC[i];
            if (c == '\0') {
                return i;
            }
            fn[i] = c;
        }
        return fn.size();
    }();
    NormalizePkgName(fn);

    auto existingArchive =
        std::find_if(pkgPackFiles.begin(), pkgPackFiles.end(), [&](const PkgPackArchive& a) {
            return strncmp(a.PkgName.data(), fn.data(), fn.size()) == 0;
        });
    const bool archiveExistsAlready = (existingArchive != pkgPackFiles.end());
    if (archiveExistsAlready) {
        printf(
            "WARNING: %s exists multiple times, only the first parsed archive will be "
            "available in the PKA (but the contents of all instances will be packed).\n",
            filenameC);
    }

    return PkgPackArchive{.FileHandle = HyoutaUtils::IO::File(std::string_view(sourcePkg.Path),
                                                              HyoutaUtils::IO::OpenMode::Read),
                          .PkgName = fn,
                          .PkgNameLength = filenameLength,
                          .IncludeInPka = !archiveExistsAlready};
}

static bool LoadPkg(PkgPackArchive& fi) {
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    HyoutaUtils::IO::File& infile = fi.FileHandle;
    if (!infile.IsOpen()) {
        printf("Failed opening pkg.\n");
        return false;
    }
    std::array<char, 8> pkgHeaderInitialBytes;
    if (infile.Read(pkgHeaderInitialBytes.data(), pkgHeaderInitialBytes.size())
        != pkgHeaderInitialBytes.size()) {
        printf("Failed to read pkg.\n");
        return false;
    }

    // PKGs start with an unknown 32-bit integer, some kind of ID or maybe timestamp.
    // As far as I can tell, this is not used by anything and can be safely discarded, so
    // the PKA format does so. However, that technically makes packing a PKA lossy.
    // If the length of the filename permits, however, we can use the end of the fixed-size
    // filename field to store this data, so we can restore it when 'extracting' the PKA.
    // This is a somewhat creative interpretation of the PKA format, but it shouldn't cause
    // any issues in practice.
    if (fi.PkgNameLength < 28) {
        const uint32_t unknownId = HyoutaUtils::EndianUtils::FromEndian(
            HyoutaUtils::MemRead::ReadUInt32(pkgHeaderInitialBytes.data()), LittleEndian);
        HyoutaUtils::MemWrite::WriteUInt32(
            fi.PkgName.data() + 28, HyoutaUtils::EndianUtils::ToEndian(unknownId, LittleEndian));
    }

    const uint32_t fileCountInPkg = HyoutaUtils::EndianUtils::FromEndian(
        HyoutaUtils::MemRead::ReadUInt32(&pkgHeaderInitialBytes[4]), LittleEndian);
    const size_t pkgHeaderLength = 8u + static_cast<size_t>(fileCountInPkg) * 0x50u;
    auto pkgHeaderData = std::make_unique_for_overwrite<char[]>(pkgHeaderLength);
    if (!pkgHeaderData) {
        printf("Failed to allocate memory.\n");
        return false;
    }
    std::memcpy(pkgHeaderData.get(), pkgHeaderInitialBytes.data(), pkgHeaderInitialBytes.size());
    if (infile.Read(pkgHeaderData.get() + pkgHeaderInitialBytes.size(),
                    pkgHeaderLength - pkgHeaderInitialBytes.size())
        != (pkgHeaderLength - pkgHeaderInitialBytes.size())) {
        printf("Failed to read pkg.\n");
        return false;
    }

    SenLib::PkgHeader pkgHeader;
    if (!SenLib::ReadPkgFromMemory(pkgHeader, pkgHeaderData.get(), pkgHeaderLength, LittleEndian)) {
        printf("Failed to read pkg header.\n");
        return false;
    }

    fi.Files.reserve(pkgHeader.FileCount);
    for (uint32_t i = 0; i < pkgHeader.FileCount; ++i) {
        const auto& f = pkgHeader.Files[i];
        fi.Files.emplace_back(PkgPackFile{
            .Filename = f.Filename,
            .OffsetInPkg = f.DataPosition,
            .CompressedSize = f.CompressedSize,
            .UncompressedSize = f.UncompressedSize,
            .Flags = f.Flags,
            .Index = i,
        });
    }

    return true;
}

static std::unique_ptr<char[]> ReadPkgFile(PkgPackArchive& fi, PkgPackFile& f) {
    HyoutaUtils::IO::File& infile = fi.FileHandle;
    auto compressedData = std::make_unique_for_overwrite<char[]>(f.CompressedSize);
    if (!compressedData) {
        printf("Failed to allocate memory.\n");
        return nullptr;
    }
    if (!infile.SetPosition(f.OffsetInPkg)) {
        printf("Failed to seek in pkg.\n");
        return nullptr;
    }
    if (infile.Read(compressedData.get(), f.CompressedSize) != f.CompressedSize) {
        printf("Failed to read pkg.\n");
        return nullptr;
    }
    return compressedData;
}

static bool
    CalculateHashForPkgFile(PkgPackArchive& fi, PkgPackFile& f, const char* compressedData) {
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;
    auto dataBuffer = std::make_unique<char[]>(f.UncompressedSize);
    if (!dataBuffer) {
        printf("Failed to allocate memory.\n");
        return false;
    }
    if (SenLib::ExtractAndDecompressPkgFile(dataBuffer.get(),
                                            f.UncompressedSize,
                                            compressedData,
                                            f.CompressedSize,
                                            f.Flags,
                                            LittleEndian)) {
        f.Hash = HyoutaUtils::Hash::CalculateSHA256(dataBuffer.get(), f.UncompressedSize);
        f.IsCorruptedFile = false;
    } else {
        // CS2 contains two corrupted files in its PKGs. Thankfully, both of the files
        // have uncorrupted copies in other PKGs, so this is recoverable. Check if we're
        // processing one of those files and, if yes, try to recover the file.
        const std::string_view pkgName(fi.PkgName.data(), fi.PkgNameLength);
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(pkgName, "C_NPC290_C00.pkg")
            && f.Index == 10) {
            printf(
                "Known corrupted file found in %s. Replacing with known good hash and "
                "assuming we have a clean copy in another pkg.\n",
                pkgName.data());
            f.Hash = HyoutaUtils::Hash::SHA256FromHexString(
                "ff37dc7dbde66c3d79da984e57864be0ebbd03ba9c8d1a9b5b85c295076dfe85");
            f.OffsetInPkg = 0xffff'ffffu;
            f.CompressedSize = 0xffff'ffffu;
            f.UncompressedSize = 0xffff'ffffu;
            f.Flags = 0xffff'ffffu;
            f.IsCorruptedFile = true;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(pkgName, "O_E6303.pkg")
                   && f.Index == 5) {
            printf(
                "Known corrupted file found in %s. Replacing with known good hash and "
                "assuming we have a clean copy in another pkg.\n",
                pkgName.data());
            f.Hash = HyoutaUtils::Hash::SHA256FromHexString(
                "27aef8d457b29e50cbbd37bf24ca1bca2c5174e23f46fed57afc43a2bacf7b7d");
            f.OffsetInPkg = 0xffff'ffffu;
            f.CompressedSize = 0xffff'ffffu;
            f.UncompressedSize = 0xffff'ffffu;
            f.Flags = 0xffff'ffffu;
            f.IsCorruptedFile = true;
        } else {
            printf("Failed to extract file %u from pkg %s.\n", f.Index, pkgName.data());
            return false;
        }
    }

    return true;
}

static bool CalculateHashForPkgFile(PkgPackArchive& fi, PkgPackFile& f) {
    auto compressedData = ReadPkgFile(fi, f);
    if (!compressedData) {
        return false;
    }
    return CalculateHashForPkgFile(fi, f, compressedData.get());
}

static bool CalculateHashForPkgFilesMultithreaded(std::vector<PkgPackArchive>& pkgPackFiles,
                                                  size_t totalNumberOfFiles,
                                                  size_t threadCount) {
    // This is designed like this:
    // - threadCount worker threads, which do the actual decompression + hashing of the files
    // - 1 file reader thread, which reads the input files and hands them over to a worker thread

    std::atomic<bool> encounteredError = false;

    struct alignas(64) WorkPacket {
        const char* Data;
        std::unique_ptr<char[]> Holder;
        PkgPackArchive* Pkg;
        PkgPackFile* PkgFile;
    };
    std::deque<WorkPacket> workQueue;
    std::mutex workQueueMutex;
    std::condition_variable workQueueDataAvailableCondVar;
    std::condition_variable workQueueShouldPushMoreCondVar;
    size_t filesLeft = totalNumberOfFiles;

    const auto do_abort = [&]() -> void {
        encounteredError.store(true);

        // break out of remaining worker threads
        {
            std::unique_lock lock(workQueueMutex);
            filesLeft = 0;
            workQueueDataAvailableCondVar.notify_all();
            workQueueShouldPushMoreCondVar.notify_all();
        }
    };

    // worker threads
    std::vector<std::thread> workerThreads;
    workerThreads.reserve(threadCount);
    for (size_t i = 0; i < threadCount; ++i) {
        workerThreads.emplace_back([&]() -> void {
            while (true) {
                WorkPacket workPacket;
                {
                    std::unique_lock lock(workQueueMutex);
                    if (filesLeft == 0) {
                        return;
                    }
                    if (workQueue.empty()) {
                        workQueueDataAvailableCondVar.wait(
                            lock, [&] { return filesLeft == 0 || !workQueue.empty(); });
                        if (filesLeft == 0) {
                            return;
                        }
                    }
                    workPacket = std::move(workQueue.front());
                    workQueue.pop_front();
                    --filesLeft;

                    workQueueShouldPushMoreCondVar.notify_one();
                }

                // process work packet
                if (!CalculateHashForPkgFile(
                        *workPacket.Pkg, *workPacket.PkgFile, workPacket.Data)) {
                    do_abort();
                    return;
                }
            }
        });
    }

    // file reader thread
    std::thread fileReaderThread = std::thread([&]() -> void {
        for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
            if (encounteredError.load(std::memory_order_relaxed)) {
                return;
            }

            auto& pkg = pkgPackFiles[i];
            auto& files = pkg.Files;
            for (size_t j = 0; j < files.size(); ++j) {
                auto& f = files[j];
                auto compressedData = ReadPkgFile(pkg, f);
                if (!compressedData) {
                    do_abort();
                    return;
                }

                // push to worker thread
                char* filedata = compressedData.get();
                WorkPacket wp{.Data = filedata,
                              .Holder = std::move(compressedData),
                              .Pkg = &pkg,
                              .PkgFile = &f};
                {
                    std::unique_lock lock(workQueueMutex);
                    workQueue.emplace_back(std::move(wp));
                    workQueueDataAvailableCondVar.notify_one();

                    // no need to wait after last push
                    if (i != (pkgPackFiles.size() - 1)) {
                        // stall until some threads have consumed data to avoid massively filling
                        // memory
                        workQueueShouldPushMoreCondVar.wait(lock, [&] {
                            return workQueue.size() < threadCount || encounteredError.load();
                        });
                    }
                }
            }
        }
    });

    // wait for all threads to finish
    for (size_t i = 0; i < threadCount; ++i) {
        workerThreads[i].join();
    }
    fileReaderThread.join();

    return !encounteredError.load();
}

static std::optional<PkaConstructionData>
    FindFilesToWriteAndGenerateHeader(std::vector<PkgPackArchive>& pkgPackFiles,
                                      const std::vector<SenLib::ReferencedPka>& existingPkas) {
    // sort pkgs by filename so that you can binary search on them.
    std::stable_sort(pkgPackFiles.begin(),
                     pkgPackFiles.end(),
                     [](const PkgPackArchive& lhs, const PkgPackArchive& rhs) {
                         const auto& l = lhs.PkgName;
                         const auto& r = rhs.PkgName;
                         return std::memcmp(l.data(), r.data(), l.size()) < 0;
                     });

    // deduplicate out the files via hash (in std::map so they're already sorted for binary search)
    std::map<HyoutaUtils::Hash::SHA256, FileReference, SHA256Sorter> filesByHash;
    for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
        PkgPackArchive& archive = pkgPackFiles[i];
        for (size_t j = 0; j < archive.Files.size(); ++j) {
            const PkgPackFile& file = archive.Files[j];
            if (file.IsCorruptedFile) {
                continue;
            }

            auto it = filesByHash.try_emplace(file.Hash,
                                              FileReference{.ArchiveIndex = i, .FileIndex = j});
            if (!it.second) {
                // sanity checks
                FileReference& existingFileReference = it.first->second;
                const PkgPackFile& existingFile = pkgPackFiles[existingFileReference.ArchiveIndex]
                                                      .Files[existingFileReference.FileIndex];
                if (file.UncompressedSize != existingFile.UncompressedSize) {
                    printf("File with same hash has different file data.\n");
                    return std::nullopt;
                }
                if (file.CompressedSize < existingFile.CompressedSize) {
                    // prefer higher compression
                    existingFileReference.ArchiveIndex = i;
                    existingFileReference.FileIndex = j;
                }
            } else {
                FileReference& fileReference = it.first->second;
                for (const auto& existingPka : existingPkas) {
                    const SenLib::PkaHashToFileData* existingFile =
                        SenLib::FindFileInPkaByHash(existingPka.PkaHeader.Files.get(),
                                                    existingPka.PkaHeader.FilesCount,
                                                    file.Hash.Hash);
                    if (existingFile) {
                        fileReference.ShouldBeWritten = false;
                        break;
                    }
                }
            }
        }
    }

    const size_t numberOfPkgsToInclude = [&]() -> size_t {
        size_t count = 0;
        for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
            if (pkgPackFiles[i].IncludeInPka) {
                ++count;
            }
        }
        return count;
    }();
    const size_t numberOfFilesToInclude = [&]() -> size_t {
        size_t count = 0;
        for (const auto& kvp : filesByHash) {
            if (kvp.second.ShouldBeWritten) {
                ++count;
            }
        }
        return count;
    }();

    // calculate file offsets
    size_t pkaHeaderLength = (8u + 4u) + (numberOfFilesToInclude * (0x20u + 8u + 4u + 4u + 4u));
    for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
        if (!pkgPackFiles[i].IncludeInPka) {
            continue;
        }

        pkaHeaderLength += (0x20u + 4u + ((0x40u + 0x20u) * pkgPackFiles[i].Files.size()));
    }

    // construct header
    using HyoutaUtils::EndianUtils::ToEndian;
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;
    using HyoutaUtils::MemWrite::WriteAdvArray;
    using HyoutaUtils::MemWrite::WriteAdvUInt32;
    using HyoutaUtils::MemWrite::WriteAdvUInt64;
    auto pkaHeader = std::make_unique<char[]>(pkaHeaderLength);
    if (!pkaHeader) {
        printf("Failed to allocate memory.\n");
        return std::nullopt;
    }

    char* pkaHeaderWritePtr = pkaHeader.get();
    WriteAdvUInt32(pkaHeaderWritePtr, ToEndian(static_cast<uint32_t>(0x7ff7cf0d), LittleEndian));
    WriteAdvUInt32(pkaHeaderWritePtr,
                   ToEndian(static_cast<uint32_t>(numberOfPkgsToInclude), LittleEndian));
    for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
        if (!pkgPackFiles[i].IncludeInPka) {
            continue;
        }

        WriteAdvArray(pkaHeaderWritePtr, pkgPackFiles[i].PkgName);
        WriteAdvUInt32(pkaHeaderWritePtr,
                       ToEndian(static_cast<uint32_t>(pkgPackFiles[i].Files.size()), LittleEndian));
        for (size_t j = 0; j < pkgPackFiles[i].Files.size(); ++j) {
            WriteAdvArray(pkaHeaderWritePtr, pkgPackFiles[i].Files[j].Filename);
            WriteAdvArray(pkaHeaderWritePtr, pkgPackFiles[i].Files[j].Hash.Hash);
        }
    }
    WriteAdvUInt32(pkaHeaderWritePtr,
                   ToEndian(static_cast<uint32_t>(numberOfFilesToInclude), LittleEndian));
    for (auto& kvp : filesByHash) {
        FileReference& ref = kvp.second;
        if (kvp.second.ShouldBeWritten) {
            WriteAdvArray(pkaHeaderWritePtr, kvp.first.Hash);
            ref.PkaHeaderPtr = pkaHeaderWritePtr;

            // dummy data for now
            WriteAdvUInt64(pkaHeaderWritePtr, 0);
            WriteAdvUInt32(pkaHeaderWritePtr, 0);
            WriteAdvUInt32(pkaHeaderWritePtr, 0);
            WriteAdvUInt32(pkaHeaderWritePtr, 0);
        }
    }
    assert(pkaHeaderWritePtr == (pkaHeader.get() + pkaHeaderLength));

    // collect the actual set of files to write
    std::vector<FileToWriteInfo> fileToWriteInfos;
    fileToWriteInfos.reserve(numberOfFilesToInclude);
    for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
        for (size_t j = 0; j < pkgPackFiles[i].Files.size(); ++j) {
            auto it = filesByHash.find(pkgPackFiles[i].Files[j].Hash);
            if (it == filesByHash.end()) {
                printf("Internal error: Failed to find file hash.\n");
                return std::nullopt;
            }
            FileReference& ref = it->second;
            if (ref.ShouldBeWritten && !ref.AlreadyWritten) {
                fileToWriteInfos.emplace_back(FileToWriteInfo{.ArchiveIndex = ref.ArchiveIndex,
                                                              .FileIndex = ref.FileIndex,
                                                              .PkaHeaderPtr = ref.PkaHeaderPtr});
                ref.AlreadyWritten = true;
            }
        }
    }
    if (fileToWriteInfos.size() != numberOfFilesToInclude) {
        printf("Internal error: Unexpected number of files.\n");
        return std::nullopt;
    }

    return PkaConstructionData{.PkaHeader = std::move(pkaHeader),
                               .PkaHeaderLength = pkaHeaderLength,
                               .FilesToWrite = std::move(fileToWriteInfos)};
}

static bool WriteToPka(HyoutaUtils::IO::File& outfile,
                       uint64_t& fileOffset,
                       FileToWriteInfo& ref,
                       std::vector<PkgPackArchive>& pkgPackFiles,
                       const std::optional<uint32_t>& recompressFlags) {
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;
    assert(fileOffset == outfile.GetPosition());
    PkgPackArchive& archive = pkgPackFiles[ref.ArchiveIndex];
    const PkgPackFile& file = archive.Files[ref.FileIndex];

    if (recompressFlags) {
        auto& infile = archive.FileHandle;
        auto data = std::make_unique_for_overwrite<char[]>(file.CompressedSize);
        if (!data) {
            printf("Failed to allocate memory.\n");
            return false;
        }
        if (!infile.SetPosition(file.OffsetInPkg)) {
            printf("Failed to seek in pkg.\n");
            return false;
        }
        if (infile.Read(data.get(), file.CompressedSize) != file.CompressedSize) {
            printf("Failed to read pkg.\n");
            return false;
        }

        auto dataBuffer = std::make_unique<char[]>(file.UncompressedSize);
        if (!dataBuffer) {
            printf("Failed to allocate memory.\n");
            return false;
        }
        if (!SenLib::ExtractAndDecompressPkgFile(dataBuffer.get(),
                                                 file.UncompressedSize,
                                                 data.get(),
                                                 file.CompressedSize,
                                                 file.Flags,
                                                 LittleEndian)) {
            printf("Failed to extract file from pkg.\n");
            return false;
        }

        std::unique_ptr<char[]> recompressedDataBuffer;
        SenLib::PkgFile pkgFile;
        if (!SenLib::CompressPkgFile(recompressedDataBuffer,
                                     pkgFile,
                                     dataBuffer.get(),
                                     file.UncompressedSize,
                                     *recompressFlags,
                                     LittleEndian)) {
            printf("Failed recompressing file.\n");
            return false;
        }

        if (outfile.Write(recompressedDataBuffer.get(), pkgFile.CompressedSize)
            != pkgFile.CompressedSize) {
            printf("Failed to write data to output file.\n");
            return false;
        }

        ref.FileOffset = fileOffset;
        ref.CompressedSize = pkgFile.CompressedSize;
        ref.UncompressedSize = file.UncompressedSize;
        ref.PkgFlags = pkgFile.Flags;

        fileOffset += pkgFile.CompressedSize;
    } else {
        auto& infile = archive.FileHandle;
        auto data = std::make_unique_for_overwrite<char[]>(file.CompressedSize);
        if (!data) {
            printf("Failed to allocate memory.\n");
            return false;
        }
        if (!infile.SetPosition(file.OffsetInPkg)) {
            printf("Failed to seek in pkg.\n");
            return false;
        }
        if (infile.Read(data.get(), file.CompressedSize) != file.CompressedSize) {
            printf("Failed to read pkg.\n");
            return false;
        }
        if (outfile.Write(data.get(), file.CompressedSize) != file.CompressedSize) {
            printf("Failed to write data to output file.\n");
            return false;
        }

        ref.FileOffset = fileOffset;
        ref.CompressedSize = file.CompressedSize;
        ref.UncompressedSize = file.UncompressedSize;
        ref.PkgFlags = file.Flags;

        fileOffset += file.CompressedSize;
    }

    return true;
}

static bool WriteToPkaMultithreaded(HyoutaUtils::IO::File& outfile,
                                    PkaConstructionData& pkaConstructionData,
                                    std::vector<PkgPackArchive>& pkgPackFiles,
                                    uint32_t recompressFlags,
                                    size_t threadCount) {
    // This is largely the same as the P3A threading logic.

    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    // This is designed like this:
    // - threadCount worker threads, which do the actual compression of the files
    // - 1 file reader thread, which reads the input files and hands them over to a worker thread
    // - 1 file writer thread, which takes the output of the worker thread and writes them to the
    //   output file

    std::atomic<bool> encounteredError = false;

    struct alignas(64) FileDataToWrite {
        bool IsValid = false;
        const char* Data;
        uint32_t CompressedSize;
        uint32_t UncompressedSize;
        std::unique_ptr<char[]> Holder;
        size_t Index;
        uint32_t RecompressFlags;
    };
    auto fileDataToWriteArray =
        std::make_unique<FileDataToWrite[]>(pkaConstructionData.FilesToWrite.size());
    std::mutex fileDataToWriteMutex;
    std::condition_variable fileDataToWriteCondVar;
    bool fileDataToWriteAbort = false;

    struct alignas(64) FileDataToCompress {
        const char* Data;
        uint32_t OriginalCompressedSize;
        uint32_t OriginalUncompressedSize;
        std::unique_ptr<char[]> Holder;
        size_t Index;
        uint32_t OriginalFlags;
        uint32_t RecompressFlags;
    };
    std::deque<FileDataToCompress> fileDataToCompressQueue;
    std::mutex fileDataToCompressMutex;
    std::condition_variable fileDataToCompressDataAvailableCondVar;
    std::condition_variable fileDataToCompressShouldPushMoreCondVar;
    size_t fileDataToCompressLeft = pkaConstructionData.FilesToWrite.size();

    const auto do_abort = [&]() -> void {
        encounteredError.store(true);

        // break out of file writer thread
        {
            std::unique_lock lock(fileDataToWriteMutex);
            fileDataToWriteAbort = true;
            fileDataToWriteCondVar.notify_all();
        }

        // break out of remaining worker threads
        {
            std::unique_lock lock(fileDataToCompressMutex);
            fileDataToCompressLeft = 0;
            fileDataToCompressDataAvailableCondVar.notify_all();
            fileDataToCompressShouldPushMoreCondVar.notify_all();
        }
    };

    // worker threads
    std::vector<std::thread> workerThreads;
    workerThreads.reserve(threadCount);
    for (size_t i = 0; i < threadCount; ++i) {
        workerThreads.emplace_back([&]() -> void {
            while (true) {
                FileDataToCompress workPacket;
                {
                    std::unique_lock lock(fileDataToCompressMutex);
                    if (fileDataToCompressLeft == 0) {
                        return;
                    }
                    if (fileDataToCompressQueue.empty()) {
                        fileDataToCompressDataAvailableCondVar.wait(lock, [&] {
                            return fileDataToCompressLeft == 0 || !fileDataToCompressQueue.empty();
                        });
                        if (fileDataToCompressLeft == 0) {
                            return;
                        }
                    }
                    workPacket = std::move(fileDataToCompressQueue.front());
                    fileDataToCompressQueue.pop_front();
                    --fileDataToCompressLeft;

                    fileDataToCompressShouldPushMoreCondVar.notify_one();
                }

                // process work packet
                auto dataBuffer = std::make_unique<char[]>(workPacket.OriginalUncompressedSize);
                if (!dataBuffer) {
                    do_abort();
                    printf("Failed to allocate memory.\n");
                    return;
                }
                if (!SenLib::ExtractAndDecompressPkgFile(dataBuffer.get(),
                                                         workPacket.OriginalUncompressedSize,
                                                         workPacket.Data,
                                                         workPacket.OriginalCompressedSize,
                                                         workPacket.OriginalFlags,
                                                         LittleEndian)) {
                    do_abort();
                    printf("Failed to extract file from pkg.\n");
                    return;
                }

                std::unique_ptr<char[]> recompressedDataBuffer;
                SenLib::PkgFile pkgFile;
                if (!SenLib::CompressPkgFile(recompressedDataBuffer,
                                             pkgFile,
                                             dataBuffer.get(),
                                             workPacket.OriginalUncompressedSize,
                                             workPacket.RecompressFlags,
                                             LittleEndian)) {
                    do_abort();
                    printf("Failed recompressing file.\n");
                    return;
                }

                // push result to writer thread
                char* recompressedData = recompressedDataBuffer.get();
                FileDataToWrite data{.IsValid = true,
                                     .Data = recompressedData,
                                     .CompressedSize = pkgFile.CompressedSize,
                                     .UncompressedSize = workPacket.OriginalUncompressedSize,
                                     .Holder = std::move(recompressedDataBuffer),
                                     .Index = workPacket.Index,
                                     .RecompressFlags = pkgFile.Flags};
                {
                    std::unique_lock lock(fileDataToWriteMutex);
                    fileDataToWriteArray[workPacket.Index] = std::move(data);
                    fileDataToWriteCondVar.notify_all();
                }
            }
        });
    }

    // file reader thread
    std::thread fileReaderThread = std::thread([&]() -> void {
        for (size_t i = 0; i < pkaConstructionData.FilesToWrite.size(); ++i) {
            if (encounteredError.load(std::memory_order_relaxed)) {
                return;
            }

            const auto& ref = pkaConstructionData.FilesToWrite[i];
            PkgPackArchive& archive = pkgPackFiles[ref.ArchiveIndex];
            const PkgPackFile& file = archive.Files[ref.FileIndex];

            auto& infile = archive.FileHandle;
            auto data = std::make_unique_for_overwrite<char[]>(file.CompressedSize);
            if (!data) {
                do_abort();
                printf("Failed to allocate memory.\n");
                return;
            }
            if (!infile.SetPosition(file.OffsetInPkg)) {
                do_abort();
                printf("Failed to seek in pkg.\n");
                return;
            }
            if (infile.Read(data.get(), file.CompressedSize) != file.CompressedSize) {
                do_abort();
                printf("Failed to read pkg.\n");
                return;
            }

            // push to worker thread
            char* filedata = data.get();
            FileDataToCompress cdata{.Data = filedata,
                                     .OriginalCompressedSize = file.CompressedSize,
                                     .OriginalUncompressedSize = file.UncompressedSize,
                                     .Holder = std::move(data),
                                     .Index = i,
                                     .OriginalFlags = file.Flags,
                                     .RecompressFlags = recompressFlags};

            {
                std::unique_lock lock(fileDataToCompressMutex);
                fileDataToCompressQueue.emplace_back(std::move(cdata));
                fileDataToCompressDataAvailableCondVar.notify_one();

                // no need to wait after last push
                if (i != (pkaConstructionData.FilesToWrite.size() - 1)) {
                    // stall until some threads have consumed data to avoid massively filling memory
                    fileDataToCompressShouldPushMoreCondVar.wait(lock, [&] {
                        return fileDataToCompressQueue.size() < threadCount
                               || encounteredError.load();
                    });
                }
            }
        }
    });

    // file writer thread
    std::thread fileWriterThread = std::thread([&]() -> void {
        uint64_t fileOffset = pkaConstructionData.PkaHeaderLength;
        for (size_t i = 0; i < pkaConstructionData.FilesToWrite.size(); ++i) {
            assert(fileOffset == outfile.GetPosition());
            auto& ref = pkaConstructionData.FilesToWrite[i];

            FileDataToWrite data;
            {
                std::unique_lock lock(fileDataToWriteMutex);
                if (fileDataToWriteAbort) {
                    return;
                }
                if (!fileDataToWriteArray[i].IsValid) {
                    fileDataToWriteCondVar.wait(lock, [&] {
                        return fileDataToWriteAbort || fileDataToWriteArray[i].IsValid;
                    });
                    if (fileDataToWriteAbort) {
                        return;
                    }
                }
                data = std::move(fileDataToWriteArray[i]);
            }

            if (outfile.Write(data.Data, data.CompressedSize) != data.CompressedSize) {
                do_abort();
                printf("Failed to write data to output file.\n");
                return;
            }

            ref.FileOffset = fileOffset;
            ref.CompressedSize = data.CompressedSize;
            ref.UncompressedSize = data.UncompressedSize;
            ref.PkgFlags = data.RecompressFlags;

            fileOffset += data.CompressedSize;
        }
    });

    // wait for all threads to finish
    for (size_t i = 0; i < threadCount; ++i) {
        workerThreads[i].join();
    }
    fileReaderThread.join();
    fileWriterThread.join();

    return !encounteredError.load();
}

int PKA_Pack_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{.Type = HyoutaUtils::ArgTypes::String,
                                                 .ShortKey = "o",
                                                 .LongKey = "output",
                                                 .Argument = "FILENAME",
                                                 .Description =
                                                     "The output filename. Must be given."};
    static constexpr HyoutaUtils::Arg arg_ref_pka{
        .Type = HyoutaUtils::ArgTypes::StringArray,
        .LongKey = "referenced-pka",
        .Argument = "PKA",
        .Description =
            "Existing pka file that already contains files. Files contained in that pka will not "
            "be packed into this pka. The referenced pka will be necessary to extract data later. "
            "Option can be provided multiple times. This is a nonstandard feature that the vanilla "
            "game does not handle."};
    static constexpr HyoutaUtils::Arg arg_recompress{
        .Type = HyoutaUtils::ArgTypes::String,
        .LongKey = "recompress",
        .Argument = "TYPE",
        .Description =
            "Recompress all files before packing them into the pka.\n"
            "Options are: 'none', 'type1', 'lz4', 'zstd'."};
    static constexpr HyoutaUtils::Arg arg_threads{
        .Type = HyoutaUtils::ArgTypes::UInt64,
        .ShortKey = "t",
        .LongKey = "threads",
        .Argument = "THREADCOUNT",
        .Description =
            "Use THREADCOUNT threads for compression. Use 0 (default) for automatic detection."};
    static constexpr std::array<const HyoutaUtils::Arg*, 4> args_array{
        {&arg_output, &arg_ref_pka, &arg_recompress, &arg_threads}};
    static constexpr HyoutaUtils::Args args(
        "sentools " PKA_Pack_Name, "directory", PKA_Pack_ShortDescription, args_array);
    auto parseResult = args.Parse(argc, argv);
    if (parseResult.IsError()) {
        printf("Argument error: %s\n\n\n", parseResult.GetErrorValue().c_str());
        args.PrintUsage();
        return -1;
    }

    const auto& options = parseResult.GetSuccessValue();
    if (options.FreeArguments.size() == 0) {
        printf("Argument error: %s\n\n\n", "No input directory given.");
        args.PrintUsage();
        return -1;
    }

    auto* output_option = options.TryGetString(&arg_output);
    if (output_option == nullptr) {
        printf("Argument error: %s\n\n\n", "No output filename given.");
        args.PrintUsage();
        return -1;
    }

    std::optional<uint32_t> recompressFlags = std::nullopt;
    if (auto* recompress_option = options.TryGetString(&arg_recompress)) {
        const auto& compressionString = *recompress_option;
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("none", compressionString)) {
            recompressFlags = static_cast<uint32_t>(0);
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("type1", compressionString)) {
            recompressFlags = static_cast<uint32_t>(1);
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("lz4", compressionString)) {
            recompressFlags = static_cast<uint32_t>(4);
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("zstd", compressionString)) {
            recompressFlags = static_cast<uint32_t>(0x10);
        } else {
            printf("Argument error: %s\n\n\n", "Invalid compression type.");
            args.PrintUsage();
            return -1;
        }
    }

    auto* threads_option = options.TryGetUInt64(&arg_threads);
    size_t desiredThreadCount = 0;
    if (threads_option != nullptr) {
        uint64_t argThreadCount = *threads_option;
        if (argThreadCount <= std::numeric_limits<size_t>::max()) {
            desiredThreadCount = static_cast<size_t>(argThreadCount);
        }
    }

    std::string_view target(*output_option);
    std::span<const std::string_view> referencedPkaPaths;
    if (auto* referenced_pka_option = options.TryGetStringArray(&arg_ref_pka)) {
        referencedPkaPaths = *referenced_pka_option;
    }

    std::vector<PackPkaPkgInfo> sourcePkgs;
    for (size_t i = 0; i < options.FreeArguments.size(); ++i) {
        std::filesystem::path rootDir =
            HyoutaUtils::IO::FilesystemPathFromUtf8(options.FreeArguments[i]);
        std::error_code ec;
        std::filesystem::directory_iterator iterator(rootDir, ec);
        if (ec) {
            printf("Failed to iterate over contents of %.*s\n",
                   static_cast<int>(options.FreeArguments[i].size()),
                   options.FreeArguments[i].data());
            return -1;
        }
        while (iterator != std::filesystem::directory_iterator()) {
            auto& entry = *iterator;
            if (!entry.is_directory()) {
                const auto ext = entry.path().extension();
#ifdef BUILD_FOR_WINDOWS
                const auto extUtf8Optional =
                    HyoutaUtils::TextUtils::WStringToUtf8(ext.native().data(), ext.native().size());
                if (!extUtf8Optional) {
                    printf("Internal error.\n");
                    return -1;
                }
                const auto& extUtf8 = *extUtf8Optional;
#else
                const auto& extUtf8 = ext.native();
#endif
                if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(".pkg", extUtf8)) {
                    const auto& path = entry.path();
                    const auto fn = path.filename();
#ifdef BUILD_FOR_WINDOWS
                    const auto pathUtf8Optional = HyoutaUtils::TextUtils::WStringToUtf8(
                        path.native().data(), path.native().size());
                    if (!pathUtf8Optional) {
                        printf("Internal error.\n");
                        return -1;
                    }
                    const auto fnUtf8Optional = HyoutaUtils::TextUtils::WStringToUtf8(
                        fn.native().data(), fn.native().size());
                    if (!fnUtf8Optional) {
                        printf("Internal error.\n");
                        return -1;
                    }
                    const auto& pathUtf8 = *pathUtf8Optional;
                    const auto& fnUtf8 = *fnUtf8Optional;
#else
                    const auto& pathUtf8 = path.native();
                    const auto& fnUtf8 = fn.native();
#endif

                    sourcePkgs.emplace_back(PackPkaPkgInfo{.Path = pathUtf8, .NameInPka = fnUtf8});
                }
            }

            iterator.increment(ec);
            if (ec) {
                printf("Failed to iterate over contents of %.*s\n",
                       static_cast<int>(options.FreeArguments[i].size()),
                       options.FreeArguments[i].data());
                return -1;
            }
        }
    }

    auto result =
        PackPka(target, sourcePkgs, referencedPkaPaths, recompressFlags, desiredThreadCount);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}

HyoutaUtils::Result<PackPkaResult, std::string>
    PackPka(std::string_view target,
            std::span<const PackPkaPkgInfo> sourcePkgs,
            std::span<const std::string_view> existingPkaPaths,
            std::optional<uint32_t> recompressFlags,
            size_t desiredThreadCount) {
    using HyoutaUtils::EndianUtils::ToEndian;
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;
    using HyoutaUtils::MemWrite::WriteAdvUInt32;
    using HyoutaUtils::MemWrite::WriteAdvUInt64;

    const size_t threadCount =
        desiredThreadCount == 0 ? std::thread::hardware_concurrency() : desiredThreadCount;

    std::vector<SenLib::ReferencedPka> existingPkas;
    {
        existingPkas.reserve(existingPkaPaths.size());
        for (const std::string_view& existingPkaPath : existingPkaPaths) {
            auto& existingPka = existingPkas.emplace_back();
            existingPka.PkaFile.Open(std::string_view(existingPkaPath),
                                     HyoutaUtils::IO::OpenMode::Read);
            if (!existingPka.PkaFile.IsOpen()) {
                return std::string("Error opening existing pka.");
            }
            if (!SenLib::ReadPkaFromFile(existingPka.PkaHeader, existingPka.PkaFile)) {
                return std::string("Error reading existing pka.");
            }
        }
    }

    // first collect info about every file in every pkg
    std::vector<PkgPackArchive> pkgPackFiles;
    {
        for (size_t i = 0; i < sourcePkgs.size(); ++i) {
            PkgPackArchive archive = InitPkgPackArchive(pkgPackFiles, sourcePkgs[i]);
            pkgPackFiles.emplace_back(std::move(archive));
        }

        for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
            if (!LoadPkg(pkgPackFiles[i])) {
                return std::format("Failed to load pkg at {}", sourcePkgs[i].Path);
            }
        }

        size_t totalNumberOfFiles = 0;
        for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
            totalNumberOfFiles += pkgPackFiles[i].Files.size();
        }

        const size_t filesToHashThreadCount = std::min(totalNumberOfFiles, threadCount);
        if (filesToHashThreadCount >= 2) {
            if (!CalculateHashForPkgFilesMultithreaded(
                    pkgPackFiles, totalNumberOfFiles, filesToHashThreadCount)) {
                return std::string("Error during hash calculation.");
            }
        } else {
            for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
                auto& pkg = pkgPackFiles[i];
                auto& files = pkg.Files;
                for (size_t j = 0; j < files.size(); ++j) {
                    if (!CalculateHashForPkgFile(pkg, files[j])) {
                        return std::string("Error during hash calculation.");
                    }
                }
            }
        }
    }

    auto pkaConstructionData = FindFilesToWriteAndGenerateHeader(pkgPackFiles, existingPkas);
    if (!pkaConstructionData) {
        return std::string("Error while building PKA header.");
    }

    // write PKA
    std::string targetTmp(target);
    targetTmp.append(".tmp");
    HyoutaUtils::IO::File outfile(std::string_view(targetTmp), HyoutaUtils::IO::OpenMode::Write);
    auto outfileGuard = HyoutaUtils::MakeDisposableScopeGuard([&outfile]() { outfile.Delete(); });
    if (!outfile.IsOpen()) {
        return std::string("Failed to open output file.");
    }
    if (outfile.Write(pkaConstructionData->PkaHeader.get(), pkaConstructionData->PkaHeaderLength)
        != pkaConstructionData->PkaHeaderLength) {
        return std::string("Failed to write temp header to output file.");
    }

    const size_t fileToWriteThreadCount =
        std::min(pkaConstructionData->FilesToWrite.size(), threadCount);
    if (recompressFlags.has_value() && fileToWriteThreadCount >= 2) {
        if (!WriteToPkaMultithreaded(outfile,
                                     *pkaConstructionData,
                                     pkgPackFiles,
                                     *recompressFlags,
                                     fileToWriteThreadCount)) {
            return std::string("Failed to write file data to PKA.");
        }
    } else {
        uint64_t fileOffset = pkaConstructionData->PkaHeaderLength;
        for (size_t i = 0; i < pkaConstructionData->FilesToWrite.size(); ++i) {
            if (!WriteToPka(outfile,
                            fileOffset,
                            pkaConstructionData->FilesToWrite[i],
                            pkgPackFiles,
                            recompressFlags)) {
                return std::string("Failed to write file data to PKA.");
            }
        }
    }

    // fix header
    for (auto& ref : pkaConstructionData->FilesToWrite) {
        char* ptr = ref.PkaHeaderPtr;
        WriteAdvUInt64(ptr, ToEndian(ref.FileOffset, LittleEndian));
        WriteAdvUInt32(ptr, ToEndian(ref.CompressedSize, LittleEndian));
        WriteAdvUInt32(ptr, ToEndian(ref.UncompressedSize, LittleEndian));
        WriteAdvUInt32(ptr, ToEndian(ref.PkgFlags, LittleEndian));
    }
    if (!outfile.SetPosition(0)) {
        return std::string("Failed to seek in output file.");
    }
    if (outfile.Write(pkaConstructionData->PkaHeader.get(), pkaConstructionData->PkaHeaderLength)
        != pkaConstructionData->PkaHeaderLength) {
        return std::string("Failed to write corrected header to output file.");
    }

    outfileGuard.Dispose();
    if (!outfile.Rename(target)) {
        return std::string("Failed to rename output file to correct filename.");
    }

    return PackPkaResult::Success;
}
} // namespace SenTools
