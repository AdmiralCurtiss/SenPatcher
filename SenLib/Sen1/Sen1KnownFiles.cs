using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Checksum;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public static class Sen1KnownFiles {
		public static KnownFile[] Files = {
			new KnownFile(new SHA1(0x1d7d909f29ac458eul, 0xf6ffc94cbd0a0a72ul, 0x49c25fdeu), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed8m2150).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
			new KnownFile(new SHA1(0xe336f59af7a03b6eul, 0xf6ec2d9938371885ul, 0x7b26ecf3u), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed8m4097).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
			new KnownFile(new SHA1(0x0056ff921028d42ful, 0x226e25555dd48336ul, 0x19a8cbc8u), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed8m4217).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
			new KnownFile(new SHA1(0x0037f27d49910859ul, 0x38a613aae6000493ul, 0x68768874u), DecompressHelper.DecompressFromBuffer(Properties.Resources.pc8v10286_15).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
			new KnownFile(new SHA1(0x73fd289006017eabul, 0x8816636d998d21aaul, 0x8fb38d68u), "data/scripts/scena/dat_us/a0006.dat", "senpatcher_bkp/data_scripts_scena_dat_us_a0006.dat"),
			new KnownFile(new SHA1(0x9a75c8439ca12500ul, 0x437af585d0ae9474ul, 0x4b620dfdu), "data/scripts/scena/dat_us/c0100.dat", "senpatcher_bkp/data_scripts_scena_dat_us_c0100.dat"),
			new KnownFile(new SHA1(0xa740904f6ad37411ul, 0x825565981daa5f59ul, 0x15b55b69u), "data/scripts/scena/dat_us/c0110.dat", "senpatcher_bkp/data_scripts_scena_dat_us_c0110.dat"),
			new KnownFile(new SHA1(0x65044a35a4c042faul, 0xbc4a5a66fd23b0cdul, 0x8163dfdbu), "data/scripts/scena/dat_us/r0600.dat", "senpatcher_bkp/data_scripts_scena_dat_us_r0600.dat"),
			new KnownFile(new SHA1(0x48d8e5ebfacb29feul, 0xd7c5c4e75c84e108ul, 0x277cb5feu), "data/scripts/scena/dat_us/r0800.dat", "senpatcher_bkp/data_scripts_scena_dat_us_r0800.dat"),
			new KnownFile(new SHA1(0x83fc174bcce22201ul, 0xfe2053f855e8879bul, 0x3091e649u), "data/scripts/scena/dat_us/t0000.dat", "senpatcher_bkp/data_scripts_scena_dat_us_t0000.dat"),
			new KnownFile(new SHA1(0x8a76ff88baf96b5eul, 0x72e675d0d5d3b75aul, 0x72cc3989u), "data/scripts/scena/dat_us/t0010.dat", "senpatcher_bkp/data_scripts_scena_dat_us_t0010.dat"),
			new KnownFile(new SHA1(0x66a22b79517c7214ul, 0xb00b2a7a4ac898bcul, 0x5f231fd8u), "data/scripts/scena/dat_us/t0031.dat", "senpatcher_bkp/data_scripts_scena_dat_us_t0031.dat"),
			new KnownFile(new SHA1(0xe7854ac057166d50ul, 0xd94c340ec39403d2ul, 0x6173ff9fu), "data/scripts/scena/dat_us/t0032.dat", "senpatcher_bkp/data_scripts_scena_dat_us_t0032.dat"),
			new KnownFile(new SHA1(0xd363fc2114ec8421ul, 0xc24b47c29a4a2badul, 0xed31cfb5u), "data/scripts/scena/dat_us/t0050.dat", "senpatcher_bkp/data_scripts_scena_dat_us_t0050.dat"),
			new KnownFile(new SHA1(0xd5805f2f25de668aul, 0x4ececc8f6cad0aaaul, 0xe64a3cf8u), "data/scripts/scena/dat_us/t0080.dat", "senpatcher_bkp/data_scripts_scena_dat_us_t0080.dat"),
			new KnownFile(new SHA1(0x84d3de50b7318f20ul, 0xb4fe48836404d134ul, 0xa124be52u), "data/scripts/scena/dat_us/t1000.dat", "senpatcher_bkp/data_scripts_scena_dat_us_t1000.dat"),
			new KnownFile(new SHA1(0x44c4abb3f8e01ddeul, 0x0e36ca1d11cd433ful, 0x37c10788u), "data/scripts/scena/dat_us/t1010.dat", "senpatcher_bkp/data_scripts_scena_dat_us_t1010.dat"),
			new KnownFile(new SHA1(0x4f2a0d4b4c1602bbul, 0xd9aec8dc0785f334ul, 0xc6c285f9u), "data/scripts/scena/dat_us/t1500.dat", "senpatcher_bkp/data_scripts_scena_dat_us_t1500.dat"),
			new KnownFile(new SHA1(0xd18f9880c045b969ul, 0xafd8c6a8836ee6e8ul, 0x6810aa4eu), "data/scripts/scena/dat_us/t3500.dat", "senpatcher_bkp/data_scripts_scena_dat_us_t3500.dat"),
			new KnownFile(new SHA1(0xaf8f356c80083c02ul, 0x8824fd41332b003ful, 0xec834cb1u), "data/scripts/talk/dat_us/tk_beryl.dat", "senpatcher_bkp/data_scripts_talk_dat_us_tk_beryl.dat"),
			new KnownFile(new SHA1(0xdc5deaa30247aafaul, 0xf2a369da2268d408ul, 0x2ba310c5u), "data/scripts/talk/dat_us/tk_edel.dat", "senpatcher_bkp/data_scripts_talk_dat_us_tk_edel.dat"),
			new KnownFile(new SHA1(0x491c365d592bb900ul, 0x29e7543d893d47bdul, 0x5e66139du), "data/scripts/talk/dat_us/tk_heinrich.dat", "senpatcher_bkp/data_scripts_talk_dat_us_tk_heinrich.dat"),
			new KnownFile(new SHA1(0xf423fb1dfddde29dul, 0x3e26a40ceed87982ul, 0xb899cdcau), "data/scripts/talk/dat_us/tk_laura.dat", "senpatcher_bkp/data_scripts_talk_dat_us_tk_laura.dat"),
			new KnownFile(new SHA1(0xb838141d25f707a7ul, 0xc95191db2f8c324aul, 0x3e0a34c0u), "data/scripts/talk/dat_us/tk_vandyck.dat", "senpatcher_bkp/data_scripts_talk_dat_us_tk_vandyck.dat"),
			new KnownFile(new SHA1(0x684cc74b0837ff14ul, 0x08124f8b8a05cfd9ul, 0xc9a09195u), "data/se/wav/ed8" + '\uff4d' + "2123.wav", "data/se/wav/ed8m2123.wav", "senpatcher_bkp/data_se_wav_ed8m2123.wav"),
			new KnownFile(new SHA1(0xd5f7bf4c4c575efdul, 0x5699e8bbd4040b81ul, 0x276a7284u), "data/text/dat_us/t_magic.tbl", "senpatcher_bkp/data_text_dat_us_t_magic.tbl"),
			new KnownFile(new SHA1(0xf4b9ff78474452aaul, 0xc44f4b0c07c5a3ccul, 0x1ce27359u), "data/text/dat_us/t_vctiming.tbl"),
			new KnownFile(new SHA1(0xdc8fa92820abc1b4ul, 0x6a646b4d75ba5d23ul, 0x9bd22ee9u), "data/text/dat_us/t_voice.tbl"),
			new KnownFile(new SHA1(0x591122d8707ebee4ul, 0xf3afa135c32c2113ul, 0x1e1a7bafu), "ed8.exe", important: false, writeToBackup: false), // old xdelta patch for battle skip removal
			new KnownFile(new SHA1(0x46feedf2f507e1dcul, 0x467c8ba002b0394bul, 0xb8fd464eu), "ed8jp.exe", important: false, writeToBackup: false), // old xdelta patch for battle skip removal
			new KnownFile(new SHA1(0x373c1d1b30001af3ul, 0x60042365ed257e07ul, 0x0bf40accu), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("ed8.exe"),
				new KnownFileAcquisitionFromGamefile("senpatcher_bkp/ed8.exe.bin"),
				new KnownFileAcquisitionFromGamefile("ed8.exe.senpatcher.bkp"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0x591122d8707ebee4ul, 0xf3afa135c32c2113ul, 0x1e1a7bafu), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x00, 0x67, 0x1d, 0x83, 0x00, 0x67, 0x1d, 0x83, 0x80, 0x00, 0x02, 0x56, 0x80, 0x81, 0x74, 0x30, 0x2e, 0x99, 0x95, 0x0f, 0x84, 0x80, 0x01, 0x00, 0x00, 0x28, 0x6c, 0x08, 0x8e, 0x03, 0xb0, 0x1c, 0xe2, 0x4d, 0x26, 0x96, 0x4a, 0x0b, 0xad, 0x99, 0x0b }))
			}),
			new KnownFile(new SHA1(0x1d56abf5aa02eeaeul, 0x334797c287ef2109ul, 0xc7a103fau), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("ed8jp.exe"),
				new KnownFileAcquisitionFromGamefile("senpatcher_bkp/ed8jp.exe.bin"),
				new KnownFileAcquisitionFromGamefile("ed8jp.exe.senpatcher.bkp"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0x46feedf2f507e1dcul, 0x467c8ba002b0394bul, 0xb8fd464eu), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x00, 0x1f, 0x1d, 0x83, 0x00, 0x1f, 0x1d, 0x83, 0x80, 0x40, 0x3f, 0x54, 0x80, 0x81, 0x74, 0x70, 0x2f, 0x99, 0x95, 0x0f, 0x84, 0x80, 0x01, 0x00, 0x00, 0x28, 0x0d, 0x08, 0x8e, 0x64, 0x17, 0x54, 0x13, 0x23, 0xe2, 0xa3, 0x29, 0x10, 0x42, 0x77, 0x67 }))
			}),
			new KnownFile(new SHA1(0x8dde2b39f128179aul, 0x0beb3301cfd56a98ul, 0xc0f98a55u), "Sen1Launcher.exe"),
		};
	}
}
