#include "p3a_repack_main.h"

#include <array>
#include <cstdio>
#include <limits>
#include <string_view>

#include "p3a/packjson.h"
#include "util/args.h"
#include "util/file.h"

namespace SenTools {
int P3A_Repack_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{.Type = HyoutaUtils::ArgTypes::String,
                                                 .ShortKey = "o",
                                                 .LongKey = "output",
                                                 .Argument = "FILENAME",
                                                 .Description =
                                                     "The output filename. Must be given."};
    static constexpr HyoutaUtils::Arg arg_threads{
        .Type = HyoutaUtils::ArgTypes::UInt64,
        .ShortKey = "t",
        .LongKey = "threads",
        .Argument = "THREADCOUNT",
        .Description =
            "Use THREADCOUNT threads for compression. Use 0 (default) for automatic detection."};
    static constexpr HyoutaUtils::Arg arg_no_deduplicate{.Type = HyoutaUtils::ArgTypes::Flag,
                                                         .LongKey = "no-deduplicate",
                                                         .Description = "Skip file deduplication."};
    static constexpr std::array<const HyoutaUtils::Arg*, 3> args_array{
        {&arg_output, &arg_threads, &arg_no_deduplicate}};
    static constexpr HyoutaUtils::Args args(
        "sentools " P3A_Repack_Name,
        "__p3a.json",
        P3A_Repack_ShortDescription
        "\n\n"
        "This re-packages a previously extracted achive and keeps all the metadata "
        "as best as possible. For example, the file order will be preserved, and "
        "the same compression type will be used for each file.\n\n"
        "To use this, extract with the -j option, then point this program at the __p3a.json that "
        "was generated during the archive extraction. You can also modify this file for some "
        "advanced packing features that are not available through the standard directory packing "
        "interface.",
        args_array);
    auto parseResult = args.Parse(argc, argv);
    if (parseResult.IsError()) {
        printf("Argument error: %s\n\n\n", parseResult.GetErrorValue().c_str());
        args.PrintUsage();
        return -1;
    }


    const auto& options = parseResult.GetSuccessValue();
    if (options.FreeArguments.size() != 1) {
        printf("Argument error: %s\n\n\n",
               options.FreeArguments.size() == 0 ? "No input file given."
                                                 : "More than 1 input file given.");
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

    bool noDeduplicate = options.IsFlagSet(&arg_no_deduplicate);

    auto* threads_option = options.TryGetUInt64(&arg_threads);
    size_t threadCount = 0;
    if (threads_option != nullptr) {
        if (*threads_option <= std::numeric_limits<size_t>::max()) {
            threadCount = static_cast<size_t>(*threads_option);
        }
    }

    std::optional<SenPatcher::P3APackData> packData =
        SenPatcher::P3APackDataFromJsonFile(HyoutaUtils::IO::FilesystemPathFromUtf8(source));
    if (!packData) {
        printf("Failed to parse or evaluate json.\n");
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
