#include "sha_file_convert.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "rapidjson/document.h"

#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/text.h"

#include "dirtree/dirtree_cs1.h"
#include "dirtree/dirtree_cs2.h"
#include "dirtree/dirtree_tx.h"
#include "dirtree/entry.h"
#include "dirtree/tree.h"

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

static TreeNode& InsertDirectory(TreeNode& tree,
                                 std::string_view dirname,
                                 size_t gameVersionBits,
                                 size_t dlcIndex) {
    auto dirnameLowercase = HyoutaUtils::TextUtils::ToLower(dirname);

    // if we've already seen this dir, update the game version of the dir and insert into that
    auto& existingDirIndices = tree.ChildLookup[dirnameLowercase];
    for (size_t existingDirIndex : existingDirIndices) {
        auto& existingDir = tree.Children[existingDirIndex];
        if (existingDir.IsDirectory
            && (existingDir.DlcIndex == 0 || existingDir.DlcIndex == dlcIndex)) {
            existingDir.GameVersionBits |= gameVersionBits;
            return existingDir;
        }
    }

    // no directory node yet, insert new
    existingDirIndices.emplace_back(tree.Children.size());
    auto& newChild = tree.Children.emplace_back(TreeNode{.IsDirectory = true,
                                                         .Name = std::string(dirname),
                                                         .Hash = HyoutaUtils::Hash::SHA1({}),
                                                         .GameVersionBits = gameVersionBits,
                                                         .DlcIndex = dlcIndex});
    return newChild;
}

static bool InsertFile(TreeNode& tree,
                       const HyoutaUtils::Hash::SHA1& hash,
                       std::string_view path,
                       uint64_t filesize,
                       size_t gameVersionBits,
                       size_t dlcIndex) {
    // found directory to put this file in
    auto pathLowercase = HyoutaUtils::TextUtils::ToLower(path);
    auto& existingFileIndices = tree.ChildLookup[pathLowercase];

    // if any existing file matches, just update the game version
    for (size_t existingFileIndex : existingFileIndices) {
        auto& existingFile = tree.Children[existingFileIndex];
        if (!existingFile.IsDirectory && existingFile.Hash == hash
            && existingFile.Filesize == filesize && existingFile.DlcIndex == dlcIndex) {
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
            child.ChildFirstIndex = child.Children.empty() ? static_cast<size_t>(0) : index;
            FillFirstChildIndex(child, index);
        }
    }
}

static size_t GetMaxDlcIndex(TreeNode& node) {
    size_t maxDlcIndex = node.DlcIndex;
    if (node.IsDirectory) {
        for (auto& child : node.Children) {
            maxDlcIndex = std::max(maxDlcIndex, GetMaxDlcIndex(child));
        }
    }
    return maxDlcIndex;
}

