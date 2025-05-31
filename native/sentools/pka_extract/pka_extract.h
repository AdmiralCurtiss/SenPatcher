#pragma once

#include <span>
#include <string>
#include <string_view>

#include "sentools/task_cancellation.h"
#include "sentools/task_reporting.h"
#include "util/result.h"

namespace SenTools {
enum class ExtractPkaResult { Success, Cancelled };

HyoutaUtils::Result<ExtractPkaResult, std::string>
    ExtractPka(SenTools::TaskCancellation* taskCancellation,
               SenTools::TaskReporting* taskReporting,
               std::string_view source,
               std::string_view target,
               std::span<const std::string> referencedPkaPaths,
               bool extractAsPkaReferenceStub);
} // namespace SenTools
