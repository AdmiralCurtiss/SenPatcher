using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c3210_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Heimdallr (Racecourse).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x34c651df94044c96ul, 0xf600a24ba99eafa2ul, 0xf04d08d1u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Now, we shall begin the award ceremony on{n}behalf of the Imperial Household Agency.	-> Now, we shall present the awards on{n}behalf of City Hall.
			bin.Position = 0x22d9b;
			List<byte> command = bin.ReadBytes(0xb1).ToList();
			List<byte> newcommand = new List<byte>();
			newcommand.AddRange(command.Take(0x1a));
			newcommand.AddRange(new byte[] { 0x70, 0x72, 0x65, 0x73, 0x65, 0x6e, 0x74 });
			newcommand.AddRange(command.Skip(0x1f).Take(0xa));
			newcommand.Add(0x73);
			newcommand.AddRange(command.Skip(0x32).Take(0xe));
			newcommand.AddRange(new byte[] { 0x43, 0x69, 0x74, 0x79, 0x20, 0x48, 0x61, 0x6c, 0x6c });
			newcommand.AddRange(command.Skip(0x5d));
			patcher.ReplaceCommand(0x22d9b, 0xb1, newcommand.ToArray());

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c3210.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x34c651df94044c96ul, 0xf600a24ba99eafa2ul, 0xf04d08d1u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c3210.dat", file) };
		}
	}
}
