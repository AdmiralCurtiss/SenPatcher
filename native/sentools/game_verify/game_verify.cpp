#include "game_verify_main.h"

#include <cstdint>
#include <filesystem>
#include <limits>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

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
#include "util/text.h"

namespace SenTools {
static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();
struct VerifiedEntry {
    // Index into the dirtree.
    size_t DirTreeIndex = 0;

    // For files, this is set to INVALID_INDEX.
    // For directories, this is initially set to INVALID_INDEX.
    // Once this directory has verified children, it is set to the index into the
    // VerificationStorage.Directories that stores the children for this entry.
    size_t VerifiedChildrenIndex = INVALID_INDEX;
};
struct CaseInsensitiveHash {
    size_t operator()(const std::string_view& sv) const {
        size_t hash = 31;
        for (size_t i = 0; i < sv.size(); ++i) {
            char c = (sv[i] >= 'A' && sv[i] <= 'Z') ? (sv[i] + ('a' - 'A')) : sv[i];
            hash = (hash * 7) + static_cast<uint8_t>(c);
        }
        return hash;
    }
};
struct CaseInsensitiveEquals {
    bool operator()(const std::string_view& lhs, const std::string_view& rhs) const {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        for (size_t i = 0; i < lhs.size(); ++i) {
            const char cl = (lhs[i] >= 'A' && lhs[i] <= 'Z') ? (lhs[i] + ('a' - 'A')) : lhs[i];
            const char cr = (rhs[i] >= 'A' && rhs[i] <= 'Z') ? (rhs[i] + ('a' - 'A')) : rhs[i];
            if (cl != cr) {
                return false;
            }
        }
        return true;
    }
};
struct VerificationDirectory {
    // key == filename
    std::unordered_map<std::string_view, VerifiedEntry, CaseInsensitiveHash, CaseInsensitiveEquals>
        Entries;
};
struct VerificationStorage {
    VerifiedEntry Root;
    std::vector<VerificationDirectory> Directories;
};

static size_t AddOrGetVerificationDirectoryIndex(VerificationStorage* verificationStorage,
                                                 VerifiedEntry* verificationEntry) {
    if (verificationEntry->VerifiedChildrenIndex == INVALID_INDEX) {
        verificationEntry->VerifiedChildrenIndex = verificationStorage->Directories.size();
        verificationStorage->Directories.emplace_back();
    }
    return verificationEntry->VerifiedChildrenIndex;
}

static VerifiedEntry* AddOrGetVerifiedEntry(size_t dirTreeIndex,
                                            std::string_view filename,
                                            VerificationStorage* verificationStorage,
                                            VerifiedEntry* verificationEntry) {
    VerifiedEntry* childVerificationEntry = nullptr;
    if (verificationStorage && verificationEntry) {
        size_t verificationDirectoryIndex =
            AddOrGetVerificationDirectoryIndex(verificationStorage, verificationEntry);
        childVerificationEntry =
            &(verificationStorage->Directories[verificationDirectoryIndex]
                  .Entries.try_emplace(filename, VerifiedEntry{.DirTreeIndex = dirTreeIndex})
                  .first->second);
    }
    return childVerificationEntry;
}

static VerifiedEntry* GetVerifiedEntry(std::string_view filename,
                                       VerificationStorage* verificationStorage,
                                       VerifiedEntry* verificationEntry) {
    if (verificationStorage && verificationEntry) {
        size_t verificationDirectoryIndex = verificationEntry->VerifiedChildrenIndex;
        if (verificationDirectoryIndex == INVALID_INDEX) {
            return nullptr;
        }
        auto& entries = verificationStorage->Directories[verificationDirectoryIndex].Entries;
        auto it = entries.find(filename);
        if (it != entries.end()) {
            return &it->second;
        }
    }
    return nullptr;
}

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
                            size_t dirTreeIndex,
                            const std::filesystem::path& parentPath,
                            VerificationStorage* verificationStorage,
                            VerifiedEntry* verificationEntry) {
    const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[dirTreeIndex];
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
            VerifiedEntry* childVerificationEntry = AddOrGetVerifiedEntry(
                dirTreeIndex, filename, verificationStorage, verificationEntry);

            // descend into the directory
            const size_t childCount = entry.GetDirectoryNumberOfEntries();
            const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
            for (size_t i = 0; i < childCount; ++i) {
                VerifyGameFiles(possibleVersions,
                                dirtree,
                                firstChildIndex + i,
                                currentPath,
                                verificationStorage,
                                childVerificationEntry);
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
        if (VerifyFile(dirtree, entry, currentPath)) {
            // printf("File %.*s verifies.\n", static_cast<int>(filename.size()), filename.data());
            AddOrGetVerifiedEntry(dirTreeIndex, filename, verificationStorage, verificationEntry);
        } else {
            // file does not verify, this can't be any game version matching this entry
            // printf("File %.*s does not verify.\n", static_cast<int>(filename.size()),
            //        filename.data());
            possibleVersions &= ~(entry.GetGameVersionBits());
            // printf("Possible versions -> %u\n", possibleVersions);
        }
    }
}

static uint32_t VerifyGameFilesFromRoot(const HyoutaUtils::DirTree::Tree& dirtree,
                                        const std::filesystem::path& gamePath,
                                        VerificationStorage* verificationStorage) {
    uint32_t possibleVersions = ((1u << dirtree.NumberOfVersions) - 1);
    VerifiedEntry* verifiedEntry = verificationStorage ? &verificationStorage->Root : nullptr;
    if (dirtree.NumberOfEntries > 0) {
        const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[0];
        const size_t childCount = entry.GetDirectoryNumberOfEntries();
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        for (size_t i = 0; i < childCount; ++i) {
            VerifyGameFiles(possibleVersions,
                            dirtree,
                            firstChildIndex + i,
                            gamePath,
                            verificationStorage,
                            verifiedEntry);
        }
    }
    return possibleVersions;
}

