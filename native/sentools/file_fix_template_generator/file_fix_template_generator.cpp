#include "file_fix_template_generator_main.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <format>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dirtree/dirtree_cs1.h"
#include "dirtree/dirtree_cs2.h"
#include "dirtree/dirtree_cs3.h"
#include "dirtree/dirtree_cs4.h"
#include "dirtree/dirtree_reverie.h"
#include "dirtree/dirtree_tx.h"
#include "dirtree/entry.h"
#include "dirtree/tree.h"
#include "util/args.h"
#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/scope.h"
#include "util/text.h"

namespace std {
template<>
struct formatter<HyoutaUtils::Hash::SHA1> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const HyoutaUtils::Hash::SHA1& hash, std::format_context& ctx) const {
        return std::format_to(ctx.out(),
                              "{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}"
                              "{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
                              hash.Hash[0],
                              hash.Hash[1],
                              hash.Hash[2],
                              hash.Hash[3],
                              hash.Hash[4],
                              hash.Hash[5],
                              hash.Hash[6],
                              hash.Hash[7],
                              hash.Hash[8],
                              hash.Hash[9],
                              hash.Hash[10],
                              hash.Hash[11],
                              hash.Hash[12],
                              hash.Hash[13],
                              hash.Hash[14],
                              hash.Hash[15],
                              hash.Hash[16],
                              hash.Hash[17],
                              hash.Hash[18],
                              hash.Hash[19]);
    }
};
} // namespace std

namespace {
// should return false to stop looping, true to continue
using ForEachFunc = std::function<bool(const std::string& path,
                                       const HyoutaUtils::DirTree::Tree& dirtree,
                                       const HyoutaUtils::DirTree::Entry& entry)>;

static bool ForEachFileInternal(const std::optional<uint32_t>& versions,
                                const std::optional<size_t>& dlcIndex,
                                const HyoutaUtils::DirTree::Tree& dirtree,
                                size_t firstDirTreeIndex,
                                size_t numberOfEntries,
                                std::string& path,
                                const ForEachFunc& callback) {
    size_t pathElementLength = std::numeric_limits<size_t>::max();
    auto pathScope = HyoutaUtils::MakeScopeGuard([&]() {
        if (pathElementLength != std::numeric_limits<size_t>::max()) {
            path.resize(path.size() - pathElementLength);
        }
    });
    for (size_t i = 0; i < numberOfEntries; ++i) {
        const size_t dirTreeIndex = firstDirTreeIndex + i;

        // check files only if they're in the current DLC
        // check folders if they're in the base game or in the current DLC
        const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[dirTreeIndex];
        if (dlcIndex.has_value()
            && !(entry.GetDlcIndex() == *dlcIndex
                 || (entry.IsDirectory() && entry.GetDlcIndex() == 0))) {
            continue;
        }
        if (versions.has_value() && (*versions & entry.GetGameVersionBits()) == 0) {
            // no overlap between possible game versions and the game versions this entry is in
            continue;
        }

        std::string_view filename(dirtree.StringTable + entry.GetFilenameOffset(),
                                  entry.GetFilenameLength());
        if (pathElementLength == std::numeric_limits<size_t>::max()) {
            path.push_back('/');
            path.append(filename);
            pathElementLength = filename.size() + 1;
        }
        if (entry.IsDirectory()) {
            // descend
            HyoutaUtils::DirTree::DirectoryIterationState dir;
            const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
            while (HyoutaUtils::DirTree::GetNextEntries(dir, dirtree, entry)) {
                if (!ForEachFileInternal(versions,
                                         dlcIndex,
                                         dirtree,
                                         dir.Begin + firstChildIndex,
                                         dir.End - dir.Begin,
                                         path,
                                         callback)) {
                    return false;
                }
            }
        } else {
            if (!callback(path, dirtree, entry)) {
                return false;
            }
        }
    }
    return true;
}

static bool ForEachFile(const std::optional<uint32_t>& versions,
                        const std::optional<size_t>& dlcIndex,
                        const HyoutaUtils::DirTree::Tree& dirtree,
                        const ForEachFunc& callback) {
    std::string path;
    if (dirtree.NumberOfEntries > 0) {
        const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[0];
        HyoutaUtils::DirTree::DirectoryIterationState dir;
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        while (HyoutaUtils::DirTree::GetNextEntries(dir, dirtree, entry)) {
            if (!ForEachFileInternal(versions,
                                     dlcIndex,
                                     dirtree,
                                     dir.Begin + firstChildIndex,
                                     dir.End - dir.Begin,
                                     path,
                                     callback)) {
                return false;
            }
        }
    }
    return true;
}

void GenerateTemplateFile(std::string_view outname,
                          const std::string& path,
                          const HyoutaUtils::DirTree::Tree& dirtree,
                          const HyoutaUtils::DirTree::Entry& entry,
                          int gameid) {
    if (HyoutaUtils::IO::Exists(outname) != HyoutaUtils::IO::ExistsResult::DoesNotExist) {
        return;
    }

    std::string_view name = HyoutaUtils::IO::GetFileName(path);
    std::string_view nameNoExt = HyoutaUtils::IO::GetFileNameWithoutExtension(path);
    std::string classname = HyoutaUtils::TextUtils::Replace(name, ".", "_");
    std::string_view relativePath =
        (path.starts_with("/")) ? std::string_view(path).substr(1) : std::string_view(path);
    std::string namespacename = "";
    switch (gameid) {
        case 1: namespacename = "Sen1"; break;
        case 2: namespacename = "Sen2"; break;
        case 3: namespacename = "Sen3"; break;
        case 4: namespacename = "Sen4"; break;
        case 5: namespacename = "Sen5"; break;
        case 6: namespacename = "TX"; break;
    }

    std::string data = std::format(
        "#include <string_view>\n"
        "#include <vector>\n"
        "\n"
        "#include \"p3a/pack.h\"\n"
        "#include \"p3a/structs.h\"\n"
        "#include \"sen/file_getter.h\"\n"
        "#include \"sen/sen_script_patcher.h\"\n"
        "#include \"util/hash/sha1.h\"\n"
        "\n"
        "extern \"C\" {{\n"
        "__declspec(dllexport) char SenPatcherFix_1_{5}[] = \"\";\n"
        "}}\n"
        "\n"
        "namespace SenLib::{0}::FileFixes::{1} {{\n"
        "bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,\n"
        "              std::vector<SenPatcher::P3APackFile>& result) {{\n"
        "    try {{\n"
        "        auto file = getCheckedFile(\n"
        "            \"{2}\",\n"
        "            {3},\n"
        "            HyoutaUtils::Hash::SHA1FromHexString(\"{4}\"));\n"
        "        if (!file) {{\n"
        "            return false;\n"
        "        }}\n"
        "\n"
        "        auto& bin = file->Data;\n"
        "        SenScriptPatcher patcher(bin);\n"
        "\n"
        "\n"
        "        result.emplace_back(std::move(bin), file->Filename, "
        "SenPatcher::P3ACompressionType::LZ4);\n"
        "\n"
        "        return true;\n"
        "    }} catch (...) {{\n"
        "        return false;\n"
        "    }}\n"
        "}}\n"
        "}} // namespace SenLib::{0}::FileFixes::{1}\n"
        "\n",
        namespacename,
        classname,
        relativePath,
        entry.GetFileSize(),
        dirtree.HashTable[entry.GetFileHashIndex()],
        nameNoExt);

    HyoutaUtils::IO::WriteFileAtomic(outname, data.data(), data.size());
}
} // namespace

