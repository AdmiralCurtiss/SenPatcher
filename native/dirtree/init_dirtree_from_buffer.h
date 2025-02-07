#pragma once

#include "dirtree/tree.h"

namespace HyoutaUtils::DirTree {
HyoutaUtils::DirTree::Tree InitDirTreeFromBuffer(const char* buffer,
                                                 size_t length,
                                                 const char* const* versionNames,
                                                 size_t numberOfVersions,
                                                 const char* const* dlcNames,
                                                 size_t numberOfDlcs);
}
