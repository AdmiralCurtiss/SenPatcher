#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sen/decompress_helper.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_9_voice[] =
    "Update various voice clips to PS4 version 1.03.";
}

namespace {
static constexpr char PatchData_v00_e0427[] = {
#include "embed_sen3_v00_e0427.h"
};
static constexpr size_t PatchLength_v00_e0427 = sizeof(PatchData_v00_e0427);
static constexpr char PatchData_v00_e1032[] = {
#include "embed_sen3_v00_e1032.h"
};
static constexpr size_t PatchLength_v00_e1032 = sizeof(PatchData_v00_e1032);
static constexpr char PatchData_v00_e1054[] = {
#include "embed_sen3_v00_e1054.h"
};
static constexpr size_t PatchLength_v00_e1054 = sizeof(PatchData_v00_e1054);
static constexpr char PatchData_v00_s0081[] = {
#include "embed_sen3_v00_s0081.h"
};
static constexpr size_t PatchLength_v00_s0081 = sizeof(PatchData_v00_s0081);
// static constexpr char PatchData_v00_s0110[] = {
// #include "embed_sen3_v00_s0110.h"
// };
// static constexpr size_t PatchLength_v00_s0110 = sizeof(PatchData_v00_s0110);
static constexpr char PatchData_v00_s0264[] = {
#include "embed_sen3_v00_s0264.h"
};
static constexpr size_t PatchLength_v00_s0264 = sizeof(PatchData_v00_s0264);
// static constexpr char PatchData_v07_e0026[] = {
// #include "embed_sen3_v07_e0026.h"
// };
// static constexpr size_t PatchLength_v07_e0026 = sizeof(PatchData_v07_e0026);
static constexpr char PatchData_v20_e0066[] = {
#include "embed_sen3_v20_e0066.h"
};
static constexpr size_t PatchLength_v20_e0066 = sizeof(PatchData_v20_e0066);
static constexpr char PatchData_v27_e0042[] = {
#include "embed_sen3_v27_e0042.h"
};
static constexpr size_t PatchLength_v27_e0042 = sizeof(PatchData_v27_e0042);
static constexpr char PatchData_v31_e0070[] = {
#include "embed_sen3_v31_e0070.h"
};
static constexpr size_t PatchLength_v31_e0070 = sizeof(PatchData_v31_e0070);
static constexpr char PatchData_v43_e0023[] = {
#include "embed_sen3_v43_e0023.h"
};
static constexpr size_t PatchLength_v43_e0023 = sizeof(PatchData_v43_e0023);
static constexpr char PatchData_v49_e0004[] = {
#include "embed_sen3_v49_e0004.h"
};
static constexpr size_t PatchLength_v49_e0004 = sizeof(PatchData_v49_e0004);
static constexpr char PatchData_v52_e0109[] = {
#include "embed_sen3_v52_e0109.h"
};
static constexpr size_t PatchLength_v52_e0109 = sizeof(PatchData_v52_e0109);
static constexpr char PatchData_v52_e0119[] = {
#include "embed_sen3_v52_e0119.h"
};
static constexpr size_t PatchLength_v52_e0119 = sizeof(PatchData_v52_e0119);
static constexpr char PatchData_v55_e0018[] = {
#include "embed_sen3_v55_e0018.h"
};
static constexpr size_t PatchLength_v55_e0018 = sizeof(PatchData_v55_e0018);
static constexpr char PatchData_v55_e0062[] = {
#include "embed_sen3_v55_e0062.h"
};
static constexpr size_t PatchLength_v55_e0062 = sizeof(PatchData_v55_e0062);
static constexpr char PatchData_v56_e0080[] = {
#include "embed_sen3_v56_e0080.h"
};
static constexpr size_t PatchLength_v56_e0080 = sizeof(PatchData_v56_e0080);
static constexpr char PatchData_v56_e0092[] = {
#include "embed_sen3_v56_e0092.h"
};
static constexpr size_t PatchLength_v56_e0092 = sizeof(PatchData_v56_e0092);
static constexpr char PatchData_v57_e0020[] = {
#include "embed_sen3_v57_e0020.h"
};
static constexpr size_t PatchLength_v57_e0020 = sizeof(PatchData_v57_e0020);
static constexpr char PatchData_v64_e0040[] = {
#include "embed_sen3_v64_e0040.h"
};
static constexpr size_t PatchLength_v64_e0040 = sizeof(PatchData_v64_e0040);
static constexpr char PatchData_v72_e0005[] = {
#include "embed_sen3_v72_e0005.h"
};
static constexpr size_t PatchLength_v72_e0005 = sizeof(PatchData_v72_e0005);
// static constexpr char PatchData_v72_e0006[] = {
// #include "embed_sen3_v72_e0006.h"
// };
// static constexpr size_t PatchLength_v72_e0006 = sizeof(PatchData_v72_e0006);
static constexpr char PatchData_v72_e0011[] = {
#include "embed_sen3_v72_e0011.h"
};
static constexpr size_t PatchLength_v72_e0011 = sizeof(PatchData_v72_e0011);
static constexpr char PatchData_v72_e0012[] = {
#include "embed_sen3_v72_e0012.h"
};
static constexpr size_t PatchLength_v72_e0012 = sizeof(PatchData_v72_e0012);
static constexpr char PatchData_v93_e0300[] = {
#include "embed_sen3_v93_e0300.h"
};
static constexpr size_t PatchLength_v93_e0300 = sizeof(PatchData_v93_e0300);
} // namespace

