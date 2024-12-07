#include "sha_file_convert.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/text.h"

namespace {
struct TreeNode {
    bool IsDirectory;
    std::string Name;
    HyoutaUtils::Hash::SHA1 Hash;                                     // only if file
    std::vector<TreeNode> Children;                                   // only if directory
    std::unordered_map<std::string, std::vector<size_t>> ChildLookup; // lookup for Children vector
    size_t ChildFirstIndex = 0;
    size_t Bucket = 0;
    uint64_t Filesize = 0;
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

static bool Insert(TreeNode& tree,
                   const HyoutaUtils::Hash::SHA1& hash,
                   std::string_view path,
                   uint64_t filesize,
                   size_t bucket) {
    // wasteful in terms of memory but whatever
    auto slashpos = path.find_first_of('/');
    if (slashpos != std::string_view::npos) {
        std::string_view dirname = path.substr(0, slashpos);
        std::string_view rest = path.substr(slashpos + 1);

        auto dirnameLowercase = HyoutaUtils::TextUtils::ToLower(dirname);

        // if we've already seen this dir, update the bucket of the dir and insert into that
        auto& existingDirIndices = tree.ChildLookup[dirnameLowercase];
        for (size_t existingDirIndex : existingDirIndices) {
            auto& existingDir = tree.Children[existingDirIndex];
            if (existingDir.IsDirectory) {
                existingDir.Bucket |= bucket;
                return Insert(existingDir, hash, rest, filesize, bucket);
            }
        }

        // no directory node yet, insert new
        existingDirIndices.emplace_back(tree.Children.size());
        auto& newChild = tree.Children.emplace_back(TreeNode{.IsDirectory = true,
                                                             .Name = std::string(dirname),
                                                             .Hash = HyoutaUtils::Hash::SHA1({}),
                                                             .Bucket = bucket,
                                                             .Filesize = filesize});
        return Insert(newChild, hash, rest, filesize, bucket);
    }

    // found directory to put this file in
    auto pathLowercase = HyoutaUtils::TextUtils::ToLower(path);
    auto& existingFileIndices = tree.ChildLookup[pathLowercase];

    // if any existing file matches, just update the bucket
    for (size_t existingFileIndex : existingFileIndices) {
        auto& existingFile = tree.Children[existingFileIndex];
        if (!existingFile.IsDirectory && existingFile.Hash == hash
            && existingFile.Filesize == filesize) {
            existingFile.Bucket |= bucket;
            return true;
        }
    }

    // no file matches, insert a new one
    existingFileIndices.emplace_back(tree.Children.size());
    tree.Children.emplace_back(TreeNode{.IsDirectory = false,
                                        .Name = std::string(path),
                                        .Hash = hash,
                                        .Bucket = bucket,
                                        .Filesize = filesize});
    return true;
}

static void FillFirstChildIndex(TreeNode& node, size_t& index) {
    if (node.IsDirectory) {
        index += node.Children.size();
        for (auto& child : node.Children) {
            child.ChildFirstIndex = index;
            FillFirstChildIndex(child, index);
        }
    }
}

static size_t InsertFilename(std::string_view name, std::string& stringTable) {
    auto existing = stringTable.find(name);
    if (existing != std::string::npos) {
        return existing;
    }

    size_t offset = stringTable.size();
    stringTable.append(name);
    return offset;
}

static size_t InsertHash(const HyoutaUtils::Hash::SHA1& hash,
                         std::vector<HyoutaUtils::Hash::SHA1>& hashTable) {
    auto existing = std::find(hashTable.begin(), hashTable.end(), hash);
    if (existing != hashTable.end()) {
        return static_cast<size_t>(existing - hashTable.begin());
    }

    size_t offset = hashTable.size();
    hashTable.push_back(hash);
    return offset;
}

static void GenerateDirTreeLine(const TreeNode& node,
                                std::string& stringTable,
                                std::vector<HyoutaUtils::Hash::SHA1>& hashTable,
                                std::string& source) {
    size_t filenameOffset = InsertFilename(node.Name, stringTable);
    size_t hashOffset = 0;
    if (node.IsDirectory) {
        source.append("MakeDirectory(");
    } else {
        hashOffset = InsertHash(node.Hash, hashTable);
        source.append("MakeFile(");
    }
    source.append(std::to_string(filenameOffset));
    source.append(", ");
    source.append(std::to_string(node.Name.size()));
    source.append(", ");
    source.append(std::to_string(node.Bucket));
    source.append(", ");
    if (node.IsDirectory) {
        source.append(std::to_string(node.ChildFirstIndex));
        source.append(", ");
        source.append(std::to_string(node.Children.size()));
    } else {
        source.append(std::to_string(hashOffset));
        source.append(", ");
        source.append(std::to_string(node.Filesize));
    }
    source.append("),\n");
}

static void GenerateDirTreeSource(const TreeNode& node,
                                  std::string& stringTable,
                                  std::vector<HyoutaUtils::Hash::SHA1>& hashTable,
                                  std::string& source) {
    if (node.IsDirectory) {
        for (const auto& child : node.Children) {
            GenerateDirTreeLine(child, stringTable, hashTable, source);
        }
        for (const auto& child : node.Children) {
            GenerateDirTreeSource(child, stringTable, hashTable, source);
        }
    }
}

int SHA_File_Convert_Function(int argc, char** argv) {
    if (argc < 1) {
        printf("First argument must be one of: CS1, CS2, CS3, CS4, Reverie, TX.\n");
        return -1;
    }

    struct Arg {
        std::string ShaFilePath;
        std::string GamePath;
        size_t Bucket;
    };
    std::vector<Arg> args;
    std::string_view arg1(argv[1]);
    std::string outputFilename;
    if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(arg1, "CS1")) {
        args.push_back({"c:\\SenPatcher\\SenLib\\Sen1\\FileInfo\\steam_version_1.6.sha",
                        "f:\\SteamLibrary\\steamapps\\common\\Trails of Cold Steel",
                        size_t(1)});
        outputFilename = "dirtree_cs1.h";
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(arg1, "CS2")) {
        // args.push_back({"c:\\SenPatcher\\SenLib\\Sen2\\FileInfo\\steam_version_1.4.sha",
        //                 "f:\\SteamLibrary\\steamapps\\common\\Trails of Cold Steel II",
        //                 size_t(1)});
        args.push_back({"c:\\SenPatcher\\SenLib\\Sen2\\FileInfo\\steam_version_1.4.1.sha",
                        "f:\\SteamLibrary\\steamapps\\common\\Trails of Cold Steel II",
                        size_t(1 << 1)});
        // args.push_back({"c:\\SenPatcher\\SenLib\\Sen2\\FileInfo\\steam_version_1.4.2.sha",
        //                 "f:\\SteamLibrary\\steamapps\\common\\Trails of Cold Steel II",
        //                 size_t(1 << 2)});
        outputFilename = "dirtree_cs2.h";
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(arg1, "CS3")) {
        args.push_back(
            {"c:\\SenPatcher\\SenLib\\Sen3\\FileInfo\\steam_version_1.06.sha",
             "f:\\SteamLibrary\\steamapps\\common\\The Legend of Heroes Trails of Cold Steel III",
             size_t(1)});
        outputFilename = "dirtree_cs3.h";
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(arg1, "CS4")) {
        args.push_back(
            {"c:\\SenPatcher\\SenLib\\Sen4\\FileInfo\\steam_version_1.2.1.sha",
             "f:\\SteamLibrary\\steamapps\\common\\The Legend of Heroes Trails of Cold Steel IV",
             size_t(1)});
        outputFilename = "dirtree_cs4.h";
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(arg1, "Reverie")) {
        // args.push_back(
        //     {"c:\\SenPatcher\\SenLib\\Sen5\\FileInfo\\steam_version_1.0.8.sha",
        //      "f:\\SteamLibrary\\steamapps\\common\\The Legend of Heroes Trails into Reverie",
        //      size_t(1)});
        args.push_back(
            {"c:\\SenPatcher\\SenLib\\Sen5\\FileInfo\\steam_version_1.1.4.sha",
             "f:\\SteamLibrary\\steamapps\\common\\The Legend of Heroes Trails into Reverie",
             size_t(1 << 1)});
        outputFilename = "dirtree_reverie.h";
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(arg1, "TX")) {
        args.push_back({"c:\\SenPatcher\\SenLib\\TX\\FileInfo\\steam_version_1.08.sha",
                        "f:\\SteamLibrary\\steamapps\\common\\Tokyo Xanadu eX+",
                        size_t(1)});
        args.push_back({"c:\\SenPatcher\\SenLib\\TX\\FileInfo\\gog_version_1.08.sha",
                        "f:\\GOG Games\\Tokyo Xanadu eX+",
                        size_t(1 << 1)});
        outputFilename = "dirtree_tx.h";
    } else {
        printf("First argument must be one of: CS1, CS2, CS3, CS4, Reverie, TX.\n");
        return -1;
    }

