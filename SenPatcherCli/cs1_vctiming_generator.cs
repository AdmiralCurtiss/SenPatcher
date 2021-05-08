using HyoutaUtils;
using SenLib;
using SenLib.Sen1;
using SenLib.Sen2;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli {
	public static class cs1_vctiming_generator {
		public static void GenerateJpFiles(bool prefer_ps3_over_ps4) {
			VoiceTable canonicalJpVoiceTable;
			using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(@"c:\_tmp_a\_cs1-voicetiming\__pc-senpatcher\t_voice_JP.tbl")) {
				canonicalJpVoiceTable = new VoiceTable(fs, EndianUtils.Endianness.LittleEndian);
			}

			VoiceTiming jpPs3VoiceTiming;
			using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(@"c:\_tmp_a\_cs1-voicetiming\_ps3-jp\t_vctiming.tbl")) {
				jpPs3VoiceTiming = new VoiceTiming(fs, EndianUtils.Endianness.BigEndian);
			}

			VoiceTiming jpPs4USVoiceTiming;
			using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(@"c:\_tmp_a\_cs1-voicetiming\_ps4-us\t_vctiming.tbl")) {
				jpPs4USVoiceTiming = new VoiceTiming(fs, EndianUtils.Endianness.LittleEndian);
			}

			SortedDictionary<ushort, VoiceTimingEntry> voiceTimingDict = new SortedDictionary<ushort, VoiceTimingEntry>();
			foreach (VoiceTimingEntry vte in jpPs3VoiceTiming.Entries) {
				if (voiceTimingDict.ContainsKey(vte.Index)) {
					Console.WriteLine($"index {vte.Index} mapped more than once?");
					voiceTimingDict.Remove(vte.Index);
				}
				voiceTimingDict.Add(vte.Index, vte);
			}

			HashSet<ushort> mappedIndices = new HashSet<ushort>();
			foreach (VoiceTableEntry vte in canonicalJpVoiceTable.Entries) {
				if (!mappedIndices.Contains(vte.Index)) {
					mappedIndices.Add(vte.Index);
				}
			}

			foreach (VoiceTimingEntry vte in jpPs4USVoiceTiming.Entries) {
				if (mappedIndices.Contains(vte.Index)) {
					if (prefer_ps3_over_ps4) {
						// only insert if there's not already an entry
						if (!voiceTimingDict.ContainsKey(vte.Index)) {
							voiceTimingDict.Add(vte.Index, vte);
						}
					} else {
						// overwrite existing entries
						if (voiceTimingDict.ContainsKey(vte.Index)) {
							voiceTimingDict.Remove(vte.Index);
						}
						voiceTimingDict.Add(vte.Index, vte);
					}
				}
			}


			jpPs3VoiceTiming.Entries.Clear();
			foreach (var kvp in voiceTimingDict) {
				jpPs3VoiceTiming.Entries.Add(kvp.Value);
			}
			using (var fs = new FileStream(@"c:\_tmp_a\_cs1-voicetiming\__pc-senpatcher\__gen_vctiming_jp_ps" + (prefer_ps3_over_ps4 ? "3" : "4") + "_variant.tbl", FileMode.Create)) {
				jpPs3VoiceTiming.WriteToStream(fs, EndianUtils.Endianness.LittleEndian);
			}
		}

		public static void GenerateEnFiles(bool prefer_ps3_over_ps4) {
			// we pretty much do the same as JP here but we have to do an ID remapping, because the PS4 internal IDs for the PC audio files are different
			VoiceTable canonicalEnVoiceTable;
			using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(@"c:\_tmp_a\_cs1-voicetiming\__pc-senpatcher\t_voice_US.tbl")) {
				canonicalEnVoiceTable = new VoiceTable(fs, EndianUtils.Endianness.LittleEndian);
			}
			VoiceTable ps4EnVoiceTable;
			using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(@"c:\_tmp_a\_cs1-voicetiming\_ps4-us\t_voice.tbl")) {
				ps4EnVoiceTable = new VoiceTable(fs, EndianUtils.Endianness.LittleEndian);
			}

			Dictionary<string, List<VoiceTableEntry>> pcFilenameToInternalIdMap = new Dictionary<string, List<VoiceTableEntry>>();
			foreach (var a in canonicalEnVoiceTable.Entries) {
				if (!pcFilenameToInternalIdMap.ContainsKey(a.Name)) {
					pcFilenameToInternalIdMap.Add(a.Name, new List<VoiceTableEntry>());
				}
				pcFilenameToInternalIdMap[a.Name].Add(a);
			}
			Dictionary<string, List<VoiceTableEntry>> ps4FilenameToInternalIdMap = new Dictionary<string, List<VoiceTableEntry>>();
			foreach (var a in ps4EnVoiceTable.Entries) {
				string n = a.Name == "pc8v10286" ? "pc8v10299" : a.Name == "pc8v10286_6" ? "pc8v10286" : a.Name; // need to match my remapping of the overwritten sara line
				if (!ps4FilenameToInternalIdMap.ContainsKey(n)) {
					ps4FilenameToInternalIdMap.Add(n, new List<VoiceTableEntry>());
				}
				ps4FilenameToInternalIdMap[n].Add(a);
			}

			if (pcFilenameToInternalIdMap.Count != ps4FilenameToInternalIdMap.Count) {
				// shouldn't happen with correct input
				Console.WriteLine($"global count mismatch PC {pcFilenameToInternalIdMap.Count} <=> PS4 {ps4FilenameToInternalIdMap.Count}");
			}

			foreach (var kvp in pcFilenameToInternalIdMap) {
				var pc = kvp.Value;
				var ps4 = ps4FilenameToInternalIdMap[kvp.Key];
				if (pc.Count != ps4.Count) {
					if (pc.Count == 2 && ps4.Count == 1) {
						// just dupe the PS4 entry to get it to match 1:1
						ps4.Add(new VoiceTableEntry(ps4[0]));
					} else {
						// shouldn't happen with correct input
						Console.WriteLine($"count mismatch PC {pc.Count} <=> PS4 {ps4.Count}");
					}
				}
			}

			// okay now we have a 1:1 mapping between PC and PS4 IDs; we can apply this on the PS4 voice timing file to get a PC-compatible one

			VoiceTiming enPs3PcVoiceTiming; // is the same file in both versions, just endian-flipped
			using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(@"c:\_tmp_a\_cs1-voicetiming\_pc-us\t_vctiming.tbl")) {
				enPs3PcVoiceTiming = new VoiceTiming(fs, EndianUtils.Endianness.LittleEndian);
			}

			VoiceTiming enPs4USVoiceTiming;
			using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(@"c:\_tmp_a\_cs1-voicetiming\_ps4-us\t_vctiming_us.tbl")) {
				enPs4USVoiceTiming = new VoiceTiming(fs, EndianUtils.Endianness.LittleEndian);
			}

			SortedDictionary<ushort, VoiceTimingEntry> voiceTimingDict = new SortedDictionary<ushort, VoiceTimingEntry>();
			foreach (VoiceTimingEntry vte in enPs3PcVoiceTiming.Entries) {
				if (voiceTimingDict.ContainsKey(vte.Index)) {
					Console.WriteLine($"index {vte.Index} mapped more than once?");
					var existingMapping = voiceTimingDict[vte.Index];
					if (existingMapping.TimingData != vte.TimingData) {
						Console.WriteLine($"timing data mismatch: {existingMapping.TimingData.ToString("x16")} <=> {vte.TimingData.ToString("x16")}");
					}
					voiceTimingDict.Remove(vte.Index);
				}
				voiceTimingDict.Add(vte.Index, vte);
			}

			HashSet<ushort> mappedIndices = new HashSet<ushort>();
			foreach (VoiceTableEntry vte in canonicalEnVoiceTable.Entries) {
				if (!mappedIndices.Contains(vte.Index)) {
					mappedIndices.Add(vte.Index);
				}
			}

			foreach (VoiceTimingEntry vte in enPs4USVoiceTiming.Entries) {
				List<VoiceTableEntry> specialPcEntries = null;
				List<VoiceTableEntry> specialPs4Entries = null;
				ushort remappedIndex = RemapPs4ToPc(vte.Index, pcFilenameToInternalIdMap, ps4FilenameToInternalIdMap, ref specialPcEntries, ref specialPs4Entries);

				ushort[] indices;
				if (specialPcEntries != null) {
					indices = new ushort[2] { specialPcEntries[0].Index, specialPcEntries[1].Index };
				} else {
					indices = new ushort[1] { remappedIndex };
				}

				foreach (ushort index in indices) {
					if (mappedIndices.Contains(index)) {
						if (prefer_ps3_over_ps4) {
							// only insert if there's not already an entry
							if (!voiceTimingDict.ContainsKey(index)) {
								VoiceTimingEntry vte2 = new VoiceTimingEntry(vte);
								vte2.Index = index;
								voiceTimingDict.Add(index, vte2);
							}
						} else {
							// overwrite existing entries
							if (voiceTimingDict.ContainsKey(index)) {
								voiceTimingDict.Remove(index);
							}
							VoiceTimingEntry vte2 = new VoiceTimingEntry(vte);
							vte2.Index = index;
							voiceTimingDict.Add(index, vte2);
						}
					}
				}
			}

			enPs3PcVoiceTiming.Entries.Clear();
			foreach (var kvp in voiceTimingDict) {
				enPs3PcVoiceTiming.Entries.Add(kvp.Value);
			}
			using (var fs = new FileStream(@"c:\_tmp_a\_cs1-voicetiming\__pc-senpatcher\__gen_vctiming_us_ps" + (prefer_ps3_over_ps4 ? "3" : "4") + "_variant.tbl", FileMode.Create)) {
				enPs3PcVoiceTiming.WriteToStream(fs, EndianUtils.Endianness.LittleEndian);
			}
		}

		private static ushort RemapPs4ToPc(
			ushort ps4index,
			Dictionary<string, List<VoiceTableEntry>> pcFilenameToInternalIdMap,
			Dictionary<string, List<VoiceTableEntry>> ps4FilenameToInternalIdMap,
			ref List<VoiceTableEntry> specialPcEntries,
			ref List<VoiceTableEntry> specialPs4Entries
		) {
			foreach (var ps4 in ps4FilenameToInternalIdMap) {
				for (int i = 0; i < ps4.Value.Count; ++i) {
					if (ps4.Value[i].Index == ps4index) {
						var pc = pcFilenameToInternalIdMap[ps4.Key];
						if (ps4.Value.Count > 1 && ps4.Value[0].Index == ps4.Value[1].Index) {
							specialPcEntries = pc;
							specialPs4Entries = ps4.Value;
						}
						return pc[i].Index;
					}
				}
			}

			throw new Exception("??? should not happen");
		}
	}
}
