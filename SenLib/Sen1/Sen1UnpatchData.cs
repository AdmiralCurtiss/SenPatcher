using HyoutaUtils.Checksum;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public static class Sen1UnpatchData {
		public static UnpatchFile[] GetFiles() {
			return new UnpatchFile[] {
				new UnpatchFile(new SHA1(0x2eca835428184ad3ul, 0x5a9935dc5d2deaa6ul, 0x0d444aadu), "data/scripts/book/dat_us/book00.dat"),
				new UnpatchFile(new SHA1(0x4243329ec1dd127cul, 0xbf68a7f68d8ce604ul, 0x2225e1ebu), "data/scripts/book/dat_us/book01.dat"),
				new UnpatchFile(new SHA1(0xec509088c67eb02bul, 0x279e132894f68986ul, 0x591d41afu), "data/scripts/book/dat_us/book02.dat"),
				new UnpatchFile(new SHA1(0x9ed835634edf89fful, 0xf1757d4fa22e4a15ul, 0x06ca0f04u), "data/scripts/book/dat_us/book03.dat"),
				new UnpatchFile(new SHA1(0x73fd289006017eabul, 0x8816636d998d21aaul, 0x8fb38d68u), "data/scripts/scena/dat_us/a0006.dat"),
				new UnpatchFile(new SHA1(0x9a75c8439ca12500ul, 0x437af585d0ae9474ul, 0x4b620dfdu), "data/scripts/scena/dat_us/c0100.dat"),
				new UnpatchFile(new SHA1(0xa740904f6ad37411ul, 0x825565981daa5f59ul, 0x15b55b69u), "data/scripts/scena/dat_us/c0110.dat"),
				new UnpatchFile(new SHA1(0xc377fcdc74a195f7ul, 0x9dba543e80a70d2eul, 0xb7fdf12au), "data/scripts/scena/dat_us/m0040.dat"),
				new UnpatchFile(new SHA1(0xa2fcccff0c7877e4ul, 0xb565ed931b62f6a0ul, 0x3191c289u), "data/scripts/scena/dat_us/m2130.dat"),
				new UnpatchFile(new SHA1(0xf44b397573f11277ul, 0x11f66ec631b2de5dul, 0xffdafc38u), "data/scripts/scena/dat_us/m3008.dat"),
				new UnpatchFile(new SHA1(0x65044a35a4c042faul, 0xbc4a5a66fd23b0cdul, 0x8163dfdbu), "data/scripts/scena/dat_us/r0600.dat"),
				new UnpatchFile(new SHA1(0x531ae02b784b6530ul, 0xf4dc08676a793c89ul, 0xf9ebae68u), "data/scripts/scena/dat_us/r0601.dat"),
				new UnpatchFile(new SHA1(0xb96565a04c292ef7ul, 0xde28bbf071c5eae2ul, 0x2dddfffeu), "data/scripts/scena/dat_us/r0610.dat"),
				new UnpatchFile(new SHA1(0x48d8e5ebfacb29feul, 0xd7c5c4e75c84e108ul, 0x277cb5feu), "data/scripts/scena/dat_us/r0800.dat"),
				new UnpatchFile(new SHA1(0x83fc174bcce22201ul, 0xfe2053f855e8879bul, 0x3091e649u), "data/scripts/scena/dat_us/t0000.dat"),
				new UnpatchFile(new SHA1(0x8bf43f6d4c7f0de5ul, 0xba13e4c14301da05ul, 0xbd1c919au), "data/scripts/scena/dat_us/t0000c.dat"),
				new UnpatchFile(new SHA1(0x8a76ff88baf96b5eul, 0x72e675d0d5d3b75aul, 0x72cc3989u), "data/scripts/scena/dat_us/t0010.dat"),
				new UnpatchFile(new SHA1(0x9668b944717fe228ul, 0x3a482367f1448ee1ul, 0xfc63e832u), "data/scripts/scena/dat_us/t0020.dat"),
				new UnpatchFile(new SHA1(0x66a22b79517c7214ul, 0xb00b2a7a4ac898bcul, 0x5f231fd8u), "data/scripts/scena/dat_us/t0031.dat"),
				new UnpatchFile(new SHA1(0xe7854ac057166d50ul, 0xd94c340ec39403d2ul, 0x6173ff9fu), "data/scripts/scena/dat_us/t0032.dat"),
				new UnpatchFile(new SHA1(0xd363fc2114ec8421ul, 0xc24b47c29a4a2badul, 0xed31cfb5u), "data/scripts/scena/dat_us/t0050.dat"),
				new UnpatchFile(new SHA1(0x2c67ece34e81dfc6ul, 0x67fd57699b818fcful, 0xd4e6b06eu), "data/scripts/scena/dat_us/t0060.dat"),
				new UnpatchFile(new SHA1(0xeab3c1e32b228763ul, 0x9eab4b87c8ce67a4ul, 0x1e0c4d80u), "data/scripts/scena/dat_us/t0070.dat"),
				new UnpatchFile(new SHA1(0xd5805f2f25de668aul, 0x4ececc8f6cad0aaaul, 0xe64a3cf8u), "data/scripts/scena/dat_us/t0080.dat"),
				new UnpatchFile(new SHA1(0xa823fb0d4b8a4ffbul, 0xdeaa6eb407bb3880ul, 0x7048b226u), "data/scripts/scena/dat_us/t0090.dat"),
				new UnpatchFile(new SHA1(0xa772085fed67d744ul, 0x12da01161b7e4b1eul, 0xac41d585u), "data/scripts/scena/dat_us/t0210.dat"),
				new UnpatchFile(new SHA1(0x84d3de50b7318f20ul, 0xb4fe48836404d134ul, 0xa124be52u), "data/scripts/scena/dat_us/t1000.dat"),
				new UnpatchFile(new SHA1(0x44c4abb3f8e01ddeul, 0x0e36ca1d11cd433ful, 0x37c10788u), "data/scripts/scena/dat_us/t1010.dat"),
				new UnpatchFile(new SHA1(0x5574310597160a8bul, 0x94e2a8ccf2ad2dfdul, 0xc22c79d2u), "data/scripts/scena/dat_us/t1020.dat"),
				new UnpatchFile(new SHA1(0x954a31dd3eaf244ful, 0x159fbe83607ae870ul, 0xf3bdc89fu), "data/scripts/scena/dat_us/t1030.dat"),
				new UnpatchFile(new SHA1(0x0c8591be2e25657dul, 0xfecdb3ead94e046cul, 0x0bf08069u), "data/scripts/scena/dat_us/t1040.dat"),
				new UnpatchFile(new SHA1(0x60ff5f6535daa1deul, 0xbc7ecab2d514a387ul, 0xeeecb587u), "data/scripts/scena/dat_us/t1050.dat"),
				new UnpatchFile(new SHA1(0x29c1901bfb4050a4ul, 0xc0f62af6129c6538ul, 0x83f4352cu), "data/scripts/scena/dat_us/t1110.dat"),
				new UnpatchFile(new SHA1(0x4f2a0d4b4c1602bbul, 0xd9aec8dc0785f334ul, 0xc6c285f9u), "data/scripts/scena/dat_us/t1500.dat"),
				new UnpatchFile(new SHA1(0x77cd6b3d02da4e22ul, 0xefac7d5a95fd19acul, 0x837cf12bu), "data/scripts/scena/dat_us/t2110.dat"),
				new UnpatchFile(new SHA1(0xd18f9880c045b969ul, 0xafd8c6a8836ee6e8ul, 0x6810aa4eu), "data/scripts/scena/dat_us/t3500.dat"),
				new UnpatchFile(new SHA1(0xee2a5c698325223eul, 0xf9aaf196af96e4a5ul, 0x5dd18f16u), "data/scripts/scena/dat_us/t4610.dat"),
				new UnpatchFile(new SHA1(0x1a29ad14cffaa8f0ul, 0x83a283c6197b882eul, 0x61d81913u), "data/scripts/scena/dat_us/t5500.dat"),
				new UnpatchFile(new SHA1(0x78aa964124be90f4ul, 0xb9c5da90418a724cul, 0x0b46cbc3u), "data/scripts/scena/dat_us/t5660.dat"),
				new UnpatchFile(new SHA1(0xe5f61f3108f8e1d4ul, 0x3ca539b93bac91cful, 0xe37b709au), "data/scripts/talk/dat_us/tk_angelica.dat"),
				new UnpatchFile(new SHA1(0xaf8f356c80083c02ul, 0x8824fd41332b003ful, 0xec834cb1u), "data/scripts/talk/dat_us/tk_beryl.dat"),
				new UnpatchFile(new SHA1(0x3eb462def36cc151ul, 0x03e48657aa18b6b0ul, 0x2abcd830u), "data/scripts/talk/dat_us/tk_becky.dat"),
				new UnpatchFile(new SHA1(0xdc5deaa30247aafaul, 0xf2a369da2268d408ul, 0x2ba310c5u), "data/scripts/talk/dat_us/tk_edel.dat"),
				new UnpatchFile(new SHA1(0x491c365d592bb900ul, 0x29e7543d893d47bdul, 0x5e66139du), "data/scripts/talk/dat_us/tk_heinrich.dat"),
				new UnpatchFile(new SHA1(0xf423fb1dfddde29dul, 0x3e26a40ceed87982ul, 0xb899cdcau), "data/scripts/talk/dat_us/tk_laura.dat"),
				new UnpatchFile(new SHA1(0x465873e53df8cb85ul, 0xe26d302a171eb4ccul, 0x1ff2d6bbu), "data/scripts/talk/dat_us/tk_thomas.dat"),
				new UnpatchFile(new SHA1(0xb838141d25f707a7ul, 0xc95191db2f8c324aul, 0x3e0a34c0u), "data/scripts/talk/dat_us/tk_vandyck.dat"),
				new UnpatchFile(new SHA1(0x684cc74b0837ff14ul, 0x08124f8b8a05cfd9ul, 0xc9a09195u), "data/se/wav/ed8" + '\uff4d' + "2123.wav"),
				new UnpatchFile(new SHA1(0x8a735f256c69b0b2ul, 0xcd2b663820953fb4ul, 0x9523723eu), "data/text/dat/t_item.tbl"),
				new UnpatchFile(new SHA1(0x2bb6ead070625281ul, 0x87e75724d828ab0ful, 0xc8336708u), "data/text/dat/t_magic.tbl"),
				new UnpatchFile(new SHA1(0xf4b9ff78474452aaul, 0xc44f4b0c07c5a3ccul, 0x1ce27359u), "data/text/dat/t_vctiming.tbl"),
				new UnpatchFile(new SHA1(0x90a5c55ed954d771ul, 0x11563b9f3bb62ce7ul, 0xc534135au), "data/text/dat/t_voice.tbl"),
				new UnpatchFile(new SHA1(0xb64ec4d8b6204216ul, 0x6e97e60c57555203ul, 0x9c49c465u), "data/text/dat_us/t_item.tbl"),
				new UnpatchFile(new SHA1(0xd5f7bf4c4c575efdul, 0x5699e8bbd4040b81ul, 0x276a7284u), "data/text/dat_us/t_magic.tbl"),
				new UnpatchFile(new SHA1(0xa3e1c05218c7059cul, 0x371f3641a1111e16ul, 0x6e899087u), "data/text/dat_us/t_notecook.tbl"),
				new UnpatchFile(new SHA1(0xf4b9ff78474452aaul, 0xc44f4b0c07c5a3ccul, 0x1ce27359u), "data/text/dat_us/t_vctiming.tbl"),
				new UnpatchFile(new SHA1(0xdc8fa92820abc1b4ul, 0x6a646b4d75ba5d23ul, 0x9bd22ee9u), "data/text/dat_us/t_voice.tbl"),
				new UnpatchFile(new SHA1(0x6d43ad75d01d9acdul, 0x887826db59961c3eul, 0x925ccc02u), "data/voice/wav/pc8v02551.wav"),
				new UnpatchFile(new SHA1(0x373c1d1b30001af3ul, 0x60042365ed257e07ul, 0x0bf40accu), "ed8.exe"),
				new UnpatchFile(new SHA1(0x1d56abf5aa02eeaeul, 0x334797c287ef2109ul, 0xc7a103fau), "ed8jp.exe"),
			};
		}
	}
}
