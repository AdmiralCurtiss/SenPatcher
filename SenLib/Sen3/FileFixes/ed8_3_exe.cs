using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen3.FileFixes {
	public class ed8_3_exe : FileMod {
		bool IsJp;
		bool FixInGameButtonMappingValidity;
		bool AllowSwitchToNightmare;

		public ed8_3_exe(
			bool jp,
			bool fixInGameButtonMappingValidity,
			bool allowSwitchToNightmare
		) {
			IsJp = jp;
			FixInGameButtonMappingValidity = fixInGameButtonMappingValidity;
			AllowSwitchToNightmare = allowSwitchToNightmare;
		}

		public string GetDescription() {
			return string.Format("Main executable code changes for {0} language.", IsJp ? "Japanese" : "English");
		}

		private HyoutaUtils.Checksum.SHA1 GetExecutableHash() {
			return IsJp ? new HyoutaUtils.Checksum.SHA1(0x2e9da161efc30b9aul, 0xf020903e99cfa9a2ul, 0x287ff1a2u) : new HyoutaUtils.Checksum.SHA1(0x72920fa16072f4b7ul, 0x51e6a5be3fb4df50ul, 0xb21da506u);
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(GetExecutableHash());
			if (s == null) {
				return null;
			}

			var mapper = new PeExe(s, EndianUtils.Endianness.LittleEndian).CreateRomMapper();
			MemoryStream ms = s.CopyToMemoryAndDispose();

			Sen3ExecutablePatchState PatchInfo = new Sen3ExecutablePatchState(IsJp, mapper);

			if (FixInGameButtonMappingValidity) {
				Sen3ExecutablePatches.FixInGameButtonMappingValidity(ms, PatchInfo);
			}
			if (AllowSwitchToNightmare) {
				Sen3ExecutablePatches.AllowSwitchToNightmare(ms, PatchInfo);
			}

			return new FileModResult[] { new FileModResult(IsJp ? "bin/x64/ed8_3_PC_JP.exe" : "bin/x64/ed8_3_PC.exe", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(GetExecutableHash());
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult(IsJp ? "bin/x64/ed8_3_PC_JP.exe" : "bin/x64/ed8_3_PC.exe", s) };
		}
	}
}
