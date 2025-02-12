#include "game_verify.h"

#include <cstdint>
#include <filesystem>
#include <functional>
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
#include "game_verify_main.h"
#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/text.h"

namespace SenTools::GameVerify {
static size_t AddOrGetVerificationDirectoryIndex(VerificationStorage* verificationStorage,
                                                 VerifiedEntry* verificationEntry) {
    if (verificationEntry->VerifiedChildrenIndex == INVALID_INDEX) {
        verificationEntry->VerifiedChildrenIndex = verificationStorage->Directories.size();
        verificationStorage->Directories.emplace_back(std::make_unique<VerificationDirectory>());
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
                  ->Entries.try_emplace(filename, VerifiedEntry{.DirTreeIndex = dirTreeIndex})
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
        auto& entries = verificationStorage->Directories[verificationDirectoryIndex]->Entries;
        auto it = entries.find(filename);
        if (it != entries.end()) {
            return &it->second;
        }
    }
    return nullptr;
}

enum class VerifyFileStateEnum {
    NotChecked,
    FileExists,
    LengthRead,
    HashCalculated,

    FileDoesNotExist,
    FileOpeningFailed,
    LengthReadingFailed,
    HashCalculationFailed,
};
struct VerifyFileState {
    VerifyFileStateEnum State = VerifyFileStateEnum::NotChecked;
    HyoutaUtils::IO::File File;
    uint64_t Length;
    HyoutaUtils::Hash::SHA1 Hash;
};
static bool VerifyFile(VerifyFileState& state,
                       const HyoutaUtils::DirTree::Tree& dirtree,
                       const HyoutaUtils::DirTree::Entry& entry,
                       const std::filesystem::path& path,
                       bool onlyCheckForExistence) {
    if (onlyCheckForExistence) {
        if (state.State == VerifyFileStateEnum::NotChecked) {
            state.State = HyoutaUtils::IO::FileExists(path) ? VerifyFileStateEnum::FileExists
                                                            : VerifyFileStateEnum::FileDoesNotExist;
        }
        return state.State == VerifyFileStateEnum::FileExists;
    }

    switch (state.State) {
        case VerifyFileStateEnum::NotChecked:
        case VerifyFileStateEnum::FileExists: {
            state.File.Open(path, HyoutaUtils::IO::OpenMode::Read);
            if (!state.File.IsOpen()) {
                // printf("Could not open file.\n");
                state.State = VerifyFileStateEnum::FileOpeningFailed;
                state.File.Close();
                return false;
            }
            auto length = state.File.GetLength();
            if (!length) {
                state.State = VerifyFileStateEnum::LengthReadingFailed;
                state.File.Close();
                return false;
            }
            state.Length = *length;
            state.State = VerifyFileStateEnum::LengthRead;
            [[fallthrough]];
        }
        case VerifyFileStateEnum::LengthRead: {
            if (state.Length != entry.GetFileSize()) {
                return false;
            }
            auto hash = HyoutaUtils::Hash::CalculateSHA1FromFile(state.File);
            if (!hash) {
                // printf("Hash calculation error.\n");
                state.State = VerifyFileStateEnum::HashCalculationFailed;
                state.File.Close();
                return false;
            }
            state.Hash = *hash;
            state.State = VerifyFileStateEnum::HashCalculated;
            state.File.Close();
            if (state.Hash != dirtree.HashTable[entry.GetFileHashIndex()]) {
                // printf("Hash mismatch.\n");
                return false;
            }
            return true;
        }
        case VerifyFileStateEnum::HashCalculated: {
            if (state.Length != entry.GetFileSize()) {
                return false;
            }
            if (state.Hash != dirtree.HashTable[entry.GetFileHashIndex()]) {
                // printf("Hash mismatch.\n");
                return false;
            }
            return true;
        }
        default: {
            return false;
        }
    }
}

enum class VerifyDirectoryStateEnum {
    NotChecked,
    DirectoryExists,
    DirectoryDoesNotExist,
};
struct VerifyDirectoryState {
    VerifyDirectoryStateEnum State = VerifyDirectoryStateEnum::NotChecked;
};
static bool VerifyDirectory(VerifyDirectoryState& state, const std::filesystem::path& path) {
    if (state.State == VerifyDirectoryStateEnum::NotChecked) {
        state.State = HyoutaUtils::IO::DirectoryExists(path)
                          ? VerifyDirectoryStateEnum::DirectoryExists
                          : VerifyDirectoryStateEnum::DirectoryDoesNotExist;
    }
    return state.State == VerifyDirectoryStateEnum::DirectoryExists;
}

// [firstDirTreeIndex, firstDirTreeIndex + numberOfEntries) must all point at entries that describe
// possible variants of the same file; that is, same filename and contained directory
static void VerifyGameFiles(uint32_t& possibleVersions,
                            const HyoutaUtils::DirTree::Tree& dirtree,
                            size_t firstDirTreeIndex,
                            size_t numberOfEntries,
                            const std::filesystem::path& parentPath,
                            VerifyMode verifyMode,
                            VerificationStorage* verificationStorage,
                            VerifiedEntry* verificationEntry) {
    VerifyDirectoryState verifyDirectoryState;
    VerifyFileState verifyFileState;
    std::optional<std::filesystem::path> currentPath;
    for (size_t i = 0; i < numberOfEntries; ++i) {
        const size_t dirTreeIndex = firstDirTreeIndex + i;
        const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[dirTreeIndex];
        if (entry.GetDlcIndex() != 0) {
            // ignore DLC for verifying the base game
            continue;
        }
        if ((possibleVersions & entry.GetGameVersionBits()) == 0) {
            // no overlap between remaining game versions and the game versions this entry is in
            continue;
        }

        std::string_view filename(dirtree.StringTable + entry.GetFilenameOffset(),
                                  entry.GetFilenameLength());
        if (verifyMode != VerifyMode::Full) {
            // is this an executable or directory that could contain an executable?
            const bool relevant =
                (entry.IsDirectory()
                     ? (filename == "bin" || filename == "Win32" || filename == "Win64"
                        || filename == "x64")
                     : (HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches(filename, "*.exe")
                        || HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches(filename, "*.dll")));
            if (!relevant) {
                continue;
            }
        }

        if (!currentPath) {
            currentPath =
                parentPath / std::u8string_view((const char8_t*)filename.data(), filename.size());
        }
        if (entry.IsDirectory()) {
            const bool dirExists = VerifyDirectory(verifyDirectoryState, *currentPath);
            if (dirExists) {
                // directory exists
                // printf("Directory %.*s exists.\n", static_cast<int>(filename.size()),
                //        filename.data());
                VerifiedEntry* childVerificationEntry = AddOrGetVerifiedEntry(
                    dirTreeIndex, filename, verificationStorage, verificationEntry);

                // descend into the directory
                HyoutaUtils::DirTree::DirectoryIterationState dir;
                const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
                while (HyoutaUtils::DirTree::GetNextEntries(dir, dirtree, entry)) {
                    VerifyGameFiles(possibleVersions,
                                    dirtree,
                                    dir.Begin + firstChildIndex,
                                    dir.End - dir.Begin,
                                    *currentPath,
                                    verifyMode,
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
            if (VerifyFile(verifyFileState,
                           dirtree,
                           entry,
                           *currentPath,
                           verifyMode == VerifyMode::IdentifyDirtree)) {
                // printf("File %.*s verifies.\n", static_cast<int>(filename.size()),
                //        filename.data());
                AddOrGetVerifiedEntry(
                    dirTreeIndex, filename, verificationStorage, verificationEntry);
            } else {
                // file does not verify, this can't be any game version matching this entry
                // printf("File %.*s does not verify.\n", static_cast<int>(filename.size()),
                //        filename.data());
                possibleVersions &= ~(entry.GetGameVersionBits());
                // printf("Possible versions -> %u\n", possibleVersions);
            }
        }
    }
}

static uint32_t VerifyGameFilesFromRoot(const HyoutaUtils::DirTree::Tree& dirtree,
                                        const std::filesystem::path& gamePath,
                                        VerifyMode verifyMode,
                                        VerificationStorage* verificationStorage) {
    uint32_t possibleVersions = ((1u << dirtree.NumberOfVersions) - 1);
    VerifiedEntry* verifiedEntry = verificationStorage ? &verificationStorage->Root : nullptr;
    if (dirtree.NumberOfEntries > 0) {
        const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[0];
        HyoutaUtils::DirTree::DirectoryIterationState dir;
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        while (HyoutaUtils::DirTree::GetNextEntries(dir, dirtree, entry)) {
            VerifyGameFiles(possibleVersions,
                            dirtree,
                            dir.Begin + firstChildIndex,
                            dir.End - dir.Begin,
                            gamePath,
                            verifyMode,
                            verificationStorage,
                            verifiedEntry);
        }
    }
    return possibleVersions;
}

static bool VerifyDlcFiles(const uint32_t possibleVersions,
                           const HyoutaUtils::DirTree::Tree& dirtree,
                           size_t firstDirTreeIndex,
                           size_t numberOfEntries,
                           const std::filesystem::path& parentPath,
                           const size_t dlcIndex,
                           VerificationStorage* verificationStorage,
                           VerifiedEntry* verificationEntry) {
    VerifyDirectoryState verifyDirectoryState;
    VerifyFileState verifyFileState;
    std::optional<std::filesystem::path> currentPath;
    for (size_t i = 0; i < numberOfEntries; ++i) {
        const size_t dirTreeIndex = firstDirTreeIndex + i;

        // check files only if they're in the current DLC
        // check folders if they're in the base game or in the current DLC
        const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[dirTreeIndex];
        if (!(entry.GetDlcIndex() == dlcIndex
              || (entry.IsDirectory() && entry.GetDlcIndex() == 0))) {
            continue;
        }
        if ((possibleVersions & entry.GetGameVersionBits()) == 0) {
            // no overlap between possible game versions and the game versions this entry is in
            continue;
        }

        std::string_view filename(dirtree.StringTable + entry.GetFilenameOffset(),
                                  entry.GetFilenameLength());
        if (!currentPath) {
            currentPath =
                parentPath / std::u8string_view((const char8_t*)filename.data(), filename.size());
        }
        if (entry.IsDirectory()) {
            VerifiedEntry* childVerificationEntry;
            if (entry.GetDlcIndex() == dlcIndex) {
                // this folder is exclusive to this DLC, need to verify it exists
                const bool dirExists = VerifyDirectory(verifyDirectoryState, *currentPath);
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
            HyoutaUtils::DirTree::DirectoryIterationState dir;
            const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
            while (HyoutaUtils::DirTree::GetNextEntries(dir, dirtree, entry)) {
                if (!VerifyDlcFiles(possibleVersions,
                                    dirtree,
                                    dir.Begin + firstChildIndex,
                                    dir.End - dir.Begin,
                                    *currentPath,
                                    dlcIndex,
                                    verificationStorage,
                                    childVerificationEntry)) {
                    return false;
                }
            }
        } else {
            if (VerifyFile(verifyFileState, dirtree, entry, *currentPath, false)) {
                // printf("DLC File %.*s verifies.\n", static_cast<int>(filename.size()),
                //        filename.data());
                AddOrGetVerifiedEntry(
                    dirTreeIndex, filename, verificationStorage, verificationEntry);
            } else {
                // printf("DLC File %.*s does not verify.\n",
                //        static_cast<int>(filename.size()),
                //        filename.data());
                return false;
            }
        }
    }
    return true;
}

static bool VerifyDlcFromRoot(const uint32_t possibleVersions,
                              const size_t dlcIndex,
                              const HyoutaUtils::DirTree::Tree& dirtree,
                              const std::filesystem::path& gamePath,
                              VerificationStorage* verificationStorage) {
    VerifiedEntry* verifiedEntry = verificationStorage ? &verificationStorage->Root : nullptr;
    if (dirtree.NumberOfEntries > 0) {
        const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[0];
        HyoutaUtils::DirTree::DirectoryIterationState dir;
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        while (HyoutaUtils::DirTree::GetNextEntries(dir, dirtree, entry)) {
            if (!VerifyDlcFiles(possibleVersions,
                                dirtree,
                                dir.Begin + firstChildIndex,
                                dir.End - dir.Begin,
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
        if (entry && it->is_directory()) {
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
    uint32_t possibleVersions =
        VerifyGameFilesFromRoot(dirtree, gamePath, VerifyMode::Full, &verificationStorage);
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

static bool IdentifyGame(const HyoutaUtils::DirTree::Tree& dirtree,
                         const std::filesystem::path& gamePath) {
    uint32_t possibleVersions =
        VerifyGameFilesFromRoot(dirtree, gamePath, VerifyMode::IdentifyDirtree, nullptr);
    return (possibleVersions != 0);
}

uint32_t VerifyGame(const HyoutaUtils::DirTree::Tree& dirtree,
                    std::string_view gamePath,
                    VerifyMode verifyMode,
                    VerificationStorage* verificationStorage) {
    return VerifyGameFilesFromRoot(dirtree,
                                   HyoutaUtils::IO::FilesystemPathFromUtf8(gamePath),
                                   verifyMode,
                                   verificationStorage);
}

bool VerifyDlc(const uint32_t possibleVersions,
               const size_t dlcIndex,
               const HyoutaUtils::DirTree::Tree& dirtree,
               std::string_view gamePath,
               VerificationStorage* verificationStorage) {
    return VerifyDlcFromRoot(possibleVersions,
                             dlcIndex,
                             dirtree,
                             HyoutaUtils::IO::FilesystemPathFromUtf8(gamePath),
                             verificationStorage);
}

static bool HasUnicodeChar(std::string_view filename) {
    for (char c : filename) {
        if (static_cast<uint8_t>(c) >= 0x80) {
            return true;
        }
    }
    return false;
}

static bool AnyChildHasUnicodeChar(uint32_t gameVersionBits,
                                   const HyoutaUtils::DirTree::Tree& dirtree,
                                   size_t dirTreeIndex) {
    const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[dirTreeIndex];
    const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
    const size_t count = entry.GetDirectoryNumberOfEntries();
    for (size_t i = 0; i < count; ++i) {
        const HyoutaUtils::DirTree::Entry& child = dirtree.Entries[firstChildIndex + i];
        if ((gameVersionBits & child.GetGameVersionBits()) == 0) {
            continue;
        }
        std::string_view filename(dirtree.StringTable + child.GetFilenameOffset(),
                                  child.GetFilenameLength());
        if (HasUnicodeChar(filename)) {
            return true;
        }
    }
    return false;
}

namespace {
struct FileInFileSystem {
    std::filesystem::path Path;
    bool IsDirectory;
    bool HasIntermediateDirectory = false;
    size_t CorrespondingDirtreeIndex = std::numeric_limits<size_t>::max();
};

static bool ConvertDirectoryToSingleFileInDirectory(FileInFileSystem& e) {
    while (e.IsDirectory) {
        std::error_code ec;
        std::filesystem::directory_iterator it(e.Path, ec);
        if (ec) {
            return false;
        }
        if (it == std::filesystem::directory_iterator()) {
            // zero files in this directory, this is clearly something else
            return false;
        }
        auto stat = it->status(ec);
        if (ec) {
            return false;
        }
        e.Path = it->path();
        e.IsDirectory = std::filesystem::is_directory(stat);
        it.increment(ec);
        if (ec) {
            return false;
        }
        if (it != std::filesystem::directory_iterator()) {
            // more than one file in this directory, this is clearly something else
            return false;
        }
    }
    return true;
}
} // namespace

static bool FixUnicodeFilenamesInDirectory(
    uint32_t gameVersionBits,
    const HyoutaUtils::DirTree::Tree& dirtree,
    size_t dirTreeIndex,
    const std::filesystem::path& gamePath,
    const std::function<bool(std::string_view path, size_t count)>& confirmationCallback) {
    if (!AnyChildHasUnicodeChar(gameVersionBits, dirtree, dirTreeIndex)) {
        return false;
    }

    // match the expected dirtree against the actual filesystem
    std::unordered_map<std::string_view, size_t, CaseInsensitiveHash, CaseInsensitiveEquals>
        filesInDirtreeButNotFilesystem;
    std::vector<FileInFileSystem> filesInFilesystemButNotDirtree;
    const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[dirTreeIndex];
    const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
    const size_t count = entry.GetDirectoryNumberOfEntries();
    for (size_t i = 0; i < count; ++i) {
        const size_t index = firstChildIndex + i;
        const HyoutaUtils::DirTree::Entry& child = dirtree.Entries[index];
        if ((gameVersionBits & child.GetGameVersionBits()) == 0) {
            continue;
        }
        std::string_view filename(dirtree.StringTable + child.GetFilenameOffset(),
                                  child.GetFilenameLength());
        filesInDirtreeButNotFilesystem.try_emplace(filename, index);
    }
    std::error_code ec;
    {
        std::filesystem::directory_iterator it(gamePath, ec);
        if (ec) {
            return false;
        }
        while (it != std::filesystem::directory_iterator()) {
            std::string filename = HyoutaUtils::IO::FilesystemPathToUtf8(it->path().filename());
            if (filesInDirtreeButNotFilesystem.erase(std::string_view(filename)) == 0) {
                auto stat = it->status(ec);
                if (ec) {
                    return false;
                }
                filesInFilesystemButNotDirtree.emplace_back(FileInFileSystem{
                    .Path = it->path(), .IsDirectory = std::filesystem::is_directory(stat)});
            }
            it.increment(ec);
            if (ec) {
                return false;
            }
        }
    }
    if (filesInDirtreeButNotFilesystem.empty()) {
        // directory matches dirtree (at least filename-wise)
        return false;
    }

    const size_t numberOfUnmatchedDirectoriesInDirtree = [&]() {
        size_t c = 0;
        for (const auto& e : filesInDirtreeButNotFilesystem) {
            if (dirtree.Entries[e.second].IsDirectory()) {
                ++c;
            }
        }
        return c;
    }();
    if (numberOfUnmatchedDirectoriesInDirtree == 0) {
        // all entries in the filesystem must be single files. it's possible that due to encoding
        // issues, intermediate directories were crated. handle this.
        // TODO: what if multiple files end up in the same wrong directory?
        for (auto& e : filesInFilesystemButNotDirtree) {
            if (e.IsDirectory) {
                if (!ConvertDirectoryToSingleFileInDirectory(e)) {
                    return false;
                }
            }
        }
        if (filesInDirtreeButNotFilesystem.size() != filesInFilesystemButNotDirtree.size()) {
            return false;
        }
    } else {
        // dirtree has at least 1 directory, this is more complicated
        if (numberOfUnmatchedDirectoriesInDirtree > 1) {
            return false; // TODO: Handle this case.
        }
        if (filesInDirtreeButNotFilesystem.size() != filesInFilesystemButNotDirtree.size()) {
            return false;
        }
        const size_t numberOfUnmatchedDirectoriesInFilesystem = [&]() {
            size_t c = 0;
            for (const auto& e : filesInFilesystemButNotDirtree) {
                if (e.IsDirectory) {
                    ++c;
                }
            }
            return c;
        }();
        if (numberOfUnmatchedDirectoriesInDirtree != numberOfUnmatchedDirectoriesInFilesystem) {
            return false;
        }
    }

    // match the two lists
    for (auto& e : filesInFilesystemButNotDirtree) {
        if (e.IsDirectory) {
            // FIXME: This only works for a single directory.
            const bool matched = [&]() -> bool {
                for (auto it = filesInDirtreeButNotFilesystem.begin();
                     it != filesInDirtreeButNotFilesystem.end();
                     ++it) {
                    const auto& entry = dirtree.Entries[it->second];
                    if (entry.IsDirectory()) {
                        e.CorrespondingDirtreeIndex = it->second;
                        filesInDirtreeButNotFilesystem.erase(it);
                        return true;
                    }
                }
                return false;
            }();
            if (!matched) {
                return false;
            }
        } else {
            auto filesize = std::filesystem::file_size(e.Path, ec);
            if (ec) {
                return false;
            }
            const bool matched = [&]() -> bool {
                std::optional<HyoutaUtils::Hash::SHA1> hash;
                for (auto it = filesInDirtreeButNotFilesystem.begin();
                     it != filesInDirtreeButNotFilesystem.end();
                     ++it) {
                    const auto& entry = dirtree.Entries[it->second];
                    if (entry.IsFile() && entry.GetFileSize() == filesize) {
                        if (!hash) {
                            HyoutaUtils::IO::File file(e.Path, HyoutaUtils::IO::OpenMode::Read);
                            if (!file.IsOpen() || file.GetLength() != filesize) {
                                return false;
                            }
                            hash = HyoutaUtils::Hash::CalculateSHA1FromFile(file);
                            if (!hash) {
                                return false;
                            }
                        }
                        if (*hash == dirtree.HashTable[entry.GetFileHashIndex()]) {
                            e.CorrespondingDirtreeIndex = it->second;
                            filesInDirtreeButNotFilesystem.erase(it);
                            return true;
                        }
                    }
                }
                return false;
            }();
            if (!matched) {
                return false;
            }
        }
    }

    if (!confirmationCallback(
            HyoutaUtils::IO::FilesystemPathToUtf8(filesInFilesystemButNotDirtree.size() == 1
                                                      ? filesInFilesystemButNotDirtree[0].Path
                                                      : gamePath),
            filesInFilesystemButNotDirtree.size())) {
        // user declined the fix
        return false;
    }

    // matched everything, rename to the corrected names
    for (auto& e : filesInFilesystemButNotDirtree) {
        const auto& entry = dirtree.Entries[e.CorrespondingDirtreeIndex];
        std::string_view filename(dirtree.StringTable + entry.GetFilenameOffset(),
                                  entry.GetFilenameLength());
        auto newPath = gamePath / HyoutaUtils::IO::FilesystemPathFromUtf8(filename);
        // printf("Rename %s -> %s\n",
        //        HyoutaUtils::IO::FilesystemPathToUtf8(e.Path).c_str(),
        //        HyoutaUtils::IO::FilesystemPathToUtf8(newPath).c_str());
        std::filesystem::rename(e.Path, newPath, ec);
        if (ec) {
            return false;
        }
    }

    return true;
}

static bool FixUnicodeFilenames(
    uint32_t gameVersionBits,
    const HyoutaUtils::DirTree::Tree& dirtree,
    size_t firstDirTreeIndex,
    size_t numberOfEntries,
    const std::filesystem::path& parentPath,
    const std::function<bool(std::string_view path, size_t count)>& confirmationCallback) {
    std::optional<std::filesystem::path> currentPath;
    for (size_t i = 0; i < numberOfEntries; ++i) {
        const size_t dirTreeIndex = firstDirTreeIndex + i;
        const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[dirTreeIndex];
        if (entry.GetDlcIndex() != 0) {
            // ignore DLC... this is a hack but it works for the supported games
            continue;
        }
        if ((gameVersionBits & entry.GetGameVersionBits()) == 0) {
            continue;
        }

        std::string_view filename(dirtree.StringTable + entry.GetFilenameOffset(),
                                  entry.GetFilenameLength());
        if (!currentPath) {
            currentPath =
                parentPath / std::u8string_view((const char8_t*)filename.data(), filename.size());
        }
        if (entry.IsDirectory()) {
            FixUnicodeFilenamesInDirectory(
                gameVersionBits, dirtree, dirTreeIndex, *currentPath, confirmationCallback);

            HyoutaUtils::DirTree::DirectoryIterationState dir;
            const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
            while (HyoutaUtils::DirTree::GetNextEntries(dir, dirtree, entry)) {
                if (!FixUnicodeFilenames(gameVersionBits,
                                         dirtree,
                                         dir.Begin + firstChildIndex,
                                         dir.End - dir.Begin,
                                         *currentPath,
                                         confirmationCallback)) {
                    return false;
                }
            }
        }
    }
    return true;
}

static bool FixUnicodeFilenamesFromRoot(
    uint32_t gameVersionBits,
    const HyoutaUtils::DirTree::Tree& dirtree,
    const std::filesystem::path& gamePath,
    const std::function<bool(std::string_view path, size_t count)>& confirmationCallback) {
    if (dirtree.NumberOfEntries > 0) {
        FixUnicodeFilenamesInDirectory(gameVersionBits, dirtree, 0, gamePath, confirmationCallback);

        const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[0];
        HyoutaUtils::DirTree::DirectoryIterationState dir;
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        while (HyoutaUtils::DirTree::GetNextEntries(dir, dirtree, entry)) {
            if (!FixUnicodeFilenames(gameVersionBits,
                                     dirtree,
                                     dir.Begin + firstChildIndex,
                                     dir.End - dir.Begin,
                                     gamePath,
                                     confirmationCallback)) {
                return false;
            }
        }
    }
    return true;
}

bool FixUnicodeFilenames(
    const uint32_t gameVersionBits,
    const HyoutaUtils::DirTree::Tree& dirtree,
    std::string_view gamePath,
    const std::function<bool(std::string_view path, size_t count)>& confirmationCallback) {
    return FixUnicodeFilenamesFromRoot(gameVersionBits,
                                       dirtree,
                                       HyoutaUtils::IO::FilesystemPathFromUtf8(gamePath),
                                       confirmationCallback);
}

static void PrintDirTreeRecursive(const HyoutaUtils::DirTree::Tree& dirtree,
                                  size_t dirTreeIndex,
                                  int indent) {
    const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[dirTreeIndex];
    std::string_view filename(dirtree.StringTable + entry.GetFilenameOffset(),
                              entry.GetFilenameLength());
    if (entry.IsDirectory()) {
        printf("%5zx %4zu %55s %*s%.*s\n",
               static_cast<size_t>(entry.GetGameVersionBits()),
               static_cast<size_t>(entry.GetDlcIndex()),
               "",
               indent,
               "",
               static_cast<int>(filename.size()),
               filename.data());

        const size_t count = entry.GetDirectoryNumberOfEntries();
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        for (size_t i = 0; i < count; ++i) {
            PrintDirTreeRecursive(dirtree, firstChildIndex + i, indent + 1);
        }
    } else {
        auto& hash = dirtree.HashTable[entry.GetFileHashIndex()].Hash;
        printf(
            "%5zx %4zu %14llu "
            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x "
            "%*s%.*s\n",
            static_cast<size_t>(entry.GetGameVersionBits()),
            static_cast<size_t>(entry.GetDlcIndex()),
            static_cast<unsigned long long>(entry.GetFileSize()),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[0])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[1])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[2])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[3])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[4])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[5])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[6])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[7])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[8])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[9])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[10])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[11])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[12])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[13])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[14])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[15])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[16])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[17])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[18])),
            static_cast<unsigned int>(static_cast<uint8_t>(hash[19])),
            indent,
            "",
            static_cast<int>(filename.size()),
            filename.data());
    }
}

