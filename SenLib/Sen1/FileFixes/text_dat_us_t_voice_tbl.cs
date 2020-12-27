using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class text_dat_us_t_voice_tbl : FileMod {
		public string GetDescription() {
			return "Fix incorrect voice clip from Sara in Finale.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var voicetable = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdc8fa92820abc1b4ul, 0x6a646b4d75ba5d23ul, 0x9bd22ee9u));
			var saraclip = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x0037f27d49910859ul, 0x38a613aae6000493ul, 0x68768874u));
			if (voicetable == null || saraclip == null) {
				return null;
			}

			var data = new VoiceTable(voicetable);
			data.Entries.Find(x => x.Index == 64300).Name = "pc8v10299";

			MemoryStream ms = new MemoryStream();
			data.WriteToStream(ms, EndianUtils.Endianness.LittleEndian);

			return new FileModResult[] {
				new FileModResult("data/text/dat_us/t_voice.tbl", ms),
				new FileModResult("data/voice/wav/pc8v10299.wav", saraclip),
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var voicetable = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdc8fa92820abc1b4ul, 0x6a646b4d75ba5d23ul, 0x9bd22ee9u));
			if (voicetable == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_us/t_voice.tbl", voicetable),
				new FileModResult("data/voice/wav/pc8v10299.wav", null),
			};
		}
	}
}
