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
	public class t_voice_tbl {
		public List<t_voice_tbl_entry> Entries;

		public t_voice_tbl(Stream stream) {
			Stream s = new DuplicatableByteArrayStream(stream.CopyToByteArray());
			EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian;
			ushort count = s.ReadUInt16(e);

			if (s.PeekUInt32(e) == 1) {
				// CS2 has something odd I don't fully understand at the start of the file, try to skip past that
				s.Position += 0xe;
			}

			Entries = new List<t_voice_tbl_entry>();
			for (int i = 0; i < count; ++i) {
				Entries.Add(new t_voice_tbl_entry(s, e));
			}

			return;
		}

		public static int CheckVoiceTable(string pathTVoice, string pathWavdir, string outputPath) {
			List<ushort> knownMultipleMapped = new List<ushort>();
			List<List<t_voice_tbl_entry>> sameIndexMappedMultipleTimes = new List<List<t_voice_tbl_entry>>();
			List<t_voice_tbl_entry> regularMapped = new List<t_voice_tbl_entry>();
			List<t_voice_tbl_entry> mappedToNonexistentFile = new List<t_voice_tbl_entry>();

			var tbl = new t_voice_tbl(new DuplicatableFileStream(pathTVoice));
			Dictionary<ushort, t_voice_tbl_entry> byIndex = new Dictionary<ushort, t_voice_tbl_entry>();
			foreach (var e in tbl.Entries) {
				if (byIndex.ContainsKey(e.Index)) {
					if (!knownMultipleMapped.Contains(e.Index)) {
						knownMultipleMapped.Add(e.Index);
						List<t_voice_tbl_entry> tmp = new List<t_voice_tbl_entry>();
						foreach (var dup in tbl.Entries.Where(x => x.Index == e.Index)) {
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
		public ulong Unknown1;
		public ulong Unknown2;

		public t_voice_tbl_entry(Stream s, EndianUtils.Endianness e) {
			string magic = s.ReadAscii(6);
			if (magic != "voice\0") {
				throw new Exception("unexpected format");
			}

			ushort entrysize = s.ReadUInt16(e);
			Index = s.ReadUInt16(e);
			Name = s.ReadAscii(entrysize - 0x11);
			Unknown1 = s.ReadUInt56(EndianUtils.Endianness.BigEndian);
			Unknown2 = s.ReadUInt64(EndianUtils.Endianness.BigEndian);

			if (Unknown1 != 0 || Unknown2 != 0x1000000c842) {
				throw new Exception("unexpected format");
			}

			return;
		}

		public override string ToString() {
			return string.Format("{0}: {1} [{2:x14}{3:x16}]", Index, Name, Unknown1, Unknown2);
		}
	}
}
