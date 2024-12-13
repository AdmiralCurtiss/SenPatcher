#include "dirtree_tx.h"

#include "generated/internal_dirtree_tx.h"

namespace SenLib::TX {
HyoutaUtils::DirTree::Tree GetDirTree() {
    return HyoutaUtils::DirTree::Tree{.Entries = s_dirtree.data(),
                                      .NumberOfEntries = s_dirtree.size(),
                                      .StringTable = s_stringtable.data(),
                                      .HashTable = s_hashtable.data()};
}
} // namespace SenLib::TX
