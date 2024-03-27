#pragma once

#include <string>

#include "sen1/tbl.h"

namespace SenLib::Sen1::FileFixes::text_dat_us_t_item_tbl_t_magic_tbl {
std::string FixHpEpCpSpacing(std::string desc, size_t start = 2);
bool IsSingleArt(const ItemData& item);
bool IsStatusQuartz(const ItemData& item);
bool IsSpecialQuartz(const ItemData& item);
} // namespace SenLib::Sen1::FileFixes::text_dat_us_t_item_tbl_t_magic_tbl