static size_t InsertFilename(std::string_view name,
                             std::string& stringTable,
                             std::vector<size_t>& stringOffsets) {
    auto existing = stringTable.find(name);
    if (existing != std::string::npos) {
        return existing;
    }

    size_t offset = stringTable.size();
    stringTable.append(name);
    stringOffsets.push_back(stringTable.size());
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

static void AppendEscaped(std::string& source, std::string_view s) {
    for (char c : s) {
        if (c == '\\') {
            source.append("\\\\");
        } else if (c >= 0x20 && c <= 0x7e) {
            source.push_back(c);
        } else if (c == '\t') {
            source.append("\\t");
        } else if (c == '\n') {
            source.append("\\n");
        } else if (c == '\v') {
            source.append("\\v");
        } else if (c == '\f') {
            source.append("\\f");
        } else if (c == '\r') {
            source.append("\\r");
        } else {
            char tmp[8];
            sprintf(tmp, "\\x%02x", static_cast<uint8_t>(c));
            source.append(tmp);
        }
    }
}

static void AppendChar(std::string& source, char c) {
    // std::array<char, 16> tmp{};
    // sprintf(tmp.data(), "'\\x%02x'", static_cast<uint8_t>(c));
    // source.append(tmp.data());
    source.append(std::to_string(c));
}

static void AppendEscapedChar(std::string& source, char c) {
    // source.push_back('\'');
    // if (c == '\'') {
    //     source.append("\\'");
    // } else {
    //     AppendEscaped(source, std::string_view(&c, 1));
    // }
    // source.push_back('\'');
    AppendChar(source, c);
}

static void GenerateDirTreeLine(const TreeNode& node,
                                std::string& stringTable,
                                std::vector<size_t>& stringOffsets,
                                std::vector<HyoutaUtils::Hash::SHA1>& hashTable,
                                std::string& source) {
    size_t filenameOffset = InsertFilename(node.Name, stringTable, stringOffsets);
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
    source.append("),");
    if (!node.Name.empty()) {
        source.append(" // ");
        AppendEscaped(source, node.Name);
    }
    source.append("\n");
}

static void GenerateDirTreeSource(const TreeNode& node,
                                  std::string& stringTable,
                                  std::vector<size_t>& stringOffsets,
                                  std::vector<HyoutaUtils::Hash::SHA1>& hashTable,
                                  std::string& source) {
    if (node.IsDirectory) {
        for (const auto& child : node.Children) {
            GenerateDirTreeLine(child, stringTable, stringOffsets, hashTable, source);
        }
        for (const auto& child : node.Children) {
            GenerateDirTreeSource(child, stringTable, stringOffsets, hashTable, source);
        }
    }
}

static bool GenerateRawDirTreeLine(const TreeNode& node,
                                   std::string& stringTable,
                                   std::vector<size_t>& stringOffsets,
                                   std::vector<HyoutaUtils::Hash::SHA1>& hashTable,
                                   std::string& source) {
    std::array<char, 64> buffer{};

    size_t filenameOffset = InsertFilename(node.Name, stringTable, stringOffsets);
    size_t filenameLength = node.Name.size();
    if (!(filenameOffset < (1u << 24))) {
        return false;
    }
    if (!(filenameLength < (1u << 8))) {
        return false;
    }

    size_t filenameValue = (filenameOffset << 8) | filenameLength;
    source.append("D(");
    AppendChar(source, static_cast<char>(filenameValue >> 24));
    source.push_back(',');
    AppendChar(source, static_cast<char>(filenameValue >> 16));
    source.push_back(',');
    AppendChar(source, static_cast<char>(filenameValue >> 8));
    source.push_back(',');
    AppendChar(source, static_cast<char>(filenameValue));
    source.append("),");

    size_t gameVersionBits = node.GameVersionBits;
    size_t dlcIndex = node.DlcIndex;
    if (!(gameVersionBits < (1u << 20))) {
        return false;
    }
    if (!(dlcIndex < (1u << 11))) {
        return false;
    }
    size_t metadataValue =
        (dlcIndex << 20) | gameVersionBits | (node.IsDirectory ? 0x8000'0000u : 0u);
    source.append("D(");
    AppendChar(source, static_cast<char>(metadataValue >> 24));
    source.push_back(',');
    AppendChar(source, static_cast<char>(metadataValue >> 16));
    source.push_back(',');
    AppendChar(source, static_cast<char>(metadataValue >> 8));
    source.push_back(',');
    AppendChar(source, static_cast<char>(metadataValue));
    source.append("),");

    if (node.IsDirectory) {
        if (node.ChildFirstIndex > 0xffff'ffffu) {
            return false;
        }
        if (node.Children.size() > 0xffff'ffffu) {
            return false;
        }

        size_t firstIndex = node.ChildFirstIndex;
        size_t childCount = node.Children.size();
        source.append("D(");
        AppendChar(source, static_cast<char>(firstIndex >> 24));
        source.push_back(',');
        AppendChar(source, static_cast<char>(firstIndex >> 16));
        source.push_back(',');
        AppendChar(source, static_cast<char>(firstIndex >> 8));
        source.push_back(',');
        AppendChar(source, static_cast<char>(firstIndex));
        source.append("),");
        source.append("D(");
        AppendChar(source, static_cast<char>(childCount >> 24));
        source.push_back(',');
        AppendChar(source, static_cast<char>(childCount >> 16));
        source.push_back(',');
        AppendChar(source, static_cast<char>(childCount >> 8));
        source.push_back(',');
        AppendChar(source, static_cast<char>(childCount));
        source.append("),");
    } else {
        size_t hashIndex = InsertHash(node.Hash, hashTable);
        uint64_t filesize = node.Filesize;
        if (!(filesize < (1ull << 44))) {
            return false;
        }
        if (!(hashIndex < (1u << 20))) {
            return false;
        }

        uint64_t hashAndSize = (filesize << 20) | hashIndex;
        source.append("Q(");
        AppendChar(source, static_cast<char>(hashAndSize >> 56));
        source.push_back(',');
        AppendChar(source, static_cast<char>(hashAndSize >> 48));
        source.push_back(',');
        AppendChar(source, static_cast<char>(hashAndSize >> 40));
        source.push_back(',');
        AppendChar(source, static_cast<char>(hashAndSize >> 32));
        source.push_back(',');
        AppendChar(source, static_cast<char>(hashAndSize >> 24));
        source.push_back(',');
        AppendChar(source, static_cast<char>(hashAndSize >> 16));
        source.push_back(',');
        AppendChar(source, static_cast<char>(hashAndSize >> 8));
        source.push_back(',');
        AppendChar(source, static_cast<char>(hashAndSize));
        source.append("),");
    }
    source.append("\n");

    return true;
}

static bool GenerateRawDirTreeSource(const TreeNode& node,
                                     std::string& stringTable,
                                     std::vector<size_t>& stringOffsets,
                                     std::vector<HyoutaUtils::Hash::SHA1>& hashTable,
                                     std::string& source) {
    if (node.IsDirectory) {
        for (const auto& child : node.Children) {
            if (!GenerateRawDirTreeLine(child, stringTable, stringOffsets, hashTable, source)) {
                return false;
            }
        }
        for (const auto& child : node.Children) {
            if (!GenerateRawDirTreeSource(child, stringTable, stringOffsets, hashTable, source)) {
                return false;
            }
        }
    }
    return true;
}

static void SortChildrenRecursive(TreeNode& node) {
    if (node.IsDirectory) {
        node.ChildLookup.clear();
        std::stable_sort(
            node.Children.begin(),
            node.Children.end(),
            [](const TreeNode& lhs, const TreeNode& rhs) -> bool {
                std::string lhsName = HyoutaUtils::TextUtils::ToLower(lhs.Name);
                std::string rhsName = HyoutaUtils::TextUtils::ToLower(rhs.Name);
                return std::tie(lhsName, lhs.GameVersionBits, lhs.DlcIndex, lhs.Filesize)
                       < std::tie(rhsName, rhs.GameVersionBits, rhs.DlcIndex, rhs.Filesize);
            });
        for (auto& child : node.Children) {
            SortChildrenRecursive(child);
        }
    }
}

static void PreInsertHashes(TreeNode& node, std::vector<HyoutaUtils::Hash::SHA1>& hashTable) {
    if (node.IsDirectory) {
        for (auto& child : node.Children) {
            PreInsertHashes(child, hashTable);
        }
    } else {
        InsertHash(node.Hash, hashTable);
    }
}

static void PreInsertNamesRecursive(TreeNode& node, std::vector<std::string>& strings) {
    if (!node.Name.empty()) {
        strings.push_back(node.Name);
    }
    if (node.IsDirectory) {
        for (auto& child : node.Children) {
            PreInsertNamesRecursive(child, strings);
        }
    }
}

static void
    PreInsertNames(TreeNode& node, std::string& stringTable, std::vector<size_t>& stringOffsets) {
    std::vector<std::string> strings;
    PreInsertNamesRecursive(node, strings);

    // sort longer strings first, so that names that are part of other names share bytes
    std::stable_sort(
        strings.begin(), strings.end(), [](const std::string& lhs, const std::string& rhs) -> bool {
            if (lhs.size() != rhs.size()) {
                return lhs.size() > rhs.size();
            }
            return lhs < rhs;
        });
    for (const std::string& string : strings) {
        InsertFilename(string, stringTable, stringOffsets);
    }
}

static void Prettify(TreeNode& node,
                     std::string& stringTable,
                     std::vector<size_t>& stringOffsets,
                     std::vector<HyoutaUtils::Hash::SHA1>& hashTable) {
    SortChildrenRecursive(node);
    PreInsertHashes(node, hashTable);
    PreInsertNames(node, stringTable, stringOffsets);

    // sort hashes. i can't think of a real use case offhand but let's make these binary searchable,
    // why not
    std::sort(hashTable.begin(),
              hashTable.end(),
              [](const HyoutaUtils::Hash::SHA1& lhs, const HyoutaUtils::Hash::SHA1& rhs) -> bool {
                  return std::memcmp(lhs.Hash.data(), rhs.Hash.data(), lhs.Hash.size()) < 0;
              });
}

namespace {
struct Arg {
    std::string GamePath;
    size_t GameVersionBits;
    size_t DlcIndex;
};
struct ProgramArgs {
    std::vector<Arg> Input;
    std::string InternalInput;
    std::string Output;
};
} // namespace

static std::optional<ProgramArgs> ParseArgs(std::string_view jsonPath) {
    HyoutaUtils::IO::File f(jsonPath, HyoutaUtils::IO::OpenMode::Read);
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
    json.Parse<rapidjson::kParseFullPrecisionFlag | rapidjson::kParseNanAndInfFlag
                   | rapidjson::kParseCommentsFlag,
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
                auto gamePath = ReadString(obj, "GamePath");
                auto gameVersion = ReadUInt64(obj, "GameVersion");
                auto gameVersionBits = ReadUInt64(obj, "GameVersionBits");
                auto dlcIndex = ReadUInt64(obj, "DlcIndex");
                if (!gamePath || !dlcIndex) {
                    return std::nullopt;
                }
                if (!gameVersion && !gameVersionBits) {
                    return std::nullopt;
                }
                size_t bits = 0;
                if (gameVersion) {
                    bits |= (size_t(1) << (*gameVersion));
                }
                if (gameVersionBits) {
                    bits |= static_cast<size_t>(*gameVersionBits);
                }
                args.Input.emplace_back(Arg{.GamePath = std::move(*gamePath),
                                            .GameVersionBits = bits,
                                            .DlcIndex = *dlcIndex});
            } else {
                return std::nullopt;
            }
        }
    } else {
        return std::nullopt;
    }
    auto internalInput = ReadString(root, "InternalInput");
    if (internalInput) {
        args.InternalInput = std::move(*internalInput);
    }
    auto output = ReadString(root, "Output");
    if (!output) {
        return std::nullopt;
    }
    args.Output = std::move(*output);

    return args;
}

