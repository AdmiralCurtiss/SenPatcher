#include "pka_extract.h"

#include <cstdio>
#include <filesystem>
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

namespace SenTools {
static std::string_view StripToNull(std::string_view sv) {
    for (size_t i = 0; i < sv.size(); ++i) {
        if (sv[i] == '\0') {
            return sv.substr(0, i);
        }
    }
    return sv;
}

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
        .metavar("PKA")
        .help("2nd pka file that could also contain files.");

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


    std::filesystem::path sourcepath(source.begin(), source.end());
    std::filesystem::path targetpath(target.begin(), target.end());
    HyoutaUtils::IO::File infile(sourcepath, HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        printf("Failed to open input file.\n");
        return -1;
    }

    SenLib::PkaHeader pkaHeader;
    if (!SenLib::ReadPkaFromFile(pkaHeader, infile)) {
        printf("Failed to read pka header.\n");
        return -1;
    }

    std::optional<HyoutaUtils::IO::File> refInfile;
    std::optional<SenLib::PkaHeader> refPkaHeader;
    if (auto* referenced_pka_option = options.get("referenced-pka")) {
        std::string_view refpath(referenced_pka_option->first_string());
        refInfile.emplace(std::filesystem::path(refpath), HyoutaUtils::IO::OpenMode::Read);
        if (!refInfile->IsOpen()) {
            printf("Error opening referenced pka.\n");
            return -1;
        }
        refPkaHeader.emplace();
        if (!SenLib::ReadPkaFromFile(*refPkaHeader, *refInfile)) {
            printf("Error reading referenced pka.\n");
            return -1;
        }
    }

    {
        std::error_code ec;
        std::filesystem::create_directories(targetpath, ec);
        if (ec) {
            printf("Failed to create output directoy.\n");
            return -1;
        }
    }

    for (size_t i = 0; i < pkaHeader.PkgCount; ++i) {
        const auto& pkgName = pkaHeader.Pkgs[i].PkgName;
        std::string_view pkgNameSv = StripToNull(std::string_view(pkgName.data(), pkgName.size()));

        SenLib::PkgHeader pkg;
        std::unique_ptr<char[]> buffer;
        if (!SenLib::ConvertPkaToSinglePkg(pkg,
                                           buffer,
                                           pkaHeader,
                                           i,
                                           infile,
                                           refPkaHeader.has_value() ? &*refPkaHeader : nullptr,
                                           refInfile.has_value() ? &*refInfile : nullptr)) {
            printf("Failed to convert archive %zu to pkg.\n", i);
            return -1;
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
            printf("Failed to convert archive %zu to pkg.\n", i);
            return -1;
        }

        HyoutaUtils::IO::File outfile(
            targetpath
                / std::u8string_view(reinterpret_cast<const char8_t*>(pkgNameSv.data()),
                                     pkgNameSv.size()),
            HyoutaUtils::IO::OpenMode::Write);
        if (!outfile.IsOpen()) {
            printf("Failed to open output file.\n");
            return -1;
        }

        if (outfile.Write(ms.get(), msSize) != msSize) {
            printf("Failed to write to output file.\n");
            return -1;
        }
    }

    return 0;
}
} // namespace SenTools
