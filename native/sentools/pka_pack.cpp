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
            for (size_t i = 0; i < fn.size() - 1; ++i) {
                const char c = static_cast<char>(filenameC[i]);
                if (c == '\0') {
                    break;
                }
                fn[i] = c;
            }
            NormalizePkgName(fn);

            auto existingArchive =
                std::find_if(pkgPackFiles.begin(),
                             pkgPackFiles.end(),
                             [&](const PkgPackArchive& a) { return a.PkgName == fn; });
            const bool archiveExistsAlready = (existingArchive != pkgPackFiles.end());
            if (archiveExistsAlready) {
                printf(
                    "WARNING: %s exists multiple times, only the first parsed archive will be "
                    "available in the PKA (but the contents of all instances will be packed).\n",
                    fn.data());
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
                auto& f = pkgHeader.Files[i];
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

                std::unique_ptr<char[]> dataBuffer;
                size_t dataLength;
                f.Data = compressedData.get();
                if (!SenLib::ExtractAndDecompressPkgFile(dataBuffer, dataLength, f, LittleEndian)) {
                    printf("Failed to extract file from pkg.\n");
                    return false;
                }
                fi.Files.emplace_back(PkgPackFile{
                    .Filename = f.Filename,
                    .Hash = HyoutaUtils::Hash::CalculateSHA256(dataBuffer.get(), dataLength),
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
        uint64_t OffsetInPka = 0;
        bool AlreadyWritten = false;
        bool ShouldBeWritten = true;
        std::unique_ptr<char[]> RecompressedData;
        uint32_t RecompressedSize;
        uint32_t RecompressedFlags;
    };
    const auto get_compressed_size = [&](const FileReference& ref) -> uint32_t {
        if (ref.RecompressedData) {
            return ref.RecompressedSize;
        } else {
            return pkgPackFiles[ref.ArchiveIndex].Files[ref.FileIndex].CompressedSize;
        }
    };
    const auto get_flags = [&](const FileReference& ref) -> uint32_t {
        if (ref.RecompressedData) {
            return ref.RecompressedFlags;
        } else {
            return pkgPackFiles[ref.ArchiveIndex].Files[ref.FileIndex].Flags;
        }
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
                if (!existingFileReference.RecompressedData
                    && file.CompressedSize < existingFile.CompressedSize) {
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
                if (recompressFlags && fileReference.ShouldBeWritten) {
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

                    std::unique_ptr<char[]> dataBuffer;
                    size_t dataLength;
                    SenLib::PkgFile f;
                    f.Filename = file.Filename;
                    f.UncompressedSize = file.UncompressedSize;
                    f.CompressedSize = file.CompressedSize;
                    f.DataPosition = file.OffsetInPkg;
                    f.Flags = file.Flags;
                    f.Data = data.get();
                    if (!SenLib::ExtractAndDecompressPkgFile(
                            dataBuffer, dataLength, f, LittleEndian)) {
                        printf("Failed to extract file from pkg.\n");
                        return false;
                    }

                    std::unique_ptr<char[]> recompressedDataBuffer;
                    SenLib::PkgFile pkgFile;
                    if (!SenLib::CompressPkgFile(
                            recompressedDataBuffer,
                            pkgFile,
                            dataBuffer.get(),
                            static_cast<uint32_t>(dataLength),
                            *recompressFlags,
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                        printf("Failed recompressing file.\n");
                        return -1;
                    }
                    fileReference.RecompressedData = std::move(recompressedDataBuffer);
                    fileReference.RecompressedSize = pkgFile.CompressedSize;
                    fileReference.RecompressedFlags = pkgFile.Flags;
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
    uint64_t fileOffset = pkaHeaderLength;
    for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
        for (size_t j = 0; j < pkgPackFiles[i].Files.size(); ++j) {
            auto it = filesByHash.find(pkgPackFiles[i].Files[j].Hash);
            if (it == filesByHash.end()) {
                printf("Internal error: Failed to find file hash.\n");
                return -1;
            }
            FileReference& ref = it->second;
            if (ref.ShouldBeWritten && ref.OffsetInPka == 0) {
                ref.OffsetInPka = fileOffset;
                fileOffset += get_compressed_size(ref);
            }
        }
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
    for (const auto& kvp : filesByHash) {
        const FileReference& ref = kvp.second;
        const PkgPackFile& file = pkgPackFiles[ref.ArchiveIndex].Files[ref.FileIndex];

        if (kvp.second.ShouldBeWritten) {
            WriteAdvArray(pkaHeaderWritePtr, kvp.first.Hash);
            WriteAdvUInt64(pkaHeaderWritePtr, ToEndian(ref.OffsetInPka, LittleEndian));
            WriteAdvUInt32(pkaHeaderWritePtr, ToEndian(get_compressed_size(ref), LittleEndian));
            WriteAdvUInt32(pkaHeaderWritePtr, ToEndian(file.UncompressedSize, LittleEndian));
            WriteAdvUInt32(pkaHeaderWritePtr, ToEndian(get_flags(ref), LittleEndian));
        }
    }
    assert(pkaHeaderWritePtr == (pkaHeader.get() + pkaHeaderLength));

    // write file
    HyoutaUtils::IO::File outfile(std::filesystem::path(target), HyoutaUtils::IO::OpenMode::Write);
    if (!outfile.IsOpen()) {
        printf("Failed to open output file.\n");
        return -1;
    }
    if (outfile.Write(pkaHeader.get(), pkaHeaderLength) != pkaHeaderLength) {
        printf("Failed to write header to output file.\n");
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
                assert(ref.OffsetInPka == outfile.GetPosition());
                PkgPackArchive& archive = pkgPackFiles[ref.ArchiveIndex];
                const PkgPackFile& file = archive.Files[ref.FileIndex];

                if (ref.RecompressedData) {
                    if (outfile.Write(ref.RecompressedData.get(), ref.RecompressedSize)
                        != ref.RecompressedSize) {
                        printf("Failed to write data to output file.\n");
                        return -1;
                    }
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
                }

                ref.AlreadyWritten = true;
            }
        }
    }

    return 0;
}
} // namespace SenTools
