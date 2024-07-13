#include "p3a_repack.h"

#include <cstdio>
#include <filesystem>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "p3a/packjson.h"

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


    if (!SenPatcher::PackP3AFromJsonFile(std::filesystem::path(source.begin(), source.end()),
                                         std::filesystem::path(target.begin(), target.end()))) {
        printf("Packing failed.\n");
        return -1;
    }
    return 0;
}
} // namespace SenTools
