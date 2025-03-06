#include "file_compressor_main.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <format>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "sen/decompress_helper.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/hash/crc32.h"
#include "util/memread.h"
#include "util/memwrite.h"
#include "util/text.h"

#ifdef BUILD_FOR_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#endif

#include "lzma/LzmaEnc.h"

namespace SenTools {
namespace {
enum class Prefilter {
    None,
    Delta2LE_0x30Lead,
    Delta4LE_0x30Lead,
    Delta2LE_Deinterleaved_0x30Lead,
};
static std::string_view PrefilterToString(Prefilter f) {
    switch (f) {
        case Prefilter::None: return "None";
        case Prefilter::Delta2LE_0x30Lead: return "Delta2LE_0x30Lead";
        case Prefilter::Delta4LE_0x30Lead: return "Delta4LE_0x30Lead";
        case Prefilter::Delta2LE_Deinterleaved_0x30Lead: return "Delta2LE_Deinterleaved_0x30Lead";
        default: return "?";
    }
}

enum class Exhaustion {
    Standard,
    SemiExhaustive,
    Exhaustive,
};
static std::string_view ExhaustionToString(Exhaustion f) {
    switch (f) {
        case Exhaustion::Standard: return "Standard";
        case Exhaustion::SemiExhaustive: return "SemiExhaustive";
        case Exhaustion::Exhaustive: return "Exhaustive";
        default: return "?";
    }
}

enum class MatchFinder : uint8_t { Hc4 = 0, Hc5 = 1, Bt2 = 2, Bt3 = 3, Bt4 = 4, Bt5 = 5 };

static int GetBtMode(MatchFinder mf) {
    return (mf == MatchFinder::Hc4 || mf == MatchFinder::Hc5) ? 0 : 1;
}

static int GetNumHashBytes(MatchFinder mf) {
    switch (mf) {
        case MatchFinder::Hc4: return 4;
        case MatchFinder::Hc5: return 5;
        case MatchFinder::Bt2: return 2;
        case MatchFinder::Bt3: return 3;
        case MatchFinder::Bt4: return 4;
        case MatchFinder::Bt5: return 5;
        default: return 4;
    }
}

static std::string_view MatchFinderToString(MatchFinder mf) {
    switch (mf) {
        case MatchFinder::Hc4: return "Hc4";
        case MatchFinder::Hc5: return "Hc5";
        case MatchFinder::Bt2: return "Bt2";
        case MatchFinder::Bt3: return "Bt3";
        case MatchFinder::Bt4: return "Bt4";
        case MatchFinder::Bt5: return "Bt5";
        default: return "?";
    }
}

struct alignas(8) FileCompressorLzmaProps {
    MatchFinder mf = MatchFinder::Hc4;
    int8_t algo = 0;
    uint8_t dictLog = 0;
    int8_t pb = 0;
    int8_t lc = 0;
    int8_t lp = 0;
    int16_t fb = 0;
};

static std::string FormatLzmaProps(const FileCompressorLzmaProps& props) {
    return std::format("mf{}_algo{}_d{}_pb{}_lc{}_lp{}_fb{}",
                       MatchFinderToString(props.mf),
                       props.algo,
                       props.dictLog,
                       props.pb,
                       props.lc,
                       props.lp,
                       props.fb);
}

static std::optional<std::vector<char>> CompressInternalLzma(std::span<const char> instream,
                                                             int algo,
                                                             uint32_t dictionary,
                                                             int posStateBits,
                                                             int litContextBits,
                                                             int litPosBits,
                                                             int numFastBytes,
                                                             int btMode,
                                                             int numHashBytes) {
    static_assert(sizeof(char) == sizeof(Byte));

    CLzmaEncProps props{};
    LzmaEncProps_Init(&props);
    props.level = 0; // meaningless since we explicitly set the rest
    props.dictSize = dictionary;
    props.lc = litContextBits;
    props.lp = litPosBits;
    props.pb = posStateBits;
    props.algo = algo;
    props.fb = numFastBytes;

    // see MatchFinder_CreateVTable() for valid combinations
    props.btMode = btMode;
    props.numHashBytes = numHashBytes;

    // props.numHashOutBits = 0;
    // props.mc = 0;

    props.writeEndMark = 0;
    props.numThreads = 1;

    static_assert(LZMA_PROPS_SIZE == 5);
    std::array<Byte, LZMA_PROPS_SIZE> propsEncoded;
    std::vector<Byte> dest;

    // FIXME: What is the actual upper bound here? I cannot find anything in the LZMA documentation
    // about this.
    size_t bound = (instream.size() * 2) + 100;
    dest.resize(bound);

    ISzAlloc alloc{
        .Alloc = []([[maybe_unused]] ISzAllocPtr p, size_t size) { return malloc(size); },
        .Free = []([[maybe_unused]] ISzAllocPtr p, void* address) { return free(address); }};
    size_t destLen = dest.size();
    size_t propsSize = propsEncoded.size();
    if (LzmaEncode(dest.data(),
                   &destLen,
                   reinterpret_cast<const Byte*>(instream.data()),
                   instream.size(),
                   &props,
                   propsEncoded.data(),
                   &propsSize,
                   0,
                   nullptr,
                   &alloc,
                   &alloc)
        != SZ_OK) {
        return std::nullopt;
    }
    if (propsSize != 5) {
        return std::nullopt;
    }

    std::vector<char> outstream;
    outstream.resize(propsSize + 8 + destLen);
    std::memcpy(outstream.data(), propsEncoded.data(), propsSize);
    HyoutaUtils::MemWrite::WriteUInt64(
        outstream.data() + propsSize,
        HyoutaUtils::EndianUtils::ToEndian(static_cast<uint64_t>(instream.size()),
                                           HyoutaUtils::EndianUtils::Endianness::LittleEndian));
    std::memcpy(outstream.data() + propsSize + 8, dest.data(), destLen);
    return outstream;
}

static std::optional<std::vector<char>>
    CompressInternalLzmaSemiOptimized(FileCompressorLzmaProps& resultProps,
                                      std::span<const char> instream,
                                      Exhaustion exhaustion,
                                      int64_t mindict,
                                      int64_t maxdict) {
    std::vector<uint8_t> dictionary;
    std::vector<int8_t> posStateBits;
    std::vector<int8_t> litContextBits;
    std::vector<int8_t> litPosBits;
    std::vector<int16_t> numFastBytes;
    MatchFinder mf = MatchFinder::Bt4;

    {
        int min = (mindict >= 0 && mindict <= 30 ? static_cast<int>(mindict) : 0);
        int max = (maxdict >= 0 && maxdict <= 30 ? static_cast<int>(maxdict) : 30);
        for (int i = min; i <= max; ++i) {
            uint32_t dictsize = static_cast<uint32_t>(1) << i;
            dictionary.push_back(static_cast<uint8_t>(i));
            if (dictsize >= instream.size()) {
                break;
            }
        }
    }

    switch (exhaustion) {
        case Exhaustion::Standard:
            posStateBits = {{0, 1, 2, 4}};
            litContextBits = {{0, 1, 2, 3, 4, 8}};
            litPosBits = {{0, 1, 2, 4}};
            numFastBytes = {{16, 64, 128, 273}};
            break;
        case Exhaustion::SemiExhaustive:
            posStateBits = {{0, 1, 2, 3, 4}};
            litContextBits = {{0, 1, 2, 3, 4, 5, 6, 7, 8}};
            litPosBits = {{0, 1, 2, 3, 4}};
            numFastBytes = {{5, 16, 32, 64, 96, 128, 192, 273}};
            break;
        case Exhaustion::Exhaustive: {
            posStateBits = {{0, 1, 2, 3, 4}};
            litContextBits = {{0, 1, 2, 3, 4, 5, 6, 7, 8}};
            litPosBits = {{0, 1, 2, 3, 4}};
            for (int i = 5; i <= 273; ++i) {
                numFastBytes.push_back(static_cast<int16_t>(i));
            }
            break;
        }
        default: printf("invalid exhaustion\n"); return std::nullopt;
    }

    std::optional<std::vector<char>> best = std::nullopt;
    FileCompressorLzmaProps bestprops;

    const auto update_best = [&](std::vector<char>& tmp,
                                 int8_t algo,
                                 uint8_t d,
                                 int8_t pb,
                                 int8_t lc,
                                 int8_t lp,
                                 int16_t fb) {
        if (!best.has_value() || best->size() > tmp.size()) {
            best.emplace(std::move(tmp));
            bestprops.mf = mf;
            bestprops.algo = algo;
            bestprops.dictLog = d;
            bestprops.pb = pb;
            bestprops.lc = lc;
            bestprops.lp = lp;
            bestprops.fb = fb;
            printf("New best compression with %zu bytes: d%u, algo%d, pb%d, lc%d, lp%d, fb%d\n",
                   best->size(),
                   static_cast<unsigned int>(d),
                   static_cast<int>(algo),
                   static_cast<int>(pb),
                   static_cast<int>(lc),
                   static_cast<int>(lp),
                   static_cast<int>(fb));
        }
    };

    for (uint8_t d : dictionary) {
        for (int8_t algo = 0; algo < 2; ++algo) {
            for (int8_t pb : posStateBits) {
                printf("Reached d%u algo%d pb%d.\n",
                       static_cast<unsigned int>(d),
                       static_cast<int>(algo),
                       static_cast<int>(pb));
                for (int8_t lc : litContextBits) {
                    for (int8_t lp : litPosBits) {
                        for (int16_t fb : numFastBytes) {
                            auto tmp = CompressInternalLzma(instream,
                                                            algo,
                                                            static_cast<uint32_t>(1) << d,
                                                            pb,
                                                            lc,
                                                            lp,
                                                            fb,
                                                            GetBtMode(mf),
                                                            GetNumHashBytes(mf));
                            if (!tmp) {
                                printf("compression error\n");
                                return std::nullopt;
                            }
                            update_best(*tmp, algo, d, pb, lc, lp, fb);
                        }
                    }
                }
            }
        }
    }

    if (exhaustion != Exhaustion::Exhaustive) {
        // one final exhaustive fastbytes loop
        printf("Exhausting fastbytes with current best values...\n");
        for (int fbb = 5; fbb <= 273; ++fbb) {
            auto tmp = CompressInternalLzma(instream,
                                            bestprops.algo,
                                            static_cast<uint32_t>(1) << bestprops.dictLog,
                                            bestprops.pb,
                                            bestprops.lc,
                                            bestprops.lp,
                                            fbb,
                                            GetBtMode(mf),
                                            GetNumHashBytes(mf));
            if (!tmp) {
                printf("compression error\n");
                return std::nullopt;
            }
            update_best(*tmp,
                        bestprops.algo,
                        bestprops.dictLog,
                        bestprops.pb,
                        bestprops.lc,
                        bestprops.lp,
                        static_cast<int16_t>(fbb));
        }
    }

    resultProps = bestprops;
    return best;
}

static uint32_t CalculateCRC32(const char* data, size_t length) {
    uint32_t crc32 = crc_init();
    crc32 = crc_update(crc32, data, length);
    crc32 = crc_finalize(crc32);
    return crc32;
}

static std::optional<std::vector<char>> Compress(FileCompressorLzmaProps& resultProps,
                                                 std::span<const char> instream,
                                                 Prefilter filter,
                                                 Exhaustion exhaustion,
                                                 int64_t mindict,
                                                 int64_t maxdict) {
    using namespace HyoutaUtils::MemRead;
    using namespace HyoutaUtils::MemWrite;
    using namespace HyoutaUtils::EndianUtils;

    uint32_t crc = CalculateCRC32(instream.data(), instream.size());
    const auto create_outstream = [&](uint8_t filterId,
                                      const std::vector<char>& compressed) -> std::vector<char> {
        std::vector<char> outstream;
        outstream.resize(5 + compressed.size());
        WriteUInt8(outstream.data(), filterId);
        WriteUInt32(outstream.data() + 1, ToEndian(crc, Endianness::LittleEndian));
        std::memcpy(outstream.data() + 5, compressed.data(), compressed.size());
        return outstream;
    };

    if (filter == Prefilter::None) {
        auto compressed =
            CompressInternalLzmaSemiOptimized(resultProps, instream, exhaustion, mindict, maxdict);
        if (!compressed) {
            return std::nullopt;
        }
        return create_outstream(0, *compressed);
    }

    if (filter == Prefilter::Delta2LE_0x30Lead) {
        if ((instream.size() % 2) != 0 || instream.size() < 0x30) {
            std::string_view str = PrefilterToString(filter);
            printf("unsupported length for %.*s\n", static_cast<int>(str.size()), str.data());
            return std::nullopt;
        }
        std::vector<char> filteredstream(instream.size());
        std::memcpy(filteredstream.data(), instream.data(), 0x30);

        uint16_t last = 0;
        size_t pos = 0x30;
        while (pos < instream.size()) {
            uint16_t curr = FromEndian(ReadUInt16(instream.data() + pos), Endianness::LittleEndian);
            uint16_t diff = static_cast<uint16_t>(last - curr);
            WriteUInt16(filteredstream.data() + pos, ToEndian(diff, Endianness::LittleEndian));
            last = curr;
            pos += 2;
        }

        auto compressed = CompressInternalLzmaSemiOptimized(
            resultProps, filteredstream, exhaustion, mindict, maxdict);
        if (!compressed) {
            return std::nullopt;
        }
        return create_outstream(1, *compressed);
    }

    if (filter == Prefilter::Delta4LE_0x30Lead) {
        if ((instream.size() % 4) != 0 || instream.size() < 0x30) {
            std::string_view str = PrefilterToString(filter);
            printf("unsupported length for %.*s\n", static_cast<int>(str.size()), str.data());
            return std::nullopt;
        }
        std::vector<char> filteredstream(instream.size());
        std::memcpy(filteredstream.data(), instream.data(), 0x30);

        uint32_t last = 0;
        size_t pos = 0x30;
        while (pos < instream.size()) {
            uint32_t curr = FromEndian(ReadUInt32(instream.data() + pos), Endianness::LittleEndian);
            uint32_t diff = static_cast<uint32_t>(last - curr);
            WriteUInt32(filteredstream.data() + pos, ToEndian(diff, Endianness::LittleEndian));
            last = curr;
            pos += 4;
        }

        auto compressed = CompressInternalLzmaSemiOptimized(
            resultProps, filteredstream, exhaustion, mindict, maxdict);
        if (!compressed) {
            return std::nullopt;
        }
        return create_outstream(2, *compressed);
    }

    if (filter == Prefilter::Delta2LE_Deinterleaved_0x30Lead) {
        if ((instream.size() % 4) != 0 || instream.size() < 0x30) {
            std::string_view str = PrefilterToString(filter);
            printf("unsupported length for %.*s\n", static_cast<int>(str.size()), str.data());
            return std::nullopt;
        }
        std::vector<char> filteredstream(instream.size());
        std::memcpy(filteredstream.data(), instream.data(), 0x30);

        size_t outpos = 0x30;
        size_t inpos = 0x30;
        uint16_t last = 0;
        uint16_t curr = 0;
        uint16_t diff = 0;
        while (inpos < instream.size()) {
            curr = FromEndian(ReadUInt16(instream.data() + inpos), Endianness::LittleEndian);
            diff = static_cast<uint16_t>(last - curr);
            WriteUInt16(filteredstream.data() + outpos, ToEndian(diff, Endianness::LittleEndian));
            last = curr;
            inpos += 4;
            outpos += 2;
        }
        inpos = 0x30;
        last = 0;
        while (inpos < instream.size()) {
            inpos += 2;
            curr = FromEndian(ReadUInt16(instream.data() + inpos), Endianness::LittleEndian);
            diff = static_cast<uint16_t>(last - curr);
            WriteUInt16(filteredstream.data() + outpos, ToEndian(diff, Endianness::LittleEndian));
            last = curr;
            inpos += 2;
            outpos += 2;
        }

        auto compressed = CompressInternalLzmaSemiOptimized(
            resultProps, filteredstream, exhaustion, mindict, maxdict);
        if (!compressed) {
            return std::nullopt;
        }
        return create_outstream(3, *compressed);
    }

    printf("unexpected filter\n");
    return std::nullopt;
}
} // namespace

int FileCompressor_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(FileCompressor_ShortDescription);

