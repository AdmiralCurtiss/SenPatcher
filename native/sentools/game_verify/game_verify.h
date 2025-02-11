#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace HyoutaUtils::DirTree {
struct Tree;
}

namespace SenTools::GameVerify {
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
    std::vector<std::unique_ptr<VerificationDirectory>> Directories;
};

enum class VerifyMode {
    Full,            // verifies everything
    ExecutablesOnly, // only verifies executables
    IdentifyDirtree, // only checks whether executables exist, for figuring out which known dirtree
                     // could possibly match the given directory
};

uint32_t VerifyGame(const HyoutaUtils::DirTree::Tree& dirtree,
                    std::string_view gamePath,
                    VerifyMode verifyMode,
                    VerificationStorage* verificationStorage);

bool VerifyDlc(const uint32_t possibleVersions,
               const size_t dlcIndex,
               const HyoutaUtils::DirTree::Tree& dirtree,
               std::string_view gamePath,
               VerificationStorage* verificationStorage);

bool FixUnicodeFilenames(const uint32_t gameVersionBits,
                         const HyoutaUtils::DirTree::Tree& dirtree,
                         std::string_view gamePath);

void PrintDirTree(const HyoutaUtils::DirTree::Tree& dirtree);
} // namespace SenTools::GameVerify
