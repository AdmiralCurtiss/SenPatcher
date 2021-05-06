using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class text_dat_us_t_voice_tbl : FileMod {
		public string GetDescription() {
			return "Fix incorrect voice clips (Alisa in Chapter 3, Sara in Finale).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var voicetable = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdc8fa92820abc1b4ul, 0x6a646b4d75ba5d23ul, 0x9bd22ee9u));
			var saraclip = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x0037f27d49910859ul, 0x38a613aae6000493ul, 0x68768874u));
			var alisaclip = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe9bd9a1d50cf6170ul, 0x728942e9fead6f2bul, 0xa635fcc6u));
			if (voicetable == null || saraclip == null || alisaclip == null) {
				return null;
			}

			var data = new VoiceTable(voicetable);
			data.Entries.Find(x => x.Index == 64300).Name = "pc8v10299";

			MemoryStream ms = new MemoryStream();
			data.WriteToStream(ms, EndianUtils.Endianness.LittleEndian);

			return new FileModResult[] {
				new FileModResult("data/text/dat_us/t_voice.tbl", ms),
				new FileModResult("data/voice/wav/pc8v10299.wav", saraclip),
				new FileModResult("data/voice/wav/pc8v02551.wav", alisaclip),
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var voicetable = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdc8fa92820abc1b4ul, 0x6a646b4d75ba5d23ul, 0x9bd22ee9u));
			var alisaclip = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x6d43ad75d01d9acdul, 0x887826db59961c3eul, 0x925ccc02u));
			if (voicetable == null || alisaclip == null) {
				return null;
			}

			return new FileModResult[] {
				new FileModResult("data/text/dat_us/t_voice.tbl", voicetable),
				new FileModResult("data/voice/wav/pc8v10299.wav", null),
				new FileModResult("data/voice/wav/pc8v02551.wav", alisaclip),
			};
		}
	}
}
