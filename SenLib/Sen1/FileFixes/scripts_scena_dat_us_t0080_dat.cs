﻿using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0080_dat : FileMod {
		public string GetDescription() {
			return "Fix double space in Student Union building (chess club).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd5805f2f25de668aul, 0x4ececc8f6cad0aaaul, 0xe64a3cf8u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x1f278, 0x88, 0x1f2df, 1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0080.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd5805f2f25de668aul, 0x4ececc8f6cad0aaaul, 0xe64a3cf8u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0080.dat", s) };
		}
	}
}
