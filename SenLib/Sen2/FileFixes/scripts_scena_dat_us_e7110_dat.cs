using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_e7110_dat : FileMod {
		public string GetDescription() {
			return "Voice clip fixes in Pantagruel main hall.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x65a894acd9f5ba6ful, 0x72a5c5f6f336c7cdul, 0xa60a4b9cu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// okay, so... there's a flashback in here to CS1 file e5010 here. this scene is NOT voiced in CS1,
			// however, Alisa's and Gaius's lines here do have unused voice clips in CS2 (pc28v010333, pc28v010334,
			// pc28v090242) which suggests that Crow's line should have one too, but I can't find it.
			// maybe they planned to have these but then Crow had no extra lines to record so they dropped it?
			// anyway, in case this clip exists, here's where to add it.
			// patcher.ExtendPartialCommand(0x53d7, 0xd5, _location_, _data_);
			// patcher.ExtendPartialCommand(0x54d1, 0x52, _location_, _data_);
			// patcher.ExtendPartialCommand(0x5547, 0xbd, _location_, _data_);

			// Rean has 6 different 'excuse me' lines in this file and PC only uses 3 audio clips for them even though
			// there are 6 different ones in the files, so use all of them.
			bin.Position = 0xa2ee;
			bin.Write(new byte[] { 0x3f, 0xff }); // pc28v01960 -> pc28v00705
			bin.Position = 0x11cf8;
			bin.Write(new byte[] { 0x4c, 0xff }); // pc28v00547 -> pc28v01975
			bin.Position = 0x18307;
			bin.Write(new byte[] { 0x4b, 0xff }); // pc28v01960 -> pc28v01969

			// Same deal with 'I can sense someone inside...', three lines and PC uses the same clip for all three
			// even though there are three takes in the files.
			bin.Position = 0x13d10;
			bin.Write(new byte[] { 0x4d, 0xff }); // pc28v01957 -> pc28v01981
			bin.Position = 0x180f8;
			bin.Write(new byte[] { 0x49, 0xff }); // pc28v01957 -> pc28v01966

			// And finally, two instances of 'That sounds like...'
			bin.Position = 0x182d1;
			bin.Write(new byte[] { 0x4a, 0xff }); // pc28v01959 -> pc28v01968

			// "..." -> "I..." to match voice clip.
			patcher.ExtendPartialCommand(0x1d784, 0x19, 0x1d798, new byte[] { 0x49 });


			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/e7110.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x65a894acd9f5ba6ful, 0x72a5c5f6f336c7cdul, 0xa60a4b9cu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/e7110.dat", file) };
		}
	}
}
