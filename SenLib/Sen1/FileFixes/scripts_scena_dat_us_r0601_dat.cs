using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_r0601_dat : FileMod {
		public string GetDescription() {
			return "Fix text issues in 1st scene of bike events.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x531ae02b784b6530ul, 0xf4dc08676a793c89ul, 0xf9ebae68u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);

			// missing period
			patcher.ExtendPartialCommand(0x1fdb, 0x5b, 0x2034, new byte[] { 0x2e });

			// missing word
			patcher.ExtendPartialCommand(0x3962, 0x42, 0x3995, new byte[] { 0x62, 0x65, 0x20 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r0601.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x531ae02b784b6530ul, 0xf4dc08676a793c89ul, 0xf9ebae68u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r0601.dat", s) };
		}
	}
}
