#include "type1_decompress.h"
#include "type1_decompress_main.h"

#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "sen/pkg_extract.h"
#include "util/file.h"
#include "util/memread.h"

namespace SenTools {
int Type1_Decompress_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(Type1_Decompress_ShortDescription);

    parser.usage("sentools " Type1_Decompress_Name " [options] file.bin");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help(
            "The output file to decompress to. Will be derived from input filename if not given.");

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
        tmp += ".dec";
        target = tmp;
    }

    auto result = DecompressType1(source, target);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}

HyoutaUtils::Result<DecompressType1Result, std::string> DecompressType1(std::string_view source,
                                                                        std::string_view target) {
    HyoutaUtils::IO::File infile(std::string_view(source), HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        return std::string("Failed to open input file.");
    }
    auto filesize = infile.GetLength();
    if (!filesize) {
        return std::string("Failed to get size of input file.");
    }
    if (*filesize < 12) {
        return std::string("File too small to be type1 compressed.");
    }
    if (*filesize > 0xffff'ffffu) {
        return std::string("File too big to be type1 compressed.");
    }

    auto pkgMemory = std::make_unique_for_overwrite<char[]>(*filesize);
    if (!pkgMemory) {
        return std::string("Failed to allocate memory.");
    }
    if (infile.Read(pkgMemory.get(), *filesize) != *filesize) {
        return std::string("Failed read input file.");
    }

    const uint32_t uncompressedSize =
        HyoutaUtils::EndianUtils::FromEndian(HyoutaUtils::MemRead::ReadUInt32(pkgMemory.get()),
                                             HyoutaUtils::EndianUtils::Endianness::LittleEndian);

    auto buffer = std::make_unique<char[]>(uncompressedSize);
    if (!buffer) {
        return std::string("Failed to allocate memory.");
    }
    if (!SenLib::ExtractAndDecompressPkgFile(buffer.get(),
                                             uncompressedSize,
                                             pkgMemory.get(),
                                             static_cast<uint32_t>(*filesize),
                                             1,
                                             HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        return std::string("Failed to decompress file.");
    }

    HyoutaUtils::IO::File outfile(std::string_view(target), HyoutaUtils::IO::OpenMode::Write);
    if (!outfile.IsOpen()) {
        return std::string("Failed to open output file.");
    }

    if (outfile.Write(buffer.get(), uncompressedSize) != uncompressedSize) {
        return std::string("Failed to write to output file.");
    }

    return DecompressType1Result::Success;
}
} // namespace SenTools
