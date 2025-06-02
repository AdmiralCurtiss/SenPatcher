#pragma once

#include <cassert>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "result.h"

namespace HyoutaUtils {
enum class ArgTypes {
    Flag,
    SignedInteger,
    UnsignedInteger,
    String,
    StringArray,
};

struct Arg {
    ArgTypes Type;
    std::string_view ShortKey;
    std::string_view LongKey;
    std::string_view Argument;
    std::string_view Description;
};

struct EvaluatedArg {
    const Arg* Argument;
    std::variant<std::monostate,
                 bool,
                 int64_t,
                 uint64_t,
                 std::string_view,
                 std::vector<std::string_view>>
        Value;
};

struct EvaluatedArgs {
    std::vector<EvaluatedArg> EvaluatedArguments;
    std::vector<std::string_view> FreeArguments;
    std::string_view ProgramName;
};

struct Args {
    constexpr Args(std::string_view programName,
                   std::string_view freeArgs,
                   std::string_view description,
                   std::span<const Arg* const> args)
      : ProgramName(programName), FreeArgs(freeArgs), Description(description), Arguments(args) {}
    constexpr ~Args() = default;


    HyoutaUtils::Result<EvaluatedArgs, std::string> Parse(int argc, char** argv) const;

private:
    std::string_view ProgramName;
    std::string_view FreeArgs;
    std::string_view Description;
    std::span<const Arg* const> Arguments;
};
} // namespace HyoutaUtils
