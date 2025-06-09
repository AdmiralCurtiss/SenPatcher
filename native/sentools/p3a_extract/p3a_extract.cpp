#include "p3a_extract_main.h"

#include <array>
#include <cstdio>
#include <string>
#include <string_view>

#include "p3a/unpackfs.h"
#include "util/args.h"

namespace SenTools {
int P3A_Extract_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{
        .Type = HyoutaUtils::ArgTypes::String,
        .ShortKey = "o",
        .LongKey = "output",
        .Argument = "DIRECTORY",
        .Description =
            "The output directory to extract to. Will be derived from "
            "input filename if not given."};
    static constexpr HyoutaUtils::Arg arg_json{
        .Type = HyoutaUtils::ArgTypes::Flag,
        .ShortKey = "j",
        .LongKey = "json",
        .Description =
            "If set, a __p3a.json will be generated that contains information about "
            "the files in the archive. This file can be used to repack the archive "
            "with the P3A.Repack option while preserving compression types and file "
            "order within the archive."};
    static constexpr HyoutaUtils::Arg arg_filter{
        .Type = HyoutaUtils::ArgTypes::String,
        .LongKey = "filter",
        .Argument = "FILTER",
        .Description =
            "Glob filter for files to extract, matched against the relative path of the file "
            "in the archive. Case-insensitive. All files will be extracted if this is not given."};
    static constexpr HyoutaUtils::Arg arg_no_decompress{
        .Type = HyoutaUtils::ArgTypes::Flag,
        .LongKey = "no-decompress",
        .Description = "If set, files will be unpacked but not decompressed."};
    static constexpr std::array<const HyoutaUtils::Arg*, 4> args_array{
        {&arg_output, &arg_json, &arg_filter, &arg_no_decompress}};
    static constexpr HyoutaUtils::Args args(
        "sentools " P3A_Extract_Name, "archive.p3a", P3A_Extract_ShortDescription, args_array);
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

    std::string_view source(options.FreeArguments[0]);
    std::string_view target;
    std::string tmp;
    if (const auto* output_option = options.TryGetString(&arg_output)) {
        target = *output_option;
    } else {
        tmp = std::string(source);
        tmp += ".ex";
        target = tmp;
    }

    std::string_view pathFilter;
    if (auto* filter_option = options.TryGetString(&arg_filter)) {
        pathFilter = *filter_option;
    } else {
        pathFilter = "*";
    }

    const bool generateJson = options.IsFlagSet(&arg_json);
    const bool noDecompress = options.IsFlagSet(&arg_no_decompress);

    auto result = SenPatcher::UnpackP3A(source, target, pathFilter, generateJson, noDecompress);
    if (result.IsError()) {
        printf("Unpacking failed: %s\n", result.GetErrorValue().c_str());
        return -1;
    }
    return 0;
}
} // namespace SenTools
