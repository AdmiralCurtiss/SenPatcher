#include "util/args.h"

#include <cstddef>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "util/number.h"
#include "util/result.h"

namespace HyoutaUtils {
HyoutaUtils::Result<EvaluatedArgs, std::string> Args::Parse(int argc, char** argv) const {
    for (const Arg* a : Arguments) {
        if (a == nullptr) {
            return std::string("Internal error.");
        }
    }
    if (argc < 0 || argv == nullptr) {
        return std::string("Internal error.");
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
                case ArgTypes::SignedInteger: {
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
                case ArgTypes::UnsignedInteger: {
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
            return std::format("'{}{}' given multiple times, not supported.",
                               isLongArg ? "--" : "-",
                               isLongArg ? key->LongKey : key->ShortKey);
        }
        return std::nullopt;
    };

    size_t count = static_cast<size_t>(argc);
    if (count > 0) {
        if (argv[0] == nullptr) {
            return std::string("Internal error.");
        }

        evaluated.ProgramName = std::string_view(argv[0]);
    }
    size_t i = 1;
    while (i < count) {
        if (argv[i] == nullptr) {
            return std::string("Internal error.");
        }

        std::string_view arg(argv[i]);
        ++i;

        const Arg* matched = nullptr;
        if (arg.starts_with("--")) {
            std::string_view longArg = arg.substr(2);
            if (longArg.empty()) {
                // stop parsing arguments, all remaining args are free args
                while (i < count) {
                    if (argv[i] == nullptr) {
                        return std::string("Internal error.");
                    }
                    evaluated.FreeArguments.push_back(argv[i]);
                    ++i;
                }
                break;
            }
            for (const Arg* a : Arguments) {
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
                    for (const Arg* a : Arguments) {
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
                        if (argv[i] == nullptr) {
                            return std::string("Internal error.");
                        }
                        value = std::string_view(argv[i]);
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

                for (const Arg* a : Arguments) {
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
                    } else {
                        if (i < count) {
                            if (argv[i] == nullptr) {
                                return std::string("Internal error.");
                            }
                            std::string_view value = std::string_view(argv[i]);
                            ++i;
                            auto error = insert_arg(matched, false, value);
                            if (error.has_value()) {
                                return *error;
                            }
                        } else {
                            return std::format(
                                "'{}{}' requires an argument, none given.", "-", shortArg);
                        }
                    }
                }
            }
        } else {
            evaluated.FreeArguments.push_back(arg);
        }
    }
    return evaluated;
}
} // namespace HyoutaUtils