static bool InsertExistingDirTreeRecursive(TreeNode& node,
                                           const HyoutaUtils::DirTree::Tree& inputTree,
                                           const HyoutaUtils::DirTree::Entry& entry,
                                           bool dlc) {
    if (!dlc && entry.GetDlcIndex() != 0) {
        // don't insert DLC unless we want to insert DLC
        return true;
    }

    std::string_view filename(inputTree.StringTable + entry.GetFilenameOffset(),
                              entry.GetFilenameLength());
    if (entry.IsDirectory()) {
        TreeNode& dirnode =
            InsertDirectory(node, filename, entry.GetGameVersionBits(), entry.GetDlcIndex());

        const size_t childCount = entry.GetDirectoryNumberOfEntries();
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        for (size_t i = 0; i < childCount; ++i) {
            const auto& child = inputTree.Entries[firstChildIndex + i];
            if (!InsertExistingDirTreeRecursive(dirnode, inputTree, child, dlc)) {
                return false;
            }
        }
    } else {
        if ((!dlc && entry.GetDlcIndex() == 0) || (dlc && entry.GetDlcIndex() != 0)) {
            return InsertFile(node,
                              inputTree.HashTable[entry.GetFileHashIndex()],
                              filename,
                              entry.GetFileSize(),
                              entry.GetGameVersionBits(),
                              entry.GetDlcIndex());
        }
    }
    return true;
}

