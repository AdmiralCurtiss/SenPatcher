#include "pka_extract.h"
#include "pka_extract_main.h"

#include <array>
#include <cstdio>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <string_view>

#include "sen/pka.h"
#include "sen/pka_to_pkg.h"
#include "sen/pkg.h"
#include "sentools/task_cancellation.h"
#include "sentools/task_reporting.h"
#include "sentools/task_reporting_dummy.h"
#include "util/args.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/memread.h"
#include "util/text.h"

namespace SenTools {
int PKA_Extract_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{
        .Type = HyoutaUtils::ArgTypes::String,
        .ShortKey = "o",
        .LongKey = "output",
        .Argument = "DIRECTORY",
        .Description =
            "The output directory to extract to. Will be derived from input filename if not "
            "given."};
    static constexpr HyoutaUtils::Arg arg_ref_pka{
        .Type = HyoutaUtils::ArgTypes::StringArray,
        .LongKey = "referenced-pka",
        .Argument = "PKA",
        .Description =
            "Referenced pka file that could also contain files, see the corresponding option in "
            "PKA.Pack for details. Option can be provided multiple times. This is a nonstandard "
            "feature that the vanilla game does not handle."};
    static constexpr HyoutaUtils::Arg arg_as_pka_ref{
        .Type = HyoutaUtils::ArgTypes::Flag,
        .LongKey = "as-pka-reference",
        .Description =
            "Generate pkg files that only contain the file hashes, not the actual file data. The "
            "pka file(s) will be needed to actually extract the data later."};
    static constexpr std::array<const HyoutaUtils::Arg*, 3> args_array{
        {&arg_output, &arg_ref_pka, &arg_as_pka_ref}};
    static constexpr HyoutaUtils::Args args(
        "sentools " PKA_Extract_Name,
        "assets.pka",
        PKA_Extract_ShortDescription
        "\n\n"
        "Note that this will duplicate every file that is stored in more than one pkg into every "
        "single of those pkg files. The extracted files will likely be much bigger than the input "
        "pka, so make sure you have enough disk space available.",
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

    std::string_view source(options.FreeArguments[0]);
    std::string_view target;
    std::string tmp;
    if (auto* output_option = options.TryGetString(&arg_output)) {
        target = std::string_view(*output_option);
    } else {
        tmp = std::string(source);
        tmp += ".ex";
        target = tmp;
    }

    std::span<const std::string_view> referencedPkaPaths;
    if (auto* referenced_pka_option = options.TryGetStringArray(&arg_ref_pka)) {
        referencedPkaPaths = *referenced_pka_option;
    }

    const bool asPkaRef = options.IsFlagSet(&arg_as_pka_ref);

    SenTools::TaskCancellation taskCancellation;
    SenTools::TaskReportingDummy taskReporting;
    auto result =
        ExtractPka(&taskCancellation, &taskReporting, source, target, referencedPkaPaths, asPkaRef);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}

HyoutaUtils::Result<ExtractPkaResult, std::string>
    ExtractPka(SenTools::TaskCancellation* taskCancellation,
               SenTools::TaskReporting* taskReporting,
               std::string_view source,
               std::string_view target,
               std::span<const std::string_view> referencedPkaPaths,
               bool extractAsPkaReferenceStub) {
    HyoutaUtils::IO::File infile(source, HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        return std::format("Failed to open input file '{}'.", source);
    }

    SenLib::PkaHeader pkaHeader;
    if (!SenLib::ReadPkaFromFile(pkaHeader, infile)) {
        return std::format("Failed to read pka header from '{}'.", source);
    }

    std::vector<SenLib::ReferencedPka> referencedPkas;
    {
        referencedPkas.reserve(referencedPkaPaths.size());
        for (const std::string_view& referencedPkaPath : referencedPkaPaths) {
            if (taskCancellation->IsCancellationRequested()) {
                return ExtractPkaResult::Cancelled;
            }

            auto& refPka = referencedPkas.emplace_back();
            refPka.PkaFile.Open(std::string_view(referencedPkaPath),
                                HyoutaUtils::IO::OpenMode::Read);
            if (!refPka.PkaFile.IsOpen()) {
                return std::format("Failed to open referenced pka '{}'.", referencedPkaPath);
            }
            if (!SenLib::ReadPkaFromFile(refPka.PkaHeader, refPka.PkaFile)) {
                return std::format("Failed to read pka header from referenced pka '{}'.",
                                   referencedPkaPath);
            }
        }
    }

    if (taskCancellation->IsCancellationRequested()) {
        return ExtractPkaResult::Cancelled;
    }

    {
        std::filesystem::path targetpath = HyoutaUtils::IO::FilesystemPathFromUtf8(target);
        std::error_code ec;
        std::filesystem::create_directories(targetpath, ec);
        if (ec) {
            return std::format("Failed to create output directory '{}'.", target);
        }
    }

    for (size_t i = 0; i < pkaHeader.PkgCount; ++i) {
        const auto& pkgName = pkaHeader.Pkgs[i].PkgName;
        std::string_view pkgNameSv = HyoutaUtils::TextUtils::StripToNull(pkgName);

        if (taskCancellation->IsCancellationRequested()) {
            return ExtractPkaResult::Cancelled;
        }

        taskReporting->ReportMessageFmt("Extracting {}...", pkgNameSv);

        SenLib::PkgHeader pkg;
        std::unique_ptr<char[]> buffer;
        if (!SenLib::ConvertPkaToSinglePkg(
                pkg, buffer, pkaHeader, i, infile, referencedPkas, extractAsPkaReferenceStub)) {
            return std::format("Failed to convert archive {} ({}) to pkg.", i, pkgNameSv);
        }

        // Restore the possibly stored initial PKG bytes.
        if (pkgNameSv.size() < 28) {
            pkg.Unknown = HyoutaUtils::EndianUtils::FromEndian(
                HyoutaUtils::MemRead::ReadUInt32(pkgName.data() + 28),
                HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        }

        std::unique_ptr<char[]> ms;
        size_t msSize;
        if (!SenLib::CreatePkgInMemory(
                ms, msSize, pkg, HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
            return std::format("Failed to convert archive {} ({}) to pkg.", i, pkgNameSv);
        }

        if (taskCancellation->IsCancellationRequested()) {
            return ExtractPkaResult::Cancelled;
        }

        std::string targetFilename(target);
        HyoutaUtils::IO::AppendPathElement(targetFilename, pkgNameSv);
        HyoutaUtils::IO::File outfile(std::string_view(targetFilename),
                                      HyoutaUtils::IO::OpenMode::Write);
        if (!outfile.IsOpen()) {
            return std::format("Failed to open output file '{}'.\n", targetFilename);
        }

        if (outfile.Write(ms.get(), msSize) != msSize) {
            return std::format("Failed to write to output file '{}'.\n", targetFilename);
        }
    }

    return ExtractPkaResult::Success;
}
} // namespace SenTools
