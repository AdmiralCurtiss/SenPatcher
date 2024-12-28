#include "p3a_extract.h"

#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "p3a/unpackfs.h"
#include "util/file.h"

namespace SenTools {
int P3A_Extract_Function(int argc, char** argv) {
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
        .action(optparse::ActionType::StoreTrue)
        .help(
            "If set, a __p3a.json will be generated that contains information about the files in "
            "the archive. This file can be used to repack the archive with the P3A.Repack option "
            "while preserving compression types and file order within the archive.");
    parser.add_option("--filter")
        .dest("filter")
        .metavar("FILTER")
        .help(
            "Glob filter for files to extract, matched against the relative path of the file in "
            "the archive. Case-insensitive. All files will be extracted if this is not given.");
    parser.add_option("--no-decompress")
        .dest("no-decompress")
        .action(optparse::ActionType::StoreTrue)
        .help("If set, files will be unpacked but not decompressed.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target;
    std::string tmp;
    if (auto* output_option = options.get("output")) {
        target = std::string_view(output_option->first_string());
    } else {
        tmp = std::string(source);
        tmp += ".ex";
        target = tmp;
    }

    std::string_view pathFilter;
    if (auto* filter_option = options.get("filter")) {
        pathFilter = std::string_view(filter_option->first_string());
    } else {
        pathFilter = "*";
    }

    if (!SenPatcher::UnpackP3A(HyoutaUtils::IO::FilesystemPathFromUtf8(source),
                               HyoutaUtils::IO::FilesystemPathFromUtf8(target),
                               pathFilter,
                               options["json"].flag(),
                               options["no-decompress"].flag())) {
        printf("Unpacking failed.\n");
        return -1;
    }
    return 0;
}
} // namespace SenTools
