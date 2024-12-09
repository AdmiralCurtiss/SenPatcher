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

#include "rapidjson/document.h"

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
    size_t GameVersionBits = 0;
    size_t DlcIndex = 0;
    uint64_t Filesize = 0;
};

template<typename T>
static std::optional<uint64_t> ReadUInt64(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsInt64()) {
        const auto i = j.GetInt64();
        if (i < 0) {
            return std::nullopt;
        }
        return static_cast<uint64_t>(i);
    }
    return std::nullopt;
}

template<typename T>
static std::optional<std::string> ReadString(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsString()) {
        const char* str = j.GetString();
        const auto len = j.GetStringLength();
        return std::string(str, len);
    }
    return std::nullopt;
}
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
                   size_t gameVersionBits,
                   size_t dlcIndex) {
    // wasteful in terms of memory but whatever
    auto slashpos = path.find_first_of('/');
    if (slashpos != std::string_view::npos) {
        std::string_view dirname = path.substr(0, slashpos);
        std::string_view rest = path.substr(slashpos + 1);

        auto dirnameLowercase = HyoutaUtils::TextUtils::ToLower(dirname);

        // if we've already seen this dir, update the game version of the dir and insert into that
        auto& existingDirIndices = tree.ChildLookup[dirnameLowercase];
        for (size_t existingDirIndex : existingDirIndices) {
            auto& existingDir = tree.Children[existingDirIndex];
            if (existingDir.IsDirectory) {
                existingDir.GameVersionBits |= gameVersionBits;
                return Insert(existingDir, hash, rest, filesize, gameVersionBits, dlcIndex);
            }
        }

        // no directory node yet, insert new
        existingDirIndices.emplace_back(tree.Children.size());
        auto& newChild = tree.Children.emplace_back(TreeNode{.IsDirectory = true,
                                                             .Name = std::string(dirname),
                                                             .Hash = HyoutaUtils::Hash::SHA1({}),
                                                             .GameVersionBits = gameVersionBits,
                                                             .DlcIndex = dlcIndex,
                                                             .Filesize = filesize});
        return Insert(newChild, hash, rest, filesize, gameVersionBits, dlcIndex);
    }

    // found directory to put this file in
    auto pathLowercase = HyoutaUtils::TextUtils::ToLower(path);
    auto& existingFileIndices = tree.ChildLookup[pathLowercase];

    // if any existing file matches, just update the game version
    for (size_t existingFileIndex : existingFileIndices) {
        auto& existingFile = tree.Children[existingFileIndex];
        if (!existingFile.IsDirectory && existingFile.Hash == hash
            && existingFile.Filesize == filesize) {
            existingFile.GameVersionBits |= gameVersionBits;
            return true;
        }
    }

    // no file matches, insert a new one
    existingFileIndices.emplace_back(tree.Children.size());
    tree.Children.emplace_back(TreeNode{.IsDirectory = false,
                                        .Name = std::string(path),
                                        .Hash = hash,
                                        .GameVersionBits = gameVersionBits,
                                        .DlcIndex = dlcIndex,
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
        source.append("HyoutaUtils::DirTree::Entry::MakeDirectory(");
    } else {
        hashOffset = InsertHash(node.Hash, hashTable);
        source.append("HyoutaUtils::DirTree::Entry::MakeFile(");
    }
    source.append(std::to_string(filenameOffset));
    source.append(", ");
    source.append(std::to_string(node.Name.size()));
    source.append(", ");
    source.append(std::to_string(node.GameVersionBits));
    source.append(", ");
    source.append(std::to_string(node.DlcIndex));
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

namespace {
struct Arg {
    std::string ShaFilePath;
    std::string GamePath;
    size_t GameVersion;
    size_t DlcIndex;
};
struct ProgramArgs {
    std::vector<Arg> Input;
    std::string Output;
};
} // namespace

static std::optional<ProgramArgs> ParseArgs(std::string_view jsonPath) {
    HyoutaUtils::IO::File f(std::filesystem::path(jsonPath), HyoutaUtils::IO::OpenMode::Read);
    if (!f.IsOpen()) {
        return std::nullopt;
    }
    auto length = f.GetLength();
    if (!length) {
        return std::nullopt;
    }
    auto buffer = std::make_unique_for_overwrite<char[]>(*length);
    if (!buffer) {
        return std::nullopt;
    }
    if (f.Read(buffer.get(), *length) != *length) {
        return std::nullopt;
    }

    rapidjson::Document json;
    json.Parse<rapidjson::kParseFullPrecisionFlag | rapidjson::kParseNanAndInfFlag,
               rapidjson::UTF8<char>>(buffer.get(), *length);
    if (json.HasParseError() || !json.IsObject()) {
        return std::nullopt;
    }

    ProgramArgs args;

    const auto root = json.GetObject();
    const auto input = root.FindMember("Input");
    if (input != root.MemberEnd() && input->value.IsArray()) {
        for (const auto& obj : input->value.GetArray()) {
            if (obj.IsObject()) {
                auto shaFilePath = ReadString(obj, "ShaFilePath");
                auto gamePath = ReadString(obj, "GamePath");
                auto gameVersion = ReadUInt64(obj, "GameVersion");
                auto dlcIndex = ReadUInt64(obj, "DlcIndex");
                if (!shaFilePath || !gamePath || !gameVersion || !dlcIndex) {
                    return std::nullopt;
                }
                args.Input.emplace_back(Arg{.ShaFilePath = std::move(*shaFilePath),
                                            .GamePath = std::move(*gamePath),
                                            .GameVersion = *gameVersion,
                                            .DlcIndex = *dlcIndex});
            } else {
                return std::nullopt;
            }
        }
    } else {
        return std::nullopt;
    }
    const auto output = ReadString(root, "Output");
    if (!output) {
        return std::nullopt;
    }
    args.Output = std::move(*output);

    return args;
}

int SHA_File_Convert_Function(int argc, char** argv) {
    if (argc <= 1) {
        printf(
            "First argument must point to .json describing the data to convert.\n");
        return -1;
    }

    const auto& programArgs = ParseArgs(argv[1]);
    if (!programArgs) {
        printf("Couldn't parse .json file.\n");
        return -1;
    }

    const std::vector<Arg>& args = programArgs->Input;
    const std::string& outputFilename = programArgs->Output;

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

            if (!Insert(root, *hash, path, length, size_t(1) << arg.GameVersion, arg.DlcIndex)) {
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
        source.append("#pragma once\n\n");
        source.append("#include <array>\n\n");
        source.append("#include \"dirtree/entry.h\"\n");
        source.append("#include \"util/hash/sha1.h\"\n\n");
        source.append("static constexpr auto s_dirtree = std::array<HyoutaUtils::DirTree::Entry, ");
        source.append(std::to_string(count));
        source.append(">{{\n");
        GenerateDirTreeLine(root, stringTable, hashTable, source);
        GenerateDirTreeSource(root, stringTable, hashTable, source);
        source.append("}};\n\n");

        source.append("static constexpr auto s_stringtable = std::array<char, ");
        source.append(std::to_string(stringTable.size()));
        source.append(">{{");
        for (size_t i = 0; i < stringTable.size(); ++i) {
            if (i != 0) {
                source.append(",");
            }
            if ((i % 32) == 0) {
                source.append("\n");
            } else {
                source.append(" ");
            }
            source.append(std::to_string((int)stringTable[i]));
        }
        source.append("\n}};\n\n");

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