    parser.usage("sentools " FileCompressor_Name " [options] file.bin");
    parser.add_option("--filter")
        .dest("filter")
        .metavar("FILTER")
        .type(optparse::DataType::String)
        .set_default("None")
        .help(
            "Options:"
            "\n - None"
            "\n - Delta2LE_0x30Lead"
            "\n - Delta4LE_0x30Lead"
            "\n - Delta2LE_Deinterleaved_0x30Lead");
    parser.add_option("--exhaustion")
        .dest("exhaustion")
        .metavar("EXHAUSTION")
        .type(optparse::DataType::String)
        .set_default("Standard")
        .help(
            "Options:"
            "\n - Standard"
            "\n - SemiExhaustive"
            "\n - Exhaustive");
    parser.add_option("--mindict")
        .dest("mindict")
        .metavar("MINDICT")
        .type(optparse::DataType::Int)
        .set_default(-1);
    parser.add_option("--maxdict")
        .dest("maxdict")
        .metavar("MAXDICT")
        .type(optparse::DataType::Int)
        .set_default(-1);
    parser.add_option("--decompress")
        .dest("decompress")
        .action(optparse::ActionType::StoreTrue)
        .set_default(false);

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

    const std::string& inpath = args[0];
    Prefilter filter = Prefilter::None;
    Exhaustion exhaustion = Exhaustion::Standard;
    bool decompress = false;
    int64_t mindict = -1;
    int64_t maxdict = -1;

