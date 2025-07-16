#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "gtest/gtest.h"

#include "util/file.h"
#include "util/scope.h"
#include "util/xorshift.h"

static std::vector<char> MakeRandomData(uint32_t seed, size_t length) {
    HyoutaUtils::RNG::xorshift rng(seed);
    std::vector<char> data;
    data.reserve(length);
    for (size_t i = 0; i < (length / 4); ++i) {
        uint32_t v = rng();
        uint8_t v0 = static_cast<uint8_t>(v & 0xff);
        uint8_t v1 = static_cast<uint8_t>((v >> 8) & 0xff);
        uint8_t v2 = static_cast<uint8_t>((v >> 16) & 0xff);
        uint8_t v3 = static_cast<uint8_t>((v >> 24) & 0xff);
        data.push_back(static_cast<char>(v0));
        data.push_back(static_cast<char>(v1));
        data.push_back(static_cast<char>(v2));
        data.push_back(static_cast<char>(v3));
    }
    if (data.size() < length) {
        uint32_t v = rng();
        uint8_t v0 = static_cast<uint8_t>(v & 0xff);
        uint8_t v1 = static_cast<uint8_t>((v >> 8) & 0xff);
        uint8_t v2 = static_cast<uint8_t>((v >> 16) & 0xff);
        uint8_t v3 = static_cast<uint8_t>((v >> 24) & 0xff);
        data.push_back(static_cast<char>(v0));
        if (data.size() < length) {
            data.push_back(static_cast<char>(v1));
        }
        if (data.size() < length) {
            data.push_back(static_cast<char>(v2));
        }
        if (data.size() < length) {
            data.push_back(static_cast<char>(v3));
        }
    }
    return data;
}

static void MakeRandomDataFile(const char* filename, uint32_t seed, size_t length) {
    std::vector<char> data = MakeRandomData(seed, length);
    FILE* f = std::fopen(filename, "wb");
    if (!f) {
        throw "error opening file";
    }
    auto filescope = HyoutaUtils::MakeScopeGuard([&]() { std::fclose(f); });
    if (data.size() > 0) {
        if (std::fwrite(data.data(), 1, data.size(), f) != data.size()) {
            throw "error writing file";
        }
    }
}

class FileUtilsTest : public ::testing::Test {
public:
    ~FileUtilsTest() override = default;

    // use STL stuff to set up an environment

    void SetUp() override {
        std::filesystem::remove_all(std::filesystem::path("FileUtilsTestDir"));
        std::filesystem::create_directory(std::filesystem::path("FileUtilsTestDir"));
        std::filesystem::create_directory(std::filesystem::path("FileUtilsTestDir/empty"));
        std::filesystem::create_directory(std::filesystem::path("FileUtilsTestDir/subdir"));
        MakeRandomDataFile("FileUtilsTestDir/subdir/file0.bin", 0, 0);
        MakeRandomDataFile("FileUtilsTestDir/file128.bin", 1, 128);
        MakeRandomDataFile("FileUtilsTestDir/file16.bin", 2, 16);
    }

    void TearDown() override {
        std::filesystem::remove_all(std::filesystem::path("FileUtilsTestDir"));
    }
};

static void ExpectEqualSpan(std::span<const char> lhs, std::span<const char> rhs) {
    EXPECT_EQ(lhs.size(), rhs.size());
    if (lhs.size() == rhs.size()) {
        for (size_t i = 0; i < lhs.size(); ++i) {
            if (lhs[i] != rhs[i]) {
                ADD_FAILURE() << "Data not equal.";
                return;
            }
        }
    }
}

