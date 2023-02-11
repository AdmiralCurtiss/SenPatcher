using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	class scripts_scena_dat_us_t5660_dat : FileMod {
		public string GetDescription() {
			return "Text/Voice matches in Garellia Fortress.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x78aa964124be90f4ul, 0xb9c5da90418a724cul, 0x0b46cbc3u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// voice clip matches (Chapter 5, scene when seeing the railway guns for the first time)
			// i think these happened when they re-recorded a few lines here for the PC version but only updated
			// the text in the Prologue variant and not the Chapter 5 variant
			patcher.RemovePartialCommand(0x3776, 0x19, 0x3780, 0x3);
			patcher.ReplacePartialCommand(0x4a05, 0x3e, 0x4a2e, 0x6, new byte[] {0x6e, 0x6e, 0x61});

			// remove 'Oh,' (not in voice clip, from PS4) (scene after Scarlet boss fight)
			bin.Position = 0xa871;
			byte[] oh = bin.ReadBytes(5);
			patcher.ReplacePartialCommand(0xa864, 0x24, 0xa871, 0x5, new byte[] {0x54});

			// add 'Oh,' (it's in voice clip, from PS4) (scene after Scarlet boss fight)
			patcher.ReplacePartialCommand(0xb4db, 0x1b, 0xb4e3, 0x1, oh);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t5660.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x78aa964124be90f4ul, 0xb9c5da90418a724cul, 0x0b46cbc3u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t5660.dat", file) };
		}
	}
}