    TreeNode root{.IsDirectory = true, .Hash = HyoutaUtils::Hash::SHA1({})};
    for (size_t i = 0; i < args.size(); ++i) {
        const auto& arg = args[i];
        HyoutaUtils::IO::File infile(std::string_view(arg.ShaFilePath),
                                     HyoutaUtils::IO::OpenMode::Read);
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

            uint64_t length = 0;
            {
                std::string fullpath = arg.GamePath + "/" + path;
                HyoutaUtils::IO::File gamefile(std::string_view(fullpath),
                                               HyoutaUtils::IO::OpenMode::Read);
                if (!gamefile.IsOpen()) {
                    printf("Failed to open file %s\n", fullpath.c_str());
                    return -1;
                }
                auto l = gamefile.GetLength();
                if (!l) {
                    printf("Failed to get length of file %s\n", fullpath.c_str());
                    return -1;
                }
                length = *l;
            }

            if (!Insert(root, *hash, path, length, arg.Bucket)) {
                printf("Insert error for file %s\n", path.c_str());
                return -1;
            }
        }
    }

    {
        std::string stringTable;
        std::vector<HyoutaUtils::Hash::SHA1> hashTable;
        size_t count = 1;
        root.ChildFirstIndex = 1;
        FillFirstChildIndex(root, count);
        std::string source;
        source.append("static constexpr auto s_dirtree = std::array<Entry, ");
        source.append(std::to_string(count));
        source.append(">{{\n");
        GenerateDirTreeLine(root, stringTable, hashTable, source);
        GenerateDirTreeSource(root, stringTable, hashTable, source);
        source.append("}};\n\n");

        source.append("static constexpr auto s_stringtable = std::array<char, ");
        source.append(std::to_string(stringTable.size()));
        source.append(">{{\n");
        for (size_t i = 0; i < stringTable.size(); ++i) {
            if (i != 0) {
                source.append(", ");
            }
            source.append(std::to_string((int)stringTable[i]));
        }
        source.append("}};\n\n");

        source.append("static constexpr auto s_hashtable = std::array<HyoutaUtils::Hash::SHA1, ");
        source.append(std::to_string(hashTable.size()));
        source.append(">{{\n");
        for (size_t i = 0; i < hashTable.size(); ++i) {
            source.append("HyoutaUtils::Hash::SHA1(std::array<char, 20>({{");
            for (size_t j = 0; j < 20; ++j) {
                if (j != 0) {
                    source.append(", ");
                }
                source.append(std::to_string((int)hashTable[i].Hash[j]));
            }
            source.append("}})),\n");
        }
        source.append("}};\n\n");

        HyoutaUtils::IO::File outfile(std::string_view(outputFilename),
                                      HyoutaUtils::IO::OpenMode::Write);
        if (!outfile.IsOpen()) {
            printf("Cannot open output file %s\n", outputFilename.c_str());
            return -1;
        }
        if (outfile.Write(source.data(), source.size()) != source.size()) {
            printf("Failed to write to output file %s\n", outputFilename.c_str());
            return -1;
        }
    }

    return 0;
}
} // namespace SenTools
