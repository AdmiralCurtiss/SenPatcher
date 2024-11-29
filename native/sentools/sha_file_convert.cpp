#include "sha_file_convert.h"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "cpp-optparse/OptionParser.h"

#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/text.h"

namespace {
struct HashWithPath {
    HyoutaUtils::Hash::SHA1 Hash;
    std::string Path;
};
} // namespace

namespace SenTools {
int SHA_File_Convert_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(SHA_File_Convert_ShortDescription);

    parser.usage("sentools " SHA_File_Convert_Name " [options] hashes.sha");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILE")
        .help("The output file to convert to. Will be derived from input filename if not given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() == 0) {
        parser.error("No input file given.");
        return -1;
    }

    std::string_view target;
    std::string tmp;
    if (auto* output_option = options.get("output")) {
        target = std::string_view(output_option->first_string());
    } else {
        tmp = std::string(args[0]);
        tmp += ".cpp";
        target = tmp;
    }

    std::vector<std::vector<HashWithPath>> entriesPerFile;
    std::filesystem::path targetpath(target.begin(), target.end());
    for (const auto& arg : args) {
        std::filesystem::path sourcepath(arg.begin(), arg.end());
        HyoutaUtils::IO::File infile(sourcepath, HyoutaUtils::IO::OpenMode::Read);
        if (!infile.IsOpen()) {
            printf("Failed to open input file.\n");
            return -1;
        }
        auto filesize = infile.GetLength();
        if (!filesize) {
            printf("Failed to get size of input file.\n");
            return -1;
        }
        auto data = std::make_unique_for_overwrite<char[]>(*filesize);
        if (!data) {
            printf("Failed to allocate memory.\n");
            return -1;
        }
        if (infile.Read(data.get(), *filesize) != *filesize) {
            printf("Failed to read input file.\n");
            return -1;
        }

        std::vector<HashWithPath> entries;
        std::string_view remaining(data.get(), *filesize);

        // skip UTF8 BOM if it's there
        if (remaining.starts_with("\xef\xbb\xbf")) {
            remaining = remaining.substr(3);
        }

        while (!remaining.empty()) {
            const size_t nextLineSeparator = remaining.find_first_of("\r\n");
            std::string_view line = remaining.substr(0, nextLineSeparator);
            remaining = nextLineSeparator != std::string_view::npos
                            ? remaining.substr(nextLineSeparator + 1)
                            : std::string_view();

            line = HyoutaUtils::TextUtils::Trim(line);
            if (line.empty()) {
                continue;
            }

            if (line.size() < 43 || line[40] != ' ' || line[41] != '*') {
                printf("Invalid file format.\n");
                return -1;
            }
            auto hash = HyoutaUtils::Hash::TrySHA1FromHexString(line.substr(0, 40));
            if (!hash) {
                printf("Invalid file format.\n");
                return -1;
            }

            entries.emplace_back(HashWithPath{.Hash = *hash, .Path = std::string(line.substr(42))});
        }

        entriesPerFile.emplace_back(std::move(entries));
    }


    return 0;
}
} // namespace SenTools
