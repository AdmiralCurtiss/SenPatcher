#include "util/args.h"

#include <algorithm>
#include <cstddef>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "util/number.h"
#include "util/result.h"

namespace HyoutaUtils {
static HyoutaUtils::Result<EvaluatedArgs, std::string>
    ParseInternal(const std::span<const Arg* const>& arguments,
                  size_t count,
                  const std::function<std::string_view(size_t idx)>& get_arg) {
    for (const Arg* a : arguments) {
        if (a == nullptr) {
            return std::string("Internal error.");
        }
    }

    EvaluatedArgs evaluated;
    std::unordered_map<const Arg*, size_t> lookup;

    const auto insert_arg =
        [&](const Arg* key, bool isLongArg, std::string_view value) -> std::optional<std::string> {
        auto it = lookup.try_emplace(key, evaluated.EvaluatedArguments.size());
        if (it.second) {
            evaluated.EvaluatedArguments.emplace_back(EvaluatedArg{.Argument = key});
        }
        auto& ea = evaluated.EvaluatedArguments[it.first->second];
        if (std::holds_alternative<std::monostate>(ea.Value)) {
            switch (key->Type) {
                case ArgTypes::Flag: ea.Value = true; break;
                case ArgTypes::Int64: {
                    auto t = HyoutaUtils::NumberUtils::ParseInt64(value);
                    if (!t) {
                        return std::format(
                            "Failed to parse '{}' as signed integer for option '{}{}'.",
                            value,
                            isLongArg ? "--" : "-",
                            isLongArg ? key->LongKey : key->ShortKey);
                    }
                    ea.Value = *t;
                    break;
                }
                case ArgTypes::UInt64: {
                    auto t = HyoutaUtils::NumberUtils::ParseUInt64(value);
                    if (!t) {
                        return std::format(
                            "Failed to parse '{}' as unsigned integer for option '{}{}'.",
                            value,
                            isLongArg ? "--" : "-",
                            isLongArg ? key->LongKey : key->ShortKey);
                    }
                    ea.Value = *t;
                    break;
                }
                case ArgTypes::Double: {
                    auto t = HyoutaUtils::NumberUtils::ParseDouble(value);
                    if (!t) {
                        return std::format(
                            "Failed to parse '{}' as floating point for option '{}{}'.",
                            value,
                            isLongArg ? "--" : "-",
                            isLongArg ? key->LongKey : key->ShortKey);
                    }
                    ea.Value = *t;
                    break;
                }
                case ArgTypes::String: ea.Value = value; break;
                case ArgTypes::StringArray:
                    ea.Value = std::vector<std::string_view>();
                    std::get<std::vector<std::string_view>>(ea.Value).push_back(value);
                    break;
                default: return std::string("Internal error.");
            }
        } else if (std::holds_alternative<std::vector<std::string_view>>(ea.Value)) {
            std::get<std::vector<std::string_view>>(ea.Value).push_back(value);
        } else {
            return std::format("'{}{}' given multiple times, can only be given once.",
                               isLongArg ? "--" : "-",
                               isLongArg ? key->LongKey : key->ShortKey);
        }
        return std::nullopt;
    };

    if (count > 0) {
        evaluated.ProgramName = get_arg(0);
    }
    size_t i = 1;
    while (i < count) {
        std::string_view arg = get_arg(i);
        ++i;

        const Arg* matched = nullptr;
        if (arg.starts_with("--")) {
            std::string_view longArg = arg.substr(2);
            if (longArg.empty()) {
                // stop parsing arguments, all remaining args are free args
                while (i < count) {
                    evaluated.FreeArguments.push_back(get_arg(i));
                    ++i;
                }
                break;
            }
            for (const Arg* a : arguments) {
                if (a->LongKey == longArg) {
                    matched = a;
                    break;
                }
            }
            std::string_view value;
            bool hasValue = false;
            if (matched == nullptr) {
                // maybe a "key=value" style arg?
                size_t pos = longArg.find('=');
                if (pos != std::string_view::npos) {
                    value = longArg.substr(pos + 1);
                    longArg = longArg.substr(0, pos);
                    hasValue = true;
                    for (const Arg* a : arguments) {
                        if (a->LongKey == longArg) {
                            matched = a;
                            break;
                        }
                    }
                }
            }
            if (matched == nullptr) {
                return std::format("'{}' is not a valid option.", arg);
            }
            if (matched->Type == ArgTypes::Flag) {
                if (!value.empty()) {
                    return std::format("'{}{}' does not take an argument.", "--", longArg);
                }
                auto error = insert_arg(matched, true, std::string_view());
                if (error.has_value()) {
                    return *error;
                }
            } else {
                if (hasValue) {
                    if (value.empty()) {
                        return std::format(
                            "'{}{}' requires an argument, none given.", "--", longArg);
                    }
                    auto error = insert_arg(matched, true, value);
                    if (error.has_value()) {
                        return *error;
                    }
                } else {
                    if (i < count) {
                        value = get_arg(i);
                        ++i;
                        auto error = insert_arg(matched, true, value);
                        if (error.has_value()) {
                            return *error;
                        }
                    } else {
                        return std::format(
                            "'{}{}' requires an argument, none given.", "--", longArg);
                    }
                }
            }
        } else if (arg.starts_with("-")) {
            std::string_view shortArg = arg.substr(1);
            if (shortArg.empty()) {
                // just "-" is a free argument
                evaluated.FreeArguments.push_back(arg);
                continue;
            }

            while (!shortArg.empty()) {
                // TODO: should this handle unicode?
                std::string_view ch = shortArg.substr(0, 1);
                shortArg = shortArg.substr(1);

                for (const Arg* a : arguments) {
                    if (a->ShortKey == ch) {
                        matched = a;
                        break;
                    }
                }
                if (matched == nullptr) {
                    return std::format("'{}' is not a valid option.", arg);
                }

                if (matched->Type == ArgTypes::Flag) {
                    // set flag and proceed to next char
                    auto error = insert_arg(matched, false, std::string_view());
                    if (error.has_value()) {
                        return *error;
                    }
                } else {
                    // remaining string if any is the argument, otherwise the next arg
                    if (!shortArg.empty()) {
                        auto error = insert_arg(matched, false, shortArg);
                        if (error.has_value()) {
                            return *error;
                        }
                        break;
                    } else {
                        if (i < count) {
                            std::string_view value = get_arg(i);
                            ++i;
                            auto error = insert_arg(matched, false, value);
                            if (error.has_value()) {
                                return *error;
                            }
                        } else {
                            return std::format("'{}{}' requires an argument, none given.", "-", ch);
                        }
                    }
                }
            }
        } else {
            evaluated.FreeArguments.push_back(arg);
        }
    }

    // sort so we can binary search for lookup
    std::sort(evaluated.EvaluatedArguments.begin(),
              evaluated.EvaluatedArguments.end(),
              [](const EvaluatedArg& lhs, const EvaluatedArg& rhs) {
                  return lhs.Argument < rhs.Argument;
              });

    return evaluated;
}

HyoutaUtils::Result<EvaluatedArgs, std::string> Args::Parse(int argc, char** argv) const {
    if (argc < 0 || argv == nullptr) {
        return std::string("Internal error.");
    }
    size_t count = static_cast<size_t>(argc);
    for (size_t i = 0; i < count; ++i) {
        if (argv[i] == nullptr) {
            return std::string("Internal error.");
        }
    }

    return ParseInternal(Arguments, count, [&](size_t idx) -> std::string_view {
        return std::string_view(argv[idx]);
    });
}

HyoutaUtils::Result<EvaluatedArgs, std::string>
    Args::Parse(std::span<const std::string_view> args) const {
    return ParseInternal(
        Arguments, args.size(), [&](size_t idx) -> std::string_view { return args[idx]; });
}

const EvaluatedArg* EvaluatedArgs::FindFromArg(const Arg* arg) const {
    const EvaluatedArg* infos = EvaluatedArguments.data();
    size_t count = EvaluatedArguments.size();
    while (true) {
        if (count == 0) {
            return nullptr;
        }

        const size_t countHalf = count / 2;
        const EvaluatedArg* middle = infos + countHalf;
        if (middle->Argument < arg) {
            infos = middle + 1;
            count = count - (countHalf + 1);
        } else if (arg < middle->Argument) {
            count = countHalf;
        } else {
            return middle;
        }
    }
}
} // namespace HyoutaUtils