static void PrintDirTreeFromRoot(const HyoutaUtils::DirTree::Tree& dirtree) {
    printf("%-5s %-4s %-14s %-40s %s\n", "ver", "dlc", "filesize", "sha1", "filename");
    if (dirtree.NumberOfEntries > 0) {
        const HyoutaUtils::DirTree::Entry& entry = dirtree.Entries[0];
        const size_t count = entry.GetDirectoryNumberOfEntries();
        const size_t firstChildIndex = entry.GetDirectoryFirstEntry();
        for (size_t i = 0; i < count; ++i) {
            PrintDirTreeRecursive(dirtree, firstChildIndex + i, 0);
        }
    }
}


void PrintDirTree(const HyoutaUtils::DirTree::Tree& dirtree) {
    PrintDirTreeFromRoot(dirtree);
}
} // namespace SenTools::GameVerify

namespace SenTools {
int Game_Verify_Function(int argc, char** argv) {
    using namespace SenTools::GameVerify;

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

    const bool couldBeCS1 = IdentifyGame(SenLib::Sen1::GetDirTree(), gamepath);
    const bool couldBeCS2 = IdentifyGame(SenLib::Sen2::GetDirTree(), gamepath);
    const bool couldBeCS3 = IdentifyGame(SenLib::Sen3::GetDirTree(), gamepath);
    const bool couldBeCS4 = IdentifyGame(SenLib::Sen4::GetDirTree(), gamepath);
    const bool couldBeRev = IdentifyGame(SenLib::Sen5::GetDirTree(), gamepath);
    const bool couldBeTX = IdentifyGame(SenLib::TX::GetDirTree(), gamepath);

    static constexpr char gameNameCS1[] = "Trails of Cold Steel";
    static constexpr char gameNameCS2[] = "Trails of Cold Steel II";
    static constexpr char gameNameCS3[] = "Trails of Cold Steel III";
    static constexpr char gameNameCS4[] = "Trails of Cold Steel IV";
    static constexpr char gameNameRev[] = "Trails into Reverie";
    static constexpr char gameNameTX[] = "Tokyo Xanadu eX+";

    size_t couldBeCount = 0;
    if (couldBeCS1) {
        printf("Looks like %s...\n", gameNameCS1);
        ++couldBeCount;
    }
    if (couldBeCS2) {
        printf("Looks like %s...\n", gameNameCS2);
        ++couldBeCount;
    }
    if (couldBeCS3) {
        printf("Looks like %s...\n", gameNameCS3);
        ++couldBeCount;
    }
    if (couldBeCS4) {
        printf("Looks like %s...\n", gameNameCS4);
        ++couldBeCount;
    }
    if (couldBeRev) {
        printf("Looks like %s...\n", gameNameRev);
        ++couldBeCount;
    }
    if (couldBeTX) {
        printf("Looks like %s...\n", gameNameTX);
        ++couldBeCount;
    }
    if (couldBeCount == 0) {
        printf("Does not appear to be any known game.\n");
        return -1;
    }
    if (couldBeCount > 1) {
        printf("Ambiguous files, could be multiple games.\n");
        return -1;
    }


    bool found = false;
    if (couldBeCS1) {
        found = VerifyGame(SenLib::Sen1::GetDirTree(), gameNameCS1, gamepath);
    } else if (couldBeCS2) {
        found = VerifyGame(SenLib::Sen2::GetDirTree(), gameNameCS2, gamepath);
    } else if (couldBeCS3) {
        found = VerifyGame(SenLib::Sen3::GetDirTree(), gameNameCS3, gamepath);
    } else if (couldBeCS4) {
        found = VerifyGame(SenLib::Sen4::GetDirTree(), gameNameCS4, gamepath);
    } else if (couldBeRev) {
        found = VerifyGame(SenLib::Sen5::GetDirTree(), gameNameRev, gamepath);
    } else if (couldBeTX) {
        found = VerifyGame(SenLib::TX::GetDirTree(), gameNameTX, gamepath);
    }
    if (!found) {
        printf("Game did not verify, likely corrupted.\n");
    }

    return 0;
}
} // namespace SenTools