    using HyoutaUtils::TextUtils::CaseInsensitiveEquals;
    if (auto* filter_option = options.get("filter")) {
        std::string str = filter_option->first_string();
        if (CaseInsensitiveEquals("None", str)) {
            filter = Prefilter::None;
        } else if (CaseInsensitiveEquals("Delta2LE_0x30Lead", str)) {
            filter = Prefilter::Delta2LE_0x30Lead;
        } else if (CaseInsensitiveEquals("Delta4LE_0x30Lead", str)) {
            filter = Prefilter::Delta4LE_0x30Lead;
        } else if (CaseInsensitiveEquals("Delta2LE_Deinterleaved_0x30Lead", str)) {
            filter = Prefilter::Delta2LE_Deinterleaved_0x30Lead;
        } else {
            printf("Invalid filter: %s\n", str.c_str());
            return -1;
        }
    }
    if (auto* exhaustion_option = options.get("exhaustion")) {
        std::string str = exhaustion_option->first_string();
        if (CaseInsensitiveEquals("Standard", str)) {
            exhaustion = Exhaustion::Standard;
        } else if (CaseInsensitiveEquals("SemiExhaustive", str)) {
            exhaustion = Exhaustion::SemiExhaustive;
        } else if (CaseInsensitiveEquals("Exhaustive", str)) {
            exhaustion = Exhaustion::Exhaustive;
        } else {
            printf("Invalid exhaustion: %s\n", str.c_str());
            return -1;
        }
    }
    if (auto* mindict_option = options.get("mindict")) {
        mindict = mindict_option->first_integer();
    }
    if (auto* maxdict_option = options.get("maxdict")) {
        maxdict = maxdict_option->first_integer();
    }
    if (auto* decompress_option = options.get("decompress")) {
        decompress = decompress_option->flag();
    }