static bool
    InsertExistingDirTree(TreeNode& root, const HyoutaUtils::DirTree::Tree& inputTree, bool dlc) {
    if (inputTree.NumberOfEntries > 0) {
        const auto& entry = inputTree.Entries[0];
        root.GameVersionBits = entry.GetGameVersionBits();
        const size_t childCount = entry.GetDirectoryNumberOfEntries();
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        for (size_t i = 0; i < childCount; ++i) {
            const auto& child = inputTree.Entries[firstChildIndex + i];
            if (!InsertExistingDirTreeRecursive(root, inputTree, child, dlc)) {
                return false;
            }
        }
    }
    return true;
}

static bool InsertDirTreeEntry(TreeNode& node,
                               const std::filesystem::directory_entry& entry,
                               size_t gameVersionBits,
                               size_t dlcIndex) {
    {
        std::string fullpath = HyoutaUtils::IO::FilesystemPathToUtf8(entry.path());
        printf("Inserting %s\n", fullpath.c_str());
    }

    std::string filename = HyoutaUtils::IO::FilesystemPathToUtf8(entry.path().filename());
    if (entry.is_directory()) {
        TreeNode& dirnode = InsertDirectory(node, filename, gameVersionBits, dlcIndex);

        std::filesystem::directory_iterator iterator(entry.path());
        for (auto const& child : iterator) {
            if (!InsertDirTreeEntry(dirnode, child, gameVersionBits, dlcIndex)) {
                return false;
            }
        }
        return true;
    } else {
        HyoutaUtils::IO::File gamefile(entry.path(), HyoutaUtils::IO::OpenMode::Read);
        if (!gamefile.IsOpen()) {
            std::string fullpath = HyoutaUtils::IO::FilesystemPathToUtf8(entry.path());
            printf("Failed to open file %s\n", fullpath.c_str());
            return -1;
        }
        auto length = gamefile.GetLength();
        if (!length) {
            std::string fullpath = HyoutaUtils::IO::FilesystemPathToUtf8(entry.path());
            printf("Failed to get length of file %s\n", fullpath.c_str());
            return -1;
        }
        auto hash = HyoutaUtils::Hash::CalculateSHA1FromFile(gamefile);
        if (!hash) {
            std::string fullpath = HyoutaUtils::IO::FilesystemPathToUtf8(entry.path());
            printf("Failed to calculate hash of file %s\n", fullpath.c_str());
            return -1;
        }
        gamefile.Close();

        return InsertFile(node, *hash, filename, *length, gameVersionBits, dlcIndex);
    }
}

