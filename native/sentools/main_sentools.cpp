#include <array>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "cpp-optparse/OptionParser.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "p3a/pack.h"
#include "p3a/packfs.h"
#include "p3a/packjson.h"
#include "p3a/structs.h"
#include "p3a/unpackfs.h"
#include "sen/pka.h"
#include "sen/pka_to_pkg.h"
#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "sen/pkg_extract.h"
#include "util/file.h"
#include "util/hash/sha256.h"
#include "util/memread.h"
#include "util/memwrite.h"
#include "util/stream.h"
#include "util/text.h"

#include "senpatcher_version.h"

using CliToolFunctionT = int (*)(int argc, char** argv);
struct CliTool {
    const char* Name;
    const char* ShortDescription;
    CliToolFunctionT Function;
};

#define P3A_Extract_Name "P3A.Extract"
#define P3A_Extract_ShortDescription "Extract a *.p3a archive to a directory."
static int P3A_Extract_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(P3A_Extract_ShortDescription);

    parser.usage("sentools " P3A_Extract_Name " [options] archive.p3a");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("DIRECTORY")
        .help(
            "The output directory to extract to. Will be derived from input filename if not "
            "given.");
    parser.add_option("-j", "--json")
        .dest("json")
        .action("store_true")
        .help(
            "If set, a __p3a.json will be generated that contains information about the files in "
            "the archive. This file can be used to repack the archive with the P3A.Repack option "
            "while preserving compression types and file order within the archive.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target;
    std::string tmp;
    if (options.is_set("output")) {
        target = std::string_view(options["output"]);
    } else {
        tmp = std::string(source);
        tmp += ".ex";
        target = tmp;
    }


    if (!SenPatcher::UnpackP3A(std::filesystem::path(source.begin(), source.end()),
                               std::filesystem::path(target.begin(), target.end()),
                               options.is_set("json"))) {
        printf("Unpacking failed.\n");
        return -1;
    }
    return 0;
}

#define P3A_Pack_Name "P3A.Pack"
#define P3A_Pack_ShortDescription "Pack a *.p3a archive from the contents of a directory."
static int P3A_Pack_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(P3A_Pack_ShortDescription);

    parser.usage("sentools " P3A_Pack_Name " [options] directory");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output filename. Must be given.");
    parser.add_option("-c", "--compression")
        .dest("compression")
        .metavar("TYPE")
        .help("Which compression to use for the files packed into the archive.")
        .choices({"none", "lz4", "zstd"})
        .set_default("none");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }

    if (!options.is_set("output")) {
        parser.error("No output filename given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target(options["output"]);

    SenPatcher::P3ACompressionType compressionType = SenPatcher::P3ACompressionType::None;
    if (options.is_set("compression")) {
        const auto& compressionString = options["compression"];
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("none", compressionString)) {
            compressionType = SenPatcher::P3ACompressionType::None;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("lz4", compressionString)) {
            compressionType = SenPatcher::P3ACompressionType::LZ4;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("zstd", compressionString)) {
            compressionType = SenPatcher::P3ACompressionType::ZSTD;
        } else {
            parser.error("Invalid compression type.");
            return -1;
        }
    }


    if (!SenPatcher::PackP3AFromDirectory(std::filesystem::path(source.begin(), source.end()),
                                          std::filesystem::path(target.begin(), target.end()),
                                          compressionType)) {
        printf("Packing failed.\n");
        return -1;
    }
    return 0;
}

