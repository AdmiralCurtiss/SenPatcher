#include "old_senpatcher_unpatch.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "hyouta_archive.h"
#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/stream.h"

namespace {
struct UnpatchFile {
    HyoutaUtils::Hash::SHA1 Hash;
    std::optional<HyoutaUtils::Hash::SHA1> AltHash;
    std::string_view Path;

    constexpr UnpatchFile(HyoutaUtils::Hash::SHA1 hash, std::string_view path)
      : Hash(hash), AltHash(std::nullopt), Path(path) {}

    constexpr UnpatchFile(HyoutaUtils::Hash::SHA1 hash,
                          HyoutaUtils::Hash::SHA1 altHash,
                          std::string_view path)
      : Hash(hash), AltHash(altHash), Path(path) {}
};

struct FileWithHashAndMaybePath {
    std::unique_ptr<char[]> Data;
    size_t DataLength;
    HyoutaUtils::Hash::SHA1 Hash;
    std::optional<std::string> Path;
};

// clang-format off
using HyoutaUtils::Hash::SHA1FromHexString;

constexpr std::array<UnpatchFile, 60> CS1UnpatchFiles = {{
    UnpatchFile(SHA1FromHexString("2eca835428184ad35a9935dc5d2deaa60d444aad"), "data/scripts/book/dat_us/book00.dat"),
    UnpatchFile(SHA1FromHexString("4243329ec1dd127cbf68a7f68d8ce6042225e1eb"), "data/scripts/book/dat_us/book01.dat"),
    UnpatchFile(SHA1FromHexString("ec509088c67eb02b279e132894f68986591d41af"), "data/scripts/book/dat_us/book02.dat"),
    UnpatchFile(SHA1FromHexString("9ed835634edf89fff1757d4fa22e4a1506ca0f04"), "data/scripts/book/dat_us/book03.dat"),
    UnpatchFile(SHA1FromHexString("73fd289006017eab8816636d998d21aa8fb38d68"), "data/scripts/scena/dat_us/a0006.dat"),
    UnpatchFile(SHA1FromHexString("9a75c8439ca12500437af585d0ae94744b620dfd"), "data/scripts/scena/dat_us/c0100.dat"),
    UnpatchFile(SHA1FromHexString("a740904f6ad37411825565981daa5f5915b55b69"), "data/scripts/scena/dat_us/c0110.dat"),
    UnpatchFile(SHA1FromHexString("c377fcdc74a195f79dba543e80a70d2eb7fdf12a"), "data/scripts/scena/dat_us/m0040.dat"),
    UnpatchFile(SHA1FromHexString("a2fcccff0c7877e4b565ed931b62f6a03191c289"), "data/scripts/scena/dat_us/m2130.dat"),
    UnpatchFile(SHA1FromHexString("f44b397573f1127711f66ec631b2de5dffdafc38"), "data/scripts/scena/dat_us/m3008.dat"),
    UnpatchFile(SHA1FromHexString("65044a35a4c042fabc4a5a66fd23b0cd8163dfdb"), "data/scripts/scena/dat_us/r0600.dat"),
    UnpatchFile(SHA1FromHexString("531ae02b784b6530f4dc08676a793c89f9ebae68"), "data/scripts/scena/dat_us/r0601.dat"),
    UnpatchFile(SHA1FromHexString("b96565a04c292ef7de28bbf071c5eae22dddfffe"), "data/scripts/scena/dat_us/r0610.dat"),
    UnpatchFile(SHA1FromHexString("48d8e5ebfacb29fed7c5c4e75c84e108277cb5fe"), "data/scripts/scena/dat_us/r0800.dat"),
    UnpatchFile(SHA1FromHexString("83fc174bcce22201fe2053f855e8879b3091e649"), "data/scripts/scena/dat_us/t0000.dat"),
    UnpatchFile(SHA1FromHexString("8bf43f6d4c7f0de5ba13e4c14301da05bd1c919a"), "data/scripts/scena/dat_us/t0000c.dat"),
    UnpatchFile(SHA1FromHexString("8a76ff88baf96b5e72e675d0d5d3b75a72cc3989"), "data/scripts/scena/dat_us/t0010.dat"),
    UnpatchFile(SHA1FromHexString("9668b944717fe2283a482367f1448ee1fc63e832"), "data/scripts/scena/dat_us/t0020.dat"),
    UnpatchFile(SHA1FromHexString("66a22b79517c7214b00b2a7a4ac898bc5f231fd8"), "data/scripts/scena/dat_us/t0031.dat"),
    UnpatchFile(SHA1FromHexString("e7854ac057166d50d94c340ec39403d26173ff9f"), "data/scripts/scena/dat_us/t0032.dat"),
    UnpatchFile(SHA1FromHexString("d363fc2114ec8421c24b47c29a4a2baded31cfb5"), "data/scripts/scena/dat_us/t0050.dat"),
    UnpatchFile(SHA1FromHexString("2c67ece34e81dfc667fd57699b818fcfd4e6b06e"), "data/scripts/scena/dat_us/t0060.dat"),
    UnpatchFile(SHA1FromHexString("eab3c1e32b2287639eab4b87c8ce67a41e0c4d80"), "data/scripts/scena/dat_us/t0070.dat"),
    UnpatchFile(SHA1FromHexString("d5805f2f25de668a4ececc8f6cad0aaae64a3cf8"), "data/scripts/scena/dat_us/t0080.dat"),
    UnpatchFile(SHA1FromHexString("a823fb0d4b8a4ffbdeaa6eb407bb38807048b226"), "data/scripts/scena/dat_us/t0090.dat"),
    UnpatchFile(SHA1FromHexString("a772085fed67d74412da01161b7e4b1eac41d585"), "data/scripts/scena/dat_us/t0210.dat"),
    UnpatchFile(SHA1FromHexString("84d3de50b7318f20b4fe48836404d134a124be52"), "data/scripts/scena/dat_us/t1000.dat"),
    UnpatchFile(SHA1FromHexString("44c4abb3f8e01dde0e36ca1d11cd433f37c10788"), "data/scripts/scena/dat_us/t1010.dat"),
    UnpatchFile(SHA1FromHexString("5574310597160a8b94e2a8ccf2ad2dfdc22c79d2"), "data/scripts/scena/dat_us/t1020.dat"),
    UnpatchFile(SHA1FromHexString("954a31dd3eaf244f159fbe83607ae870f3bdc89f"), "data/scripts/scena/dat_us/t1030.dat"),
    UnpatchFile(SHA1FromHexString("0c8591be2e25657dfecdb3ead94e046c0bf08069"), "data/scripts/scena/dat_us/t1040.dat"),
    UnpatchFile(SHA1FromHexString("60ff5f6535daa1debc7ecab2d514a387eeecb587"), "data/scripts/scena/dat_us/t1050.dat"),
    UnpatchFile(SHA1FromHexString("29c1901bfb4050a4c0f62af6129c653883f4352c"), "data/scripts/scena/dat_us/t1110.dat"),
    UnpatchFile(SHA1FromHexString("4f2a0d4b4c1602bbd9aec8dc0785f334c6c285f9"), "data/scripts/scena/dat_us/t1500.dat"),
    UnpatchFile(SHA1FromHexString("77cd6b3d02da4e22efac7d5a95fd19ac837cf12b"), "data/scripts/scena/dat_us/t2110.dat"),
    UnpatchFile(SHA1FromHexString("d18f9880c045b969afd8c6a8836ee6e86810aa4e"), "data/scripts/scena/dat_us/t3500.dat"),
    UnpatchFile(SHA1FromHexString("ee2a5c698325223ef9aaf196af96e4a55dd18f16"), "data/scripts/scena/dat_us/t4610.dat"),
    UnpatchFile(SHA1FromHexString("1a29ad14cffaa8f083a283c6197b882e61d81913"), "data/scripts/scena/dat_us/t5500.dat"),
    UnpatchFile(SHA1FromHexString("78aa964124be90f4b9c5da90418a724c0b46cbc3"), "data/scripts/scena/dat_us/t5660.dat"),
    UnpatchFile(SHA1FromHexString("e5f61f3108f8e1d43ca539b93bac91cfe37b709a"), "data/scripts/talk/dat_us/tk_angelica.dat"),
    UnpatchFile(SHA1FromHexString("af8f356c80083c028824fd41332b003fec834cb1"), "data/scripts/talk/dat_us/tk_beryl.dat"),
    UnpatchFile(SHA1FromHexString("3eb462def36cc15103e48657aa18b6b02abcd830"), "data/scripts/talk/dat_us/tk_becky.dat"),
    UnpatchFile(SHA1FromHexString("dc5deaa30247aafaf2a369da2268d4082ba310c5"), "data/scripts/talk/dat_us/tk_edel.dat"),
    UnpatchFile(SHA1FromHexString("491c365d592bb90029e7543d893d47bd5e66139d"), "data/scripts/talk/dat_us/tk_heinrich.dat"),
    UnpatchFile(SHA1FromHexString("f423fb1dfddde29d3e26a40ceed87982b899cdca"), "data/scripts/talk/dat_us/tk_laura.dat"),
    UnpatchFile(SHA1FromHexString("465873e53df8cb85e26d302a171eb4cc1ff2d6bb"), "data/scripts/talk/dat_us/tk_thomas.dat"),
    UnpatchFile(SHA1FromHexString("b838141d25f707a7c95191db2f8c324a3e0a34c0"), "data/scripts/talk/dat_us/tk_vandyck.dat"),
    UnpatchFile(SHA1FromHexString("684cc74b0837ff1408124f8b8a05cfd9c9a09195"), "data/se/wav/ed8" "\xef\xbd\x8d" "2123.wav"),
    UnpatchFile(SHA1FromHexString("8a735f256c69b0b2cd2b663820953fb49523723e"), "data/text/dat/t_item.tbl"),
    UnpatchFile(SHA1FromHexString("2bb6ead07062528187e75724d828ab0fc8336708"), "data/text/dat/t_magic.tbl"),
    UnpatchFile(SHA1FromHexString("f4b9ff78474452aac44f4b0c07c5a3cc1ce27359"), "data/text/dat/t_vctiming.tbl"),
    UnpatchFile(SHA1FromHexString("90a5c55ed954d77111563b9f3bb62ce7c534135a"), "data/text/dat/t_voice.tbl"),
    UnpatchFile(SHA1FromHexString("b64ec4d8b62042166e97e60c575552039c49c465"), "data/text/dat_us/t_item.tbl"),
    UnpatchFile(SHA1FromHexString("d5f7bf4c4c575efd5699e8bbd4040b81276a7284"), "data/text/dat_us/t_magic.tbl"),
    UnpatchFile(SHA1FromHexString("a3e1c05218c7059c371f3641a1111e166e899087"), "data/text/dat_us/t_notecook.tbl"),
    UnpatchFile(SHA1FromHexString("f4b9ff78474452aac44f4b0c07c5a3cc1ce27359"), "data/text/dat_us/t_vctiming.tbl"),
    UnpatchFile(SHA1FromHexString("dc8fa92820abc1b46a646b4d75ba5d239bd22ee9"), "data/text/dat_us/t_voice.tbl"),
    UnpatchFile(SHA1FromHexString("6d43ad75d01d9acd887826db59961c3e925ccc02"), "data/voice/wav/pc8v02551.wav"),
    UnpatchFile(SHA1FromHexString("373c1d1b30001af360042365ed257e070bf40acc"), "ed8.exe"),
    UnpatchFile(SHA1FromHexString("1d56abf5aa02eeae334797c287ef2109c7a103fa"), "ed8jp.exe"),
}};

constexpr std::array<UnpatchFile, 35> CS2UnpatchFiles = {{
    UnpatchFile(SHA1FromHexString("0ab9f575af6113694b18c0128cf1343ac6b48300"), "data/text/dat_us/t_item.tbl"),
    UnpatchFile(SHA1FromHexString("92de0d29c0ad4a9ea935870674976924d5df756d"), "data/text/dat_us/t_magic.tbl"),
    UnpatchFile(SHA1FromHexString("42419bfb7e473d177b088716276180500c5d76f5"), "data/text/dat_us/t_notecook.tbl"),
    UnpatchFile(SHA1FromHexString("9019e12bf1d93039254892c87f70a572d2ffc248"), "data/text/dat_us/t_voice.tbl"),
    UnpatchFile(SHA1FromHexString("7a68747acbd132c5215ea9c7543c37f146b39d63"), "data/scripts/book/dat_us/book00.dat"),
    UnpatchFile(SHA1FromHexString("033e04f8a870f0764460bf5e915604993c035bf5"), "data/scripts/book/dat_us/book01.dat"),
    UnpatchFile(SHA1FromHexString("4152ba749355789cf55a3311cd1c5527c1bc8de1"), "data/scripts/book/dat_us/book02.dat"),
    UnpatchFile(SHA1FromHexString("da6f79f56616cb073cc7205f895b1c0d389c0e2d"), "data/scripts/book/dat_us/book03.dat"),
    UnpatchFile(SHA1FromHexString("5b2fee612159bcb93b2c6831f94f7b1f4dd6231c"), "data/scripts/book/dat_us/book04.dat"),
    UnpatchFile(SHA1FromHexString("dcd1d5c83e8201c42492abbe664f61ee3a2187ff"), "data/scripts/book/dat_us/book05.dat"),
    UnpatchFile(SHA1FromHexString("d0a9a39e3aa04d573b9a7b51a170fb1dc4d79f17"), "data/scripts/scena/dat_us/e7050.dat"),
    UnpatchFile(SHA1FromHexString("8000797757b16c64656b0403584bafa1e85af342"), "data/scripts/scena/dat_us/e7060.dat"),
    UnpatchFile(SHA1FromHexString("34005eaf8e8f7823f5079700cf130b2b3b73d047"), "data/scripts/scena/dat_us/e7090.dat"),
    UnpatchFile(SHA1FromHexString("cc2ff5a5eadebb18a6db19106284dd35ac201243"), "data/scripts/scena/dat_us/e7101.dat"),
    UnpatchFile(SHA1FromHexString("65a894acd9f5ba6f72a5c5f6f336c7cda60a4b9c"), "data/scripts/scena/dat_us/e7110.dat"),
    UnpatchFile(SHA1FromHexString("1e69ef573a1888e5545850c97d6252fb1f5b5596"), "data/scripts/scena/dat_us/r0920.dat"),
    UnpatchFile(SHA1FromHexString("c21cb1a876196b7551f3bc3ef57620c1ffc0deb1"), "data/scripts/scena/dat_us/r1010.dat"),
    UnpatchFile(SHA1FromHexString("9ba616b8ea524fff05a5ac7e709f4c5fa4561a4b"), "data/scripts/scena/dat_us/t0001.dat"),
    UnpatchFile(SHA1FromHexString("7f1a94208801acaf1a3ba2fd7cba496444904a1b"), "data/scripts/scena/dat_us/t0010.dat"),
    UnpatchFile(SHA1FromHexString("acbda7e71e5622fe0c600792f18d25ad2c4eca0a"), "data/scripts/scena/dat_us/t1010.dat"),
    UnpatchFile(SHA1FromHexString("7da332f0c421b6796b4c2e0cc53b570c4a4d974e"), "data/scripts/scena/dat_us/t3060.dat"),
    UnpatchFile(SHA1FromHexString("3f981082b2affc869785a08896aad11a50c44343"), "data/scripts/scena/dat_us/t3500.dat"),
    UnpatchFile(SHA1FromHexString("985274ffb868643445b91d5f76be284ac2c79a1c"), "data/scripts/scena/dat_us/t3740.dat"),
    UnpatchFile(SHA1FromHexString("8b39f1bd64e65c40194b616b4c43f5d9c4d14c73"), "data/scripts/scena/dat_us/t4000.dat"),
    UnpatchFile(SHA1FromHexString("18c7ad19818f25d7a2ad61c3f7c711bf16cff933"), "data/scripts/scena/dat_us/t4080.dat"),
    UnpatchFile(SHA1FromHexString("df089163a734c202ebb8e05491ff6537fb1dbff7"), "data/scripts/scena/dat_us/t5501.dat"),
    UnpatchFile(SHA1FromHexString("a7bff27d9558648cb36dde4ab7cf4e9b1ca120c2"), "data/scripts/scena/dat_us/t6500.dat"),
    UnpatchFile(SHA1FromHexString("2a5b55ee43ae4498eccb409a491c7a6ea1e4471a"), "data/scripts/scena/dat_us/system.dat"),
    UnpatchFile(SHA1FromHexString("73f84be48f549fb71ac265a5d7519f3d8ca16f1d"), "data/scripts/talk/dat_us/tk_beryl.dat"),
    UnpatchFile(SHA1FromHexString("ea9e2d4575e334782fde96e3fa4d0c3fd5545af6"), "data/scripts/talk/dat_us/tk_emily.dat"),
    UnpatchFile(SHA1FromHexString("1e88dbcff39609facaaca7a29803247739ff1f14"), "data/scripts/talk/dat_us/tk_heinrich.dat"),
    UnpatchFile(SHA1FromHexString("836140611baf2149c7ac1a28182762b25354b54f"), "data/scripts/talk/dat_us/tk_monica.dat"),
    UnpatchFile(SHA1FromHexString("ea737c88d0648621c297ed0c139348aa6213a40d"), "data/scripts/talk/dat_us/tk_rosine.dat"),
    UnpatchFile(
        SHA1FromHexString("d5c333b4cd517d43e3868e159fbec37dba4122d6"), // v1.4.1
        SHA1FromHexString("b08ece4ee38e6e3a99e58eb11cffb45e49704f86"), // v1.4.2
        "bin/Win32/ed8_2_PC_US.exe"
    ),
    UnpatchFile(
        SHA1FromHexString("b8158fb59e43c02e904f813150d841336d1a13e5"), // v1.4.1
        SHA1FromHexString("7d1db7e0bb91ab77a3fd1eba53b0ed25806186c1"), // v1.4.2
        "bin/Win32/ed8_2_PC_JP.exe"
    ),
}};

constexpr std::array<UnpatchFile, 127> CS3UnpatchFiles = {{
    UnpatchFile(SHA1FromHexString("27648dfed57f40857139008f5f9e3bb8347cbb90"), "data/asset/D3D11_us/I_CVIS0061.pkg"),
    UnpatchFile(SHA1FromHexString("3721996e74336af0133f096b1f717e83b9bdc075"), "data/asset/D3D11_us/I_CVIS1008.pkg"),
    UnpatchFile(SHA1FromHexString("05f162efc3d880e94398ae4e10108aa018f30fee"), "data/asset/D3D11_us/I_JMP009.pkg"),
    UnpatchFile(SHA1FromHexString("6c33cf8b1cf93950d802f5ac9de1b213c0b45033"), "data/movie_us/webm/insa_05.webm"),
    UnpatchFile(SHA1FromHexString("97b37cb0d324014d8db6b2965f1836effdb1ce01"), "data/movie_us/webm/insa_08.webm"),
    UnpatchFile(SHA1FromHexString("97b37cb0d324014d8db6b2965f1836effdb1ce01"), "data/movie_us/webm/insa_09.webm"),
    UnpatchFile(SHA1FromHexString("2c6088c4f1e8847e49f5f5f48b410fe2aec3ef54"), "data/scripts/battle/dat_en/alchr022.dat"),
    UnpatchFile(SHA1FromHexString("b2b35b9d531658a45ed2a53477757d5a72c066ab"), "data/scripts/book/dat_en/book05.dat"),
    UnpatchFile(SHA1FromHexString("fa7e69755a3592cdb5196cd45ac861671798384c"), "data/scripts/book/dat_en/book06.dat"),
    UnpatchFile(SHA1FromHexString("4531fd2f226f5c641e09fc35e118d1c1bb2a4144"), "data/scripts/book/dat_en/book07.dat"),
    UnpatchFile(SHA1FromHexString("2b0b345b51b705b71ddc5f7989836dc3139f62f5"), "data/scripts/scena/dat_en/a0417.dat"),
    UnpatchFile(SHA1FromHexString("8a6e007a75ff9d1b06cc96f67d383ff7de6d2eb6"), "data/scripts/scena/dat_en/c0200.dat"),
    UnpatchFile(SHA1FromHexString("e9ef91d4cf274a0fb055303c3d29dcd1af99fd49"), "data/scripts/scena/dat_en/c0250.dat"),
    UnpatchFile(SHA1FromHexString("91fe1bb9b0861e93a37835c7678dd389ad365655"), "data/scripts/scena/dat_en/c0400.dat"),
    UnpatchFile(SHA1FromHexString("4d4183d8d45f846c7659a7db1d8c39c758ffb50a"), "data/scripts/scena/dat_en/c0420.dat"),
    UnpatchFile(SHA1FromHexString("2a1b99c4395efd230bc10942ae1c06f436f4e504"), "data/scripts/scena/dat_en/c0430.dat"),
    UnpatchFile(SHA1FromHexString("a5da1241a9c17b2fbb2fc9c93dc691d51471ec8a"), "data/scripts/scena/dat_en/c0820.dat"),
    UnpatchFile(SHA1FromHexString("bb6a5b0a31ebe05ceb06ae8c0b773e2f0b8823af"), "data/scripts/scena/dat_en/c0830.dat"),
    UnpatchFile(SHA1FromHexString("2fae040a29bafe3cf202c2d5eddadef2424523d8"), "data/scripts/scena/dat_en/c2430.dat"),
    UnpatchFile(SHA1FromHexString("37161993d10c65821c11026fedb1af5290ceff4d"), "data/scripts/scena/dat_en/c2440.dat"),
    UnpatchFile(SHA1FromHexString("d89a6f84a665a650f32269931c874bf3d1cb0b1b"), "data/scripts/scena/dat_en/c2610.dat"),
    UnpatchFile(SHA1FromHexString("af0504554a605f3e154b967683fed54617d7a0eb"), "data/scripts/scena/dat_en/c2620.dat"),
    UnpatchFile(SHA1FromHexString("6c8a21013585ce0f1ddf378cf8963aec5f283a75"), "data/scripts/scena/dat_en/c3000.dat"),
    UnpatchFile(SHA1FromHexString("648885e17c3f2371f9a71b07990e054db09242fb"), "data/scripts/scena/dat_en/c3010.dat"),
    UnpatchFile(SHA1FromHexString("34c651df94044c96f600a24ba99eafa2f04d08d1"), "data/scripts/scena/dat_en/c3210.dat"),
    UnpatchFile(SHA1FromHexString("043cfd82b96630f56a838aadce36591d2001196f"), "data/scripts/scena/dat_en/c3610.dat"),
    UnpatchFile(SHA1FromHexString("24b0dc7a0f3f7c945f199de834a1df5e8a9ec78b"), "data/scripts/scena/dat_en/f0000.dat"),
    UnpatchFile(SHA1FromHexString("1c5d1ffde19b3fe0b14ce4b4d8c29ebae850b8cb"), "data/scripts/scena/dat_en/f0010.dat"),
    UnpatchFile(SHA1FromHexString("bb3220cb9f85e554fd1196688bcc1ea578a3d234"), "data/scripts/scena/dat_en/f2000.dat"),
    UnpatchFile(SHA1FromHexString("289204f27043cf2fdc6272bffd1ec0857f3230d3"), "data/scripts/scena/dat_en/m0000.dat"),
    UnpatchFile(SHA1FromHexString("b0f3a274fd1e54528118b4a68706e7c330285fef"), "data/scripts/scena/dat_en/m0100.dat"),
    UnpatchFile(SHA1FromHexString("7103fd901920a78637b5abe65b071220d0962783"), "data/scripts/scena/dat_en/m0300.dat"),
    UnpatchFile(SHA1FromHexString("09161c59396b4aa8932f628fa9709ca8899fe58c"), "data/scripts/scena/dat_en/m0600.dat"),
    UnpatchFile(SHA1FromHexString("03bec649d56e048772de4fd1ce013005b651d784"), "data/scripts/scena/dat_en/m3000.dat"),
    UnpatchFile(SHA1FromHexString("2eff0eb2d6a770b42069380f42d7daecdd33a96d"), "data/scripts/scena/dat_en/m3420.dat"),
    UnpatchFile(SHA1FromHexString("8a880ba00b221cdc176eb66cabb23573750251d5"), "data/scripts/scena/dat_en/m3430.dat"),
    UnpatchFile(SHA1FromHexString("5941f93f60a4ab76661ae37e51103b8f1cf35011"), "data/scripts/scena/dat_en/m4004.dat"),
    UnpatchFile(SHA1FromHexString("098c038fe41d7629d13428c3484f0d899113937f"), "data/scripts/scena/dat_en/r0000.dat"),
    UnpatchFile(SHA1FromHexString("b636a0c274714e93b74db3871e7019bf6ca39a7c"), "data/scripts/scena/dat_en/r0210.dat"),
    UnpatchFile(SHA1FromHexString("0c5fbbb90cb3459e0d2f5b9769eb5f7d9d87992c"), "data/scripts/scena/dat_en/r2290.dat"),
    UnpatchFile(SHA1FromHexString("9bde3d287daabc7df76207d20b04da2ad1c7120a"), "data/scripts/scena/dat_en/r3000.dat"),
    UnpatchFile(SHA1FromHexString("48c59f32ff001518dcef78c47b5f2050f3e0de55"), "data/scripts/scena/dat_en/r3200.dat"),
    UnpatchFile(SHA1FromHexString("3efbd8764d61274a8750342972e75143131d7721"), "data/scripts/scena/dat_en/r3430.dat"),
    UnpatchFile(SHA1FromHexString("d6dcc55f71cf2e6193c6a33b53b8879c0d4d5958"), "data/scripts/scena/dat_en/r4200.dat"),
    UnpatchFile(SHA1FromHexString("871c9c02460dde4acbb7712111af384ed76a3bdc"), "data/scripts/scena/dat_en/r4290.dat"),
    UnpatchFile(SHA1FromHexString("895e048dd4b006b1bda8d2434de9edfb20142ef9"), "data/scripts/scena/dat_en/t0000.dat"),
    UnpatchFile(SHA1FromHexString("53812a2a94c94c7bc9068c4e43d8973eb32d5034"), "data/scripts/scena/dat_en/t0010.dat"),
    UnpatchFile(SHA1FromHexString("dacda5af52ca6ab3efee4f9b51606a5e3d676a7e"), "data/scripts/scena/dat_en/t0080.dat"),
    UnpatchFile(SHA1FromHexString("02b743e33e402065f67af2fa318064f41314d353"), "data/scripts/scena/dat_en/t0100.dat"),
    UnpatchFile(SHA1FromHexString("77098ac08dc593d1b4eb1f2be2b5fe5665de36e4"), "data/scripts/scena/dat_en/t0200.dat"),
    UnpatchFile(SHA1FromHexString("c45eb8312b11f6a87476165fe69d646789d5e48b"), "data/scripts/scena/dat_en/t0210.dat"),
    UnpatchFile(SHA1FromHexString("1ba8784695ed86309b72a3104c7b0b81b67b503b"), "data/scripts/scena/dat_en/t0250.dat"),
    UnpatchFile(SHA1FromHexString("f00fc1a818c84469fd34cfb593d03ad424393ace"), "data/scripts/scena/dat_en/t0260.dat"),
    UnpatchFile(SHA1FromHexString("05ebfadeed23981b39da835f2eb179877003492a"), "data/scripts/scena/dat_en/t0400.dat"),
    UnpatchFile(SHA1FromHexString("4d0f1d1d3b57eb70de562a52f4367495426d1896"), "data/scripts/scena/dat_en/t0410.dat"),
    UnpatchFile(SHA1FromHexString("d0fd8367295246ab1eafede393303d34f7f5bcc5"), "data/scripts/scena/dat_en/t3000.dat"),
    UnpatchFile(SHA1FromHexString("11f838467868bad61623e965fbc3c0607d2a3356"), "data/scripts/scena/dat_en/t3200.dat"),
    UnpatchFile(SHA1FromHexString("4cdac7c910724aa27bfe5ada8fc25f9b9118833b"), "data/scripts/scena/dat_en/t3220.dat"),
    UnpatchFile(SHA1FromHexString("d48fd0f978975b3d4854d72475ba670076961db8"), "data/scripts/scena/dat_en/t3400.dat"),
    UnpatchFile(SHA1FromHexString("7d6eddb306de371fd7fcfd55b599301683e18853"), "data/scripts/scena/dat_en/t3510.dat"),
    UnpatchFile(SHA1FromHexString("f20f8c36a44c88fecc44155250b42fa0259cd699"), "data/scripts/scena/dat_en/t3600.dat"),
    UnpatchFile(SHA1FromHexString("4c2781204c47f2457e831b6a50413068b498c5b3"), "data/scripts/scena/dat_en/v0010.dat"),
    UnpatchFile(SHA1FromHexString("b1f43611bbf1130ff552405bc50d2f75637e49e1"), "data/scripts/scena/dat_en/v0050.dat"),
    UnpatchFile(SHA1FromHexString("4c1c3cd56be5eb1a950812709685b2f029096f2d"), "data/scripts/talk/dat_en/tk_ada.dat"),
    UnpatchFile(SHA1FromHexString("65aa739d1b7a5f3ff91e7fcf4840a5db26bfcdfb"), "data/scripts/talk/dat_en/tk_linde.dat"),
    UnpatchFile(SHA1FromHexString("6ed1ba8558121db318927df8771712233f41fcce"), "data/scripts/talk/dat_en/tk_patrick.dat"),
    UnpatchFile(SHA1FromHexString("095615a115f3653b2ecb59a16a756f8c3f12b68d"), "data/scripts/talk/dat_en/tk_stark.dat"),
    UnpatchFile(SHA1FromHexString("5f392e50537f7ef779cb276c8d04c4eb7b7198d4"), "data/scripts/talk/dat_en/tk_tovar.dat"),
    UnpatchFile(SHA1FromHexString("8a9a5ca2e438e36ea47c147124623407ae9b6b5b"), "data/scripts/talk/dat_en/tk_zessica.dat"),
    UnpatchFile(SHA1FromHexString("660380324204ba12b6b100333cff246fe6e18674"), "data/text/dat/t_item.tbl"),
    UnpatchFile(SHA1FromHexString("5deee9b833b2bb93b0a326f586943f3d2e2424b9"), "data/text/dat_en/t_item_en.tbl"),
    UnpatchFile(SHA1FromHexString("5deee9b833b2bb93b0a326f586943f3d2e2424b9"), "data/text/dat_fr/t_item_en.tbl"),
    UnpatchFile(SHA1FromHexString("78602fdfed633bb3e41303f135d9367440c25c38"), "data/text/dat_en/t_item_fr.tbl"),
    UnpatchFile(SHA1FromHexString("78602fdfed633bb3e41303f135d9367440c25c38"), "data/text/dat_fr/t_item_fr.tbl"),
    UnpatchFile(SHA1FromHexString("1ee95c9c9cd1ff0ab3367f142bb02a180a4036ef"), "data/text/dat/t_itemhelp.tbl"),
    UnpatchFile(SHA1FromHexString("cb9135407b8264ac813e921329374a844f55036b"), "data/text/dat_en/t_itemhelp.tbl"),
    UnpatchFile(SHA1FromHexString("c7df5e2a0b4365fbf5a19b4710e50d7d3758fbd7"), "data/text/dat_fr/t_itemhelp.tbl"),
    UnpatchFile(SHA1FromHexString("57dd3ac63f52d21b677d5e1e7c20a7f255d0689f"), "data/text/dat_en/t_jump.tbl"),
    UnpatchFile(SHA1FromHexString("f575ce0fc47988122ec8107110238ad3c902529e"), "data/text/dat/t_magic.tbl"),
    UnpatchFile(SHA1FromHexString("c0b07b04d56268a7c42471d6671dc5cb6243286f"), "data/text/dat_en/t_magic.tbl"),
    UnpatchFile(SHA1FromHexString("f0460f7c90178135eae01a737c98f614a2227b38"), "data/text/dat_fr/t_magic.tbl"),
    UnpatchFile(SHA1FromHexString("e1f521da8d1a98a4e9945859287da9cf2b0cdd38"), "data/text/dat_en/t_mons.tbl"),
    UnpatchFile(SHA1FromHexString("86c4d62ac6efc4ab5dbfdc7f5bf36c4eaef64c3b"), "data/text/dat/t_mstqrt.tbl"),
    UnpatchFile(SHA1FromHexString("494b68fcbc463581f79f7dd1f6444df9c4ad6204"), "data/text/dat_en/t_mstqrt.tbl"),
    UnpatchFile(SHA1FromHexString("b8022803ece579b5342e5143dc91ae4706fd7f7c"), "data/text/dat_fr/t_mstqrt.tbl"),
    UnpatchFile(SHA1FromHexString("4ddf1af1f31eeb6c9311804d88b3f779519df457"), "data/text/dat_en/t_name.tbl"),
    UnpatchFile(SHA1FromHexString("0b0b536b6e6d29ec059cdde6900b4614789d84a9"), "data/text/dat/t_notecook.tbl"),
    UnpatchFile(SHA1FromHexString("9155e5158bb3e333654dd761072a04e0a8be6728"), "data/text/dat_en/t_notecook.tbl"),
    UnpatchFile(SHA1FromHexString("9e6bf6a3bb2bf3a5a06fbe4aa54e451bd61bc20d"), "data/text/dat_fr/t_notecook.tbl"),
    UnpatchFile(SHA1FromHexString("6f74b48ecea8b47ec2d951d0cdc29f313ec82738"), "data/text/dat_en/t_place.tbl"),
    UnpatchFile(SHA1FromHexString("5bf0cb69ea8f50fcf4ea4854581e2f460e539634"), "data/text/dat/t_text.tbl"),
    UnpatchFile(SHA1FromHexString("a2720e94f597640decd1d978b6b8f731147578a6"), "data/text/dat_en/t_text.tbl"),
    UnpatchFile(SHA1FromHexString("ec2b652621dd2b319beabce7c923f2e867bd1abf"), "data/text/dat_fr/t_text.tbl"),
    UnpatchFile(SHA1FromHexString("82c78ea6050e064869f7428b92788e8aa22dbac1"), "data/text/dat_en/t_vctiming_us.tbl"),
    UnpatchFile(SHA1FromHexString("82c78ea6050e064869f7428b92788e8aa22dbac1"), "data/text/dat_fr/t_vctiming_us.tbl"),
    UnpatchFile(SHA1FromHexString("3f0b2757fd9c17ab1aeb6f38e323d6fb4d6cdaad"), "data/text/dat_en/t_voice.tbl"),
    UnpatchFile(SHA1FromHexString("3f0b2757fd9c17ab1aeb6f38e323d6fb4d6cdaad"), "data/text/dat_fr/t_voice.tbl"),
    UnpatchFile(SHA1FromHexString("47177b904afb4c7f6a7b3dcd71d9f336a510479a"), "data/voice_us/opus/v00_e0427.opus"),
    UnpatchFile(SHA1FromHexString("07fb5a4afb6719ff611f4641e90ad65223d0fae3"), "data/voice_us/opus/v00_e0441.opus"),
    UnpatchFile(SHA1FromHexString("705db9fda94fcee79ee44e1c7a81f8adfbd9aadd"), "data/voice_us/opus/v00_e1032.opus"),
    UnpatchFile(SHA1FromHexString("85d5c7e49da20757219ed878a8c38c177ce62f9e"), "data/voice_us/opus/v00_e1054.opus"),
    UnpatchFile(SHA1FromHexString("28b5f314748c04e60a250a6147ed3b8c7b3694c8"), "data/voice_us/opus/v00_s0027.opus"),
    UnpatchFile(SHA1FromHexString("2efce24a4badc67b00111edf1a0bf3985c1f26aa"), "data/voice_us/opus/v00_s0028.opus"),
    UnpatchFile(SHA1FromHexString("78027c66bc6713e296941f1bc2fe0afa16d58652"), "data/voice_us/opus/v00_s0081.opus"),
    UnpatchFile(SHA1FromHexString("4544b1a36fcd43c53354d7b281413c12d6fe0499"), "data/voice_us/opus/v00_s0110.opus"),
    UnpatchFile(SHA1FromHexString("381144f8c25042245630653690900a52424b67a3"), "data/voice_us/opus/v00_s0264.opus"),
    UnpatchFile(SHA1FromHexString("cbadccb5b83c966e70828c23337c75774fc7dfb5"), "data/voice_us/opus/v07_e0026.opus"),
    UnpatchFile(SHA1FromHexString("83908f383e62ed46c7f569c8c1d44dd054bd98eb"), "data/voice_us/opus/v20_e0066.opus"),
    UnpatchFile(SHA1FromHexString("b260dc2e520f9a415fdc2a18d8c80988a7014b25"), "data/voice_us/opus/v27_e0042.opus"),
    UnpatchFile(SHA1FromHexString("300f81a7f4f2b1b48f089984127d57cf0849518f"), "data/voice_us/opus/v31_e0070.opus"),
    UnpatchFile(SHA1FromHexString("d5b0d8ee474c452da547e3f6f0ae89d74e7a447e"), "data/voice_us/opus/v43_e0023.opus"),
    UnpatchFile(SHA1FromHexString("d68debc3a28dd4885a1febbec70206b837d3446b"), "data/voice_us/opus/v49_e0004.opus"),
    UnpatchFile(SHA1FromHexString("d736113e4beca1bf4c71b748de04e423db017434"), "data/voice_us/opus/v52_e0109.opus"),
    UnpatchFile(SHA1FromHexString("5091eb68f9ac6480d81e6eb46d2adae923e808bc"), "data/voice_us/opus/v52_e0119.opus"),
    UnpatchFile(SHA1FromHexString("c76ad49eabeaaba612c4224d12d0190e433804f8"), "data/voice_us/opus/v55_e0018.opus"),
    UnpatchFile(SHA1FromHexString("87776fe9380ea836926af327a1b902a811185d93"), "data/voice_us/opus/v55_e0062.opus"),
    UnpatchFile(SHA1FromHexString("a0f9afb6a537571f5da12451744d494d70cfa8e3"), "data/voice_us/opus/v56_e0080.opus"),
    UnpatchFile(SHA1FromHexString("ae14dafaed0743374fd8eb76d016b80a17726b3f"), "data/voice_us/opus/v56_e0092.opus"),
    UnpatchFile(SHA1FromHexString("074dc69d2c9b654155cc88d5e0357c19fd2c0c3d"), "data/voice_us/opus/v57_e0020.opus"),
    UnpatchFile(SHA1FromHexString("0663cc677885ab2b75c95129edda0daad50810ba"), "data/voice_us/opus/v64_e0040.opus"),
    UnpatchFile(SHA1FromHexString("e1c6310dc5d14adc58b05740a8d02420a3a6c084"), "data/voice_us/opus/v72_e0005.opus"),
    UnpatchFile(SHA1FromHexString("756da12dbf0a90a89d28ee1641d289dd79f57887"), "data/voice_us/opus/v72_e0006.opus"),
    UnpatchFile(SHA1FromHexString("e9cdc46863bfe152cd70a0f096ef3eae3d93287e"), "data/voice_us/opus/v72_e0011.opus"),
    UnpatchFile(SHA1FromHexString("ef803a8f30d4185cbe4eb1f9e837246ae53d5d79"), "data/voice_us/opus/v72_e0012.opus"),
    UnpatchFile(SHA1FromHexString("4090e1aa67d9b5f3f8d610f6e7eaa066cc82698b"), "data/voice_us/opus/v93_e0300.opus"),
    UnpatchFile(SHA1FromHexString("a48022b2504c430f2f017ab580aa6516638ed104"), "bin/x64/ed8_3_PC.exe"),
    UnpatchFile(SHA1FromHexString("14af222cbeee7c434169c6a5bf2d21f688000a26"), "bin/x64/ed8_3_PC_JP.exe"),
}};

constexpr std::array<UnpatchFile, 11> CS4UnpatchFiles = {{
    UnpatchFile(SHA1FromHexString("21ac0c6c99dcf57c0f75b00d44d42d8dae99fe3f"), "data/scripts/scena/dat_en/t3600.dat"),
    UnpatchFile(SHA1FromHexString("cdc0b7a54ba0420a0c7c314d2a021f6949f5cb2e"), "data/scripts/scena/dat_en/m9031.dat"),
    UnpatchFile(SHA1FromHexString("d1cd26a05828553bb7bf03e370717226f28353a0"), "data/scripts/scena/dat_en/f4200.dat"),
    UnpatchFile(SHA1FromHexString("4e83e0152b272f6e7739c89a07c1b0c5e2499e8a"), "data/text/dat_en/t_item_en.tbl"),
    UnpatchFile(SHA1FromHexString("a6a6031210ad60fee7cf1949e4c31be2db4fc413"), "data/text/dat_en/t_itemhelp.tbl"),
    UnpatchFile(SHA1FromHexString("2c71852245a5d7a10c5c7e687e4a095ac1f54b60"), "data/text/dat_en/t_magic.tbl"),
    UnpatchFile(SHA1FromHexString("40ae4f525021b550b77e045a00841a42bc460a77"), "data/text/dat_en/t_mstqrt.tbl"),
    UnpatchFile(SHA1FromHexString("c755b53e8ba8618e9e55d4e7c83cccb7e7298fbb"), "data/text/dat_en/t_notecook.tbl"),
    UnpatchFile(SHA1FromHexString("ed242395ee9b5aa15f11bf83138b161bb116d20a"), "data/text/dat_en/t_text.tbl"),
    UnpatchFile(SHA1FromHexString("0d424434330d4368c1ae187cae83e4dba296ccf1"), "bin/Win64/ed8_4_PC.exe"),
    UnpatchFile(SHA1FromHexString("79e81f7a977e918041b66fb27dc8805d035e83ff"), "bin/Win64/ed8_4_PC_JP.exe"),
}};
// clang-format on
} // namespace