namespace SenTools {
int File_Fix_Template_Generator_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_game{.Type = HyoutaUtils::ArgTypes::String,
                                               .ShortKey = "g",
                                               .LongKey = "game",
                                               .Argument = "GAME",
                                               .Description =
                                                   "Game identifier. 'CS[n]', 'Reverie', 'TX'."};

    static constexpr std::array<const HyoutaUtils::Arg*, 1> args_array{{&arg_game}};
    static constexpr HyoutaUtils::Args args("sentools " File_Fix_Template_Generator_Name,
                                            "filename",
                                            File_Fix_Template_Generator_ShortDescription,
                                            args_array);
    auto parseResult = args.Parse(argc, argv);
    if (parseResult.IsError()) {
        printf("Argument error: %s\n\n\n", parseResult.GetErrorValue().c_str());
        args.PrintUsage();
        return -1;
    }

    const auto& options = parseResult.GetSuccessValue();
    if (options.FreeArguments.size() == 0) {
        printf("Argument error: %s\n\n\n", "No filename given.");
        args.PrintUsage();
        return -1;
    }

    const std::string_view* game = options.TryGetString(&arg_game);
    if (game == nullptr) {
        printf("Argument error: %s\n\n\n", "No game given.");
        args.PrintUsage();
        return -1;
    }
    HyoutaUtils::DirTree::Tree dirtree;
    int gameid = 0;
    if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "CS1")
        || HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "Sen1")) {
        dirtree = SenLib::Sen1::GetDirTree();
        gameid = 1;
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "CS2")
               || HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "Sen2")) {
        dirtree = SenLib::Sen2::GetDirTree();
        gameid = 2;
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "CS3")
               || HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "Sen3")) {
        dirtree = SenLib::Sen3::GetDirTree();
        gameid = 3;
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "CS4")
               || HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "Sen4")) {
        dirtree = SenLib::Sen4::GetDirTree();
        gameid = 4;
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "CS5")
               || HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "Sen5")
               || HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "Reverie")) {
        dirtree = SenLib::Sen5::GetDirTree();
        gameid = 5;
    } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(*game, "TX")) {
        dirtree = SenLib::TX::GetDirTree();
        gameid = 6;
    } else {
        printf("Argument error: '%.*s' is not a valid game.\n\n\n",
               static_cast<int>(game->size()),
               game->data());
        args.PrintUsage();
        return -1;
    }

    // always just pick the latest version
    uint32_t versionBitmask = 1u << (dirtree.NumberOfVersions - 1);
    for (const std::string_view& filename : options.FreeArguments) {
        ForEachFile(versionBitmask,
                    std::nullopt,
                    dirtree,
                    [&](const std::string& path,
                        const HyoutaUtils::DirTree::Tree& dirtree,
                        const HyoutaUtils::DirTree::Entry& entry) -> bool {
                        if (HyoutaUtils::TextUtils::CaseInsensitiveContains(path, filename)) {
                            std::string_view name = HyoutaUtils::IO::GetFileName(path);
                            std::string cppname =
                                HyoutaUtils::TextUtils::Replace(name, ".", "_") + ".cpp";
                            GenerateTemplateFile(cppname, path, dirtree, entry, gameid);
                        }
                        return true;
                    });
    }

    return 0;
}
} // namespace SenTools
