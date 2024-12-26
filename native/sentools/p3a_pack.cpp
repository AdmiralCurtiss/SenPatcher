#include "p3a_pack.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "p3a/packfs.h"
#include "p3a/structs.h"
#include "util/file.h"
#include "util/text.h"

namespace SenTools {
int P3A_Pack_Function(int argc, char** argv) {
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
        .choices({"auto", "none", "lz4", "zstd"})
        .set_default("auto");
    parser.add_option("-t", "--threads")
        .type(optparse::DataType::Int)
        .dest("threads")
        .metavar("THREADCOUNT")
        .set_default(0)
        .help("Use THREADCOUNT threads for compression. Use 0 (default) for automatic detection.");
    parser.add_option("--archive-version")
        .type(optparse::DataType::Int)
        .dest("archive-version")
        .metavar("VERSION")
        .set_default(SenPatcher::P3AHighestSupportedVersion)
        .help(
            "The version of the archive to pack. Defaults to the newest supported version.\n"
            "1100 and 1200 are supported.");
    parser.add_option("--allow-uppercase")
        .action(optparse::ActionType::StoreTrue)
        .dest("allow-uppercase")
        .set_default(false)
        .help(
            "P3A typically wants all-lowercase in filenames. This can be disabled with this "
            "option. SenPatcher handles files with uppercase filenames correctly, but the official "
            "implementation may not.");
    parser.add_option("--no-deduplicate")
        .action(optparse::ActionType::StoreTrue)
        .dest("no-deduplicate")
        .set_default(false)
        .help("Skip file deduplication.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }

    auto* output_option = options.get("output");
    if (output_option == nullptr) {
        parser.error("No output filename given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target(output_option->first_string());

    std::optional<SenPatcher::P3ACompressionType> compressionType = std::nullopt;
    if (auto* compression_option = options.get("compression")) {
        const auto& compressionString = compression_option->first_string();
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("auto", compressionString)) {
            compressionType = std::nullopt;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("none", compressionString)) {
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

    auto* threads_option = options.get("threads");
    size_t threadCount = 0;
    if (threads_option != nullptr) {
        int64_t argThreadCount = threads_option->first_integer();
        if (argThreadCount > 0
            && static_cast<uint64_t>(argThreadCount) <= std::numeric_limits<size_t>::max()) {
            threadCount = static_cast<size_t>(argThreadCount);
        }
    }

    auto* archiveVersion_option = options.get("archive-version");
    uint32_t archiveVersion = SenPatcher::P3AHighestSupportedVersion;
    if (archiveVersion_option != nullptr) {
        int64_t argVersion = archiveVersion_option->first_integer();
        if (argVersion < 0
            || static_cast<uint64_t>(argVersion) > std::numeric_limits<uint32_t>::max()) {
            parser.error("Invalid archive version.");
            return -1;
        }
        auto it = std::find(SenPatcher::P3ASupportedVersions.begin(),
                            SenPatcher::P3ASupportedVersions.end(),
                            static_cast<uint32_t>(argVersion));
        if (it != SenPatcher::P3ASupportedVersions.end()) {
            archiveVersion = *it;
        } else {
            parser.error("Invalid archive version.");
            return -1;
        }
    }

    bool allowUppercaseInFilenames = false;
    auto* allowUppercase_option = options.get("allow-uppercase");
    if (allowUppercase_option != nullptr) {
        allowUppercaseInFilenames = allowUppercase_option->flag();
    }

    bool noDeduplicate = false;
    auto* noDeduplicate_option = options.get("no-deduplicate");
    if (noDeduplicate_option != nullptr) {
        noDeduplicate = noDeduplicate_option->flag();
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