#define P3A_Repack_Name "P3A.Repack"
#define P3A_Repack_ShortDescription "Repack a previously extracted *.p3a archive."
static int P3A_Repack_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(
        P3A_Repack_ShortDescription
        "\n\n"
        "This re-packages a previously extracted achive and keeps all the metadata "
        "as best as possible. For example, the file order will be preserved, and "
        "the same compression type will be used for each file.\n\n"
        "To use this, extract with the -j option, then point this program at the __p3a.json that "
        "was generated during the archive extraction. You can also modify this file for some "
        "advanced packing features that are not available through the standard directory packing "
        "interface.");

    parser.usage("sentools " P3A_Repack_Name " [options] __p3a.json");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output filename. Must be given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }

    if (!options.is_set("output")) {
        parser.error("No output filename given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target(options["output"]);


    if (!SenPatcher::PackP3AFromJsonFile(std::filesystem::path(source.begin(), source.end()),
                                         std::filesystem::path(target.begin(), target.end()))) {
        printf("Packing failed.\n");
        return -1;
    }
    return 0;
}

#define PKG_Extract_Name "PKG.Extract"
#define PKG_Extract_ShortDescription "Extract a *.pkg archive to a directory."
static int PKG_Extract_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(PKG_Extract_ShortDescription);

    parser.usage("sentools " PKG_Extract_Name " [options] archive.pkg");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("DIRECTORY")
        .help(
            "The output directory to extract to. Will be derived from input filename if not "
            "given.");
    parser.add_option("-j", "--json")
        .dest("json")
        .action("store_true")
        .help(
            "If set, a __pkg.json will be generated that contains information about the files in "
            "the archive. This file can be used to repack the archive with the PKG.Repack option "
            "while preserving compression types and file order within the archive.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

    const bool generateJson = options.is_set("json");
    std::string_view source(args[0]);
    std::string_view target;
    std::string tmp;
    if (options.is_set("output")) {
        target = std::string_view(options["output"]);
    } else {
        tmp = std::string(source);
        tmp += ".ex";
        target = tmp;
    }


    std::filesystem::path sourcepath(source.begin(), source.end());
    std::filesystem::path targetpath(target.begin(), target.end());
    HyoutaUtils::IO::File infile(sourcepath, HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        printf("Failed to open input file.\n");
        return -1;
    }
    auto filesize = infile.GetLength();
    if (!filesize) {
        printf("Failed to get size of input file.\n");
        return -1;
    }

    auto pkgMemory = std::make_unique_for_overwrite<char[]>(*filesize);
    if (!pkgMemory) {
        printf("Failed to allocate memory.\n");
        return -1;
    }
    if (infile.Read(pkgMemory.get(), *filesize) != *filesize) {
        printf("Failed read input file.\n");
        return -1;
    }

    SenLib::PkgHeader pkg;
    if (!SenLib::ReadPkgFromMemory(
            pkg, pkgMemory.get(), *filesize, HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        printf("Failed to parse pkg header.\n");
        return -1;
    }

    {
        std::error_code ec;
        std::filesystem::create_directories(targetpath, ec);
        if (ec) {
            printf("Failed to create output directoy.\n");
            return -1;
        }
    }

    rapidjson::StringBuffer jsonbuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> json(jsonbuffer);
    json.StartObject();
    json.Key("Unknown");
    json.Uint(pkg.Unknown);

    json.Key("Files");
    json.StartArray();
    for (size_t i = 0; i < pkg.FileCount; ++i) {
        json.StartObject();

        const auto& pkgFile = pkg.Files[i];

        std::unique_ptr<char[]> buffer;
        size_t length;
        if (!SenLib::ExtractAndDecompressPkgFile(
                buffer, length, pkgFile, HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
            printf("Failed to extract file %zu from pkg.\n", i);
            return -1;
        }

        const auto& pkgName = pkgFile.Filename;
        std::u8string_view pkgNameSv(reinterpret_cast<const char8_t*>(pkgName.data()),
                                     pkgName.size());
        const size_t pkgFilenameFirstNull = pkgNameSv.find_first_of(u8'\0');
        const size_t pkgFilenameLength = (pkgFilenameFirstNull == std::u8string_view::npos)
                                             ? pkgName.size()
                                             : pkgFilenameFirstNull;
        HyoutaUtils::IO::File outfile(targetpath / pkgNameSv.substr(0, pkgFilenameLength),
                                     HyoutaUtils::IO::OpenMode::Write);
        if (!outfile.IsOpen()) {
            printf("Failed to open output file.\n");
            return -1;
        }

        if (outfile.Write(buffer.get(), length) != length) {
            printf("Failed to write to output file.\n");
            return -1;
        }

        json.Key("NameInArchive");
        json.String(pkgFile.Filename.data(), pkgFilenameLength);
        json.Key("PathOnDisk");
        json.String(pkgFile.Filename.data(), pkgFilenameLength);
        json.Key("Flags");
        json.Uint(pkgFile.Flags);
        json.EndObject();
    }
    json.EndArray();
    json.EndObject();

    if (generateJson) {
        HyoutaUtils::IO::File f2(targetpath / L"__pkg.json", HyoutaUtils::IO::OpenMode::Write);
        if (!f2.IsOpen()) {
            return false;
        }

        const char* jsonstring = jsonbuffer.GetString();
        const size_t jsonstringsize = jsonbuffer.GetSize();
        if (f2.Write(jsonstring, jsonstringsize) != jsonstringsize) {
            return false;
        }
    }

    return 0;
}

#define PKA_Extract_Name "PKA.Extract"
#define PKA_Extract_ShortDescription "Extract a *.pka archive to a directory."
static int PKA_Extract_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(
        PKA_Extract_ShortDescription
        "\n\n"
        "Note that this will duplicate every file that is stored in more than one pkg into every "
        "single of those pkg files. The extracted files will likely be much bigger than the input "
        "pka, so make sure you have enough disk space available.");

    parser.usage("sentools " PKA_Extract_Name " [options] assets.pka");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("DIRECTORY")
        .help(
            "The output directory to extract to. Will be derived from input filename if not "
            "given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target;
    std::string tmp;
    if (options.is_set("output")) {
        target = std::string_view(options["output"]);
    } else {
        tmp = std::string(source);
        tmp += ".ex";
        target = tmp;
    }


    std::filesystem::path sourcepath(source.begin(), source.end());
    std::filesystem::path targetpath(target.begin(), target.end());
    HyoutaUtils::IO::File infile(sourcepath, HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        printf("Failed to open input file.\n");
        return -1;
    }

    SenLib::PkaHeader pkaHeader;
    if (!SenLib::ReadPkaFromFile(pkaHeader, infile)) {
        printf("Failed to read pka header.\n");
        return -1;
    }

    {
        std::error_code ec;
        std::filesystem::create_directories(targetpath, ec);
        if (ec) {
            printf("Failed to create output directoy.\n");
            return -1;
        }
    }

    for (size_t i = 0; i < pkaHeader.PkgCount; ++i) {
        SenLib::PkgHeader pkg;
        std::unique_ptr<char[]> buffer;
        if (!SenLib::ConvertPkaToSinglePkg(pkg, buffer, pkaHeader, i, infile)) {
            printf("Failed to convert archive %zu to pkg.\n", i);
            return -1;
        }

        std::unique_ptr<char[]> ms;
        size_t msSize;
        if (!SenLib::CreatePkgInMemory(
                ms, msSize, pkg, HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
            printf("Failed to convert archive %zu to pkg.\n", i);
            return -1;
        }

        const auto& pkgName = pkaHeader.Pkgs[i].PkgName;
        std::u8string_view pkgNameSv(reinterpret_cast<const char8_t*>(pkgName.data()),
                                     pkgName.size());
        HyoutaUtils::IO::File outfile(targetpath
                                         / pkgNameSv.substr(0, pkgNameSv.find_first_of(u8'\0')),
                                     HyoutaUtils::IO::OpenMode::Write);
        if (!outfile.IsOpen()) {
            printf("Failed to open output file.\n");
            return -1;
        }

        if (outfile.Write(ms.get(), msSize) != msSize) {
            printf("Failed to write to output file.\n");
            return -1;
        }
    }

    return 0;
}

#define PKA_Pack_Name "PKA.Pack"
#define PKA_Pack_ShortDescription "Pack a *.pka archive from a set of *.pkg files."
static int PKA_Pack_Function(int argc, char** argv) {
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    optparse::OptionParser parser;
    parser.description(PKA_Pack_ShortDescription);

    parser.usage("sentools " PKA_Pack_Name " [options] directory");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output filename. Must be given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }

    if (!options.is_set("output")) {
        parser.error("No output filename given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target(options["output"]);

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
    };
    std::vector<PkgPackArchive> pkgPackFiles;
    {
        std::filesystem::path rootDir(source);
        std::error_code ec;
        std::filesystem::directory_iterator iterator(rootDir, ec);
        if (ec) {
            return false;
        }
        for (auto const& entry : iterator) {
            if (entry.is_directory()) {
                continue;
            }
            const auto ext = entry.path().extension().generic_u8string();
            if (!HyoutaUtils::TextUtils::CaseInsensitiveEquals(
                    ".pkg",
                    std::string_view(reinterpret_cast<const char*>(ext.data()), ext.size()))) {
                continue;
            }

            const auto relativePath = std::filesystem::relative(entry.path(), rootDir, ec);
            if (relativePath.empty()) {
                printf("Error while collecting files.\n");
                return -1;
            }
            const auto filename = relativePath.u8string();
            const char8_t* filenameC = filename.c_str();

            std::array<char, 0x20> fn{};
            for (size_t i = 0; i < fn.size(); ++i) {
                const char c = static_cast<char>(filenameC[i]);
                if (c == '\0') {
                    break;
                }
                fn[i] = c;
            }
            auto& fi = pkgPackFiles.emplace_back(PkgPackArchive{
                .FileHandle = HyoutaUtils::IO::File(entry.path(), HyoutaUtils::IO::OpenMode::Read),
                .PkgName = fn});

            HyoutaUtils::IO::File& infile = fi.FileHandle;
            if (!infile.IsOpen()) {
                printf("Failed opening pkg.\n");
                return -1;
            }
            std::array<char, 8> pkgHeaderInitialBytes;
            if (infile.Read(pkgHeaderInitialBytes.data(), pkgHeaderInitialBytes.size())
                != pkgHeaderInitialBytes.size()) {
                printf("Failed to read pkg.\n");
                return -1;
            }
            const uint32_t fileCountInPkg = HyoutaUtils::EndianUtils::FromEndian(
                HyoutaUtils::MemRead::ReadUInt32(&pkgHeaderInitialBytes[4]), LittleEndian);
            const size_t pkgHeaderLength = 8u + static_cast<size_t>(fileCountInPkg) * 0x50u;
            auto pkgHeaderData = std::make_unique_for_overwrite<char[]>(pkgHeaderLength);
            if (!pkgHeaderData) {
                printf("Failed to allocate memory.\n");
                return -1;
            }
            std::memcpy(
                pkgHeaderData.get(), pkgHeaderInitialBytes.data(), pkgHeaderInitialBytes.size());
            if (infile.Read(pkgHeaderData.get() + pkgHeaderInitialBytes.size(),
                            pkgHeaderLength - pkgHeaderInitialBytes.size())
                != (pkgHeaderLength - pkgHeaderInitialBytes.size())) {
                printf("Failed to read pkg.\n");
                return -1;
            }

            SenLib::PkgHeader pkgHeader;
            if (!SenLib::ReadPkgFromMemory(
                    pkgHeader, pkgHeaderData.get(), pkgHeaderLength, LittleEndian)) {
                printf("Failed to read pkg header.\n");
                return -1;
            }

            fi.Files.reserve(pkgHeader.FileCount);
            for (uint32_t i = 0; i < pkgHeader.FileCount; ++i) {
                auto& f = pkgHeader.Files[i];
                auto compressedData = std::make_unique_for_overwrite<char[]>(f.CompressedSize);
                if (!compressedData) {
                    printf("Failed to allocate memory.\n");
                    return -1;
                }
                if (!infile.SetPosition(f.DataPosition)) {
                    printf("Failed to seek in pkg.\n");
                    return -1;
                }
                if (infile.Read(compressedData.get(), f.CompressedSize) != f.CompressedSize) {
                    printf("Failed to read pkg.\n");
                    return -1;
                }

                std::unique_ptr<char[]> dataBuffer;
                size_t dataLength;
                f.Data = compressedData.get();
                if (!SenLib::ExtractAndDecompressPkgFile(dataBuffer, dataLength, f, LittleEndian)) {
                    printf("Failed to extract file from pkg.\n");
                    return -1;
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
        }
    }

    // in the vanilla games the pkgs are always uppercase except for the extension which is
    // lowercase, so make sure we pack like that too...
    for (PkgPackArchive& archive : pkgPackFiles) {
        const size_t length = [&]() {
            for (size_t i = 0; i < archive.PkgName.size(); ++i) {
                if (archive.PkgName[i] == '\0') {
                    return i;
                }
            }
            return archive.PkgName.size();
        }();
        const size_t extensionSeparator = [&]() {
            for (size_t i = 0; i < length; ++i) {
                if (archive.PkgName[i] == '.') {
                    return i;
                }
            }
            return length;
        }();
        for (size_t i = 0; i < extensionSeparator; ++i) {
            char& c = archive.PkgName[i];
            if (c >= 'a' && c <= 'z') {
                c = c + ('A' - 'a');
            }
        }
        for (size_t i = extensionSeparator; i < length; ++i) {
            char& c = archive.PkgName[i];
            if (c >= 'A' && c <= 'Z') {
                c = c + ('a' - 'A');
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
        const PkgPackArchive& archive = pkgPackFiles[i];
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
            }
        }
    }

    // calculate file offsets
    size_t pkaHeaderLength = (8u + 4u) + (filesByHash.size() * (0x20u + 8u + 4u + 4u + 4u));
    for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
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
            if (ref.OffsetInPka == 0) {
                ref.OffsetInPka = fileOffset;
                fileOffset += pkgPackFiles[ref.ArchiveIndex].Files[ref.FileIndex].CompressedSize;
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
                   ToEndian(static_cast<uint32_t>(pkgPackFiles.size()), LittleEndian));
    for (size_t i = 0; i < pkgPackFiles.size(); ++i) {
        WriteAdvArray(pkaHeaderWritePtr, pkgPackFiles[i].PkgName);
        WriteAdvUInt32(pkaHeaderWritePtr,
                       ToEndian(static_cast<uint32_t>(pkgPackFiles[i].Files.size()), LittleEndian));
        for (size_t j = 0; j < pkgPackFiles[i].Files.size(); ++j) {
            WriteAdvArray(pkaHeaderWritePtr, pkgPackFiles[i].Files[j].Filename);
            WriteAdvArray(pkaHeaderWritePtr, pkgPackFiles[i].Files[j].Hash.Hash);
        }
    }
    WriteAdvUInt32(pkaHeaderWritePtr,
                   ToEndian(static_cast<uint32_t>(filesByHash.size()), LittleEndian));
    for (const auto& kvp : filesByHash) {
        const FileReference& ref = kvp.second;
        const PkgPackFile& file = pkgPackFiles[ref.ArchiveIndex].Files[ref.FileIndex];

        WriteAdvArray(pkaHeaderWritePtr, kvp.first.Hash);
        WriteAdvUInt64(pkaHeaderWritePtr, ToEndian(ref.OffsetInPka, LittleEndian));
        WriteAdvUInt32(pkaHeaderWritePtr, ToEndian(file.CompressedSize, LittleEndian));
        WriteAdvUInt32(pkaHeaderWritePtr, ToEndian(file.UncompressedSize, LittleEndian));
        WriteAdvUInt32(pkaHeaderWritePtr, ToEndian(file.Flags, LittleEndian));
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
            if (!ref.AlreadyWritten) {
                assert(ref.OffsetInPka == outfile.GetPosition());
                PkgPackArchive& archive = pkgPackFiles[ref.ArchiveIndex];
                const PkgPackFile& file = archive.Files[ref.FileIndex];

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

                ref.AlreadyWritten = true;
            }
        }
    }

    return 0;
}

#define PKG_Pack_Name "PKG.Pack"
#define PKG_Pack_ShortDescription "Pack a *.pkg archive from the contents of a directory."
static int PKG_Pack_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(PKG_Pack_ShortDescription);

    parser.usage("sentools " PKG_Pack_Name " [options] directory");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output filename. Must be given.");
    parser.add_option("-c", "--compression")
        .dest("compression")
        .metavar("TYPE")
        .help(
            "Which compression to use for the files packed into the archive. Note that lz4 and "
            "zstd are not supported by all games!")
        .choices({"none", "type1", "lz4", "zstd"})
        .set_default("none");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }

    if (!options.is_set("output")) {
        parser.error("No output filename given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target(options["output"]);

    uint32_t flags = 0;
    if (options.is_set("compression")) {
        const auto& compressionString = options["compression"];
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("none", compressionString)) {
            flags = 0;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("type1", compressionString)) {
            flags = 1;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("lz4", compressionString)) {
            flags = 4;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("zstd", compressionString)) {
            flags = 0x10;
        } else {
            parser.error("Invalid compression type.");
            return -1;
        }
    }


    std::vector<SenLib::PkgFile> fileinfos;
    std::vector<std::unique_ptr<char[]>> filedatas;
    {
        std::filesystem::path rootDir(source);
        std::error_code ec;
        std::filesystem::directory_iterator iterator(rootDir, ec);
        if (ec) {
            return false;
        }
        for (auto const& entry : iterator) {
            if (entry.is_directory()) {
                continue;
            }

            const auto relativePath = std::filesystem::relative(entry.path(), rootDir, ec);
            if (relativePath.empty()) {
                printf("Error while collecting files.\n");
                return -1;
            }
            const auto filename = relativePath.u8string();
            const char8_t* filenameC = filename.c_str();

            std::array<char, 0x40> fn{};
            for (size_t i = 0; i < fn.size(); ++i) {
                const char c = static_cast<char>(filenameC[i]);
                if (c == '\0') {
                    break;
                }
                fn[i] = c;
            }
            auto& fi = fileinfos.emplace_back(SenLib::PkgFile{.Filename = fn});
            auto& fd = filedatas.emplace_back();

            HyoutaUtils::IO::File infile(entry.path(), HyoutaUtils::IO::OpenMode::Read);
            if (!infile.IsOpen()) {
                printf("Failed opening file.\n");
                return -1;
            }
            const auto uncompressedLength = infile.GetLength();
            if (!uncompressedLength) {
                printf("Failed getting size of file.\n");
                return -1;
            }
            if (*uncompressedLength > UINT32_MAX) {
                printf("File too big to put into pkg.\n");
                return -1;
            }
            auto uncompressedData = std::make_unique_for_overwrite<char[]>(*uncompressedLength);
            if (infile.Read(uncompressedData.get(), *uncompressedLength) != *uncompressedLength) {
                printf("Failed to read file.\n");
                return -1;
            }

            if (!SenLib::CompressPkgFile(fd,
                                         fi,
                                         uncompressedData.get(),
                                         static_cast<uint32_t>(*uncompressedLength),
                                         flags,
                                         HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                printf("Failed adding file to pkg.\n");
                return -1;
            }
        }
    }

    std::unique_ptr<char[]> ms;
    size_t msSize;
    if (!SenLib::CreatePkgInMemory(ms,
                                   msSize,
                                   fileinfos.data(),
                                   static_cast<uint32_t>(fileinfos.size()),
                                   0,
                                   HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        printf("Failed to create pkg.\n");
        return -1;
    }

    HyoutaUtils::IO::File outfile(std::filesystem::path(target), HyoutaUtils::IO::OpenMode::Write);
    if (!outfile.IsOpen()) {
        printf("Failed to open output file.\n");
        return -1;
    }

    if (outfile.Write(ms.get(), msSize) != msSize) {
        printf("Failed to write to output file.\n");
        return -1;
    }

    return 0;
}

template<typename T>
static std::optional<uint32_t> JsonReadUInt32(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsUint()) {
        const auto i = j.GetUint();
        return static_cast<uint32_t>(i);
    }
    return std::nullopt;
}

template<typename T>
static std::optional<std::string> JsonReadString(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsString()) {
        const char* str = j.GetString();
        const auto len = j.GetStringLength();
        return std::string(str, len);
    }
    return std::nullopt;
}

#define PKG_Repack_Name "PKG.Repack"
#define PKG_Repack_ShortDescription "Repack a previously extracted *.pkg archive."
static int PKG_Repack_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(
        PKG_Repack_ShortDescription
        "\n\n"
        "This re-packages a previously extracted achive and keeps all the metadata "
        "as best as possible. For example, the file order will be preserved, and "
        "the same compression type will be used for each file.\n\n"
        "To use this, extract with the -j option, then point this program at the __pkg.json that "
        "was generated during the archive extraction. You can also modify this file for some "
        "advanced packing features that are not available through the standard directory packing "
        "interface.");

    parser.usage("sentools " PKG_Repack_Name " [options] __pkg.json");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output filename. Must be given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }

    if (!options.is_set("output")) {
        parser.error("No output filename given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target(options["output"]);


    std::filesystem::path sourcepath(source.begin(), source.end());
    HyoutaUtils::IO::File f(sourcepath, HyoutaUtils::IO::OpenMode::Read);
    if (!f.IsOpen()) {
        return -1;
    }
    auto length = f.GetLength();
    if (!length) {
        return -1;
    }
    auto buffer = std::make_unique_for_overwrite<char[]>(*length);
    if (!buffer) {
        return -1;
    }
    if (f.Read(buffer.get(), *length) != *length) {
        return -1;
    }

    rapidjson::Document json;
    json.Parse<rapidjson::kParseFullPrecisionFlag | rapidjson::kParseNanAndInfFlag,
               rapidjson::UTF8<char>>(buffer.get(), *length);
    if (json.HasParseError() || !json.IsObject()) {
        return -1;
    }

    const auto root = json.GetObject();

    std::vector<SenLib::PkgFile> fileinfos;
    std::vector<std::unique_ptr<char[]>> filedatas;
    const uint32_t unknownValue = JsonReadUInt32(root, "Unknown").value_or(0);

    const auto files = root.FindMember("Files");
    if (files != root.MemberEnd() && files->value.IsArray()) {
        for (const auto& file : files->value.GetArray()) {
            if (file.IsObject()) {
                const auto fileobj = file.GetObject();
                const auto nameInArchive = JsonReadString(file, "NameInArchive");
                const auto pathOnDisk = JsonReadString(file, "PathOnDisk");
                const auto flags = JsonReadUInt32(file, "Flags");
                if (!nameInArchive || !pathOnDisk || !flags) {
                    return -1;
                }
                std::array<char, 0x40> fn{};
                if (nameInArchive->size() > fn.size()) {
                    return -1;
                }
                std::memcpy(fn.data(), nameInArchive->data(), nameInArchive->size());

                auto& fi = fileinfos.emplace_back(SenLib::PkgFile{.Filename = fn});
                auto& fd = filedatas.emplace_back();

                HyoutaUtils::IO::File infile(
                    sourcepath.parent_path().append(std::u8string_view(
                        (const char8_t*)pathOnDisk->data(),
                        ((const char8_t*)pathOnDisk->data()) + pathOnDisk->size())),
                    HyoutaUtils::IO::OpenMode::Read);
                if (!infile.IsOpen()) {
                    printf("Failed opening file.\n");
                    return -1;
                }
                const auto uncompressedLength = infile.GetLength();
                if (!uncompressedLength) {
                    printf("Failed getting size of file.\n");
                    return -1;
                }
                if (*uncompressedLength > UINT32_MAX) {
                    printf("File too big to put into pkg.\n");
                    return -1;
                }
                auto uncompressedData = std::make_unique_for_overwrite<char[]>(*uncompressedLength);
                if (infile.Read(uncompressedData.get(), *uncompressedLength)
                    != *uncompressedLength) {
                    printf("Failed to read file.\n");
                    return -1;
                }

                if (!SenLib::CompressPkgFile(fd,
                                             fi,
                                             uncompressedData.get(),
                                             static_cast<uint32_t>(*uncompressedLength),
                                             *flags,
                                             HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                    printf("Failed adding file to pkg.\n");
                    return -1;
                }
            } else {
                return -1;
            }
        }
    } else {
        return -1;
    }

    std::unique_ptr<char[]> ms;
    size_t msSize;
    if (!SenLib::CreatePkgInMemory(ms,
                                   msSize,
                                   fileinfos.data(),
                                   static_cast<uint32_t>(fileinfos.size()),
                                   unknownValue,
                                   HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        printf("Failed to create pkg.\n");
        return -1;
    }

    HyoutaUtils::IO::File outfile(std::filesystem::path(target), HyoutaUtils::IO::OpenMode::Write);
    if (!outfile.IsOpen()) {
        printf("Failed to open output file.\n");
        return -1;
    }

    if (outfile.Write(ms.get(), msSize) != msSize) {
        printf("Failed to write to output file.\n");
        return -1;
    }

    return 0;
}

static constexpr auto CliTools = {
    CliTool{.Name = P3A_Extract_Name,
            .ShortDescription = P3A_Extract_ShortDescription,
            .Function = P3A_Extract_Function},
    CliTool{.Name = P3A_Pack_Name,
            .ShortDescription = P3A_Pack_ShortDescription,
            .Function = P3A_Pack_Function},
    CliTool{.Name = P3A_Repack_Name,
            .ShortDescription = P3A_Repack_ShortDescription,
            .Function = P3A_Repack_Function},
    CliTool{.Name = PKG_Extract_Name,
            .ShortDescription = PKG_Extract_ShortDescription,
            .Function = PKG_Extract_Function},
    CliTool{.Name = PKG_Pack_Name,
            .ShortDescription = PKG_Pack_ShortDescription,
            .Function = PKG_Pack_Function},
    CliTool{.Name = PKG_Repack_Name,
            .ShortDescription = PKG_Repack_ShortDescription,
            .Function = PKG_Repack_Function},
    CliTool{.Name = PKA_Extract_Name,
            .ShortDescription = PKA_Extract_ShortDescription,
            .Function = PKA_Extract_Function},
    CliTool{.Name = PKA_Pack_Name,
            .ShortDescription = PKA_Pack_ShortDescription,
            .Function = PKA_Pack_Function},
};

static void PrintUsage() {
    printf("SenTools from SenPatcher " SENPATCHER_VERSION "\n");
    printf("Select one of the following tools via the first argument:\n");
    for (const auto& tool : CliTools) {
        printf(" %-15s %s\n", tool.Name, tool.ShortDescription);
    }
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            PrintUsage();
            return -1;
        }

        const std::string_view name = argv[1];
        for (const auto& tool : CliTools) {
            if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(name, tool.Name)) {
                return tool.Function(argc - 1, argv + 1);
            }
        }

        PrintUsage();
        return -1;
    } catch (const std::exception& ex) {
        printf("Exception occurred: %s\n", ex.what());
        return -3;
    } catch (...) {
        printf("Unknown error occurred.\n");
        return -4;
    }
}
