#include "p3a_pack.h"

#include <cstdio>
#include <filesystem>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "p3a/packfs.h"
#include "p3a/structs.h"
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
        .choices({"none", "lz4", "zstd"})
        .set_default("none");

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

    SenPatcher::P3ACompressionType compressionType = SenPatcher::P3ACompressionType::None;
    if (auto* compression_option = options.get("compression")) {
        const auto& compressionString = compression_option->first_string();
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
} // namespace SenTools
