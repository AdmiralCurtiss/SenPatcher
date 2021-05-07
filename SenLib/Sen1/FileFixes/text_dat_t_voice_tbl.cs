using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class text_dat_t_voice_tbl : FileMod {
		public string GetDescription() {
			return "Fix Japanese voice tables.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var voicetable = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x90a5c55ed954d771ul, 0x11563b9f3bb62ce7ul, 0xc534135au));
			if (voicetable == null) {
				return null;
			}

			// remove the new-for-PC english voice clips from the JP voice table
			var data = new VoiceTable(voicetable, EndianUtils.Endianness.LittleEndian);
			data.Entries.RemoveRange(9495, data.Entries.Count - 9495);
			MemoryStream newvoicetable = new MemoryStream();
			data.WriteToStream(newvoicetable, EndianUtils.Endianness.LittleEndian);

			return new FileModResult[] {
				new FileModResult("data/text/dat/t_voice.tbl", newvoicetable),
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var voicetable = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x90a5c55ed954d771ul, 0x11563b9f3bb62ce7ul, 0xc534135au));
			if (voicetable == null) {
				return null;
			}

			return new FileModResult[] {
				new FileModResult("data/text/dat/t_voice.tbl", voicetable),
			};
		}
	}
}
