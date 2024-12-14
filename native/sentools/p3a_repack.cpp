#include "p3a_repack.h"

#include <cstdio>
#include <filesystem>
#include <limits>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "p3a/packjson.h"
#include "util/file.h"

namespace SenTools {
int P3A_Repack_Function(int argc, char** argv) {
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
    parser.add_option("-t", "--threads")
        .type(optparse::DataType::Int)
        .dest("threads")
        .metavar("THREADCOUNT")
        .set_default(0)
        .help("Use THREADCOUNT threads for compression. Use 0 (default) for automatic detection.");

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

    auto* threads_option = options.get("threads");
    size_t threadCount = 0;
    if (threads_option != nullptr) {
        int64_t argThreadCount = threads_option->first_integer();
        if (argThreadCount > 0
            && static_cast<uint64_t>(argThreadCount) <= std::numeric_limits<size_t>::max()) {
            threadCount = static_cast<size_t>(argThreadCount);
        }
    }

    if (!SenPatcher::PackP3AFromJsonFile(HyoutaUtils::IO::FilesystemPathFromUtf8(source),
                                         HyoutaUtils::IO::FilesystemPathFromUtf8(target),
                                         threadCount)) {
        printf("Packing failed.\n");
        return -1;
    }
    return 0;
}
} // namespace SenTools
