using HyoutaUtils;
using HyoutaUtils.Streams;
using SenLib;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli {
	public static class t_voice_tbl {
		public static int CheckVoiceTable(string voicetablefilename, string pathWavdir, int sengame, string outputPath, EndianUtils.Endianness endian, TextUtils.GameTextEncoding encoding) {
			List<ushort> knownMultipleMapped = new List<ushort>();
			List<List<t_voice_tbl_entry>> sameIndexMappedMultipleTimes = new List<List<t_voice_tbl_entry>>();
			List<t_voice_tbl_entry> regularMapped = new List<t_voice_tbl_entry>();
			List<t_voice_tbl_entry> mappedToNonexistentFile = new List<t_voice_tbl_entry>();
			List<t_voice_tbl_entry> tblEntries = new List<t_voice_tbl_entry>();

			if (sengame == 1) {
				var tbl = new SenLib.Sen1.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
				foreach (var e in tbl.Entries) {
					var vd = new SenLib.Sen1.VoiceDataCS1(e.Data, endian, encoding);
					tblEntries.Add(new t_voice_tbl_entry(vd.Index, vd.Name));
				}
			} else if (sengame == 2) {
				var tbl = new SenLib.Sen2.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
				foreach (var e in tbl.Entries) {
					var vd = new SenLib.Sen1.VoiceDataCS1(e.Data, endian, encoding);
					tblEntries.Add(new t_voice_tbl_entry(vd.Index, vd.Name));
				}
			} else if (sengame == 3) {
				var tbl = new SenLib.Sen3.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
				foreach (var e in tbl.Entries) {
					var vd = new SenLib.Sen3.VoiceDataCS3(e.Data, endian, encoding);
					tblEntries.Add(new t_voice_tbl_entry(vd.Index, vd.Name));
				}
			} else if (sengame == 4) {
				var tbl = new SenLib.Sen4.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
				foreach (var e in tbl.Entries) {
					var vd = new SenLib.Sen4.VoiceDataCS4(e.Data, endian, encoding);
					tblEntries.Add(new t_voice_tbl_entry(vd.Index, vd.Name));
				}
			}

			Dictionary<ushort, t_voice_tbl_entry> byIndex = new Dictionary<ushort, t_voice_tbl_entry>();
			foreach (var e in tblEntries) {
				if (byIndex.ContainsKey(e.Index)) {
					if (!knownMultipleMapped.Contains(e.Index)) {
						knownMultipleMapped.Add(e.Index);
						List<t_voice_tbl_entry> tmp = new List<t_voice_tbl_entry>();
						foreach (var dup in tblEntries.Where(x => x.Index == e.Index)) {
							tmp.Add(dup);
						}
						sameIndexMappedMultipleTimes.Add(tmp);
					}
				} else {
					byIndex.Add(e.Index, e);
				}
			}

			HashSet<string> existingVoiceFiles = new HashSet<string>();
			foreach (var f in new DirectoryInfo(pathWavdir).GetFiles()) {
				existingVoiceFiles.Add(Path.GetFileNameWithoutExtension(f.FullName));
			}

			foreach (var e in byIndex.OrderBy(x => x.Key)) {
				if (existingVoiceFiles.Contains(e.Value.Name)) {
					regularMapped.Add(e.Value);
				} else {
					mappedToNonexistentFile.Add(e.Value);
				}
			}
			foreach (var e in byIndex.OrderBy(x => x.Key)) {
				if (existingVoiceFiles.Contains(e.Value.Name)) {
					existingVoiceFiles.Remove(e.Value.Name);
				}
			}

			StringBuilder sb = new StringBuilder();
			sb.AppendLine("The following files have mappings, but no corresponding audio file:").AppendLine();
			foreach (t_voice_tbl_entry e in mappedToNonexistentFile) {
				sb.AppendFormat("{0} => {1} (file does not exist)", e.Index, e.Name).AppendLine();
			}
			sb.AppendLine().AppendLine().AppendLine();

			sb.AppendLine("The following indices map to multiple files:");
			sb.AppendLine("(note: it's unclear which file is actually used in this case)").AppendLine();
			foreach (List<t_voice_tbl_entry> mm in sameIndexMappedMultipleTimes) {
				sb.AppendFormat("index {0} maps to:", mm[0].Index);
				foreach (var dup in mm) {
					sb.Append(" ").Append(dup.Name);
				}
				sb.AppendLine();
			}
			sb.AppendLine().AppendLine().AppendLine();

			sb.AppendLine("The following audio files have no mapping:");
			sb.AppendLine("(note: may be wrong for files with multiple indices)").AppendLine();
			foreach (string vf in existingVoiceFiles) {
				sb.AppendFormat("{0} is not mapped", vf).AppendLine();
			}
			sb.AppendLine().AppendLine().AppendLine();

			sb.AppendLine("Used index to file mappings below:");
			sb.AppendLine("(note: may be wrong for files with multiple indices)").AppendLine();
			foreach (t_voice_tbl_entry e in regularMapped) {
				sb.AppendFormat("{0} => {1}", e.Index, e.Name).AppendLine();
			}
			sb.AppendLine().AppendLine().AppendLine();

			File.WriteAllText(outputPath, sb.ToString());

			return 0;
		}
	}

	public class t_voice_tbl_entry {
		public ushort Index;
		public string Name;

		public t_voice_tbl_entry(ushort index, string name) {
			Index = index;
			Name = name;
		}

		public override string ToString() {
			return string.Format("{0}: {1}", Index, Name);
		}
	}
}
