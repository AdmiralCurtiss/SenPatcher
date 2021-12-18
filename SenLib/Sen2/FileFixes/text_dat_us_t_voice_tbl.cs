using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	internal class VoiceData {
		public ushort idx;
		public string name;
		public ulong unknown1;
		public ushort unknown2;
		public uint unknown3;

		internal VoiceData(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			idx = stream.ReadUInt16();
			name = stream.ReadUTF8Nullterm();
			unknown1 = stream.ReadUInt64();
			unknown2 = stream.ReadUInt16();
			unknown3 = stream.ReadUInt32();
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUInt16(idx);
			ms.WriteUTF8Nullterm(name);
			ms.WriteUInt64(unknown1);
			ms.WriteUInt16(unknown2);
			ms.WriteUInt32(unknown3);
			return ms.CopyToByteArrayAndDispose();
		}
	}
	public class text_dat_us_t_voice_tbl : FileMod {
		public string GetDescription() {
			return "Adjust voice file to ID mappings (required for some voice fixes).";
		}

		public class VoiceReplaceInfo {
			public string Name;
			public ushort VoiceId;
			public int? ReplaceIndex;
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9019e12bf1d93039ul, 0x254892c87f70a572ul, 0xd2ffc248u));
			if (s == null) {
				return null;
			}
			var tbl = new Tbl(s, EndianUtils.Endianness.LittleEndian);

			// add unmapped entries into the voice table; replace a few that are defined twice while we're here too
			var list = new List<VoiceReplaceInfo>();
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00110", VoiceId = 65323, ReplaceIndex = 15736 });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00113", VoiceId = 65324, ReplaceIndex = 15737 });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00232", VoiceId = 65325, ReplaceIndex = 15738 });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00374", VoiceId = 65326, ReplaceIndex = 15740 });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00376", VoiceId = 65327, ReplaceIndex = 15741 });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00378", VoiceId = 65328, ReplaceIndex = 15742 });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00379", VoiceId = 65329, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00380", VoiceId = 65330, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00384", VoiceId = 65331, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00394", VoiceId = 65332, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00396", VoiceId = 65333, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00424", VoiceId = 65334, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00430", VoiceId = 65335, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00431", VoiceId = 65336, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00434", VoiceId = 65337, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00441", VoiceId = 65338, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00444", VoiceId = 65339, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00445", VoiceId = 65340, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00449", VoiceId = 65341, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00450", VoiceId = 65342, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v00705", VoiceId = 65343, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v010333", VoiceId = 65344, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v010334", VoiceId = 65345, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v010448", VoiceId = 65346, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v01055", VoiceId = 65347, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v01325", VoiceId = 65348, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v01644", VoiceId = 65349, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v01694", VoiceId = 65350, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v01796", VoiceId = 65351, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v01800", VoiceId = 65352, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v01966", VoiceId = 65353, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v01968", VoiceId = 65354, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v01969", VoiceId = 65355, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v01975", VoiceId = 65356, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v01981", VoiceId = 65357, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v020309", VoiceId = 65358, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v060407", VoiceId = 65359, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v080364", VoiceId = 65360, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v080401", VoiceId = 65361, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v090242", VoiceId = 65362, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v090265", VoiceId = 65363, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v100377", VoiceId = 65364, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v100378", VoiceId = 65365, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v110354", VoiceId = 65366, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v110357", VoiceId = 65367, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v250333", VoiceId = 65368, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v260310", VoiceId = 65369, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v90385", VoiceId = 65370, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v90403", VoiceId = 65371, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v950034", VoiceId = 65372, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v950114", VoiceId = 65373, ReplaceIndex = null });
			list.Add(new VoiceReplaceInfo() { Name = "pc28v950134", VoiceId = 65374, ReplaceIndex = null });
			foreach (var vri in list) {
				var entry = tbl.Entries[vri.ReplaceIndex == null ? 0 : vri.ReplaceIndex.Value];
				var item = new VoiceData(entry.Data);
				item.name = vri.Name;
				item.idx = vri.VoiceId;
				if (vri.ReplaceIndex == null) {
					tbl.Entries.Add(new TblEntry() { Name = entry.Name, Data = item.ToBinary() });
				} else {
					entry.Data = item.ToBinary();
				}
			}

			MemoryStream ms = new MemoryStream();
			tbl.WriteToStream(ms, EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_voice.tbl", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9019e12bf1d93039ul, 0x254892c87f70a572ul, 0xd2ffc248u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_voice.tbl", s) };
		}
	}
}
