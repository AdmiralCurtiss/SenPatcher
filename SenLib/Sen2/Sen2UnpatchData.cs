using HyoutaUtils;
using HyoutaUtils.Checksum;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static class Sen2UnpatchData {
		public static UnpatchFile[] GetFiles() {
			return new UnpatchFile[] {
				new UnpatchFile(new SHA1(0x0ab9f575af611369ul, 0x4b18c0128cf1343aul, 0xc6b48300u), "data/text/dat_us/t_item.tbl"),
				new UnpatchFile(new SHA1(0x92de0d29c0ad4a9eul, 0xa935870674976924ul, 0xd5df756du), "data/text/dat_us/t_magic.tbl"),
				new UnpatchFile(new SHA1(0x42419bfb7e473d17ul, 0x7b08871627618050ul, 0x0c5d76f5u), "data/text/dat_us/t_notecook.tbl"),
				new UnpatchFile(new SHA1(0x9019e12bf1d93039ul, 0x254892c87f70a572ul, 0xd2ffc248u), "data/text/dat_us/t_voice.tbl"),
				new UnpatchFile(new SHA1(0x7a68747acbd132c5ul, 0x215ea9c7543c37f1ul, 0x46b39d63u), "data/scripts/book/dat_us/book00.dat"),
				new UnpatchFile(new SHA1(0x033e04f8a870f076ul, 0x4460bf5e91560499ul, 0x3c035bf5u), "data/scripts/book/dat_us/book01.dat"),
				new UnpatchFile(new SHA1(0x4152ba749355789cul, 0xf55a3311cd1c5527ul, 0xc1bc8de1u), "data/scripts/book/dat_us/book02.dat"),
				new UnpatchFile(new SHA1(0xda6f79f56616cb07ul, 0x3cc7205f895b1c0dul, 0x389c0e2du), "data/scripts/book/dat_us/book03.dat"),
				new UnpatchFile(new SHA1(0x5b2fee612159bcb9ul, 0x3b2c6831f94f7b1ful, 0x4dd6231cu), "data/scripts/book/dat_us/book04.dat"),
				new UnpatchFile(new SHA1(0xdcd1d5c83e8201c4ul, 0x2492abbe664f61eeul, 0x3a2187ffu), "data/scripts/book/dat_us/book05.dat"),
				new UnpatchFile(new SHA1(0xd0a9a39e3aa04d57ul, 0x3b9a7b51a170fb1dul, 0xc4d79f17u), "data/scripts/scena/dat_us/e7050.dat"),
				new UnpatchFile(new SHA1(0x8000797757b16c64ul, 0x656b0403584bafa1ul, 0xe85af342u), "data/scripts/scena/dat_us/e7060.dat"),
				new UnpatchFile(new SHA1(0x34005eaf8e8f7823ul, 0xf5079700cf130b2bul, 0x3b73d047u), "data/scripts/scena/dat_us/e7090.dat"),
				new UnpatchFile(new SHA1(0xcc2ff5a5eadebb18ul, 0xa6db19106284dd35ul, 0xac201243u), "data/scripts/scena/dat_us/e7101.dat"),
				new UnpatchFile(new SHA1(0x65a894acd9f5ba6ful, 0x72a5c5f6f336c7cdul, 0xa60a4b9cu), "data/scripts/scena/dat_us/e7110.dat"),
				new UnpatchFile(new SHA1(0x1e69ef573a1888e5ul, 0x545850c97d6252fbul, 0x1f5b5596u), "data/scripts/scena/dat_us/r0920.dat"),
				new UnpatchFile(new SHA1(0xc21cb1a876196b75ul, 0x51f3bc3ef57620c1ul, 0xffc0deb1u), "data/scripts/scena/dat_us/r1010.dat"),
				new UnpatchFile(new SHA1(0x9ba616b8ea524ffful, 0x05a5ac7e709f4c5ful, 0xa4561a4bu), "data/scripts/scena/dat_us/t0001.dat"),
				new UnpatchFile(new SHA1(0x7f1a94208801acaful, 0x1a3ba2fd7cba4964ul, 0x44904a1bu), "data/scripts/scena/dat_us/t0010.dat"),
				new UnpatchFile(new SHA1(0xacbda7e71e5622feul, 0x0c600792f18d25adul, 0x2c4eca0au), "data/scripts/scena/dat_us/t1010.dat"),
				new UnpatchFile(new SHA1(0x7da332f0c421b679ul, 0x6b4c2e0cc53b570cul, 0x4a4d974eu), "data/scripts/scena/dat_us/t3060.dat"),
				new UnpatchFile(new SHA1(0x3f981082b2affc86ul, 0x9785a08896aad11aul, 0x50c44343u), "data/scripts/scena/dat_us/t3500.dat"),
				new UnpatchFile(new SHA1(0x985274ffb8686434ul, 0x45b91d5f76be284aul, 0xc2c79a1cu), "data/scripts/scena/dat_us/t3740.dat"),
				new UnpatchFile(new SHA1(0x8b39f1bd64e65c40ul, 0x194b616b4c43f5d9ul, 0xc4d14c73u), "data/scripts/scena/dat_us/t4000.dat"),
				new UnpatchFile(new SHA1(0x18c7ad19818f25d7ul, 0xa2ad61c3f7c711bful, 0x16cff933u), "data/scripts/scena/dat_us/t4080.dat"),
				new UnpatchFile(new SHA1(0xdf089163a734c202ul, 0xebb8e05491ff6537ul, 0xfb1dbff7u), "data/scripts/scena/dat_us/t5501.dat"),
				new UnpatchFile(new SHA1(0xa7bff27d9558648cul, 0xb36dde4ab7cf4e9bul, 0x1ca120c2u), "data/scripts/scena/dat_us/t6500.dat"),
				new UnpatchFile(new SHA1(0x2a5b55ee43ae4498ul, 0xeccb409a491c7a6eul, 0xa1e4471au), "data/scripts/scena/dat_us/system.dat"),
				new UnpatchFile(new SHA1(0x73f84be48f549fb7ul, 0x1ac265a5d7519f3dul, 0x8ca16f1du), "data/scripts/talk/dat_us/tk_beryl.dat"),
				new UnpatchFile(new SHA1(0xea9e2d4575e33478ul, 0x2fde96e3fa4d0c3ful, 0xd5545af6u), "data/scripts/talk/dat_us/tk_emily.dat"),
				new UnpatchFile(new SHA1(0x1e88dbcff39609faul, 0xcaaca7a298032477ul, 0x39ff1f14u), "data/scripts/talk/dat_us/tk_heinrich.dat"),
				new UnpatchFile(new SHA1(0x836140611baf2149ul, 0xc7ac1a28182762b2ul, 0x5354b54fu), "data/scripts/talk/dat_us/tk_monica.dat"),
				new UnpatchFile(new SHA1(0xea737c88d0648621ul, 0xc297ed0c139348aaul, 0x6213a40du), "data/scripts/talk/dat_us/tk_rosine.dat"),
				new UnpatchFile(
					new SHA1(0xd5c333b4cd517d43ul, 0xe3868e159fbec37dul, 0xba4122d6u), // v1.4.1
					new SHA1(0xb08ece4ee38e6e3aul, 0x99e58eb11cffb45eul, 0x49704f86u), // v1.4.2
					"bin/Win32/ed8_2_PC_US.exe"),
				new UnpatchFile(
					new SHA1(0xb8158fb59e43c02eul, 0x904f813150d84133ul, 0x6d1a13e5u), // v1.4.1
					new SHA1(0x7d1db7e0bb91ab77ul, 0xa3fd1eba53b0ed25ul, 0x806186c1u), // v1.4.2
					"bin/Win32/ed8_2_PC_JP.exe"),
			};
		}
	}
}
