#include "pka_extract.h"
#include "pka_extract_main.h"

#include <cstdio>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "sen/pka.h"
#include "sen/pka_to_pkg.h"
#include "sen/pkg.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/memread.h"
#include "util/text.h"

namespace SenTools {
int PKA_Extract_Function(int argc, char** argv) {
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
    parser.add_option("--referenced-pka")
        .dest("referenced-pka")
        .action(optparse::ActionType::Append)
        .metavar("PKA")
        .help(
            "Referenced pka file that could also contain files, see the corresponding option in "
            "PKA.Pack for details. Option can be provided multiple times. This is a nonstandard "
            "feature that the vanilla game does not handle.");

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

    std::span<const std::string> referencedPkaPaths;
    if (auto* referenced_pka_option = options.get("referenced-pka")) {
        referencedPkaPaths = referenced_pka_option->strings();
    }

    auto result = ExtractPka(source, target, referencedPkaPaths);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}

HyoutaUtils::Result<ExtractPkaResult, std::string>
    ExtractPka(std::string_view source,
               std::string_view target,
               std::span<const std::string> referencedPkaPaths) {
    std::filesystem::path sourcepath = HyoutaUtils::IO::FilesystemPathFromUtf8(source);
    std::filesystem::path targetpath = HyoutaUtils::IO::FilesystemPathFromUtf8(target);
    HyoutaUtils::IO::File infile(sourcepath, HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        return std::string("Failed to open input file.");
    }

    SenLib::PkaHeader pkaHeader;
    if (!SenLib::ReadPkaFromFile(pkaHeader, infile)) {
        return std::string("Failed to read pka header.");
    }

    std::vector<SenLib::ReferencedPka> referencedPkas;
    {
        referencedPkas.reserve(referencedPkaPaths.size());
        for (const auto& referencedPkaPath : referencedPkaPaths) {
            auto& refPka = referencedPkas.emplace_back();
            refPka.PkaFile.Open(std::filesystem::path(referencedPkaPath),
                                HyoutaUtils::IO::OpenMode::Read);
            if (!refPka.PkaFile.IsOpen()) {
                return std::string("Error opening referenced pka.");
            }
            if (!SenLib::ReadPkaFromFile(refPka.PkaHeader, refPka.PkaFile)) {
                return std::string("Error reading referenced pka.");
            }
        }
    }

    {
        std::error_code ec;
        std::filesystem::create_directories(targetpath, ec);
        if (ec) {
            return std::string("Failed to create output directoy.");
        }
    }

    for (size_t i = 0; i < pkaHeader.PkgCount; ++i) {
        const auto& pkgName = pkaHeader.Pkgs[i].PkgName;
        std::string_view pkgNameSv = HyoutaUtils::TextUtils::StripToNull(pkgName);

        SenLib::PkgHeader pkg;
        std::unique_ptr<char[]> buffer;
        if (!SenLib::ConvertPkaToSinglePkg(pkg, buffer, pkaHeader, i, infile, referencedPkas)) {
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

        HyoutaUtils::IO::File outfile(
            targetpath
                / std::u8string_view(reinterpret_cast<const char8_t*>(pkgNameSv.data()),
                                     pkgNameSv.size()),
            HyoutaUtils::IO::OpenMode::Write);
        if (!outfile.IsOpen()) {
            return std::string("Failed to open output file.\n");
        }

        if (outfile.Write(ms.get(), msSize) != msSize) {
            return std::string("Failed to write to output file.\n");
        }
    }

    return ExtractPkaResult::Success;
}
} // namespace SenTools
