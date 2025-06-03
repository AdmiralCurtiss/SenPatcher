#include <algorithm>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "gtest/gtest.h"

#include "util/args.h"

template<typename T, typename T2>
static bool ContainsAt(const std::vector<T>& container, const T2& value, size_t where) {
    if (where >= container.size()) {
        return false;
    }
    if (container[where] == value) {
        return true;
    }
    return false;
}

TEST(Args, GeneralTest) {
    static constexpr HyoutaUtils::Arg arg_flag1{.Type = HyoutaUtils::ArgTypes::Flag,
                                                .ShortKey = "1",
                                                .LongKey = "flag1",
                                                .Description = "flag description"};
    static constexpr HyoutaUtils::Arg arg_flag2{.Type = HyoutaUtils::ArgTypes::Flag,
                                                .ShortKey = "2",
                                                .LongKey = "flag2",
                                                .Description = "flag description"};
    static constexpr HyoutaUtils::Arg arg_flag3{.Type = HyoutaUtils::ArgTypes::Flag,
                                                .ShortKey = "3",
                                                .LongKey = "flag3",
                                                .Description = "flag description"};
    static constexpr HyoutaUtils::Arg arg_int{.Type = HyoutaUtils::ArgTypes::Int64,
                                              .ShortKey = "i",
                                              .LongKey = "int",
                                              .Argument = "integer",
                                              .Description = "signed int description"};
    static constexpr HyoutaUtils::Arg arg_uint{.Type = HyoutaUtils::ArgTypes::UInt64,
                                               .ShortKey = "u",
                                               .LongKey = "uint",
                                               .Argument = "integer",
                                               .Description = "unsigned int description"};
    static constexpr HyoutaUtils::Arg arg_double{.Type = HyoutaUtils::ArgTypes::Double,
                                                 .ShortKey = "d",
                                                 .LongKey = "double",
                                                 .Argument = "floating",
                                                 .Description = "floating point description"};
    static constexpr HyoutaUtils::Arg arg_str{.Type = HyoutaUtils::ArgTypes::String,
                                              .ShortKey = "s",
                                              .LongKey = "str",
                                              .Argument = "string",
                                              .Description = "string description"};
    static constexpr HyoutaUtils::Arg arg_strarr{.Type = HyoutaUtils::ArgTypes::StringArray,
                                                 .ShortKey = "a",
                                                 .LongKey = "strarr",
                                                 .Argument = "string",
                                                 .Description = "string array description"};
    static constexpr auto args_array = {&arg_flag1,
                                        &arg_flag2,
                                        &arg_flag3,
                                        &arg_int,
                                        &arg_uint,
                                        &arg_double,
                                        &arg_str,
                                        &arg_strarr};
    static constexpr HyoutaUtils::Args args("test", "file.bin", "program description", args_array);

    {
        auto p = args.Parse(std::span<const std::string_view>());
        EXPECT_TRUE(p.IsSuccess());
        if (p.IsSuccess()) {
            EXPECT_EQ("", p.GetSuccessValue().ProgramName);
            EXPECT_EQ(0, p.GetSuccessValue().EvaluatedArguments.size());
            EXPECT_EQ(0, p.GetSuccessValue().FreeArguments.size());
        }
    }
    {
        auto p = args.Parse({{"name"}});
        EXPECT_TRUE(p.IsSuccess());
        if (p.IsSuccess()) {
            EXPECT_EQ("name", p.GetSuccessValue().ProgramName);
            EXPECT_EQ(0, p.GetSuccessValue().EvaluatedArguments.size());
            EXPECT_EQ(0, p.GetSuccessValue().FreeArguments.size());
        }
    }
    {
        auto p = args.Parse({{"name", "a1", "a2", "a3"}});
        EXPECT_TRUE(p.IsSuccess());
        if (p.IsSuccess()) {
            EXPECT_EQ("name", p.GetSuccessValue().ProgramName);
            EXPECT_EQ(0, p.GetSuccessValue().EvaluatedArguments.size());
            EXPECT_EQ(3, p.GetSuccessValue().FreeArguments.size());
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "a1", 0));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "a2", 1));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "a3", 2));
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "-1",
            "-2",
            "-i",
            "-50",
            "-u",
            "600",
            "-d",
            "5.5",
            "-s",
            "text",
            "-a",
            "e1",
            "-a",
            "e2",
            "-a",
            "e3",
            "arg1",
        }});
        EXPECT_TRUE(p.IsSuccess());
        if (p.IsSuccess()) {
            EXPECT_EQ("name", p.GetSuccessValue().ProgramName);
            EXPECT_EQ(7, p.GetSuccessValue().EvaluatedArguments.size());
            EXPECT_EQ(1, p.GetSuccessValue().FreeArguments.size());

            EXPECT_TRUE(p.GetSuccessValue().IsFlagSet(&arg_flag1));
            EXPECT_TRUE(p.GetSuccessValue().IsFlagSet(&arg_flag2));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_flag3));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_int));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_uint));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_double));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_str));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_strarr));

            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag3) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetInt64(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_int) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetUInt64(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_uint) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetDouble(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_double) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetString(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_str) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetStringArray(&arg_strarr) != nullptr);

            if (const auto* v = p.GetSuccessValue().TryGetInt64(&arg_int)) {
                EXPECT_EQ(-50, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetUInt64(&arg_uint)) {
                EXPECT_EQ(600, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetDouble(&arg_double)) {
                EXPECT_EQ(5.5, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetString(&arg_str)) {
                EXPECT_EQ("text", *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetStringArray(&arg_strarr)) {
                EXPECT_EQ(3, v->size());
                EXPECT_TRUE(ContainsAt(*v, "e1", 0));
                EXPECT_TRUE(ContainsAt(*v, "e2", 1));
                EXPECT_TRUE(ContainsAt(*v, "e3", 2));
            }

            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "arg1", 0));
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "-23i70",
            "-u800",
            "-d9.5",
            "-sstuff",
            "-ax1",
            "-ax2",
        }});
        EXPECT_TRUE(p.IsSuccess());
        if (p.IsSuccess()) {
            EXPECT_EQ("name", p.GetSuccessValue().ProgramName);
            EXPECT_EQ(7, p.GetSuccessValue().EvaluatedArguments.size());
            EXPECT_EQ(0, p.GetSuccessValue().FreeArguments.size());

            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_flag1));
            EXPECT_TRUE(p.GetSuccessValue().IsFlagSet(&arg_flag2));
            EXPECT_TRUE(p.GetSuccessValue().IsFlagSet(&arg_flag3));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_int));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_uint));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_double));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_str));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_strarr));

            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag3) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetInt64(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_int) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetUInt64(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_uint) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetDouble(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_double) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetString(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_str) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetStringArray(&arg_strarr) != nullptr);

            if (const auto* v = p.GetSuccessValue().TryGetInt64(&arg_int)) {
                EXPECT_EQ(70, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetUInt64(&arg_uint)) {
                EXPECT_EQ(800, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetDouble(&arg_double)) {
                EXPECT_EQ(9.5, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetString(&arg_str)) {
                EXPECT_EQ("stuff", *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetStringArray(&arg_strarr)) {
                EXPECT_EQ(2, v->size());
                EXPECT_TRUE(ContainsAt(*v, "x1", 0));
                EXPECT_TRUE(ContainsAt(*v, "x2", 1));
            }
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "free1",
            "--str",
            "str",
            "--flag3",
            "free2",
            "--strarr",
            "z1",
            "--uint",
            "6000",
            "--int",
            "-99",
            "--double",
            "5.75",
            "--strarr",
            "z2",
            "--strarr",
            "z3",
        }});
        EXPECT_TRUE(p.IsSuccess());
        if (p.IsSuccess()) {
            EXPECT_EQ("name", p.GetSuccessValue().ProgramName);
            EXPECT_EQ(6, p.GetSuccessValue().EvaluatedArguments.size());
            EXPECT_EQ(2, p.GetSuccessValue().FreeArguments.size());

            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_flag1));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_flag2));
            EXPECT_TRUE(p.GetSuccessValue().IsFlagSet(&arg_flag3));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_int));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_uint));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_double));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_str));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_strarr));

            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag3) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetInt64(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_int) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetUInt64(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_uint) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetDouble(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_double) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetString(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_str) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetStringArray(&arg_strarr) != nullptr);

            if (const auto* v = p.GetSuccessValue().TryGetInt64(&arg_int)) {
                EXPECT_EQ(-99, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetUInt64(&arg_uint)) {
                EXPECT_EQ(6000, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetDouble(&arg_double)) {
                EXPECT_EQ(5.75, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetString(&arg_str)) {
                EXPECT_EQ("str", *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetStringArray(&arg_strarr)) {
                EXPECT_EQ(3, v->size());
                EXPECT_TRUE(ContainsAt(*v, "z1", 0));
                EXPECT_TRUE(ContainsAt(*v, "z2", 1));
                EXPECT_TRUE(ContainsAt(*v, "z3", 2));
            }

            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "free1", 0));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "free2", 1));
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "--strarr=z1",
            "free1",
            "--strarr=z2",
            "--double=10.25",
            "--str=s?",
            "--uint=0",
            "--int=-1",
            "free2",
            "--flag2",
        }});
        EXPECT_TRUE(p.IsSuccess());
        if (p.IsSuccess()) {
            EXPECT_EQ("name", p.GetSuccessValue().ProgramName);
            EXPECT_EQ(6, p.GetSuccessValue().EvaluatedArguments.size());
            EXPECT_EQ(2, p.GetSuccessValue().FreeArguments.size());

            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_flag1));
            EXPECT_TRUE(p.GetSuccessValue().IsFlagSet(&arg_flag2));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_flag3));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_int));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_uint));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_double));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_str));
            EXPECT_FALSE(p.GetSuccessValue().IsFlagSet(&arg_strarr));

            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_flag3) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetInt64(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetInt64(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_int) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetUInt64(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetUInt64(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_uint) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetDouble(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetDouble(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_double) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetString(&arg_str) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetString(&arg_strarr) != nullptr);

            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag1) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag2) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_flag3) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_int) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_uint) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_double) != nullptr);
            EXPECT_FALSE(p.GetSuccessValue().TryGetStringArray(&arg_str) != nullptr);
            EXPECT_TRUE(p.GetSuccessValue().TryGetStringArray(&arg_strarr) != nullptr);

            if (const auto* v = p.GetSuccessValue().TryGetInt64(&arg_int)) {
                EXPECT_EQ(-1, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetUInt64(&arg_uint)) {
                EXPECT_EQ(0, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetDouble(&arg_double)) {
                EXPECT_EQ(10.25, *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetString(&arg_str)) {
                EXPECT_EQ("s?", *v);
            }
            if (const auto* v = p.GetSuccessValue().TryGetStringArray(&arg_strarr)) {
                EXPECT_EQ(2, v->size());
                EXPECT_TRUE(ContainsAt(*v, "z1", 0));
                EXPECT_TRUE(ContainsAt(*v, "z2", 1));
            }

            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "free1", 0));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "free2", 1));
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "free1",
            "--",
            "-s",
            "all",
            "--strarr=z1",
            "--strarr",
            "z2",
            "--double=10.25",
            "--str=s?",
            "--uint=0",
            "-i-1",
            "-13",
            "--flag2",
        }});
        EXPECT_TRUE(p.IsSuccess());
        if (p.IsSuccess()) {
            EXPECT_EQ("name", p.GetSuccessValue().ProgramName);
            EXPECT_EQ(0, p.GetSuccessValue().EvaluatedArguments.size());
            EXPECT_EQ(12, p.GetSuccessValue().FreeArguments.size());

            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "free1", 0));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "-s", 1));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "all", 2));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "--strarr=z1", 3));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "--strarr", 4));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "z2", 5));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "--double=10.25", 6));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "--str=s?", 7));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "--uint=0", 8));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "-i-1", 9));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "-13", 10));
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "--flag2", 11));
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "-",
        }});
        EXPECT_TRUE(p.IsSuccess());
        if (p.IsSuccess()) {
            EXPECT_EQ("name", p.GetSuccessValue().ProgramName);
            EXPECT_EQ(0, p.GetSuccessValue().EvaluatedArguments.size());
            EXPECT_EQ(1, p.GetSuccessValue().FreeArguments.size());
            EXPECT_TRUE(ContainsAt(p.GetSuccessValue().FreeArguments, "-", 0));
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "--fake-param",
            "blub",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "-x",
            "blub",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "--fake=blub",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "-xblub",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "-iNotAnInteger",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }
    {
        // unsigned should not accept negative number
        auto p = args.Parse({{
            "name",
            "-u",
            "-100",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "--double=NotAFloat",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "--str=",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }

    // same option multiple times is invalid for non-array types
    {
        auto p = args.Parse({{
            "name",
            "-i10",
            "-i20",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "-u10",
            "-u20",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "-d10",
            "-d20",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }
    {
        auto p = args.Parse({{
            "name",
            "-sTest",
            "--str=Text",
        }});
        EXPECT_TRUE(p.IsError());
        if (p.IsError()) {
            printf("%s\n", p.GetErrorValue().c_str());
        }
    }
}
