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
    Int64,
    UInt64,
    Double,
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
                 double,
                 std::string_view,
                 std::vector<std::string_view>>
        Value;
};

struct EvaluatedArgs {
    std::vector<EvaluatedArg> EvaluatedArguments;
    std::vector<std::string_view> FreeArguments;
    std::string_view ProgramName;

    const EvaluatedArg* FindFromArg(const Arg* arg) const;

private:
    template<typename T>
    const T* TryGetValue(const Arg* arg) const {
        const EvaluatedArg* ea = FindFromArg(arg);
        if (ea == nullptr) {
            return nullptr;
        }
        if (!std::holds_alternative<T>(ea->Value)) {
            return nullptr;
        }
        return &std::get<T>(ea->Value);
    }

public:
    bool IsFlagSet(const Arg* arg) const {
        const EvaluatedArg* ea = FindFromArg(arg);
        return ea != nullptr && std::holds_alternative<bool>(ea->Value)
               && std::get<bool>(ea->Value);
    }

    const int64_t* TryGetInt64(const Arg* arg) const {
        return TryGetValue<int64_t>(arg);
    }

    const uint64_t* TryGetUInt64(const Arg* arg) const {
        return TryGetValue<uint64_t>(arg);
    }

    const double* TryGetDouble(const Arg* arg) const {
        return TryGetValue<double>(arg);
    }

    const std::string_view* TryGetString(const Arg* arg) const {
        return TryGetValue<std::string_view>(arg);
    }

    const std::vector<std::string_view>* TryGetStringArray(const Arg* arg) const {
        return TryGetValue<std::vector<std::string_view>>(arg);
    }
};

struct Args {
    constexpr Args(std::string_view programName,
                   std::string_view freeArgs,
                   std::string_view description,
                   std::span<const Arg* const> args)
      : ProgramName(programName), FreeArgs(freeArgs), Description(description), Arguments(args) {}
    constexpr ~Args() = default;


    HyoutaUtils::Result<EvaluatedArgs, std::string> Parse(int argc, char** argv) const;
    HyoutaUtils::Result<EvaluatedArgs, std::string>
        Parse(std::span<const std::string_view> args) const;
    void PrintUsage() const;

private:
    std::string_view ProgramName;
    std::string_view FreeArgs;
    std::string_view Description;
    std::span<const Arg* const> Arguments;
};
} // namespace HyoutaUtils