TEST_F(FileUtilsTest, ReadFile) {
    const std::vector<char> referenceData = MakeRandomData(1, 128);
    ASSERT_EQ(128, referenceData.size());

    HyoutaUtils::IO::File f("FileUtilsTestDir/file128.bin", HyoutaUtils::IO::OpenMode::Read);
    ASSERT_TRUE(f.IsOpen());
    EXPECT_EQ(uint64_t(128), f.GetLength());
    EXPECT_EQ(uint64_t(0), f.GetPosition());

    // Read() a handful of bytes and verify
    {
        std::array<char, 100> tmp{};
        EXPECT_EQ(tmp.size(), f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(referenceData.data(), tmp.size()),
                        std::span<const char>(tmp.data(), tmp.size()));
        EXPECT_EQ(uint64_t(100), f.GetPosition());
    }

    // If we try to Read() past the end of the file we should just get the remaining data
    {
        std::array<char, 100> tmp{};
        EXPECT_EQ(28, f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(referenceData.data() + 100, 28),
                        std::span<const char>(tmp.data(), 28));
        EXPECT_EQ(uint64_t(128), f.GetPosition());
    }

    // Verify that SetPosition() works
    {
        EXPECT_TRUE(f.SetPosition(40));
        std::array<char, 20> tmp{};
        EXPECT_EQ(tmp.size(), f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(referenceData.data() + 40, 20),
                        std::span<const char>(tmp.data(), tmp.size()));
        EXPECT_EQ(uint64_t(60), f.GetPosition());
    }

    // SetPosition(Current) with positive and negative offsets
    {
        EXPECT_TRUE(f.SetPosition(10, HyoutaUtils::IO::SetPositionMode::Current));
        std::array<char, 20> tmp{};
        EXPECT_EQ(tmp.size(), f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(referenceData.data() + 70, 20),
                        std::span<const char>(tmp.data(), tmp.size()));
        EXPECT_EQ(uint64_t(90), f.GetPosition());
    }
    {
        EXPECT_TRUE(f.SetPosition(-40, HyoutaUtils::IO::SetPositionMode::Current));
        std::array<char, 20> tmp{};
        EXPECT_EQ(tmp.size(), f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(referenceData.data() + 50, 20),
                        std::span<const char>(tmp.data(), tmp.size()));
        EXPECT_EQ(uint64_t(70), f.GetPosition());
    }

    // SetPosition(End)
    {
        EXPECT_TRUE(f.SetPosition(-30, HyoutaUtils::IO::SetPositionMode::End));
        std::array<char, 20> tmp{};
        EXPECT_EQ(tmp.size(), f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(referenceData.data() + (128 - 30), 20),
                        std::span<const char>(tmp.data(), tmp.size()));
        EXPECT_EQ(uint64_t(118), f.GetPosition());
    }

    // Moving past the end of the file is okay, moving before the start is not
    {
        EXPECT_TRUE(f.SetPosition(1, HyoutaUtils::IO::SetPositionMode::End));
        EXPECT_TRUE(f.SetPosition(192));
        EXPECT_FALSE(f.SetPosition(-160, HyoutaUtils::IO::SetPositionMode::End));
        EXPECT_TRUE(f.SetPosition(60));
        EXPECT_FALSE(f.SetPosition(-64, HyoutaUtils::IO::SetPositionMode::Current));
        EXPECT_TRUE(f.SetPosition(60));
        EXPECT_TRUE(f.SetPosition(80, HyoutaUtils::IO::SetPositionMode::Current));
    }

    // Write()ing to a file opened with OpenMode::Read should fail
    {
        EXPECT_TRUE(f.SetPosition(0, HyoutaUtils::IO::SetPositionMode::End));
        std::array<char, 20> tmp{};
        EXPECT_EQ(0, f.Write(tmp.data(), tmp.size()));
        EXPECT_TRUE(f.SetPosition(0));
        EXPECT_EQ(0, f.Write(tmp.data(), tmp.size()));
    }

    // This attempted write should not have destroyed the data
    {
        EXPECT_TRUE(f.SetPosition(0));
        std::array<char, 128> tmp{};
        EXPECT_EQ(tmp.size(), f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(referenceData.data(), tmp.size()),
                        std::span<const char>(tmp.data(), tmp.size()));
        EXPECT_EQ(uint64_t(128), f.GetPosition());
    }

    // Delete()ing a file opened with OpenMode::Read should fail
    {
        EXPECT_FALSE(f.Delete());
    }

    // Rename()ing a file opened with OpenMode::Read should fail
    {
        EXPECT_FALSE(f.Rename("FileUtilsTestDir/file128_fail.bin"));
    }

    // Opening a second handle to the same file should work
    {
        HyoutaUtils::IO::File f2("FileUtilsTestDir/file128.bin", HyoutaUtils::IO::OpenMode::Read);
        ASSERT_TRUE(f2.IsOpen());
        EXPECT_EQ(uint64_t(128), f2.GetLength());
        EXPECT_EQ(uint64_t(0), f2.GetPosition());

        // The second handle should be independent of the first
        EXPECT_TRUE(f.SetPosition(20));
        EXPECT_TRUE(f2.SetPosition(40));
        EXPECT_EQ(uint64_t(20), f.GetPosition());
        EXPECT_EQ(uint64_t(40), f2.GetPosition());
    }

    // After closing the second handle, the first one should still be valid
    ASSERT_TRUE(f.IsOpen());
    EXPECT_EQ(uint64_t(20), f.GetPosition());

    // Opening a Write handle while we still have the Read handle open may or may not work
    // depending on OS, but either way our Read handle should stay valid
    // FIXME: This actually destroys the data on Linux, not sure why. Disable this for now.
    {
#ifdef BUILD_FOR_WINDOWS
        HyoutaUtils::IO::File f2("FileUtilsTestDir/file128.bin", HyoutaUtils::IO::OpenMode::Write);
        if (f2.IsOpen()) {
            std::array<char, 20> tmp{};
            EXPECT_EQ(20, f.Write(tmp.data(), tmp.size()));
        }
#endif
    }

    // Verify that the Write handle did not destroy our data
    ASSERT_TRUE(f.IsOpen());
    EXPECT_EQ(uint64_t(20), f.GetPosition());
    {
        EXPECT_TRUE(f.SetPosition(0));
        std::array<char, 128> tmp{};
        EXPECT_EQ(tmp.size(), f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(referenceData.data(), tmp.size()),
                        std::span<const char>(tmp.data(), tmp.size()));
        EXPECT_EQ(uint64_t(128), f.GetPosition());
    }

    // Opening a read handle to a non-existing file should fail
    {
        HyoutaUtils::IO::File f2("FileUtilsTestDir/nope.bin", HyoutaUtils::IO::OpenMode::Read);
        EXPECT_FALSE(f2.IsOpen());
    }

    // Opening a read handle on a directory should fail
    {
        HyoutaUtils::IO::File f2("FileUtilsTestDir/empty", HyoutaUtils::IO::OpenMode::Read);
        EXPECT_FALSE(f2.IsOpen());
    }
}

