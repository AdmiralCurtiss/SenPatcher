#include "pka_pack.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "cpp-optparse/OptionParser.h"

#include "sen/pka.h"
#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "sen/pkg_extract.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/hash/sha256.h"
#include "util/memread.h"
#include "util/memwrite.h"
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

int PKA_Pack_Function(int argc, char** argv) {
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    optparse::OptionParser parser;
    parser.description(PKA_Pack_ShortDescription);

    parser.usage("sentools " PKA_Pack_Name " [options] directory");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output filename. Must be given.");
    parser.add_option("--referenced-pka")
        .dest("referenced-pka")
        .metavar("PKA")
        .help(
            "Existing pka file that already contains files. Files contained in that pka will not "
            "be packed into this pka. The referenced pka will be necessary to extract data later. "
            "This is a nonstandard feature that the vanilla game does not handle.");
    parser.add_option("--recompress")
        .dest("recompress")
        .metavar("TYPE")
        .help("Recompress all files before packing them into the pka.")
        .choices({"none", "type1", "lz4", "zstd"});

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() == 0) {
        parser.error("No input directory given.");
        return -1;
    }

    if (!options.is_set("output")) {
        parser.error("No output filename given.");
        return -1;
    }

    std::optional<uint32_t> recompressFlags = std::nullopt;
    if (options.is_set("recompress")) {
        const auto& compressionString = options["recompress"];
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("none", compressionString)) {
            recompressFlags = static_cast<uint32_t>(0);
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("type1", compressionString)) {
            recompressFlags = static_cast<uint32_t>(1);
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("lz4", compressionString)) {
            recompressFlags = static_cast<uint32_t>(4);
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("zstd", compressionString)) {
            recompressFlags = static_cast<uint32_t>(0x10);
        } else {
            parser.error("Invalid compression type.");
            return -1;
        }
    }

    std::string_view target(options["output"]);

    std::optional<HyoutaUtils::IO::File> existingPkaFile = std::nullopt;
    std::optional<SenLib::PkaHeader> existingPkaHeader = std::nullopt;
    if (options.is_set("referenced-pka")) {
        std::string_view existingPkaPath(options["referenced-pka"]);
        existingPkaFile.emplace(std::filesystem::path(existingPkaPath),
                                HyoutaUtils::IO::OpenMode::Read);
        if (!existingPkaFile->IsOpen()) {
            printf("Error opening existing pka.\n");
            return -1;
        }
        existingPkaHeader.emplace();
        if (!SenLib::ReadPkaFromFile(*existingPkaHeader, *existingPkaFile)) {
            printf("Error reading existing pka.\n");
            return -1;
        }
    }

    // first collect info about every file in every pkg
    struct PkgPackFile {
        std::array<char, 0x40> Filename;
        HyoutaUtils::Hash::SHA256 Hash;
        uint32_t OffsetInPkg;
        uint32_t CompressedSize;
        uint32_t UncompressedSize;
        uint32_t Flags;
    };
    struct PkgPackArchive {
        HyoutaUtils::IO::File FileHandle;
        std::array<char, 0x20> PkgName;
        std::vector<PkgPackFile> Files;
        bool IncludeInPka = true;
    };
    std::vector<PkgPackArchive> pkgPackFiles;
    {
        // returns true if we should continue processing, or false if we should exit
        const auto add_pkg = [&](const std::filesystem::path& rootDir,
                                 const std::filesystem::directory_entry& entry) -> bool {
            std::error_code ec;
            if (entry.is_directory()) {
                return true;
            }
            const auto ext = entry.path().extension().generic_u8string();
            if (!HyoutaUtils::TextUtils::CaseInsensitiveEquals(
                    ".pkg",
                    std::string_view(reinterpret_cast<const char*>(ext.data()), ext.size()))) {
                return true;
            }

            const auto relativePath = std::filesystem::relative(entry.path(), rootDir, ec);
            if (relativePath.empty()) {
                printf("Error while collecting files.\n");
                return false;
            }
            const auto filename = relativePath.u8string();
            const char8_t* filenameC = filename.c_str();

            std::array<char, 0x20> fn{};
            const size_t filenameLength = [&]() -> size_t {
                for (size_t i = 0; i < fn.size() - 1; ++i) {
                    const char c = static_cast<char>(filenameC[i]);
                    if (c == '\0') {
                        return i;
                    }
                    fn[i] = c;
                }
                return fn.size();
            }();
            NormalizePkgName(fn);

            auto existingArchive = std::find_if(
                pkgPackFiles.begin(), pkgPackFiles.end(), [&](const PkgPackArchive& a) {
                    return strncmp(a.PkgName.data(), fn.data(), fn.size()) == 0;
                });
            const bool archiveExistsAlready = (existingArchive != pkgPackFiles.end());
            if (archiveExistsAlready) {
                printf(
                    "WARNING: %s exists multiple times, only the first parsed archive will be "
                    "available in the PKA (but the contents of all instances will be packed).\n",
                    filenameC);
            }

            auto& fi = pkgPackFiles.emplace_back(PkgPackArchive{
                .FileHandle = HyoutaUtils::IO::File(entry.path(), HyoutaUtils::IO::OpenMode::Read),
                .PkgName = fn,
                .IncludeInPka = !archiveExistsAlready});

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
            if (filenameLength < 28) {
                const uint32_t unknownId = HyoutaUtils::EndianUtils::FromEndian(
                    HyoutaUtils::MemRead::ReadUInt32(pkgHeaderInitialBytes.data()), LittleEndian);
                HyoutaUtils::MemWrite::WriteUInt32(
                    fi.PkgName.data() + 28,
                    HyoutaUtils::EndianUtils::ToEndian(unknownId, LittleEndian));
            }

            const uint32_t fileCountInPkg = HyoutaUtils::EndianUtils::FromEndian(
                HyoutaUtils::MemRead::ReadUInt32(&pkgHeaderInitialBytes[4]), LittleEndian);
            const size_t pkgHeaderLength = 8u + static_cast<size_t>(fileCountInPkg) * 0x50u;
            auto pkgHeaderData = std::make_unique_for_overwrite<char[]>(pkgHeaderLength);
            if (!pkgHeaderData) {
                printf("Failed to allocate memory.\n");
                return false;
            }
            std::memcpy(
                pkgHeaderData.get(), pkgHeaderInitialBytes.data(), pkgHeaderInitialBytes.size());
            if (infile.Read(pkgHeaderData.get() + pkgHeaderInitialBytes.size(),
                            pkgHeaderLength - pkgHeaderInitialBytes.size())
                != (pkgHeaderLength - pkgHeaderInitialBytes.size())) {
                printf("Failed to read pkg.\n");
                return false;
            }

            SenLib::PkgHeader pkgHeader;
            if (!SenLib::ReadPkgFromMemory(
                    pkgHeader, pkgHeaderData.get(), pkgHeaderLength, LittleEndian)) {
                printf("Failed to read pkg header.\n");
                return false;
            }

            fi.Files.reserve(pkgHeader.FileCount);
            for (uint32_t i = 0; i < pkgHeader.FileCount; ++i) {
                const auto& f = pkgHeader.Files[i];
                auto compressedData = std::make_unique_for_overwrite<char[]>(f.CompressedSize);
                if (!compressedData) {
                    printf("Failed to allocate memory.\n");
                    return false;
                }
                if (!infile.SetPosition(f.DataPosition)) {
                    printf("Failed to seek in pkg.\n");
                    return false;
                }
                if (infile.Read(compressedData.get(), f.CompressedSize) != f.CompressedSize) {
                    printf("Failed to read pkg.\n");
                    return false;
                }

                auto dataBuffer = std::make_unique<char[]>(f.UncompressedSize);
                if (!dataBuffer) {
                    printf("Failed to allocate memory.\n");
                    return false;
                }
                if (!SenLib::ExtractAndDecompressPkgFile(dataBuffer.get(),
                                                         f.UncompressedSize,
                                                         compressedData.get(),
                                                         f.CompressedSize,
                                                         f.Flags,
                                                         LittleEndian)) {
                    printf("Failed to extract file from pkg.\n");
                    return false;
                }
                fi.Files.emplace_back(PkgPackFile{
                    .Filename = f.Filename,
                    .Hash =
                        HyoutaUtils::Hash::CalculateSHA256(dataBuffer.get(), f.UncompressedSize),
                    .OffsetInPkg = f.DataPosition,
                    .CompressedSize = f.CompressedSize,
                    .UncompressedSize = f.UncompressedSize,
                    .Flags = f.Flags,
                });
            }

            return true;
        };

        for (size_t i = 0; i < args.size(); ++i) {
            std::filesystem::path rootDir(args[i]);
            std::error_code ec;
            std::filesystem::directory_iterator iterator(rootDir, ec);
            if (ec) {
                return false;
            }
            for (const std::filesystem::directory_entry& entry : iterator) {
                if (!add_pkg(rootDir, entry)) {
                    return -1;
                }
            }
        }
    }

    // sort pkgs by filename so that you can binary search on them.
    std::stable_sort(pkgPackFiles.begin(),
                     pkgPackFiles.end(),
                     [](const PkgPackArchive& lhs, const PkgPackArchive& rhs) {
                         const auto& l = lhs.PkgName;
                         const auto& r = rhs.PkgName;
                         return std::memcmp(l.data(), r.data(), l.size()) < 0;
                     });

    // deduplicate out the files via hash (in std::map so they're already sorted for binary search)
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
    std::map<HyoutaUtils::Hash::SHA256, FileReference, SHA256Sorter> filesByHash;
    for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
        PkgPackArchive& archive = pkgPackFiles[i];
        for (size_t j = 0; j < archive.Files.size(); ++j) {
            const PkgPackFile& file = archive.Files[j];
            auto it = filesByHash.try_emplace(file.Hash,
                                              FileReference{.ArchiveIndex = i, .FileIndex = j});
            if (!it.second) {
                // sanity checks
                FileReference& existingFileReference = it.first->second;
                const PkgPackFile& existingFile = pkgPackFiles[existingFileReference.ArchiveIndex]
                                                      .Files[existingFileReference.FileIndex];
                if (file.UncompressedSize != existingFile.UncompressedSize) {
                    printf("File with same hash has different file data.\n");
                    return -1;
                }
                if (file.CompressedSize < existingFile.CompressedSize) {
                    // prefer higher compression
                    existingFileReference.ArchiveIndex = i;
                    existingFileReference.FileIndex = j;
                }
            } else {
                FileReference& fileReference = it.first->second;
                if (existingPkaHeader.has_value()) {
                    const SenLib::PkaHashToFileData* existingFile =
                        SenLib::FindFileInPkaByHash(existingPkaHeader->Files.get(),
                                                    existingPkaHeader->FilesCount,
                                                    file.Hash.Hash);
                    if (existingFile) {
                        fileReference.ShouldBeWritten = false;
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
    using HyoutaUtils::MemWrite::WriteAdvArray;
    using HyoutaUtils::MemWrite::WriteAdvUInt32;
    using HyoutaUtils::MemWrite::WriteAdvUInt64;
    auto pkaHeader = std::make_unique<char[]>(pkaHeaderLength);
    if (!pkaHeader) {
        printf("Failed to allocate memory.\n");
        return -1;
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

    // write file
    uint64_t fileOffset = pkaHeaderLength;
    HyoutaUtils::IO::File outfile(std::filesystem::path(target), HyoutaUtils::IO::OpenMode::Write);
    if (!outfile.IsOpen()) {
        printf("Failed to open output file.\n");
        return -1;
    }
    if (outfile.Write(pkaHeader.get(), pkaHeaderLength) != pkaHeaderLength) {
        printf("Failed to write temp header to output file.\n");
        return -1;
    }
    for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
        for (size_t j = 0; j < pkgPackFiles[i].Files.size(); ++j) {
            auto it = filesByHash.find(pkgPackFiles[i].Files[j].Hash);
            if (it == filesByHash.end()) {
                printf("Internal error: Failed to find file hash.\n");
                return -1;
            }
            FileReference& ref = it->second;
            if (ref.ShouldBeWritten && !ref.AlreadyWritten) {
                assert(fileOffset == outfile.GetPosition());
                PkgPackArchive& archive = pkgPackFiles[ref.ArchiveIndex];
                const PkgPackFile& file = archive.Files[ref.FileIndex];

                if (recompressFlags) {
                    auto& infile = archive.FileHandle;
                    auto data = std::make_unique_for_overwrite<char[]>(file.CompressedSize);
                    if (!data) {
                        printf("Failed to allocate memory.\n");
                        return -1;
                    }
                    if (!infile.SetPosition(file.OffsetInPkg)) {
                        printf("Failed to seek in pkg.\n");
                        return -1;
                    }
                    if (infile.Read(data.get(), file.CompressedSize) != file.CompressedSize) {
                        printf("Failed to read pkg.\n");
                        return -1;
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
                    if (!SenLib::CompressPkgFile(
                            recompressedDataBuffer,
                            pkgFile,
                            dataBuffer.get(),
                            file.UncompressedSize,
                            *recompressFlags,
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                        printf("Failed recompressing file.\n");
                        return -1;
                    }

                    if (outfile.Write(recompressedDataBuffer.get(), pkgFile.CompressedSize)
                        != pkgFile.CompressedSize) {
                        printf("Failed to write data to output file.\n");
                        return -1;
                    }

                    char* ptr = ref.PkaHeaderPtr;
                    WriteAdvUInt64(ptr, ToEndian(fileOffset, LittleEndian));
                    WriteAdvUInt32(ptr, ToEndian(pkgFile.CompressedSize, LittleEndian));
                    WriteAdvUInt32(ptr, ToEndian(file.UncompressedSize, LittleEndian));
                    WriteAdvUInt32(ptr, ToEndian(pkgFile.Flags, LittleEndian));

                    fileOffset += pkgFile.CompressedSize;
                } else {
                    auto& infile = archive.FileHandle;
                    auto data = std::make_unique_for_overwrite<char[]>(file.CompressedSize);
                    if (!data) {
                        printf("Failed to allocate memory.\n");
                        return -1;
                    }
                    if (!infile.SetPosition(file.OffsetInPkg)) {
                        printf("Failed to seek in pkg.\n");
                        return -1;
                    }
                    if (infile.Read(data.get(), file.CompressedSize) != file.CompressedSize) {
                        printf("Failed to read pkg.\n");
                        return -1;
                    }
                    if (outfile.Write(data.get(), file.CompressedSize) != file.CompressedSize) {
                        printf("Failed to write data to output file.\n");
                        return -1;
                    }

                    char* ptr = ref.PkaHeaderPtr;
                    WriteAdvUInt64(ptr, ToEndian(fileOffset, LittleEndian));
                    WriteAdvUInt32(ptr, ToEndian(file.CompressedSize, LittleEndian));
                    WriteAdvUInt32(ptr, ToEndian(file.UncompressedSize, LittleEndian));
                    WriteAdvUInt32(ptr, ToEndian(file.Flags, LittleEndian));

                    fileOffset += file.CompressedSize;
                }

                ref.AlreadyWritten = true;
            }
        }
    }
    if (!outfile.SetPosition(0)) {
        printf("Failed to seek in output file.\n");
        return -1;
    }
    if (outfile.Write(pkaHeader.get(), pkaHeaderLength) != pkaHeaderLength) {
        printf("Failed to write corrected header to output file.\n");
        return -1;
    }

    return 0;
}
} // namespace SenTools