static bool VerifyDlc(const uint32_t possibleVersions,
                      const HyoutaUtils::DirTree::Tree& dirtree,
                      size_t dirTreeIndex,
                      const std::filesystem::path& parentPath,
                      const size_t dlcIndex,
                      VerificationStorage* verificationStorage,
                      VerifiedEntry* verificationEntry) {
    // check files only if they're in the current DLC
    // check folders if they're in the base game or in the current DLC
    const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[dirTreeIndex];
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
        VerifiedEntry* childVerificationEntry;
        if (entry.GetDlcIndex() == dlcIndex) {
            // this folder is exclusive to this DLC, need to verify it exists
            const bool dirExists = HyoutaUtils::IO::DirectoryExists(currentPath);
            if (!dirExists) {
                return false;
            }
            childVerificationEntry = AddOrGetVerifiedEntry(
                dirTreeIndex, filename, verificationStorage, verificationEntry);
        } else {
            childVerificationEntry =
                GetVerifiedEntry(filename, verificationStorage, verificationEntry);
        }

        // then descend and check the children
        // printf("DLC descending into %.*s.\n", static_cast<int>(filename.size()),
        //        filename.data());
        const size_t childCount = entry.GetDirectoryNumberOfEntries();
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        for (size_t i = 0; i < childCount; ++i) {
            if (!VerifyDlc(possibleVersions,
                           dirtree,
                           firstChildIndex + i,
                           currentPath,
                           dlcIndex,
                           verificationStorage,
                           childVerificationEntry)) {
                return false;
            }
        }
        return true;
    } else {
        if (VerifyFile(dirtree, entry, currentPath)) {
            // printf("DLC File %.*s verifies.\n", static_cast<int>(filename.size()),
            //        filename.data());
            AddOrGetVerifiedEntry(dirTreeIndex, filename, verificationStorage, verificationEntry);
            return true;
        } else {
            // printf("DLC File %.*s does not verify.\n",
            //        static_cast<int>(filename.size()),
            //        filename.data());
            return false;
        }
    }
}

static bool VerifyDlcFromRoot(const uint32_t possibleVersions,
                              const size_t dlcIndex,
                              const HyoutaUtils::DirTree::Tree& dirtree,
                              const std::filesystem::path& gamePath,
                              VerificationStorage* verificationStorage) {
    VerifiedEntry* verifiedEntry = verificationStorage ? &verificationStorage->Root : nullptr;
    if (dirtree.NumberOfEntries > 0) {
        const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[0];
        const size_t childCount = entry.GetDirectoryNumberOfEntries();
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        for (size_t i = 0; i < childCount; ++i) {
            if (!VerifyDlc(possibleVersions,
                           dirtree,
                           firstChildIndex + i,
                           gamePath,
                           dlcIndex,
                           verificationStorage,
                           verifiedEntry)) {
                return false;
            }
        }
    }
    return true;
}

static void ListExtraFiles(const std::filesystem::path& parentPath,
                           VerificationStorage* verificationStorage,
                           VerifiedEntry* verificationEntry) {
    std::error_code ec;
    std::filesystem::directory_iterator it(parentPath, ec);
    if (ec) {
        printf("Error while iterating %s\n",
               HyoutaUtils::IO::FilesystemPathToUtf8(parentPath).c_str());
        return;
    }
    while (it != std::filesystem::directory_iterator()) {
        VerifiedEntry* entry =
            GetVerifiedEntry(HyoutaUtils::IO::FilesystemPathToUtf8(it->path().filename()),
                             verificationStorage,
                             verificationEntry);
        if (!entry) {
            printf("Not part of the game files: %s\n",
                   HyoutaUtils::IO::FilesystemPathToUtf8(it->path()).c_str());
        }
        if (it->is_directory()) {
            ListExtraFiles(it->path(), verificationStorage, entry);
        }

        it.increment(ec);
        if (ec) {
            printf("Error while iterating %s\n",
                   HyoutaUtils::IO::FilesystemPathToUtf8(parentPath).c_str());
            return;
        }
    }
}

static bool VerifyGame(const HyoutaUtils::DirTree::Tree& dirtree,
                       const char* gameName,
                       const std::filesystem::path& gamePath) {
    VerificationStorage verificationStorage;
    uint32_t possibleVersions = VerifyGameFilesFromRoot(dirtree, gamePath, &verificationStorage);
    if (possibleVersions == 0) {
        return false;
    }

    for (size_t i = 0; i < dirtree.NumberOfVersions; ++i) {
        if (possibleVersions & (1u << i)) {
            printf("Matches %s version %s.\n", gameName, dirtree.VersionNames[i]);
        }
    }

    for (size_t i = 0; i < dirtree.NumberOfDlcs; ++i) {
        if (VerifyDlcFromRoot(possibleVersions, i + 1, dirtree, gamePath, &verificationStorage)) {
            printf("DLC %s is installed.\n", dirtree.DlcNames[i]);
        } else {
            printf("DLC %s is not installed.\n", dirtree.DlcNames[i]);
        }
    }

    ListExtraFiles(gamePath, &verificationStorage, &verificationStorage.Root);

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