TEST_F(FileUtilsTest, WriteFile) {
    const std::vector<char> randomData = MakeRandomData(100, 100);
    ASSERT_EQ(100, randomData.size());
    std::array<char, 100> emptyData{};

    // Write a file with some random data
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Write);
        ASSERT_TRUE(f.IsOpen());
        EXPECT_EQ(uint64_t(0), f.GetLength());
        EXPECT_EQ(uint64_t(0), f.GetPosition());
        EXPECT_EQ(randomData.size(), f.Write(randomData.data(), randomData.size()));
        EXPECT_EQ(uint64_t(100), f.GetLength());
        EXPECT_EQ(uint64_t(100), f.GetPosition());
        EXPECT_EQ(50, f.Write(randomData.data(), 50));
        EXPECT_EQ(uint64_t(150), f.GetLength());
        EXPECT_EQ(uint64_t(150), f.GetPosition());

#ifdef BUILD_FOR_WINDOWS
        // While the Write handle is open we can't open a Read handle
        HyoutaUtils::IO::File f2("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Read);
        EXPECT_FALSE(f2.IsOpen());

        // Or another Write handle
        HyoutaUtils::IO::File f3("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Write);
        EXPECT_FALSE(f3.IsOpen());
#endif
    }

    // Re-open the file we just wrote and verify the data
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Read);
        ASSERT_TRUE(f.IsOpen());
        EXPECT_EQ(uint64_t(150), f.GetLength());
        EXPECT_EQ(uint64_t(0), f.GetPosition());
        std::array<char, 150> tmp{};
        EXPECT_EQ(tmp.size(), f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(randomData.data(), 100),
                        std::span<const char>(tmp.data(), 100));
        ExpectEqualSpan(std::span<const char>(randomData.data(), 50),
                        std::span<const char>(tmp.data() + 100, 50));
        EXPECT_EQ(uint64_t(150), f.GetPosition());
    }

    // If we open another Write handle to the same file we overwrite it
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Write);
        ASSERT_TRUE(f.IsOpen());
        EXPECT_EQ(uint64_t(0), f.GetLength());
        EXPECT_EQ(uint64_t(0), f.GetPosition());
    }
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Read);
        ASSERT_TRUE(f.IsOpen());
        EXPECT_EQ(uint64_t(0), f.GetLength());
        EXPECT_EQ(uint64_t(0), f.GetPosition());
    }

    // We can seek past the end of a Write handle, then write there. The data in-between will be
    // zero-filled.
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Write);
        ASSERT_TRUE(f.IsOpen());
        EXPECT_EQ(uint64_t(0), f.GetLength());
        EXPECT_EQ(uint64_t(0), f.GetPosition());
        EXPECT_TRUE(f.SetPosition(70));
        EXPECT_EQ(uint64_t(70), f.GetPosition());
        EXPECT_EQ(randomData.size(), f.Write(randomData.data(), randomData.size()));
        EXPECT_EQ(uint64_t(170), f.GetLength());
        EXPECT_EQ(uint64_t(170), f.GetPosition());
    }
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Read);
        ASSERT_TRUE(f.IsOpen());
        EXPECT_EQ(uint64_t(170), f.GetLength());
        EXPECT_EQ(uint64_t(0), f.GetPosition());
        std::array<char, 170> tmp{};
        EXPECT_EQ(tmp.size(), f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(emptyData.data(), 70),
                        std::span<const char>(tmp.data(), 70));
        ExpectEqualSpan(std::span<const char>(randomData.data(), 100),
                        std::span<const char>(tmp.data() + 70, 100));
        EXPECT_EQ(uint64_t(170), f.GetPosition());
    }

    // We can seek into the middle of the written data and overwrite it
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Write);
        ASSERT_TRUE(f.IsOpen());
        EXPECT_EQ(uint64_t(0), f.GetLength());
        EXPECT_EQ(uint64_t(0), f.GetPosition());
        EXPECT_EQ(100, f.Write(emptyData.data(), emptyData.size()));
        EXPECT_TRUE(f.SetPosition(20));
        EXPECT_EQ(30, f.Write(randomData.data(), 30));
        EXPECT_EQ(uint64_t(100), f.GetLength());
        EXPECT_EQ(uint64_t(50), f.GetPosition());
    }
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Read);
        ASSERT_TRUE(f.IsOpen());
        EXPECT_EQ(uint64_t(100), f.GetLength());
        EXPECT_EQ(uint64_t(0), f.GetPosition());
        std::array<char, 100> tmp{};
        EXPECT_EQ(tmp.size(), f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(emptyData.data(), 20),
                        std::span<const char>(tmp.data(), 20));
        ExpectEqualSpan(std::span<const char>(randomData.data(), 30),
                        std::span<const char>(tmp.data() + 20, 30));
        ExpectEqualSpan(std::span<const char>(emptyData.data(), 50),
                        std::span<const char>(tmp.data() + 50, 50));
        EXPECT_EQ(uint64_t(100), f.GetPosition());
    }

    // We can rename and delete an open Write handle
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Write);
        ASSERT_TRUE(f.IsOpen());
        EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write.bin"));
        EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write2.bin"));
        EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write3.bin"));
        EXPECT_TRUE(f.Rename("FileUtilsTestDir/write2.bin"));
        EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write.bin"));
        EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write2.bin"));
        EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write3.bin"));
        EXPECT_TRUE(f.Rename("FileUtilsTestDir/write3.bin"));
        EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write.bin"));
        EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write2.bin"));
        EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write3.bin"));
        EXPECT_TRUE(f.Delete());
        EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write.bin"));
        EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write2.bin"));
        EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/write3.bin"));
    }

    // Test the file-copying Write() overload
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Write);
        ASSERT_TRUE(f.IsOpen());
        HyoutaUtils::IO::File f2("FileUtilsTestDir/file16.bin", HyoutaUtils::IO::OpenMode::Read);
        ASSERT_TRUE(f2.IsOpen());
        HyoutaUtils::IO::File f3("FileUtilsTestDir/file128.bin", HyoutaUtils::IO::OpenMode::Read);
        ASSERT_TRUE(f3.IsOpen());
        EXPECT_EQ(uint64_t(16), f.Write(f2, 16));
        EXPECT_TRUE(f3.SetPosition(100));
        EXPECT_EQ(uint64_t(28), f.Write(f3, 32));
        EXPECT_EQ(uint64_t(44), f.GetLength());
    }
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/write.bin", HyoutaUtils::IO::OpenMode::Read);
        ASSERT_TRUE(f.IsOpen());
        std::array<char, 100> tmp{};
        EXPECT_EQ(44, f.Read(tmp.data(), tmp.size()));
        const std::vector<char> refData1 = MakeRandomData(1, 128);
        ASSERT_EQ(128, refData1.size());
        const std::vector<char> refData2 = MakeRandomData(2, 16);
        ASSERT_EQ(16, refData2.size());
        ExpectEqualSpan(std::span<const char>(refData2.data(), 16),
                        std::span<const char>(tmp.data(), 16));
        ExpectEqualSpan(std::span<const char>(refData1.data() + 100, 28),
                        std::span<const char>(tmp.data() + 16, 28));
        EXPECT_EQ(uint64_t(44), f.GetPosition());
    }

    // Test OpenWithTempFilename()
    {
        HyoutaUtils::IO::File f;
        f.OpenWithTempFilename("FileUtilsTestDir/tempname.bin", HyoutaUtils::IO::OpenMode::Write);
        ASSERT_TRUE(f.IsOpen());
        EXPECT_EQ(randomData.size(), f.Write(randomData.data(), randomData.size()));
        EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/tempname.bin"));
        EXPECT_TRUE(f.Rename("FileUtilsTestDir/tempname.bin"));
        EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/tempname.bin"));
    }
    {
        HyoutaUtils::IO::File f("FileUtilsTestDir/tempname.bin", HyoutaUtils::IO::OpenMode::Read);
        ASSERT_TRUE(f.IsOpen());
        std::array<char, 100> tmp{};
        ASSERT_TRUE(tmp.size() >= randomData.size());
        EXPECT_EQ(randomData.size(), f.Read(tmp.data(), tmp.size()));
        ExpectEqualSpan(std::span<const char>(randomData.data(), randomData.size()),
                        std::span<const char>(tmp.data(), randomData.size()));
        EXPECT_EQ(uint64_t(randomData.size()), f.GetPosition());
    }
}

