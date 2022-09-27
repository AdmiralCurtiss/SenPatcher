using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	internal class VoiceTimingData {
		public ushort Index;
		public byte[] Unknown;

		public VoiceTimingData(byte[] data, EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			var s = new DuplicatableByteArrayStream(data);
			Index = s.ReadUInt16(e);
			Unknown = s.ReadBytes(s.Length - s.Position);
		}

		public byte[] ToBinary(EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			MemoryStream s = new MemoryStream();
			s.WriteUInt16(Index, e);
			s.Write(Unknown);
			return s.CopyToByteArrayAndDispose();
		}
	}

	class t_vctiming_us : FileMod {
		public string GetDescription() {
			return "Update lipsync to match changed voice lines.";
		}

		private static void CreateEntryCopy(Tbl tbl, int source, int target, ushort newIndex) {
			var e = tbl.Entries[source];
			var vd = new VoiceTimingData(e.Data, EndianUtils.Endianness.LittleEndian);
			vd.Index = newIndex;
			TblEntry newEntry = new TblEntry();
			newEntry.Name = e.Name;
			newEntry.Data = vd.ToBinary(EndianUtils.Endianness.LittleEndian);
			tbl.Entries.Insert(target, newEntry);
		}

		private static void CopyEntryData(Tbl tbl, int source, int target) {
			var e_source = tbl.Entries[source];
			var e_target = tbl.Entries[target];
			var vd_source = new VoiceTimingData(e_source.Data, EndianUtils.Endianness.LittleEndian);
			var vd_target = new VoiceTimingData(e_target.Data, EndianUtils.Endianness.LittleEndian);
			vd_target.Unknown = vd_source.Unknown;
			e_target.Data = vd_target.ToBinary(EndianUtils.Endianness.LittleEndian);
		}

		private static void CreateEntryData(Tbl tbl, int target, byte[] data) {
			var e_target = tbl.Entries[target];
			var vd_target = new VoiceTimingData(e_target.Data, EndianUtils.Endianness.LittleEndian);
			vd_target.Unknown = data;
			e_target.Data = vd_target.ToBinary(EndianUtils.Endianness.LittleEndian);
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x82c78ea6050e0648ul, 0x69f7428b92788e8aul, 0xa22dbac1u));
			if (file_en == null) {
				return null;
			}

			var s = file_en;

			var tbl_pc = new Tbl(s);

			// add entries for voice clips that don't have entries yet
			CreateEntryCopy(tbl_pc, 312, 6608, 2349); // v04_s0049
			CreateEntryCopy(tbl_pc, 904, 6452, 1847); // v02_s0047
			CreateEntryCopy(tbl_pc, 2660, 4986, 40377); // v72_e0011
			CreateEntryCopy(tbl_pc, 445, 4547, 39412); // v31_e0070
			CreateEntryCopy(tbl_pc, 2553, 2120, 34417); // v27_e0042
			CreateEntryCopy(tbl_pc, 613, 1462, 33253); // v00_e0427

			// copy data to match PS4 patch for changed voice clips
			CopyEntryData(tbl_pc, 1864, 2592); // v20_e0066
			CopyEntryData(tbl_pc, 1684, 3326); // v56_e0092
			CopyEntryData(tbl_pc, 57, 3911); // v72_e0005
			CopyEntryData(tbl_pc, 118, 4435); // v43_e0023
			CopyEntryData(tbl_pc, 598, 4890); // v52_e0109
			CopyEntryData(tbl_pc, 1321, 5042); // v72_e0012
			CopyEntryData(tbl_pc, 105, 6131); // v00_s0110
			CopyEntryData(tbl_pc, 4, 6181); // v00_s0028
			CopyEntryData(tbl_pc, 122, 6530); // v02_s0249
			CopyEntryData(tbl_pc, 4488, 6846); // v11_s0050
			CopyEntryData(tbl_pc, 72, 6952); // v12_s0049

			// these don't have exact equivalents
			// CopyEntryData(tbl_pc, 254, 986); // v07_e0026
			// tbl_pc.Entries[986].Data[4] = 0x13;
			CopyEntryData(tbl_pc, 435, 1554); // v00_e0441
			tbl_pc.Entries[1554].Data[5] = 0x0f;
			CopyEntryData(tbl_pc, 1454, 3756); // v49_e0004
			tbl_pc.Entries[3756].Data[6] = 0x03;
			CopyEntryData(tbl_pc, 370, 4494); // v55_e0018
			tbl_pc.Entries[4494].Data[5] = tbl_pc.Entries[33].Data[5];
			tbl_pc.Entries[4494].Data[6] = tbl_pc.Entries[33].Data[6];
			CopyEntryData(tbl_pc, 5519, 4579); // v00_e1032
			tbl_pc.Entries[4579].Data[5] = 0x78;
			CopyEntryData(tbl_pc, 5836, 4598); // v55_e0062
			tbl_pc.Entries[4598].Data[6] = 0xef;
			tbl_pc.Entries[4598].Data[7] = 0xff;
			CopyEntryData(tbl_pc, 624, 4618); // v00_e1054
			tbl_pc.Entries[4618].Data[6] = 0x03;
			CopyEntryData(tbl_pc, 743, 4899); // v52_e0119
			tbl_pc.Entries[4899].Data[6] = 0xf7;
			CopyEntryData(tbl_pc, 1431, 6793); // v06_s0047
			tbl_pc.Entries[6793].Data[4] = 0x0d;

			// these are not in the PS4 patch
			tbl_pc.Entries.RemoveAt(6614); // v04_s0054
			tbl_pc.Entries.RemoveAt(5307); // v57_e0020

			//var p = new DuplicatableFileStream("ps4_1.03_t_vctiming_us.tbl");
			//var tbl_ps4 = new Tbl(p);
			//
			//var voice = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x3f0b2757fd9c17abul, 0x1aeb6f38e323d6fbul, 0x4d6cdaadu));
			//var tbl_voice = new Tbl(voice);
			//
			//List<VoiceDataCS3> voiceData = new List<VoiceDataCS3>();
			//for (int i = 0; i < tbl_voice.Entries.Count; ++i) {
			//	var e = tbl_voice.Entries[i];
			//	if (e.Name == "voice") {
			//		voiceData.Add(new VoiceDataCS3(e.Data, EndianUtils.Endianness.LittleEndian));
			//	}
			//}
			//
			//List<VoiceTimingData> voiceTiming_pc = new List<VoiceTimingData>();
			//for (int i = 0; i < tbl_pc.Entries.Count; ++i) {
			//	var e = tbl_pc.Entries[i];
			//	if (e.Name == "VoiceTiming") {
			//		voiceTiming_pc.Add(new VoiceTimingData(e.Data, EndianUtils.Endianness.LittleEndian));
			//	}
			//}
			//List<VoiceTimingData> voiceTiming_ps4 = new List<VoiceTimingData>();
			//for (int i = 0; i < tbl_ps4.Entries.Count; ++i) {
			//	var e = tbl_ps4.Entries[i];
			//	if (e.Name == "VoiceTiming") {
			//		voiceTiming_ps4.Add(new VoiceTimingData(e.Data, EndianUtils.Endianness.LittleEndian));
			//	}
			//}

			//HashSet<ushort> indices_pc = new HashSet<ushort>();
			//HashSet<ushort> indices_ps4 = new HashSet<ushort>();
			//for (int i = 0; i < voiceTiming_pc.Count; ++i) {
			//	indices_pc.Add(voiceTiming_pc[i].Index);
			//}
			//for (int i = 0; i < voiceTiming_ps4.Count; ++i) {
			//	indices_ps4.Add(voiceTiming_ps4[i].Index);
			//}

			//SortedSet<ushort> indices_both = new SortedSet<ushort>();
			//var voiceTimingLookup_pc = new Dictionary<ushort, VoiceTimingData>();
			//var voiceTimingLookup_ps4 = new Dictionary<ushort, VoiceTimingData>();
			//for (int i = 0; i < voiceTiming_pc.Count; ++i) {
			//	indices_both.Add(voiceTiming_pc[i].Index);
			//	voiceTimingLookup_pc.Add(voiceTiming_pc[i].Index, voiceTiming_pc[i]);
			//}
			//for (int i = 0; i < voiceTiming_ps4.Count; ++i) {
			//	indices_both.Add(voiceTiming_ps4[i].Index);
			//	voiceTimingLookup_ps4.Add(voiceTiming_ps4[i].Index, voiceTiming_ps4[i]);
			//}
			//
			//foreach (ushort index in indices_both) {
			//	VoiceTimingData vt_pc;
			//	VoiceTimingData vt_ps4;
			//	voiceTimingLookup_pc.TryGetValue(index, out vt_pc);
			//	voiceTimingLookup_ps4.TryGetValue(index, out vt_ps4);
			//	bool different = vt_pc == null || vt_ps4 == null;
			//	if (!different) {
			//		different = !Enumerable.SequenceEqual(vt_pc.Unknown, vt_ps4.Unknown);
			//	}
			//
			//	if (different) {
			//		Console.WriteLine("Difference in Index {0}, matches file {1} (exists on PS4: {2}, exists on PC: {3})",
			//			index, voiceData.Where(x => x.Index == index).First().Name, vt_ps4 != null, vt_pc != null);
			//	}
			//}

			Stream result_en = new MemoryStream();
			tbl_pc.RecalcNumberOfEntries();
			tbl_pc.WriteToStream(result_en, EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_vctiming_us.tbl", result_en.CopyToByteArrayStream()),
				new FileModResult("data/text/dat_fr/t_vctiming_us.tbl", result_en.CopyToByteArrayStream()),
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x82c78ea6050e0648ul, 0x69f7428b92788e8aul, 0xa22dbac1u));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_vctiming_us.tbl", file_en.Duplicate()),
				new FileModResult("data/text/dat_fr/t_vctiming_us.tbl", file_en.Duplicate()),
			};
		}
	}
}
