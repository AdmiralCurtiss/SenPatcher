using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Checksum;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static class Sen2KnownFiles {
		public static KnownFile[] Files { get { return GetFiles(); } }

		private static object Mutex = new object();
		private static KnownFile[] Files_ = null;
		private static KnownFile[] GetFiles() {
			lock (Mutex) {
				if (Files_ == null) {
					Files_ = InitKnownFiles();
				}
				return Files_;
			}
		}
		private static KnownFile[] InitKnownFiles() {
		return new KnownFile[] {
			new KnownFile(new SHA1(0x92de0d29c0ad4a9eul, 0xa935870674976924ul, 0xd5df756du), "data/text/dat_us/t_magic.tbl", "senpatcher_bkp/data_text_dat_us_t_magic.tbl"),
			new KnownFile(new SHA1(0x5b2fee612159bcb9ul, 0x3b2c6831f94f7b1ful, 0x4dd6231cu), "data/scripts/book/dat_us/book04.dat"),

			// v1.4.1
			new KnownFile(new SHA1(0xfb45b576c39bcd2eul, 0x2806c2344c4f131eul, 0xc9611a1cu), "bin/Win32/ed8_2_PC_US.exe", important: false, writeToBackup: false), // old xdelta patch for battle skip removal
			new KnownFile(new SHA1(0x5c5f2469efd7f692ul, 0x97b3694365a277abul, 0x834617f9u), "bin/Win32/ed8_2_PC_JP.exe", important: false, writeToBackup: false), // old xdelta patch for battle skip removal
			new KnownFile(new SHA1(0xd5c333b4cd517d43ul, 0xe3868e159fbec37dul, 0xba4122d6u), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_US.exe"),
				new KnownFileAcquisitionFromGamefile("senpatcher_bkp/ed8_2_PC_US.exe.bin"),
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_US.exe.senpatcher.bkp"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0xfb45b576c39bcd2eul, 0x2806c2344c4f131eul, 0xc9611a1cu), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x00, 0x3b, 0x04, 0x82, 0x00, 0x3b, 0x04, 0x82, 0x80, 0x28, 0x03, 0xf8, 0x81, 0x74, 0x78, 0x5e, 0x88, 0x81, 0x74, 0x04, 0xf5, 0x81, 0x74, 0x68, 0x23, 0x8d, 0x95, 0x0f, 0x84, 0x6a, 0x01, 0x00, 0x00, 0x3c, 0x6f, 0x02, 0x8a, 0x7c, 0xb0, 0xc9, 0x0e, 0xf1, 0x55, 0x2a, 0xf4, 0x95, 0x41, 0xbc, 0x9b }))
			}, important: false),
			new KnownFile(new SHA1(0xb8158fb59e43c02eul, 0x904f813150d84133ul, 0x6d1a13e5u), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_JP.exe"),
				new KnownFileAcquisitionFromGamefile("senpatcher_bkp/ed8_2_PC_JP.exe.bin"),
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_JP.exe.senpatcher.bkp"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0x5c5f2469efd7f692ul, 0x97b3694365a277abul, 0x834617f9u), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x00, 0x33, 0x03, 0x82, 0x00, 0x33, 0x03, 0x82, 0x80, 0x68, 0x77, 0xf7, 0x81, 0x74, 0x38, 0x5d, 0x88, 0x81, 0x74, 0x04, 0xf5, 0x81, 0x74, 0x28, 0x1f, 0x8d, 0x95, 0x0f, 0x84, 0x6a, 0x01, 0x00, 0x00, 0x7c, 0x60, 0x7e, 0x89, 0xf7, 0x4d, 0x06, 0x64, 0x00, 0x2c, 0xa1, 0x40, 0x07, 0xc9, 0xca, 0x8d }))
			}, important: false),

			// v1.4.2
			new KnownFile(new SHA1(0xef0b17046732ecb9ul, 0x0a830a3255848e05ul, 0x8bbdce15u), "bin/Win32/ed8_2_PC_US.exe", important: false, writeToBackup: false), // old xdelta patch for battle skip removal
			new KnownFile(new SHA1(0x984aae6849d0f7e4ul, 0x484107481d0a3e1eul, 0x374bb7ccu), "bin/Win32/ed8_2_PC_JP.exe", important: false, writeToBackup: false), // old xdelta patch for battle skip removal
			new KnownFile(new SHA1(0xb08ece4ee38e6e3aul, 0x99e58eb11cffb45eul, 0x49704f86u), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_US.exe"),
				new KnownFileAcquisitionFromGamefile("senpatcher_bkp/ed8_2_PC_US.exe.bin"),
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_US.exe.senpatcher.bkp"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0xd5c333b4cd517d43ul, 0xe3868e159fbec37dul, 0xba4122d6u), HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82j142), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82u142j)).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0xef0b17046732ecb9ul, 0x0a830a3255848e05ul, 0x8bbdce15u), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x00, 0x77, 0x06, 0x82, 0x00, 0x77, 0x06, 0x82, 0x80, 0x28, 0x03, 0xf8, 0x81, 0x74, 0x78, 0x5e, 0x88, 0x81, 0x74, 0x04, 0xf5, 0x81, 0x74, 0x68, 0x23, 0x8d, 0x95, 0x0f, 0x84, 0x6a, 0x01, 0x00, 0x00, 0x3c, 0x5f, 0x0c, 0x8a, 0x18, 0xe9, 0x1b, 0xf2, 0x59, 0xf8, 0xcd, 0x0a, 0xa7, 0x5d, 0x0d, 0xfb }))
			}),
			new KnownFile(new SHA1(0x7d1db7e0bb91ab77ul, 0xa3fd1eba53b0ed25ul, 0x806186c1u), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_JP.exe"),
				new KnownFileAcquisitionFromGamefile("senpatcher_bkp/ed8_2_PC_JP.exe.bin"),
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_JP.exe.senpatcher.bkp"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0xb8158fb59e43c02eul, 0x904f813150d84133ul, 0x6d1a13e5u), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82j142).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0x984aae6849d0f7e4ul, 0x484107481d0a3e1eul, 0x374bb7ccu), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x00, 0x6f, 0x05, 0x82, 0x00, 0x6f, 0x05, 0x82, 0x80, 0x68, 0x77, 0xf7, 0x81, 0x74, 0x38, 0x5d, 0x88, 0x81, 0x74, 0x04, 0xf5, 0x81, 0x74, 0x28, 0x1f, 0x8d, 0x95, 0x0f, 0x84, 0x6a, 0x01, 0x00, 0x00, 0x7c, 0x50, 0x08, 0x8a, 0x57, 0x73, 0x17, 0xd7, 0x27, 0x4f, 0x1a, 0xa2, 0xdd, 0x9d, 0xd2, 0xfd }))
			}),

			new KnownFile(new SHA1(0x81024410cc1fd1b4ul, 0x62c600e0378714bdul, 0x7704b202u), "Sen2Launcher.exe"),
			};
		}
	}
}
