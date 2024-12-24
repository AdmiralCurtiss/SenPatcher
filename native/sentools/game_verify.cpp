#include "game_verify.h"

#include <cstdint>
#include <filesystem>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "dirtree/dirtree_cs1.h"
#include "dirtree/dirtree_cs2.h"
#include "dirtree/dirtree_cs3.h"
#include "dirtree/dirtree_cs4.h"
#include "dirtree/dirtree_reverie.h"
#include "dirtree/dirtree_tx.h"
#include "dirtree/entry.h"
#include "dirtree/tree.h"
#include "util/file.h"
#include "util/hash/sha1.h"

namespace SenTools {
static bool VerifyFile(const HyoutaUtils::DirTree::Tree& dirtree,
                       const HyoutaUtils::DirTree::Entry& entry,
                       const std::filesystem::path& path) {
    HyoutaUtils::IO::File file(path, HyoutaUtils::IO::OpenMode::Read);
    if (!file.IsOpen()) {
        // printf("Could not open file.\n");
        return false;
    }
    auto hash = HyoutaUtils::Hash::CalculateSHA1FromFile(file);
    if (!hash) {
        // printf("Hash calculation error.\n");
        return false;
    }
    if (*hash != dirtree.HashTable[entry.GetFileHashIndex()]) {
        // printf("Hash mismatch.\n");
        return false;
    }
    return true;
}

static void VerifyGameFiles(uint32_t& possibleVersions,
                            const HyoutaUtils::DirTree::Tree& dirtree,
                            const HyoutaUtils::DirTree::Entry& entry,
                            const std::filesystem::path& parentPath) {
    if (entry.GetDlcIndex() != 0) {
        // ignore DLC for verifying the base game
        return;
    }
    if ((possibleVersions & entry.GetGameVersionBits()) == 0) {
        // no overlap between remaining game versions and the game versions this entry is in
        return;
    }

    std::string_view filename(dirtree.StringTable + entry.GetFilenameOffset(),
                              entry.GetFilenameLength());
    std::filesystem::path currentPath =
        parentPath / std::u8string_view((const char8_t*)filename.data(), filename.size());
    if (entry.IsDirectory()) {
        const bool dirExists = HyoutaUtils::IO::DirectoryExists(currentPath);
        if (dirExists) {
            // directory exists
            // printf("Directory %.*s exists.\n", static_cast<int>(filename.size()),
            //        filename.data());

            // descend into the directory
            const size_t childCount = entry.GetDirectoryNumberOfEntries();
            const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
            for (size_t i = 0; i < childCount; ++i) {
                const auto& child = dirtree.Entries[firstChildIndex + i];
                VerifyGameFiles(possibleVersions, dirtree, child, currentPath);
            }
        } else {
            // directory does not exist, this can't be any game version matching this entry
            // printf("Directory %.*s does not exist.\n",
            //        static_cast<int>(filename.size()),
            //        filename.data());
            possibleVersions &= ~(entry.GetGameVersionBits());
            // printf("Possible versions -> %u\n", possibleVersions);
        }
    } else {
        if (!VerifyFile(dirtree, entry, currentPath)) {
            // file does not verify, this can't be any game version matching this entry
            // printf("File %.*s does not verify.\n", static_cast<int>(filename.size()),
            //        filename.data());
            possibleVersions &= ~(entry.GetGameVersionBits());
            // printf("Possible versions -> %u\n", possibleVersions);
        } else {
            // printf("File %.*s verifies.\n", static_cast<int>(filename.size()), filename.data());
        }
    }
}

static bool VerifyDlc(const uint32_t possibleVersions,
                      const HyoutaUtils::DirTree::Tree& dirtree,
                      const HyoutaUtils::DirTree::Entry& entry,
                      const std::filesystem::path& parentPath,
                      const size_t dlcIndex) {
    // check files only if they're in the current DLC
    // check folders if they're in the base game or in the current DLC
    if (!(entry.GetDlcIndex() == dlcIndex || (entry.IsDirectory() && entry.GetDlcIndex() == 0))) {
        return true;
    }
    if ((possibleVersions & entry.GetGameVersionBits()) == 0) {
        // no overlap between possible game versions and the game versions this entry is in
        return true;
    }

    std::string_view filename(dirtree.StringTable + entry.GetFilenameOffset(),
                              entry.GetFilenameLength());
    std::filesystem::path currentPath =
        parentPath / std::u8string_view((const char8_t*)filename.data(), filename.size());
    if (entry.IsDirectory()) {
        if (entry.GetDlcIndex() == dlcIndex) {
            // this folder is exclusive to this DLC, need to verify it exists
            const bool dirExists = HyoutaUtils::IO::DirectoryExists(currentPath);
            if (!dirExists) {
                return false;
            }
        }

        // then descend and check the children
        // printf("DLC descending into %.*s.\n", static_cast<int>(filename.size()),
        //        filename.data());
        const size_t childCount = entry.GetDirectoryNumberOfEntries();
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        for (size_t i = 0; i < childCount; ++i) {
            const auto& child = dirtree.Entries[firstChildIndex + i];
            if (!VerifyDlc(possibleVersions, dirtree, child, currentPath, dlcIndex)) {
                return false;
            }
        }
        return true;
    } else {
        if (VerifyFile(dirtree, entry, currentPath)) {
            // printf("DLC File %.*s verifies.\n", static_cast<int>(filename.size()),
            //        filename.data());
            return true;
        } else {
            // printf("DLC File %.*s does not verify.\n",
            //        static_cast<int>(filename.size()),
            //        filename.data());
            return false;
        }
    }
}

static bool VerifyGame(const HyoutaUtils::DirTree::Tree& dirtree,
                       const char* gameName,
                       const std::filesystem::path& gamepath) {
    uint32_t possibleVersions = ((1u << dirtree.NumberOfVersions) - 1);
    if (dirtree.NumberOfEntries > 0) {
        VerifyGameFiles(possibleVersions, dirtree, dirtree.Entries[0], gamepath);
    }

    if (possibleVersions == 0) {
        return false;
    }

    for (size_t i = 0; i < dirtree.NumberOfVersions; ++i) {
        if (possibleVersions & (1u << i)) {
            printf("Matches %s version %s.\n", gameName, dirtree.VersionNames[i]);
        }
    }

    for (size_t i = 0; i < dirtree.NumberOfDlcs; ++i) {
        if (VerifyDlc(possibleVersions, dirtree, dirtree.Entries[0], gamepath, i + 1)) {
            printf("DLC %s is installed.\n", dirtree.DlcNames[i]);
        } else {
            printf("DLC %s is not installed.\n", dirtree.DlcNames[i]);
        }
    }

    return true;
}

int Game_Verify_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(Game_Verify_ShortDescription);
    parser.usage("sentools " Game_Verify_Name " [options] directory");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }
    const auto gamepath = HyoutaUtils::IO::FilesystemPathFromUtf8(args[0]);

    bool found = false;
    found = found || VerifyGame(SenLib::Sen1::GetDirTree(), "Trails of Cold Steel", gamepath);
    found = found || VerifyGame(SenLib::Sen2::GetDirTree(), "Trails of Cold Steel II", gamepath);
    found = found || VerifyGame(SenLib::Sen3::GetDirTree(), "Trails of Cold Steel III", gamepath);
    found = found || VerifyGame(SenLib::Sen4::GetDirTree(), "Trails of Cold Steel IV", gamepath);
    found = found || VerifyGame(SenLib::Sen5::GetDirTree(), "Trails into Reverie", gamepath);
    found = found || VerifyGame(SenLib::TX::GetDirTree(), "Tokyo Xanadu eX+", gamepath);

    if (!found) {
        printf("No known game or game version matches.\n");
    }

    return 0;
}
} // namespace SenTools
