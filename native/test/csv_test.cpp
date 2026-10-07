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
    {
        static constexpr const char CSV[] = " ";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        EXPECT_EQ(1, csv.GetRowCount());
        EXPECT_EQ(1, csv.GetValues().size());
        if (csv.GetRowCount() == 1) {
            const auto& row = csv.GetRow(0);
            EXPECT_EQ(1, row.size());
            if (row.size() == 1) {
                EXPECT_EQ(" ", HyoutaUtils::Csv::GetStringView(CSV, row[0]));
            }
        }
    }
    {
        static constexpr const char CSV[] = " \n";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        EXPECT_EQ(1, csv.GetRowCount());
        EXPECT_EQ(1, csv.GetValues().size());
        if (csv.GetRowCount() == 1) {
            const auto& row = csv.GetRow(0);
            EXPECT_EQ(1, row.size());
            if (row.size() == 1) {
                EXPECT_EQ(" ", HyoutaUtils::Csv::GetStringView(CSV, row[0]));
            }
        }
    }
    {
        static constexpr const char CSV[] = " \n\t";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        EXPECT_EQ(2, csv.GetRowCount());
        EXPECT_EQ(2, csv.GetValues().size());
        if (csv.GetRowCount() == 2) {
            const auto& row0 = csv.GetRow(0);
            const auto& row1 = csv.GetRow(1);
            EXPECT_EQ(1, row0.size());
            EXPECT_EQ(1, row1.size());
            if (row0.size() == 1) {
                EXPECT_EQ(" ", HyoutaUtils::Csv::GetStringView(CSV, row0[0]));
            }
            if (row1.size() == 1) {
                EXPECT_EQ("\t", HyoutaUtils::Csv::GetStringView(CSV, row1[0]));
            }
        }
    }
    {
        static constexpr const char CSV[] = ",";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        EXPECT_EQ(1, csv.GetRowCount());
        EXPECT_EQ(2, csv.GetValues().size());
        if (csv.GetRowCount() == 1) {
            const auto& row = csv.GetRow(0);
            EXPECT_EQ(2, row.size());
            if (row.size() == 2) {
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[0]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[1]));
            }
        }
    }
    {
        static constexpr const char CSV[] = ",\n";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        EXPECT_EQ(1, csv.GetRowCount());
        EXPECT_EQ(2, csv.GetValues().size());
        if (csv.GetRowCount() == 1) {
            const auto& row = csv.GetRow(0);
            EXPECT_EQ(2, row.size());
            if (row.size() == 2) {
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[0]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[1]));
            }
        }
    }
    {
        static constexpr const char CSV[] = ",,,a,b,,,";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        EXPECT_EQ(1, csv.GetRowCount());
        EXPECT_EQ(8, csv.GetValues().size());
        if (csv.GetRowCount() == 1) {
            const auto& row = csv.GetRow(0);
            EXPECT_EQ(8, row.size());
            if (row.size() == 8) {
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[0]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[1]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[2]));
                EXPECT_EQ("a", HyoutaUtils::Csv::GetStringView(CSV, row[3]));
                EXPECT_EQ("b", HyoutaUtils::Csv::GetStringView(CSV, row[4]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[5]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[6]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[7]));
            }
        }
    }
    {
        static constexpr const char CSV[] = ",,,a,b,,,\n";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        EXPECT_EQ(1, csv.GetRowCount());
        EXPECT_EQ(8, csv.GetValues().size());
        if (csv.GetRowCount() == 1) {
            const auto& row = csv.GetRow(0);
            EXPECT_EQ(8, row.size());
            if (row.size() == 8) {
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[0]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[1]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[2]));
                EXPECT_EQ("a", HyoutaUtils::Csv::GetStringView(CSV, row[3]));
                EXPECT_EQ("b", HyoutaUtils::Csv::GetStringView(CSV, row[4]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[5]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[6]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[7]));
            }
        }
    }
    {
        static constexpr const char CSV[] = ",a,,b,,,c,,,,d,";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        EXPECT_EQ(1, csv.GetRowCount());
        EXPECT_EQ(12, csv.GetValues().size());
        if (csv.GetRowCount() == 1) {
            const auto& row = csv.GetRow(0);
            EXPECT_EQ(12, row.size());
            if (row.size() == 12) {
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[0]));
                EXPECT_EQ("a", HyoutaUtils::Csv::GetStringView(CSV, row[1]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[2]));
                EXPECT_EQ("b", HyoutaUtils::Csv::GetStringView(CSV, row[3]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[4]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[5]));
                EXPECT_EQ("c", HyoutaUtils::Csv::GetStringView(CSV, row[6]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[7]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[8]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[9]));
                EXPECT_EQ("d", HyoutaUtils::Csv::GetStringView(CSV, row[10]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[11]));
            }
        }
    }
    {
        static constexpr const char CSV[] = ",a,,b,,,c,,,,d,\n";
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(CSV, sizeof(CSV) - 1));
        EXPECT_EQ(1, csv.GetRowCount());
        EXPECT_EQ(12, csv.GetValues().size());
        if (csv.GetRowCount() == 1) {
            const auto& row = csv.GetRow(0);
            EXPECT_EQ(12, row.size());
            if (row.size() == 12) {
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[0]));
                EXPECT_EQ("a", HyoutaUtils::Csv::GetStringView(CSV, row[1]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[2]));
                EXPECT_EQ("b", HyoutaUtils::Csv::GetStringView(CSV, row[3]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[4]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[5]));
                EXPECT_EQ("c", HyoutaUtils::Csv::GetStringView(CSV, row[6]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[7]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[8]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[9]));
                EXPECT_EQ("d", HyoutaUtils::Csv::GetStringView(CSV, row[10]));
                EXPECT_EQ("", HyoutaUtils::Csv::GetStringView(CSV, row[11]));
            }
        }
    }
    {
        static constexpr const char CSV[] = "a,\"line\nbreak\",\"\rw\n\"\n\"x\ry\nz\"";
        std::array<char, sizeof(CSV) - 1> buffer;
        std::memcpy(buffer.data(), CSV, buffer.size());
        HyoutaUtils::Csv::CsvFile csv;
        EXPECT_TRUE(csv.ParseExternalMemory(buffer.data(), buffer.size()));
        EXPECT_EQ(2, csv.GetRowCount());
        EXPECT_EQ(4, csv.GetValues().size());
        if (csv.GetRowCount() == 2) {
            const auto& row0 = csv.GetRow(0);
            const auto& row1 = csv.GetRow(1);
            EXPECT_EQ(3, row0.size());
            EXPECT_EQ(1, row1.size());
            if (row0.size() == 3) {
                EXPECT_EQ("a", HyoutaUtils::Csv::GetStringView(buffer.data(), row0[0]));
                EXPECT_EQ("\"line\nbreak\"", HyoutaUtils::Csv::GetStringView(buffer.data(), row0[1]));
                EXPECT_EQ("\"\rw\n\"", HyoutaUtils::Csv::GetStringView(buffer.data(), row0[2]));
            }
            if (row1.size() == 1) {
                EXPECT_EQ("\"x\ry\nz\"", HyoutaUtils::Csv::GetStringView(buffer.data(), row1[0]));
            }
            EXPECT_TRUE(csv.UnescapeAllValues(buffer.data()));
            if (row0.size() == 3) {
                EXPECT_EQ("a", HyoutaUtils::Csv::GetStringView(buffer.data(), row0[0]));
                EXPECT_EQ("line\nbreak", HyoutaUtils::Csv::GetStringView(buffer.data(), row0[1]));
                EXPECT_EQ("\rw\n", HyoutaUtils::Csv::GetStringView(buffer.data(), row0[2]));
            }
            if (row1.size() == 1) {
                EXPECT_EQ("x\ry\nz", HyoutaUtils::Csv::GetStringView(buffer.data(), row1[0]));
            }
        }
    }
}
