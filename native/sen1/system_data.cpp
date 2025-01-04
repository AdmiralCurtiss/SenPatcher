#include "system_data.h"

#include <array>
#include <cassert>
#include <cstdint>

#include "util/endian.h"
#include "util/memread.h"
#include "util/memwrite.h"

namespace SenLib::Sen1 {
bool SystemData::Deserialize(const char* buffer,
                             size_t length,
                             HyoutaUtils::EndianUtils::Endianness e) {
    if (length < FileLength) {
        return false;
    }

    using namespace HyoutaUtils::MemRead;
    using HyoutaUtils::EndianUtils::FromEndian;
    const char* s = buffer;
    Always6 = FromEndian(ReadAdvUInt32(s), e);
    Unknown1 = ReadAdvUInt8(s);
    Unknown2 = ReadAdvUInt8(s);
    TitleScreenVariant = ReadAdvUInt8(s);
    Unknown4 = ReadAdvUInt8(s);
    Unknown5 = ReadAdvUInt8(s);
    Unknown6 = ReadAdvUInt8(s);
    Unknown7 = ReadAdvUInt8(s);
    Unknown8 = ReadAdvUInt8(s);
    Unknown9 = ReadAdvUInt8(s);
    Unknown10 = ReadAdvUInt8(s);
    Unknown11 = ReadAdvUInt8(s);
    Unknown12 = ReadAdvUInt8(s);
    BgmVolume = FromEndian(ReadAdvUInt16(s), e);
    SeVolume = FromEndian(ReadAdvUInt16(s), e);
    VoiceVolume = FromEndian(ReadAdvUInt16(s), e);
    VerticalCamera = FromEndian(ReadAdvUInt16(s), e);
    HorizontalCamera = FromEndian(ReadAdvUInt16(s), e);
    L2ButtonMapping = FromEndian(ReadAdvUInt16(s), e);
    R2ButtonMapping = FromEndian(ReadAdvUInt16(s), e);
    DPadUpButtonMapping = FromEndian(ReadAdvUInt16(s), e);
    DPadDownButtonMapping = FromEndian(ReadAdvUInt16(s), e);
    DPadLeftButtonMapping = FromEndian(ReadAdvUInt16(s), e);
    DPadRightButtonMapping = FromEndian(ReadAdvUInt16(s), e);
    L1ButtonMapping = FromEndian(ReadAdvUInt16(s), e);
    Minimap = FromEndian(ReadAdvUInt16(s), e);
    Vibration = FromEndian(ReadAdvUInt16(s), e);
    ScreenWidth = FromEndian(ReadAdvUInt16(s), e);
    ScreenHeight = FromEndian(ReadAdvUInt16(s), e);
    ScreenBrightness = FromEndian(ReadAdvUInt16(s), e);
    Unused = FromEndian(ReadAdvUInt16(s), e);

    assert(s == (buffer + FileLength));
    return true;
}

std::array<char, SystemData::FileLength>
    SystemData::Serialize(HyoutaUtils::EndianUtils::Endianness e) const {
    std::array<char, FileLength> buffer;

    using namespace HyoutaUtils::MemWrite;
    using HyoutaUtils::EndianUtils::ToEndian;
    char* s = buffer.data();
    WriteAdvUInt32(s, ToEndian(Always6, e));
    WriteAdvUInt8(s, Unknown1);
    WriteAdvUInt8(s, Unknown2);
    WriteAdvUInt8(s, TitleScreenVariant);
    WriteAdvUInt8(s, Unknown4);
    WriteAdvUInt8(s, Unknown5);
    WriteAdvUInt8(s, Unknown6);
    WriteAdvUInt8(s, Unknown7);
    WriteAdvUInt8(s, Unknown8);
    WriteAdvUInt8(s, Unknown9);
    WriteAdvUInt8(s, Unknown10);
    WriteAdvUInt8(s, Unknown11);
    WriteAdvUInt8(s, Unknown12);
    WriteAdvUInt16(s, ToEndian(BgmVolume, e));
    WriteAdvUInt16(s, ToEndian(SeVolume, e));
    WriteAdvUInt16(s, ToEndian(VoiceVolume, e));
    WriteAdvUInt16(s, ToEndian(VerticalCamera, e));
    WriteAdvUInt16(s, ToEndian(HorizontalCamera, e));
    WriteAdvUInt16(s, ToEndian(L2ButtonMapping, e));
    WriteAdvUInt16(s, ToEndian(R2ButtonMapping, e));
    WriteAdvUInt16(s, ToEndian(DPadUpButtonMapping, e));
    WriteAdvUInt16(s, ToEndian(DPadDownButtonMapping, e));
    WriteAdvUInt16(s, ToEndian(DPadLeftButtonMapping, e));
    WriteAdvUInt16(s, ToEndian(DPadRightButtonMapping, e));
    WriteAdvUInt16(s, ToEndian(L1ButtonMapping, e));
    WriteAdvUInt16(s, ToEndian(Minimap, e));
    WriteAdvUInt16(s, ToEndian(Vibration, e));
    WriteAdvUInt16(s, ToEndian(ScreenWidth, e));
    WriteAdvUInt16(s, ToEndian(ScreenHeight, e));
    WriteAdvUInt16(s, ToEndian(ScreenBrightness, e));
    WriteAdvUInt16(s, ToEndian(Unused, e));

    assert(s == (buffer.data() + FileLength));
    return buffer;
}
} // namespace SenLib::Sen1