namespace SenPatcher {
bool HasOldSenpatcherBackups(std::string_view gamepath, int sengame) {
    std::string senpatcher_rerun_revert_data_path(gamepath);
    senpatcher_rerun_revert_data_path.append("/senpatcher_rerun_revert_data.bin");
    if (HyoutaUtils::IO::FileExists(std::string_view(senpatcher_rerun_revert_data_path))) {
        return true;
    }
    std::string senpatcher_bkp_path(gamepath);
    senpatcher_bkp_path.append("/senpatcher_bkp");
    if (HyoutaUtils::IO::DirectoryExists(std::string_view(senpatcher_bkp_path))) {
        return true;
    }
    if (sengame == 1) {
        std::string ed8exepath(gamepath);
        ed8exepath.append("/ed8.exe.senpatcher.bkp");
        if (HyoutaUtils::IO::FileExists(std::string_view(ed8exepath))) {
            return true;
        }
        std::string ed8jpexepath(gamepath);
        ed8jpexepath.append("/ed8jp.exe.senpatcher.bkp");
        if (HyoutaUtils::IO::FileExists(std::string_view(ed8jpexepath))) {
            return true;
        }
    }
    if (sengame == 2) {
        std::string ed82usexepath(gamepath);
        ed82usexepath.append("/bin/Win32/ed8_2_PC_US.exe.senpatcher.bkp");
        if (HyoutaUtils::IO::FileExists(std::string_view(ed82usexepath))) {
            return true;
        }
        std::string ed82jpexepath(gamepath);
        ed82jpexepath.append("/bin/Win32/ed8_2_PC_JP.exe.senpatcher.bkp");
        if (HyoutaUtils::IO::FileExists(std::string_view(ed82jpexepath))) {
            return true;
        }
    }
    return false;
}

static void CheckFile(std::vector<FileWithHashAndMaybePath>& existingFiles,
                      std::span<const UnpatchFile> unpatchFiles,
                      std::string_view path) {
    try {
        HyoutaUtils::IO::File f(path, HyoutaUtils::IO::OpenMode::Read);
        if (!f.IsOpen()) {
            return;
        }
        const auto len = f.GetLength();
        if (!len || *len > (12u * 1024u * 1024u)) {
            // the largest file we ever modified was under 12 MB
            return;
        }
        const size_t szlen = static_cast<size_t>(*len);
        auto data = std::make_unique_for_overwrite<char[]>(szlen);
        if (f.Read(data.get(), szlen) != szlen) {
            return;
        }
        const auto hash = HyoutaUtils::Hash::CalculateSHA1(data.get(), szlen);
        const auto unpatchFile = std::find_if(
            unpatchFiles.begin(), unpatchFiles.end(), [&](const UnpatchFile& unpatchFile) {
                return hash == unpatchFile.Hash || hash == unpatchFile.AltHash;
            });
        if (unpatchFile != unpatchFiles.end()) {
            existingFiles.emplace_back(FileWithHashAndMaybePath{.Data = std::move(data),
                                                                .DataLength = szlen,
                                                                .Hash = hash,
                                                                .Path = std::string(path)});
        }
    } catch (...) {
        // Logging.Log("Failed to open file " + path + ":" + ex.Message);
    }
}

static void CheckHyoutaArchive(std::vector<FileWithHashAndMaybePath>& existingFiles,
                               std::span<const UnpatchFile> unpatchFiles,
                               std::string_view path) {
    HyoutaUtils::IO::File f(path, HyoutaUtils::IO::OpenMode::Read);
    if (!f.IsOpen()) {
        return;
    }
    auto len = f.GetLength();
    if (!len) {
        return;
    }
    auto hacdata = std::make_unique_for_overwrite<char[]>(*len);
    if (f.Read(hacdata.get(), *len) != *len) {
        return;
    }
    try {
        HyoutaUtils::Stream::DuplicatableByteArrayStream bs(hacdata.get(), *len);
        HyoutaUtils::HyoutaArchive::HyoutaArchiveContainer hac(bs);
        uint64_t count = hac.GetFilecount();
        for (uint64_t i = 0; i < count; ++i) {
            std::unique_ptr<HyoutaUtils::Stream::DuplicatableStream> filestream =
                hac.GetFile(i)->GetDataStream();
            uint64_t filelength = filestream->GetLength();
            if (filelength > (12u * 1024u * 1024u)) {
                // the largest file we ever modified was under 12 MB
                continue;
            }
            const size_t szlen = static_cast<size_t>(filelength);
            auto data = std::make_unique_for_overwrite<char[]>(szlen);
            if (filestream->Read(data.get(), szlen) != szlen) {
                continue;
            }
            auto hash = HyoutaUtils::Hash::CalculateSHA1(data.get(), szlen);
            auto unpatchFile = std::find_if(
                unpatchFiles.begin(), unpatchFiles.end(), [&](const UnpatchFile& unpatchFile) {
                    return hash == unpatchFile.Hash || hash == unpatchFile.AltHash;
                });
            if (unpatchFile != unpatchFiles.end()) {
                existingFiles.emplace_back(FileWithHashAndMaybePath{
                    .Data = std::move(data), .DataLength = szlen, .Hash = hash});
            }
        }
    } catch (...) {
        // Logging.Log("Error in rerun/revert data:" + ex.Message);
    }
}

bool UnpatchGame(std::string_view gamepath, int sengame) {
    std::span<const UnpatchFile> unpatchFiles;
    if (sengame == 1) {
        unpatchFiles = CS1UnpatchFiles;
    } else if (sengame == 2) {
        unpatchFiles = CS2UnpatchFiles;
    } else if (sengame == 3) {
        unpatchFiles = CS3UnpatchFiles;
    } else if (sengame == 4) {
        unpatchFiles = CS4UnpatchFiles;
    } else {
        return false;
    }

    std::vector<FileWithHashAndMaybePath> existingFiles;
    std::string senpatcher_bkp_path(gamepath);
    senpatcher_bkp_path.append("/senpatcher_bkp");
    if (HyoutaUtils::IO::DirectoryExists(std::string_view(senpatcher_bkp_path))) {
        std::error_code ec{};
        std::filesystem::directory_iterator iterator(
            HyoutaUtils::IO::FilesystemPathFromUtf8(std::string_view(senpatcher_bkp_path)), ec);
        if (!ec) {
            while (iterator != std::filesystem::directory_iterator()) {
                std::string path = HyoutaUtils::IO::FilesystemPathToUtf8(iterator->path());
                CheckFile(existingFiles, unpatchFiles, path);
                iterator.increment(ec);
                if (ec) {
                    break;
                }
            }
        }
    }
    if (sengame == 1) {
        std::string ed8exepath(gamepath);
        std::string ed8jpexepath(gamepath);
        ed8exepath.append("/ed8.exe.senpatcher.bkp");
        ed8jpexepath.append("/ed8jp.exe.senpatcher.bkp");
        if (HyoutaUtils::IO::FileExists(std::string_view(ed8exepath))) {
            CheckFile(existingFiles, unpatchFiles, ed8exepath);
        }
        if (HyoutaUtils::IO::FileExists(std::string_view(ed8jpexepath))) {
            CheckFile(existingFiles, unpatchFiles, ed8jpexepath);
        }
    }
    if (sengame == 2) {
        std::string ed82usexepath(gamepath);
        std::string ed82jpexepath(gamepath);
        ed82usexepath.append("/bin/Win32/ed8_2_PC_US.exe.senpatcher.bkp");
        ed82jpexepath.append("/bin/Win32/ed8_2_PC_JP.exe.senpatcher.bkp");
        if (HyoutaUtils::IO::FileExists(std::string_view(ed82usexepath))) {
            CheckFile(existingFiles, unpatchFiles, ed82usexepath);
        }
        if (HyoutaUtils::IO::FileExists(std::string_view(ed82jpexepath))) {
            CheckFile(existingFiles, unpatchFiles, ed82jpexepath);
        }
    }
    std::string senpatcher_rerun_revert_data_path(gamepath);
    senpatcher_rerun_revert_data_path.append("/senpatcher_rerun_revert_data.bin");
    if (HyoutaUtils::IO::FileExists(std::string_view(senpatcher_rerun_revert_data_path))) {
        CheckHyoutaArchive(existingFiles, unpatchFiles, senpatcher_rerun_revert_data_path);
    }

    bool unpatchedAll = true;
    for (const UnpatchFile& unpatchFile : unpatchFiles) {
        std::string gamefilepath(gamepath);
        gamefilepath.push_back('/');
        gamefilepath.append(unpatchFile.Path);
        {
            HyoutaUtils::IO::File file(std::string_view(gamefilepath),
                                       HyoutaUtils::IO::OpenMode::Read);
            if (file.IsOpen()) {
                auto hash = HyoutaUtils::Hash::CalculateSHA1FromFile(file);
                if (hash) {
                    if (unpatchFile.Hash == *hash || unpatchFile.AltHash == *hash) {
                        // this is already unpatched, nothing to do
                        continue;
                    }
                }
            }
        }

        // if we reach here we need to unpatch the file, see if we have a copy in our files
        auto existingFile = std::find_if(
            existingFiles.begin(), existingFiles.end(), [&](const FileWithHashAndMaybePath& f) {
                return f.Hash == unpatchFile.Hash;
            });
        if (existingFile == existingFiles.end() && unpatchFile.AltHash.has_value()) {
            existingFile = std::find_if(
                existingFiles.begin(), existingFiles.end(), [&](const FileWithHashAndMaybePath& f) {
                    return f.Hash == *unpatchFile.AltHash;
                });
        }
        if (existingFile == existingFiles.end()) {
            // we don't, we can't unpatch all files
            unpatchedAll = false;
            continue;
        }

        if (!HyoutaUtils::IO::WriteFileAtomic(std::string_view(gamefilepath),
                                              existingFile->Data.get(),
                                              existingFile->DataLength)) {
            unpatchedAll = false;
            continue;
        }
    }

    if (unpatchedAll) {
        // all files are unpatched, we can remove the backup files
        for (const FileWithHashAndMaybePath& existingFile : existingFiles) {
            if (existingFile.Path.has_value()) {
                HyoutaUtils::IO::DeleteFile(std::string_view(*existingFile.Path));
            }
        }
        HyoutaUtils::IO::DeleteFile(std::string_view(senpatcher_rerun_revert_data_path));
        HyoutaUtils::IO::DeleteDirectory(std::string_view(senpatcher_bkp_path));
    }

    return unpatchedAll;
}
} // namespace SenPatcher