    HyoutaUtils::IO::File infile(std::string_view(inpath), HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        printf("Failed to open input file\n");
        return -1;
    }
    const auto filelen = infile.GetLength();
    if (!filelen) {
        printf("Failed to get input file size\n");
        return -1;
    }
    if (*filelen > std::numeric_limits<size_t>::max()) {
        printf("Input file too large\n");
        return -1;
    }
    const size_t bufferSize = static_cast<size_t>(*filelen);
    std::unique_ptr<char[]> buffer(new (std::nothrow) char[bufferSize]);
    if (!buffer) {
        printf("Failed to allocate memory\n");
        return -1;
    }
    if (infile.Read(buffer.get(), bufferSize) != bufferSize) {
        printf("Failed to read input file\n");
        return -1;
    }

    if (decompress) {
        std::string outpath = inpath + ".dec";
        auto decompressed = SenLib::DecompressFromBuffer(buffer.get(), bufferSize);
        if (!decompressed) {
            printf("Failed to decompress\n");
            return -1;
        }
        if (!HyoutaUtils::IO::WriteFileAtomic(
                std::string_view(outpath), decompressed->data(), decompressed->size())) {
            printf("Failed to write output\n");
            return -1;
        }
    } else {
        std::string outpath = std::format(
            "{}_{}_{}", inpath, PrefilterToString(filter), ExhaustionToString(exhaustion));
        if (mindict >= 0) {
            outpath = outpath + "_mind" + std::to_string(mindict);
        }
        if (maxdict >= 0) {
            outpath = outpath + "_maxd" + std::to_string(maxdict);
        }
        printf("Generating: %s\n", outpath.c_str());

        FileCompressorLzmaProps resultProps;
        auto compressed = Compress(resultProps,
                                   std::span<const char>(buffer.get(), bufferSize),
                                   filter,
                                   exhaustion,
                                   mindict,
                                   maxdict);
        if (!compressed) {
            printf("Failed to compress\n");
            return -1;
        }
        std::string finalOutpath = std::format("{}__{}.bin", outpath, FormatLzmaProps(resultProps));
        if (!HyoutaUtils::IO::WriteFileAtomic(
                std::string_view(finalOutpath), compressed->data(), compressed->size())) {
            printf("Failed to write output\n");
            return -1;
        }
    }

    return 0;
}
} // namespace SenTools