namespace SenLib::Sen3::FileFixes::voice_opus_ps4_103 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        // Rean t0200, "Even by Your Majesty's imperial command." -> "Even by Your Highness'
        // imperial command."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v00_e0427, PatchLength_v00_e0427),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_e0427.opus"),
            SenPatcher::P3ACompressionType::None);

        // Rean c2440, "I've...heard about the chancellor.{n}But, Your Highness...?" ->
        // "I've...heard about the chancellor.{n}But, Your Majesty...?"
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v00_e1032, PatchLength_v00_e1032),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_e1032.opus"),
            SenPatcher::P3ACompressionType::None);

        // Rean c2440, "(Both the chancellor and His Highness{n}understand the situation fully. Yet
        // they...)" -> "(Both the chancellor and His Majesty{n}understand the situation fully. Yet
        // they...)"
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v00_e1054, PatchLength_v00_e1054),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_e1054.opus"),
            SenPatcher::P3ACompressionType::None);

        // Rean (post-battle line), "Nothing compared to you, Your Majesty." -> "Nothing compared to
        // you, Your Highness."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v00_s0081, PatchLength_v00_s0081),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_s0081.opus"),
            SenPatcher::P3ACompressionType::None);

        // Rean (Rush callout?), "Your Majesty" -> "Your Highness"
        // this one has the wrong filter but we don't have a better copy...
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v00_s0264, PatchLength_v00_s0264),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_s0110.opus"),
            SenPatcher::P3ACompressionType::None);

        // Rean (Rush callout?), "Your Majesty" -> "Your Highness"
        // filter is correct here
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v00_s0264, PatchLength_v00_s0264),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v00_s0264.opus"),
            SenPatcher::P3ACompressionType::None);

        // Laura f0000, "Heehee." -> "It's been some time, Major Nei--erm, pardon.{n}It's
        // 'Lieutenant Colonel' now, isn't it?" this is a Laura line from her CS4 voice actor, which
        // is really obviously different than her CS3 one, so let's not use this
        // result.emplace_back(
        //     SenLib::DecompressFromBuffer(PatchData_v07_e0026, PatchLength_v07_e0026),
        //     SenPatcher::InitializeP3AFilename("data/voice_us/opus/v07_e0026.opus"),
        //     SenPatcher::P3ACompressionType::None);

        // Elise t0010, "Oh, and, Rean? Here are the letters{n}from Father and Master Yun." -> "Oh,
        // and, Rean? Here are the letters{n}from Father and Master Ka-fai."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v20_e0066, PatchLength_v20_e0066),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v20_e0066.opus"),
            SenPatcher::P3ACompressionType::None);

        // Olivert c0250, "Ah, but it is BECAUSE I am a member{n}of the royal family that I must do
        // this." -> "Ah, but it is BECAUSE I am a member{n}of the Imperial family that I must do
        // this."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v27_e0042, PatchLength_v27_e0042),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v27_e0042.opus"),
            SenPatcher::P3ACompressionType::None);

        // Rufus c2430, "To tell the truth, I was at a party held{n}by the Imperial Household
        // Agency." -> "To tell the truth, I was at a party held{n}by City Hall."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v31_e0070, PatchLength_v31_e0070),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v31_e0070.opus"),
            SenPatcher::P3ACompressionType::None);

        // Carl Regnitz c3210, "Now, we shall begin the award ceremony on{n}behalf of the Imperial
        // Household Agency." -> "Now, we shall present the awards on{n}behalf of City Hall."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v43_e0023, PatchLength_v43_e0023),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v43_e0023.opus"),
            SenPatcher::P3ACompressionType::None);

        // Priscilla c2430, "He was only a young boy when he lost his mother{n}and was adopted into
        // the royal family..." -> "He was only a young boy when he lost his mother{n}and was
        // adopted into the Imperial family..."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v49_e0004, PatchLength_v49_e0004),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v49_e0004.opus"),
            SenPatcher::P3ACompressionType::None);

        // Roselia c3010, "In the beginning, this land housed two{n}of the Sept-Terrion." -> "In the
        // beginning, this land housed two{n}of the Sept-Terrions."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v52_e0109, PatchLength_v52_e0109),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v52_e0109.opus"),
            SenPatcher::P3ACompressionType::None);

        // Roselia c3010, "Exhausting their power in one final strike, the{n}two Sept-Terrion were
        // blown away, left as nothing{n}but empty shells." -> "Exhausting their power in one final
        // strike, the{n}two Sept-Terrions were blown away, left as nothing{n}but empty shells."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v52_e0119, PatchLength_v52_e0119),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v52_e0119.opus"),
            SenPatcher::P3ACompressionType::None);

        // Eugent c2430, "All of these brave people have already been{n}recognized by the Imperial
        // Household Agency, but{n}let us give them yet another round of applause!" -> "All of these
        // brave people have already been{n}recognized by City Hall, but let us give{n}them yet
        // another round of applause!"
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v55_e0018, PatchLength_v55_e0018),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v55_e0018.opus"),
            SenPatcher::P3ACompressionType::None);

        // Eugent c2440, "During the War of the Lions, the brothers{n}of the royal family spilled
        // each other's blood{n}and pulled the nation into their feud." -> "During the War of the
        // Lions, the brothers{n}of the Imperial family spilled each other's blood{n}and pulled the
        // nation into their feud."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v55_e0062, PatchLength_v55_e0062),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v55_e0062.opus"),
            SenPatcher::P3ACompressionType::None);

        // Michael r2290, "To be seen off by the royal family is{n}truly more than we deserve..." ->
        // "To be seen off by the Imperial family is{n}truly more than we deserve..."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v56_e0080, PatchLength_v56_e0080),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v56_e0080.opus"),
            SenPatcher::P3ACompressionType::None);

        // Michael r3430, "Zero Artisan Randonneur.{n}Show me what an A-rank bracer is capable of."
        // -> "Zero Artisan Randonneur. Show me what{n}an A-rank level bracer is capable of."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v56_e0092, PatchLength_v56_e0092),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v56_e0092.opus"),
            SenPatcher::P3ACompressionType::None);

        // Alberich m4004, "After the Sept-Terrion of Flame, Ark Rouge, and the{n}Sept-Terrion of
        // Earth, Lost Zem, battled 1,200 years ago..." -> "After the Sept-Terrion of Fire, Ark
        // Rouge, and the{n}Sept-Terrion of Earth, Lost Zem, battled 1,200 years ago..."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v57_e0020, PatchLength_v57_e0020),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v57_e0020.opus"),
            SenPatcher::P3ACompressionType::None);

        // Aurelia t0210, "We seem to have majors from both the{n}RMP and Intelligence Agency here
        // today." -> "We seem to have majors from both the{n}RMP and Intelligence Division here
        // today."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v64_e0040, PatchLength_v64_e0040),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v64_e0040.opus"),
            SenPatcher::P3ACompressionType::None);

        // Rosine t0000, "(Just as Sir Thomas said...)" -> "(Just as Father Thomas said...)"
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v72_e0005, PatchLength_v72_e0005),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v72_e0005.opus"),
            SenPatcher::P3ACompressionType::None);

        // Rosine t0000, "(...I will need to be ready to{n}move at a moment's notice.)"
        // I'm not sure why this was changed, the new line is just the old one but it fades out
        // slightly earlier
        // result.emplace_back(
        //     SenLib::DecompressFromBuffer(PatchData_v72_e0006, PatchLength_v72_e0006),
        //     SenPatcher::InitializeP3AFilename("data/voice_us/opus/v72_e0006.opus"),
        //     SenPatcher::P3ACompressionType::None);

        // Rosine c3010, "Sir Lysander! Sir Gaius! Cryptids and Magic Knights{n}have appeared all
        // over the city!" -> "Father Thomas! Father Gaius! Cryptids and Magic{n}Knights have
        // appeared all over the city!"
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v72_e0011, PatchLength_v72_e0011),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v72_e0011.opus"),
            SenPatcher::P3ACompressionType::None);

        // Rosine c3000, "Yes, I'll try to get in contact with{n}Sir Hemisphere somehow." -> "Yes,
        // I'll try to get in contact with{n}Father Wazy somehow."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v72_e0012, PatchLength_v72_e0012),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v72_e0012.opus"),
            SenPatcher::P3ACompressionType::None);

        // (random NPC guard) c2430, "Pardon me, Your Highness." -> "Pardon me, Your Majesty."
        result.emplace_back(
            SenLib::DecompressFromBuffer(PatchData_v93_e0300, PatchLength_v93_e0300),
            SenPatcher::InitializeP3AFilename("data/voice_us/opus/v93_e0300.opus"),
            SenPatcher::P3ACompressionType::None);
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::voice_opus_ps4_103
