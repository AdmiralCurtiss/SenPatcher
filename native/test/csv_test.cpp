#include <array>
#include <cstring>
#include <string_view>

#include "gtest/gtest.h"

#include "util/csv.h"

TEST(Csv, BasicTests) {
    {
        static constexpr const char CSV[] = "";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        ASSERT_EQ(0, csv.GetRowCount());
        ASSERT_EQ(0, csv.GetValues().size());
    }
    {
        static constexpr const char CSV[] = "\r\n";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        ASSERT_EQ(0, csv.GetRowCount());
        ASSERT_EQ(0, csv.GetValues().size());
    }
    {
        static constexpr const char CSV[] =
            "a,b,c\n"
            "d,ef";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        ASSERT_EQ(2, csv.GetRowCount());
        ASSERT_EQ(5, csv.GetValues().size());
        ASSERT_EQ(3, csv.GetRow(0).size());
        ASSERT_EQ(2, csv.GetRow(1).size());
        EXPECT_EQ("a", HyoutaUtils::Csv::GetStringView(CSV, csv.GetRow(0)[0]));
        EXPECT_EQ("b", HyoutaUtils::Csv::GetStringView(CSV, csv.GetRow(0)[1]));
        EXPECT_EQ("c", HyoutaUtils::Csv::GetStringView(CSV, csv.GetRow(0)[2]));
        EXPECT_EQ("d", HyoutaUtils::Csv::GetStringView(CSV, csv.GetRow(1)[0]));
        EXPECT_EQ("ef", HyoutaUtils::Csv::GetStringView(CSV, csv.GetRow(1)[1]));
    }
    {
        static constexpr const char CSV[] =
            "\r\n\n\n\r\r\r"
            "a,b,c\n\r\n\r\n"
            "d,ef\r\n\r";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        ASSERT_EQ(2, csv.GetRowCount());
        ASSERT_EQ(5, csv.GetValues().size());
        ASSERT_EQ(3, csv.GetRow(0).size());
        ASSERT_EQ(2, csv.GetRow(1).size());
        EXPECT_EQ("a", HyoutaUtils::Csv::GetStringView(CSV, csv.GetRow(0)[0]));
        EXPECT_EQ("b", HyoutaUtils::Csv::GetStringView(CSV, csv.GetRow(0)[1]));
        EXPECT_EQ("c", HyoutaUtils::Csv::GetStringView(CSV, csv.GetRow(0)[2]));
        EXPECT_EQ("d", HyoutaUtils::Csv::GetStringView(CSV, csv.GetRow(1)[0]));
        EXPECT_EQ("ef", HyoutaUtils::Csv::GetStringView(CSV, csv.GetRow(1)[1]));
    }
    {
        static constexpr const char CSV[] =
            "\"a\",b,\"c\r\n\"\n"
            "\"d,x\",,\"\",\"e\"\"f\"";
        std::array<char, sizeof(CSV) - 1> buffer;
        std::memcpy(buffer.data(), CSV, buffer.size());
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(buffer.data(), buffer.size()));
        ASSERT_EQ(2, csv.GetRowCount());
        ASSERT_EQ(7, csv.GetValues().size());
        ASSERT_EQ(3, csv.GetRow(0).size());
        ASSERT_EQ(4, csv.GetRow(1).size());
        EXPECT_EQ("\"a\"", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[0]));
        EXPECT_EQ("b", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[1]));
        EXPECT_EQ("\"c\r\n\"", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[2]));
        EXPECT_EQ("\"d,x\"", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(1)[0]));
        EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(1)[1]));
        EXPECT_EQ("\"\"", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(1)[2]));
        EXPECT_EQ("\"e\"\"f\"", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(1)[3]));
        EXPECT_TRUE(csv.UnescapeAllValues(buffer.data()));
        EXPECT_EQ("a", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[0]));
        EXPECT_EQ("b", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[1]));
        EXPECT_EQ("c\r\n", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[2]));
        EXPECT_EQ("d,x", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(1)[0]));
        EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(1)[1]));
        EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(1)[2]));
        EXPECT_EQ("e\"f", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(1)[3]));
    }
    {
        static constexpr const char CSV[] = "\"\",\"\"\"\",\"\"\"\"\"\"";
        std::array<char, sizeof(CSV) - 1> buffer;
        std::memcpy(buffer.data(), CSV, buffer.size());
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(buffer.data(), buffer.size()));
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        ASSERT_EQ(1, csv.GetRowCount());
        ASSERT_EQ(3, csv.GetValues().size());
        ASSERT_EQ(3, csv.GetRow(0).size());
        EXPECT_EQ("\"\"", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[0]));
        EXPECT_EQ("\"\"\"\"", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[1]));
        EXPECT_EQ("\"\"\"\"\"\"", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[2]));
        EXPECT_TRUE(csv.UnescapeAllValues(buffer.data()));
        EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[0]));
        EXPECT_EQ("\"", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[1]));
        EXPECT_EQ("\"\"", HyoutaUtils::Csv::GetStringView(buffer.data(), csv.GetRow(0)[2]));
    }
    {
        static constexpr const char CSV[] = "\"";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_FALSE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
    }
    {
        static constexpr const char CSV[] = "\"\"\"";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_FALSE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
    }
    {
        static constexpr const char CSV[] = "\"\"\"\"\"";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_FALSE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
    }
    {
        static constexpr const char CSV[] = "\"a,b";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_FALSE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
    }
    {
        static constexpr const char CSV[] = "a\",b";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_FALSE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
    }
    {
        static constexpr const char CSV[] = "a,\"b";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_FALSE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
    }
    {
        static constexpr const char CSV[] = "a,b\"";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_FALSE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
    }
}
