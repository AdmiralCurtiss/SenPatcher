using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class text_dat_t_voice_tbl : FileMod {
		public string GetDescription() {
			return "Fix Japanese voice tables. This is half of a fix for JP voice lipsync.";
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

			// the PS3 and PS4 versions disagree slightly on some lines, I took the PS3 timings here but maybe the PS4 ones are better?
			var newvoicetiming3 = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x60e82f9eb05d4b5dul, 0x15fbff5e3b341b0cul, 0xabfcae9du));
			//var newvoicetiming4 = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf94e4188145f004cul, 0x86ae6fa0b6ff8a1bul, 0x818b4562u));
			if (newvoicetiming3 == null) {
				return null;
			}

			return new FileModResult[] {
				new FileModResult("data/text/dat/t_voice.tbl", newvoicetable),
				new FileModResult("data/text/dat/t_vctiming.tbl", newvoicetiming3),
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var voicetable = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x90a5c55ed954d771ul, 0x11563b9f3bb62ce7ul, 0xc534135au));
			var voicetiming = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf4b9ff78474452aaul, 0xc44f4b0c07c5a3ccul, 0x1ce27359u));
			if (voicetable == null || voicetiming == null) {
				return null;
			}

			return new FileModResult[] {
				new FileModResult("data/text/dat/t_voice.tbl", voicetable),
				new FileModResult("data/text/dat/t_vctiming.tbl", voicetiming),
			};
		}
	}
}
