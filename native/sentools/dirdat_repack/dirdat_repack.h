#pragma once

#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools::DirDat {
enum class RepackDirDatResult { Success };

HyoutaUtils::Result<RepackDirDatResult, std::string> RepackDirDat(std::string_view sourcePath,
                                                                  std::string_view targetPathDir,
                                                                  std::string_view targetPathDat,
                                                                  size_t maxChunkSize,
                                                                  bool skipBitCompression,
                                                                  bool skipByteCompression);
} // namespace SenTools::DirDat
