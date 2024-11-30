#include "sha_file_convert.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "cpp-optparse/OptionParser.h"

#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/text.h"

namespace {
struct TreeNode {
    bool IsDirectory;
    std::string Name;
    HyoutaUtils::Hash::SHA1 Hash;                        // only if file
    std::vector<TreeNode> Children;                      // only if directory
    std::unordered_map<std::string, size_t> ChildLookup; // lookup for Children vector
};
} // namespace

namespace SenTools {
static std::string FilterPath(std::string_view in_path) {
    std::string result;
    size_t in = 0;

    // drop leading path separators
    while (in < in_path.size() && (in_path[in] == '/' || in_path[in] == '\\')) {
        ++in;
    }

    while (in < in_path.size()) {
        const char c = in_path[in];
        if (c == '\0') {
            break;
        }
        if (c == '\\' || c == '/') {
            // collapse sequential path separators
            result.push_back('/');
            ++in;
            while (in_path[in] == '\\' || in_path[in] == '/') {
                ++in;
            }
        } else {
            result.push_back(c);
            ++in;
        }
    }

    // drop trailing path separator
    if (result.ends_with('/')) {
        result.pop_back();
    }

    return result;
}

static bool Insert(TreeNode& tree, const HyoutaUtils::Hash::SHA1& hash, std::string_view path) {
    // wasteful in terms of memory but whatever
    auto slashpos = path.find_first_of('/');
    if (slashpos != std::string_view::npos) {
        std::string_view dirname = path.substr(0, slashpos);
        std::string_view rest = path.substr(slashpos + 1);

        auto dirnameLowercase = HyoutaUtils::TextUtils::ToLower(dirname);
        auto existingDirIt = tree.ChildLookup.find(dirnameLowercase);
        if (existingDirIt == tree.ChildLookup.end()) {
            // no directory node yet, insert new
            tree.ChildLookup.emplace(std::move(dirnameLowercase), tree.Children.size());
            auto& newChild =
                tree.Children.emplace_back(TreeNode{.IsDirectory = true,
                                                    .Name = std::string(dirname),
                                                    .Hash = HyoutaUtils::Hash::SHA1({})});
            return Insert(newChild, hash, rest);
        }
        auto& child = tree.Children[existingDirIt->second];
        if (!child.IsDirectory) {
            return false;
        }
        return Insert(child, hash, rest);
    }

    // found directory to put this file in
    auto pathLowercase = HyoutaUtils::TextUtils::ToLower(path);
    auto existingFileIt = tree.ChildLookup.find(pathLowercase);
    if (existingFileIt != tree.ChildLookup.end()) {
        // file already exists
        return false;
    }

    tree.ChildLookup.emplace(std::move(pathLowercase), tree.Children.size());
    tree.Children.emplace_back(
        TreeNode{.IsDirectory = false, .Name = std::string(path), .Hash = hash});
    return true;
}

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

    std::vector<TreeNode> treePerPath;
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

        TreeNode& root = treePerPath.emplace_back(
            TreeNode{.IsDirectory = true, .Hash = HyoutaUtils::Hash::SHA1({})});
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

            auto path = FilterPath(line.substr(42));
            if (path.empty()) {
                printf("Invalid file format.\n");
                return -1;
            }

            Insert(root, *hash, path);
        }
    }


    return 0;
}
} // namespace SenTools