static bool InsertDirTreeFromFolder(TreeNode& root,
                                    const std::filesystem::path& folder,
                                    size_t gameVersionBits,
                                    size_t dlcIndex) {
    root.GameVersionBits |= gameVersionBits;

    std::filesystem::directory_iterator iterator(folder);
    for (auto const& entry : iterator) {
        if (!InsertDirTreeEntry(root, entry, gameVersionBits, dlcIndex)) {
            return false;
        }
    }
    return true;
}

int SHA_File_Convert_Function(int argc, char** argv) {
    if (argc <= 1) {
        printf("First argument must point to .json describing the data to convert.\n");
        return -1;
    }

    const auto& programArgs = ParseArgs(argv[1]);
    if (!programArgs) {
        printf("Couldn't parse .json file.\n");
        return -1;
    }

    const std::vector<Arg>& args = programArgs->Input;
    const std::string& internalInputKey = programArgs->InternalInput;
    const std::string& outputFilename = programArgs->Output;

    TreeNode root{.IsDirectory = true, .Hash = HyoutaUtils::Hash::SHA1({})};

    HyoutaUtils::DirTree::Tree internalInputTree{};
    if (internalInputKey.empty()) {
        // that's fine, just skip
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("CS1", internalInputKey)) {
        internalInputTree = SenLib::Sen1::GetDirTree();
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("CS2", internalInputKey)) {
        internalInputTree = SenLib::Sen2::GetDirTree();
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("TX", internalInputKey)) {
        internalInputTree = SenLib::TX::GetDirTree();
    } else {
        printf("Invalid internal input key.\n");
        return -1;
    }

    // we need to insert all non-DLC files before inserting the DLC files, otherwise the directories
    // might end up with wrong DLC indices
    for (size_t i = 0; i < 2; ++i) {
        bool dlc = (i != 0);
        if (!InsertExistingDirTree(root, internalInputTree, dlc)) {
            printf("Internal error.\n");
            return -1;
        }
        for (size_t i = 0; i < args.size(); ++i) {
            const auto& arg = args[i];
            if ((!dlc && arg.DlcIndex == 0) || (dlc && arg.DlcIndex != 0)) {
                if (!InsertDirTreeFromFolder(root,
                                             HyoutaUtils::IO::FilesystemPathFromUtf8(arg.GamePath),
                                             arg.GameVersionBits,
                                             arg.DlcIndex)) {
                    printf("Failed to insert %s\n", arg.GamePath.c_str());
                }
            }
        }
    }

    {
        std::string stringTable;
        std::vector<size_t> stringOffsets;
        std::vector<HyoutaUtils::Hash::SHA1> hashTable;
        Prettify(root, stringTable, stringOffsets, hashTable);
        size_t count = 1;
        root.ChildFirstIndex =
            root.Children.empty() ? static_cast<size_t>(0) : static_cast<size_t>(1);
        FillFirstChildIndex(root, count);
        size_t maxDlcIndex = GetMaxDlcIndex(root);

        // I previously had this printed all nice and tidy with std::array and constexpr and
        // properly calling HyoutaUtils::DirTree::Entry::MakeFile() and the like... but it turns out
        // MSVC is *really bad* at compiling this. It takes several minutes, consumes 25 GB of
        // memory, and Reverie's huge amount of files not only runs into the default constexpr step
        // limit for the Entries but also results in a "fatal error C1060: compiler is out of heap
        // space". So we'll be dumb and just pre-generate a huge byte array, and then
        // reinterpret_cast that to the correct data types, technical UB be damned.
        static constexpr bool generateRawBytes = true;

        std::string source;
        source.append("#pragma once\n\n");
        if (generateRawBytes) {
            source.append("#ifdef D\n");
            source.append("#undef D\n");
            source.append("#endif\n");
            source.append("#ifdef Q\n");
            source.append("#undef Q\n");
            source.append("#endif\n");

            source.append(
                "#if defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == "
                "__ORDER_BIG_ENDIAN__)\n");
            source.append("#define D(a,b,c,d) a,b,c,d\n");
            source.append("#define Q(a,b,c,d,e,f,g,h) a,b,c,d,e,f,g,h\n");
            source.append("#else\n");
            source.append("#define D(a,b,c,d) d,c,b,a\n");
            source.append("#define Q(a,b,c,d,e,f,g,h) h,g,f,e,d,c,b,a\n");
            source.append("#endif\n");
            source.append("alignas(16) static constexpr char s_raw_dirtree[] = {\n");
            if (!GenerateRawDirTreeLine(root, stringTable, stringOffsets, hashTable, source)) {
                printf("Dir tree generation failed.\n");
                return -1;
            }
            if (!GenerateRawDirTreeSource(root, stringTable, stringOffsets, hashTable, source)) {
                printf("Dir tree generation failed.\n");
                return -1;
            }
            source.push_back('\n');
            for (size_t i = 0; i < hashTable.size(); ++i) {
                for (size_t j = 0; j < 20; ++j) {
                    AppendChar(source, hashTable[i].Hash[j]);
                    source.push_back(',');
                }
                source.push_back('\n');
            }
            source.push_back('\n');
            {
                size_t o = 0;
                for (size_t i = 0; i < stringTable.size(); ++i) {
                    if (o < stringOffsets.size() && stringOffsets[o] == i) {
                        source.push_back('\n');
                        ++o;
                    }
                    AppendEscapedChar(source, stringTable[i]);
                    source.push_back(',');
                }
            }
            source.append("\n};\n");

            source.append("static constexpr size_t s_raw_dirtree_entry_count = ");
            source.append(std::to_string(count));
            source.append(";\n");
            source.append("static constexpr size_t s_raw_dirtree_hash_count = ");
            source.append(std::to_string(hashTable.size()));
            source.append(";\n");
            source.append("static constexpr size_t s_raw_dirtree_string_size = ");
            source.append(std::to_string(stringTable.size()));
            source.append(";\n");
            source.append("static constexpr size_t s_max_dlc_index = ");
            source.append(std::to_string(maxDlcIndex));
            source.append(";\n");
        } else {
            source.append("#include <array>\n\n");
            source.append("#include \"dirtree/entry.h\"\n");
            source.append("#include \"util/hash/sha1.h\"\n\n");
            source.append(
                "static constexpr auto s_dirtree = std::array<HyoutaUtils::DirTree::Entry, ");
            source.append(std::to_string(count));
            source.append(">{{\n");
            GenerateDirTreeLine(root, stringTable, stringOffsets, hashTable, source);
            GenerateDirTreeSource(root, stringTable, stringOffsets, hashTable, source);
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

            source.append(
                "static constexpr auto s_hashtable = std::array<HyoutaUtils::Hash::SHA1, ");
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

            source.append("static constexpr size_t s_max_dlc_index = ");
            source.append(std::to_string(maxDlcIndex));
            source.append(";\n");
        }

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
