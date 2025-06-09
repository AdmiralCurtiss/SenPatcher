#include "p3a_pack_main.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <string_view>

#include "p3a/packfs.h"
#include "p3a/structs.h"
#include "util/args.h"
#include "util/file.h"
#include "util/text.h"

namespace SenTools {
int P3A_Pack_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{.Type = HyoutaUtils::ArgTypes::String,
                                                 .ShortKey = "o",
                                                 .LongKey = "output",
                                                 .Argument = "FILENAME",
                                                 .Description =
                                                     "The output filename. Must be given."};
    static constexpr HyoutaUtils::Arg arg_compression{
        .Type = HyoutaUtils::ArgTypes::String,
        .ShortKey = "c",
        .LongKey = "compression",
        .Argument = "TYPE",
        .Description =
            "Which compression to use for the files packed into the archive. "
            "Options are: 'auto', 'none, 'lz4', 'zstd'."};
    static constexpr HyoutaUtils::Arg arg_threads{
        .Type = HyoutaUtils::ArgTypes::UInt64,
        .ShortKey = "t",
        .LongKey = "threads",
        .Argument = "THREADCOUNT",
        .Description =
            "Use THREADCOUNT threads for compression. Use 0 (default) for automatic detection."};
    static constexpr HyoutaUtils::Arg arg_archive_version{
        .Type = HyoutaUtils::ArgTypes::UInt64,
        .LongKey = "archive-version",
        .Argument = "VERSION",
        .Description =
            "The version of the archive to pack. Defaults to the newest supported version.\n"
            "1100 and 1200 are supported."};
    static constexpr HyoutaUtils::Arg arg_allow_uppercase{
        .Type = HyoutaUtils::ArgTypes::Flag,
        .LongKey = "allow-uppercase",
        .Description =
            "P3A typically wants all-lowercase in filenames. This can be disabled with this "
            "option. SenPatcher handles files with uppercase filenames correctly, but the official "
            "implementation may not."};
    static constexpr HyoutaUtils::Arg arg_no_deduplicate{.Type = HyoutaUtils::ArgTypes::Flag,
                                                         .LongKey = "no-deduplicate",
                                                         .Description = "Skip file deduplication."};
    static constexpr HyoutaUtils::Arg arg_alignment{
        .Type = HyoutaUtils::ArgTypes::UInt64,
        .LongKey = "alignment",
        .Argument = "ALIGNMENT",
        .Description = "Set the data alignment for the individual files packed into the archive."};
    static constexpr std::array<const HyoutaUtils::Arg*, 7> args_array{{&arg_output,
                                                                        &arg_compression,
                                                                        &arg_threads,
                                                                        &arg_archive_version,
                                                                        &arg_allow_uppercase,
                                                                        &arg_no_deduplicate,
                                                                        &arg_alignment}};
    static constexpr HyoutaUtils::Args args(
        "sentools " P3A_Pack_Name, "directory", P3A_Pack_ShortDescription, args_array);
    auto parseResult = args.Parse(argc, argv);
    if (parseResult.IsError()) {
        printf("Argument error: %s\n\n\n", parseResult.GetErrorValue().c_str());
        args.PrintUsage();
        return -1;
    }

    const auto& options = parseResult.GetSuccessValue();
    if (options.FreeArguments.size() != 1) {
        printf("Argument error: %s\n\n\n",
               options.FreeArguments.size() == 0 ? "No input directory given."
                                                 : "More than 1 input directory given.");
        args.PrintUsage();
        return -1;
    }

    auto* output_option = options.TryGetString(&arg_output);
    if (output_option == nullptr) {
        printf("Argument error: %s\n\n\n", "No output filename given.");
        args.PrintUsage();
        return -1;
    }

    std::string_view source(options.FreeArguments[0]);
    std::string_view target(*output_option);

    std::optional<SenPatcher::P3ACompressionType> compressionType = std::nullopt;
    if (auto* compression_option = options.TryGetString(&arg_compression)) {
        const auto& compressionString = *compression_option;
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("auto", compressionString)) {
            compressionType = std::nullopt;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("none", compressionString)) {
            compressionType = SenPatcher::P3ACompressionType::None;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("lz4", compressionString)) {
            compressionType = SenPatcher::P3ACompressionType::LZ4;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("zstd", compressionString)) {
            compressionType = SenPatcher::P3ACompressionType::ZSTD;
        } else {
            printf("Argument error: %s\n\n\n", "Invalid compression type.");
            args.PrintUsage();
            return -1;
        }
    }

    auto* threads_option = options.TryGetUInt64(&arg_threads);
    size_t threadCount = 0;
    if (threads_option != nullptr) {
        if (*threads_option <= std::numeric_limits<size_t>::max()) {
            threadCount = static_cast<size_t>(*threads_option);
        }
    }

    auto* archiveVersion_option = options.TryGetUInt64(&arg_archive_version);
    uint32_t archiveVersion = SenPatcher::P3AHighestSupportedVersion;
    if (archiveVersion_option != nullptr) {
        if (*archiveVersion_option > std::numeric_limits<uint32_t>::max()) {
            printf("Argument error: %s\n\n\n", "Invalid archive version.");
            args.PrintUsage();
            return -1;
        }
        auto it = std::find(SenPatcher::P3ASupportedVersions.begin(),
                            SenPatcher::P3ASupportedVersions.end(),
                            static_cast<uint32_t>(*archiveVersion_option));
        if (it != SenPatcher::P3ASupportedVersions.end()) {
            archiveVersion = *it;
        } else {
            printf("Argument error: %s\n\n\n", "Invalid archive version.");
            args.PrintUsage();
            return -1;
        }
    }

    bool allowUppercaseInFilenames = options.IsFlagSet(&arg_allow_uppercase);
    bool noDeduplicate = options.IsFlagSet(&arg_no_deduplicate);

    size_t alignment = 0x40;
    auto* alignment_option = options.TryGetUInt64(&arg_alignment);
    if (alignment_option != nullptr) {
        if (*alignment_option <= std::numeric_limits<size_t>::max()) {
            alignment = static_cast<size_t>(*alignment_option);
        }
    }

    std::optional<SenPatcher::P3APackData> packData =
        SenPatcher::P3APackDataFromDirectory(HyoutaUtils::IO::FilesystemPathFromUtf8(source),
                                             archiveVersion,
                                             compressionType,
                                             std::filesystem::path(),
                                             allowUppercaseInFilenames);
    if (!packData) {
        printf("Failed to collect input files.\n");
        return -1;
    }

    packData->SetAlignment(alignment);

    if (!noDeduplicate) {
        if (!SenPatcher::DeduplicateP3APackFiles(*packData)) {
            printf("File deduplication failed.\n");
            return -1;
        }
    }

    std::string tmpTargetFilePath(target);
    tmpTargetFilePath += ".tmp";
    HyoutaUtils::IO::File targetFile(std::string_view(tmpTargetFilePath),
                                     HyoutaUtils::IO::OpenMode::Write);
    if (!SenPatcher::PackP3A(targetFile, *packData, threadCount)) {
        printf("Packing failed.\n");
        targetFile.Delete();
        return -1;
    }
    if (!targetFile.Rename(target)) {
        printf("Renaming temp file failed.\n");
        targetFile.Delete();
        return -1;
    }
    return 0;
}
} // namespace SenTools