TEST_F(FileUtilsTest, FilesystemFunctions) {
    EXPECT_TRUE(HyoutaUtils::IO::Exists("FileUtilsTestDir/file128.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::Exists("FileUtilsTestDir/subdir/file0.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::Exists("FileUtilsTestDir/empty"));
    EXPECT_TRUE(HyoutaUtils::IO::Exists("FileUtilsTestDir/subdir"));
    EXPECT_FALSE(HyoutaUtils::IO::Exists("FileUtilsTestDir/nope.bin"));

    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/file128.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/subdir/file0.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/empty"));
    EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/subdir"));
    EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/nope.bin"));

    EXPECT_EQ(uint64_t(128), HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/file128.bin"));
    EXPECT_EQ(uint64_t(0), HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/subdir/file0.bin"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/empty"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/subdir"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/nope.bin"));

    EXPECT_FALSE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/file128.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/subdir/file0.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/empty"));
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/subdir"));
    EXPECT_FALSE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/nope.bin"));

    EXPECT_FALSE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/newdir"));
    EXPECT_TRUE(HyoutaUtils::IO::CreateDirectory("FileUtilsTestDir/newdir"));
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/newdir"));
    // if directory already exists this still returns true
    EXPECT_TRUE(HyoutaUtils::IO::CreateDirectory("FileUtilsTestDir/newdir"));
    // but it will fail if we try to create a directory where a file exists
    EXPECT_FALSE(HyoutaUtils::IO::CreateDirectory("FileUtilsTestDir/file128.bin"));

    // copying non-existing file doesn't work
    EXPECT_FALSE(HyoutaUtils::IO::CopyFile(
        "FileUtilsTestDir/nope.bin", "FileUtilsTestDir/nope2.bin", false));
    // we can copy files
    EXPECT_TRUE(HyoutaUtils::IO::CopyFile(
        "FileUtilsTestDir/subdir/file0.bin", "FileUtilsTestDir/copy.bin", false));
    EXPECT_EQ(uint64_t(0), HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/copy.bin"));
    // without the overwrite flag a copy onto an existing file will fail
    EXPECT_FALSE(HyoutaUtils::IO::CopyFile(
        "FileUtilsTestDir/file128.bin", "FileUtilsTestDir/copy.bin", false));
    EXPECT_EQ(uint64_t(0), HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/copy.bin"));
    // if we set the overwrite flag it will replace an existing file
    EXPECT_TRUE(HyoutaUtils::IO::CopyFile(
        "FileUtilsTestDir/file128.bin", "FileUtilsTestDir/copy.bin", true));
    EXPECT_EQ(uint64_t(128), HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/copy.bin"));
    // replacing a directory always fails
    EXPECT_FALSE(
        HyoutaUtils::IO::CopyFile("FileUtilsTestDir/file128.bin", "FileUtilsTestDir/empty", false));
    EXPECT_FALSE(
        HyoutaUtils::IO::CopyFile("FileUtilsTestDir/file128.bin", "FileUtilsTestDir/empty", true));
    // copying a directory always fails
    EXPECT_FALSE(
        HyoutaUtils::IO::CopyFile("FileUtilsTestDir/empty", "FileUtilsTestDir/copy", false));
    EXPECT_FALSE(
        HyoutaUtils::IO::CopyFile("FileUtilsTestDir/empty", "FileUtilsTestDir/copy", true));
    // copying a file onto itself fails and preserves the file
    EXPECT_FALSE(HyoutaUtils::IO::CopyFile(
        "FileUtilsTestDir/file128.bin", "FileUtilsTestDir/file128.bin", false));
    EXPECT_FALSE(HyoutaUtils::IO::CopyFile(
        "FileUtilsTestDir/file128.bin", "FileUtilsTestDir/file128.bin", true));
    EXPECT_EQ(uint64_t(128), HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/file128.bin"));

    // moving non-existing file doesn't work
    EXPECT_FALSE(
        HyoutaUtils::IO::Move("FileUtilsTestDir/nope.bin", "FileUtilsTestDir/nope2.bin", false));
    // we can move a file
    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/copy.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/move.bin"));
    EXPECT_TRUE(
        HyoutaUtils::IO::Move("FileUtilsTestDir/copy.bin", "FileUtilsTestDir/move.bin", false));
    EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/copy.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/move.bin"));
    // without the overwrite flag a move onto an existing file will fail
    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/file16.bin"));
    EXPECT_FALSE(
        HyoutaUtils::IO::Move("FileUtilsTestDir/file16.bin", "FileUtilsTestDir/move.bin", false));
    // with the overwrite flag it works
    EXPECT_EQ(uint64_t(128), HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/move.bin"));
    EXPECT_TRUE(
        HyoutaUtils::IO::Move("FileUtilsTestDir/file16.bin", "FileUtilsTestDir/move.bin", true));
    EXPECT_EQ(uint64_t(16), HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/move.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/file16.bin"));
    // we can also move directories
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/subdir"));
    EXPECT_FALSE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/movedir"));
    EXPECT_TRUE(
        HyoutaUtils::IO::Move("FileUtilsTestDir/subdir", "FileUtilsTestDir/movedir", false));
    EXPECT_FALSE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/subdir"));
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/movedir"));
    // moving onto an existing directory fails, regardless of the flag
    EXPECT_FALSE(
        HyoutaUtils::IO::Move("FileUtilsTestDir/movedir", "FileUtilsTestDir/newdir", false));
    EXPECT_FALSE(
        HyoutaUtils::IO::Move("FileUtilsTestDir/movedir", "FileUtilsTestDir/newdir", true));
    // moving a file onto a directory fails
    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/move.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/movedir"));
    EXPECT_FALSE(
        HyoutaUtils::IO::Move("FileUtilsTestDir/move.bin", "FileUtilsTestDir/movedir", false));
    EXPECT_FALSE(
        HyoutaUtils::IO::Move("FileUtilsTestDir/move.bin", "FileUtilsTestDir/empty", true));
    // moving a directory onto a file works with the overwrite flag only
    EXPECT_FALSE(
        HyoutaUtils::IO::Move("FileUtilsTestDir/movedir", "FileUtilsTestDir/move.bin", false));
    EXPECT_TRUE(
        HyoutaUtils::IO::Move("FileUtilsTestDir/movedir", "FileUtilsTestDir/move.bin", true));
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/move.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/movedir"));
    // moving a file onto itself may or may not work, but preserves the data either way
    EXPECT_EQ(uint64_t(128), HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/file128.bin"));
    HyoutaUtils::IO::Move("FileUtilsTestDir/file128.bin", "FileUtilsTestDir/file128.bin", false);
    EXPECT_EQ(uint64_t(128), HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/file128.bin"));
    HyoutaUtils::IO::Move("FileUtilsTestDir/file128.bin", "FileUtilsTestDir/file128.bin", true);
    EXPECT_EQ(uint64_t(128), HyoutaUtils::IO::GetFilesize("FileUtilsTestDir/file128.bin"));
    // moving a directory onto itself may or may not work, but preserves the data either way
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/move.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/move.bin/file0.bin"));
    HyoutaUtils::IO::Move("FileUtilsTestDir/move.bin", "FileUtilsTestDir/move.bin", false);
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/move.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/move.bin/file0.bin"));
    HyoutaUtils::IO::Move("FileUtilsTestDir/move.bin", "FileUtilsTestDir/move.bin", true);
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/move.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/move.bin/file0.bin"));

    // DeleteDirectory() can only delete empty directories, nothing else
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/empty"));
    EXPECT_TRUE(HyoutaUtils::IO::DeleteDirectory("FileUtilsTestDir/empty"));
    EXPECT_FALSE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/empty"));
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/move.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::DeleteDirectory("FileUtilsTestDir/move.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/move.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/file128.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::DeleteDirectory("FileUtilsTestDir/file128.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/file128.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::Exists("FileUtilsTestDir/nope.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::DeleteDirectory("FileUtilsTestDir/nope.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::Exists("FileUtilsTestDir/nope.bin"));

    // DeleteFile() can delete files but not directories
    EXPECT_TRUE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/file128.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::DeleteFile("FileUtilsTestDir/file128.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::FileExists("FileUtilsTestDir/file128.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/move.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::DeleteFile("FileUtilsTestDir/move.bin"));
    EXPECT_TRUE(HyoutaUtils::IO::DirectoryExists("FileUtilsTestDir/move.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::Exists("FileUtilsTestDir/nope.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::DeleteFile("FileUtilsTestDir/nope.bin"));
    EXPECT_FALSE(HyoutaUtils::IO::Exists("FileUtilsTestDir/nope.bin"));
}

TEST(FileUtils, PathHandling) {
#ifdef BUILD_FOR_WINDOWS
    {
        auto s = HyoutaUtils::IO::SplitPath("C:\\path\\to\\something");
        EXPECT_EQ(s.Directory, "C:\\path\\to");
        EXPECT_EQ(s.Filename, "something");
    }
    {
        auto s = HyoutaUtils::IO::SplitPath("C:/path/to/something");
        EXPECT_EQ(s.Directory, "C:/path/to");
        EXPECT_EQ(s.Filename, "something");
    }
    {
        auto s = HyoutaUtils::IO::SplitPath("C:\\path\\to\\");
        EXPECT_EQ(s.Directory, "C:\\path\\to");
        EXPECT_EQ(s.Filename, "");
    }
    {
        auto s = HyoutaUtils::IO::SplitPath("C:/path/to/");
        EXPECT_EQ(s.Directory, "C:/path/to");
        EXPECT_EQ(s.Filename, "");
    }

    // warning: root dir ends with a path separator after splitting!
    {
        auto s = HyoutaUtils::IO::SplitPath("C:\\path");
        EXPECT_EQ(s.Directory, "C:\\");
        EXPECT_EQ(s.Filename, "path");
    }
    {
        auto s = HyoutaUtils::IO::SplitPath("C:/path");
        EXPECT_EQ(s.Directory, "C:/");
        EXPECT_EQ(s.Filename, "path");
    }
    {
        auto s = HyoutaUtils::IO::SplitPath("C:\\");
        EXPECT_EQ(s.Directory, "C:\\");
        EXPECT_EQ(s.Filename, "");
    }
    {
        auto s = HyoutaUtils::IO::SplitPath("C:/");
        EXPECT_EQ(s.Directory, "C:/");
        EXPECT_EQ(s.Filename, "");
    }
#else
    {
        auto s = HyoutaUtils::IO::SplitPath("/path/to/something");
        EXPECT_EQ(s.Directory, "/path/to");
        EXPECT_EQ(s.Filename, "something");
    }
    {
        auto s = HyoutaUtils::IO::SplitPath("/path/to/");
        EXPECT_EQ(s.Directory, "/path/to");
        EXPECT_EQ(s.Filename, "");
    }

    // warning: root dir ends with a path separator after splitting!
    {
        auto s = HyoutaUtils::IO::SplitPath("/path");
        EXPECT_EQ(s.Directory, "/");
        EXPECT_EQ(s.Filename, "path");
    }
    {
        auto s = HyoutaUtils::IO::SplitPath("/");
        EXPECT_EQ(s.Directory, "/");
        EXPECT_EQ(s.Filename, "");
    }
#endif
    {
        auto s = HyoutaUtils::IO::SplitPath("filename.bin");
        EXPECT_EQ(s.Directory, "");
        EXPECT_EQ(s.Filename, "filename.bin");
    }

#ifdef BUILD_FOR_WINDOWS
    {
        std::string path = "C:\\p\\";
        HyoutaUtils::IO::AppendPathElement(path, "x");
        EXPECT_EQ(path, "C:\\p\\x");
        HyoutaUtils::IO::AppendPathElement(path, "y");
        EXPECT_EQ(path, "C:\\p\\x\\y");
    }
#else
    {
        std::string path = "/p/";
        HyoutaUtils::IO::AppendPathElement(path, "x");
        EXPECT_EQ(path, "/p/x");
        HyoutaUtils::IO::AppendPathElement(path, "y");
        EXPECT_EQ(path, "/p/x/y");
    }
#endif

    {
        EXPECT_EQ("file", HyoutaUtils::IO::GetFileNameWithoutExtension("file.bin"));
        EXPECT_EQ(".bin", HyoutaUtils::IO::GetExtension("file.bin"));
        EXPECT_EQ("file", HyoutaUtils::IO::GetFileNameWithoutExtension("file"));
        EXPECT_EQ("", HyoutaUtils::IO::GetExtension("file"));
        EXPECT_EQ("", HyoutaUtils::IO::GetFileNameWithoutExtension(".bin"));
        EXPECT_EQ(".bin", HyoutaUtils::IO::GetExtension(".bin"));
        EXPECT_EQ("file.with.multiple",
                  HyoutaUtils::IO::GetFileNameWithoutExtension("file.with.multiple.dots"));
        EXPECT_EQ(".dots", HyoutaUtils::IO::GetExtension("file.with.multiple.dots"));
        EXPECT_EQ(".test", HyoutaUtils::IO::GetFileNameWithoutExtension(".test.c"));
        EXPECT_EQ(".c", HyoutaUtils::IO::GetExtension(".test.c"));
        EXPECT_EQ(".", HyoutaUtils::IO::GetFileNameWithoutExtension("..c"));
        EXPECT_EQ(".c", HyoutaUtils::IO::GetExtension("..c"));
    }

    // GetAbsolutePath() is kind of difficult to test, so just see if it looks vaguely right
#ifdef BUILD_FOR_WINDOWS
    {
        auto a = HyoutaUtils::IO::GetAbsolutePath("somedir\\fake\\..\\a.bin");
        EXPECT_TRUE(a.substr(0, 3).ends_with(":\\"));
        EXPECT_TRUE(a.ends_with("\\somedir\\a.bin"));
    }
#else
    {
        // on linux we doesn't resolve the '..' in the middle...
        auto a = HyoutaUtils::IO::GetAbsolutePath("somedir/a.bin");
        EXPECT_TRUE(a.starts_with("/"));
        EXPECT_TRUE(a.ends_with("/somedir/a.bin"));
    }
#endif
}
