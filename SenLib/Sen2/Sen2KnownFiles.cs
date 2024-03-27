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
			Logging.Log("Initializing CS2 file data.");
			return new KnownFile[] {
			// v1.4
			new KnownFile(new SHA1(0xe5f2e2682557af7aul, 0x2f52b2299ba0980ful, 0x218c5e66u), "bin/Win32/ed8_2_PC_US.exe", important: false),
			new KnownFile(new SHA1(0x825e264333896356ul, 0x5f49e3c40aa0aec1ul, 0xd77229fau), "bin/Win32/ed8_2_PC_JP.exe", important: false),
			new KnownFile(new SHA1(0x3d75d79e3201f8f5ul, 0xac61c206f8cc86dbul, 0x7c4651ddu), "data/scripts/scena/dat/t1001.dat", important: false),
			new KnownFile(new SHA1(0xace845b437df94fbul, 0xfe2d638a2ec162b4ul, 0x92a657b3u), "data/scripts/scena/dat_us/t1001.dat", important: false),

			// v1.4.1
			new KnownFile(new SHA1(0xfb45b576c39bcd2eul, 0x2806c2344c4f131eul, 0xc9611a1cu), "bin/Win32/ed8_2_PC_US.exe", important: false, writeToBackup: false), // old xdelta patch for battle skip removal
			new KnownFile(new SHA1(0x5c5f2469efd7f692ul, 0x97b3694365a277abul, 0x834617f9u), "bin/Win32/ed8_2_PC_JP.exe", important: false, writeToBackup: false), // old xdelta patch for battle skip removal
			new KnownFile(new SHA1(0xd5c333b4cd517d43ul, 0xe3868e159fbec37dul, 0xba4122d6u), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_US.exe"),
				new KnownFileAcquisitionFromGamefile("senpatcher_bkp/ed8_2_PC_US.exe.bin"),
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_US.exe.senpatcher.bkp"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0xfb45b576c39bcd2eul, 0x2806c2344c4f131eul, 0xc9611a1cu), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x00, 0x3b, 0x04, 0x82, 0x00, 0x3b, 0x04, 0x82, 0x80, 0x28, 0x03, 0xf8, 0x81, 0x74, 0x78, 0x5e, 0x88, 0x81, 0x74, 0x04, 0xf5, 0x81, 0x74, 0x68, 0x23, 0x8d, 0x95, 0x0f, 0x84, 0x6a, 0x01, 0x00, 0x00, 0x3c, 0x6f, 0x02, 0x8a, 0x7c, 0xb0, 0xc9, 0x0e, 0xf1, 0x55, 0x2a, 0xf4, 0x95, 0x41, 0xbc, 0x9b })),
			}, important: false),
			new KnownFile(new SHA1(0xb8158fb59e43c02eul, 0x904f813150d84133ul, 0x6d1a13e5u), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_JP.exe"),
				new KnownFileAcquisitionFromGamefile("senpatcher_bkp/ed8_2_PC_JP.exe.bin"),
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_JP.exe.senpatcher.bkp"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0x5c5f2469efd7f692ul, 0x97b3694365a277abul, 0x834617f9u), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x00, 0x33, 0x03, 0x82, 0x00, 0x33, 0x03, 0x82, 0x80, 0x68, 0x77, 0xf7, 0x81, 0x74, 0x38, 0x5d, 0x88, 0x81, 0x74, 0x04, 0xf5, 0x81, 0x74, 0x28, 0x1f, 0x8d, 0x95, 0x0f, 0x84, 0x6a, 0x01, 0x00, 0x00, 0x7c, 0x60, 0x7e, 0x89, 0xf7, 0x4d, 0x06, 0x64, 0x00, 0x2c, 0xa1, 0x40, 0x07, 0xc9, 0xca, 0x8d })),
			}, important: false),
			new KnownFile(new SHA1(0x24b90bc222efb431ul, 0xa05941973b3bcbd7ul, 0xe3599d81u), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("data/scripts/scena/dat/t1001.dat"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0x3d75d79e3201f8f5ul, 0xac61c206f8cc86dbul, 0x7c4651ddu), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x51, 0x59, 0x89, 0x51, 0x59, 0x89, 0x80, 0x50, 0x13, 0x93, 0x81, 0xfe, 0x68, 0x54, 0x95, 0xd1, 0x34, 0x92, 0x1c, 0x08, 0x78, 0xd2, 0xa8, 0x99, 0xe0, 0x0c, 0x42 }), writeToBackup: false),
			}),
			new KnownFile(new SHA1(0xfae1d23cd07aa0c9ul, 0x90ca63607e64fcddul, 0xd60a80dau), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("data/scripts/scena/dat_us/t1001.dat"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0xace845b437df94fbul, 0xfe2d638a2ec162b4ul, 0x92a657b3u), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x11, 0x6a, 0x8a, 0x11, 0x6a, 0x8a, 0x80, 0x28, 0x76, 0x94, 0x81, 0xfe, 0x10, 0x34, 0x98, 0x4b, 0x18, 0x20, 0x67, 0x25, 0xf8, 0x4d, 0x9d, 0xc0, 0x33, 0x34, 0xcd }), writeToBackup: false),
			}),
			new KnownFile(new SHA1(0x568a1ae375a6077eul, 0xf5c6fb8e277a333ful, 0x1979505bu), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("data/scripts/scena/asm/t1001.tbl"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0x24b90bc222efb431ul, 0xa05941973b3bcbd7ul, 0xe3599d81u), DecompressHelper.DecompressFromBuffer(Properties.Resources.t1001_dat_to_tbl).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
			}),

			// v1.4.2
			new KnownFile(new SHA1(0xef0b17046732ecb9ul, 0x0a830a3255848e05ul, 0x8bbdce15u), "bin/Win32/ed8_2_PC_US.exe", important: false, writeToBackup: false), // old xdelta patch for battle skip removal
			new KnownFile(new SHA1(0x984aae6849d0f7e4ul, 0x484107481d0a3e1eul, 0x374bb7ccu), "bin/Win32/ed8_2_PC_JP.exe", important: false, writeToBackup: false), // old xdelta patch for battle skip removal
			new KnownFile(new SHA1(0xb08ece4ee38e6e3aul, 0x99e58eb11cffb45eul, 0x49704f86u), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_US.exe"),
				new KnownFileAcquisitionFromGamefile("senpatcher_bkp/ed8_2_PC_US.exe.bin"),
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_US.exe.senpatcher.bkp"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0xd5c333b4cd517d43ul, 0xe3868e159fbec37dul, 0xba4122d6u), HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82j142), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82u142j)).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0xef0b17046732ecb9ul, 0x0a830a3255848e05ul, 0x8bbdce15u), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x00, 0x77, 0x06, 0x82, 0x00, 0x77, 0x06, 0x82, 0x80, 0x28, 0x03, 0xf8, 0x81, 0x74, 0x78, 0x5e, 0x88, 0x81, 0x74, 0x04, 0xf5, 0x81, 0x74, 0x68, 0x23, 0x8d, 0x95, 0x0f, 0x84, 0x6a, 0x01, 0x00, 0x00, 0x3c, 0x5f, 0x0c, 0x8a, 0x18, 0xe9, 0x1b, 0xf2, 0x59, 0xf8, 0xcd, 0x0a, 0xa7, 0x5d, 0x0d, 0xfb })),
				new Patch_CS2_140_to_142(false),
			}),
			new KnownFile(new SHA1(0x7d1db7e0bb91ab77ul, 0xa3fd1eba53b0ed25ul, 0x806186c1u), new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_JP.exe"),
				new KnownFileAcquisitionFromGamefile("senpatcher_bkp/ed8_2_PC_JP.exe.bin"),
				new KnownFileAcquisitionFromGamefile("bin/Win32/ed8_2_PC_JP.exe.senpatcher.bkp"),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0xb8158fb59e43c02eul, 0x904f813150d84133ul, 0x6d1a13e5u), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82j142).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
				new KnownFileAcquisitionFromBpsPatch(new SHA1(0x984aae6849d0f7e4ul, 0x484107481d0a3e1eul, 0x374bb7ccu), new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x00, 0x6f, 0x05, 0x82, 0x00, 0x6f, 0x05, 0x82, 0x80, 0x68, 0x77, 0xf7, 0x81, 0x74, 0x38, 0x5d, 0x88, 0x81, 0x74, 0x04, 0xf5, 0x81, 0x74, 0x28, 0x1f, 0x8d, 0x95, 0x0f, 0x84, 0x6a, 0x01, 0x00, 0x00, 0x7c, 0x50, 0x08, 0x8a, 0x57, 0x73, 0x17, 0xd7, 0x27, 0x4f, 0x1a, 0xa2, 0xdd, 0x9d, 0xd2, 0xfd })),
				new Patch_CS2_140_to_142(true),
			}),

			new KnownFile(new SHA1(0x81024410cc1fd1b4ul, 0x62c600e0378714bdul, 0x7704b202u), "Sen2Launcher.exe"),
			};
		}
	}

	internal class SectionSplitExe {
		public DuplicatableStream header;
		public DuplicatableStream text;
		public DuplicatableStream rdata;
		public DuplicatableStream data;
		public DuplicatableStream rsrc;

		public SectionSplitExe(DuplicatableStream s) {
			var exe = new PeExe(s, EndianUtils.Endianness.LittleEndian);
			header = new HyoutaUtils.Streams.PartialStream(s, 0, exe.SectionHeaders[0].PointerToRawData);
			text = new HyoutaUtils.Streams.PartialStream(s, exe.SectionHeaders[0].PointerToRawData, exe.SectionHeaders[0].SizeOfRawData);
			rdata = new HyoutaUtils.Streams.PartialStream(s, exe.SectionHeaders[1].PointerToRawData, exe.SectionHeaders[1].SizeOfRawData);
			data = new HyoutaUtils.Streams.PartialStream(s, exe.SectionHeaders[2].PointerToRawData, exe.SectionHeaders[2].SizeOfRawData);
			rsrc = new HyoutaUtils.Streams.PartialStream(s, exe.SectionHeaders[3].PointerToRawData, exe.SectionHeaders[3].SizeOfRawData);
		}
	}

	public class Patch_CS2_140_to_142 : KnownFileAcquisitionMethodFunction {
		private bool Jp;

		public Patch_CS2_140_to_142(bool jp) { Jp = jp; }

		public bool Exec(FileStorage storage, List<string> localErrors, ref bool shouldWriteBackupArchive) {
			using (var jp140 = storage.TryGetDuplicate(new SHA1(0x825e264333896356ul, 0x5f49e3c40aa0aec1ul, 0xd77229fau)))
			using (var us140 = storage.TryGetDuplicate(new SHA1(0xe5f2e2682557af7aul, 0x2f52b2299ba0980ful, 0x218c5e66u))) {
				if (jp140 != null && us140 != null) {
					var jp140exe = new SectionSplitExe(jp140.CopyToByteArrayStream());
					var jp142header = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp140exe.header.Duplicate(), new HyoutaUtils.Streams.DuplicatableByteArrayStream(Properties.Resources.ed82_j140_to_j142_header)).CopyToByteArrayStreamAndDispose();
					var jp142text = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp140exe.text.Duplicate(), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82_j140_to_j142_text).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
					var jp142rdata = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp140exe.rdata.Duplicate(), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82_j140_to_j142_rdata).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
					var jp142data = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp140exe.data.Duplicate(), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82_j140_to_j142_data).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
					var jp142rsrc = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp140exe.rsrc.Duplicate(), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82_j140_to_j142_rsrc).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();

					if (Jp) {
						var jp142 = HyoutaUtils.Streams.ConcatenatedStream.CreateConcatenatedStream(new List<DuplicatableStream>() { jp142header.Duplicate(), jp142text.Duplicate(), jp142rdata.Duplicate(), jp142data.Duplicate(), jp142rsrc.Duplicate() }).CopyToByteArrayStreamAndDispose();
						var hash = new SHA1(0x7d1db7e0bb91ab77ul, 0xa3fd1eba53b0ed25ul, 0x806186c1u);
						if (ChecksumUtils.CalculateSHA1ForEntireStream(jp142) == hash) {
							SenLib.Logging.Log(string.Format("Acquired {0} (CS2 JP 1.4.2) from 1.4.0!", hash.ToString()));
							storage.Add(hash, jp142, true);
							shouldWriteBackupArchive = true;
							return true;
						} else {
							// very unlikely
							localErrors.Add("Patch did not apply correctly.");
							SenLib.Logging.Log("Patch did not apply correctly.");
							return false;
						}
					} else {
						var us140exe = new SectionSplitExe(us140.CopyToByteArrayStream());
						var us142header = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp142header.Duplicate(), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82_j142_to_u142_header).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
						var us142text = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp142text.Duplicate(), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82_j142_to_u142_text).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
						var us142rdata = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(HyoutaUtils.Streams.ConcatenatedStream.CreateConcatenatedStream(new List<DuplicatableStream>() { us140exe.rdata.Duplicate(), jp142rdata.Duplicate() }), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82_u140j142_to_u142_rdata).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
						var us142data = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp142data.Duplicate(), DecompressHelper.DecompressFromBuffer(Properties.Resources.ed82_j142_to_u142_data).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
						var us142rsrc = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp142rsrc.Duplicate(), new HyoutaUtils.Streams.DuplicatableByteArrayStream(Properties.Resources.ed82_j142_to_u142_rsrc)).CopyToByteArrayStreamAndDispose();
						var us142 = HyoutaUtils.Streams.ConcatenatedStream.CreateConcatenatedStream(new List<DuplicatableStream>() { us142header.Duplicate(), us142text.Duplicate(), us142rdata.Duplicate(), us142data.Duplicate(), us142rsrc.Duplicate() }).CopyToByteArrayStreamAndDispose();
						var hash = new SHA1(0xb08ece4ee38e6e3aul, 0x99e58eb11cffb45eul, 0x49704f86u);
						if (ChecksumUtils.CalculateSHA1ForEntireStream(us142) == hash) {
							SenLib.Logging.Log(string.Format("Acquired {0} (CS2 US 1.4.2) from 1.4.0!", hash.ToString()));
							storage.Add(hash, us142, true);
							shouldWriteBackupArchive = true;
							return true;
						} else {
							// very unlikely
							localErrors.Add("Patch did not apply correctly.");
							SenLib.Logging.Log("Patch did not apply correctly.");
							return false;
						}
					}
				} else {
					string error = "Couldn't find CS2 1.4.0.";
					localErrors.Add(error);
					SenLib.Logging.Log(error);
					return false;
				}
			}
		}
	}
}
