using HyoutaPluginBase;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public class Sen1Mapper : IRomMapper {
		private struct Region {
			public ulong RomStart;
			public ulong RamStart;
			public ulong Length;
		}

		private Region[] Regions;

		public Sen1Mapper(bool jp) {
			if (jp) {
				Regions = new Region[] {
					new Region() { RomStart = 0x00000000, RamStart = 0x00400000, Length = 0x00000400 },
					new Region() { RomStart = 0x00000400, RamStart = 0x00401000, Length = 0x00734e00 },
					new Region() { RomStart = 0x00735200, RamStart = 0x00b36000, Length = 0x00104800 },
					new Region() { RomStart = 0x00839a00, RamStart = 0x00c3b000, Length = 0x0003a600 },
					new Region() { RomStart = 0x00874000, RamStart = 0x01367000, Length = 0x00003c00 },
					new Region() { RomStart = 0x00877c00, RamStart = 0x0136b000, Length = 0x00001400 },
				};
			} else {
				Regions = new Region[] {
					new Region() { RomStart = 0x00000000, RamStart = 0x00400000, Length = 0x00000400 },
					new Region() { RomStart = 0x00000400, RamStart = 0x00401000, Length = 0x00736a00 },
					new Region() { RomStart = 0x00736e00, RamStart = 0x00b38000, Length = 0x00105000 },
					new Region() { RomStart = 0x0083be00, RamStart = 0x00c3d000, Length = 0x0003a600 },
					new Region() { RomStart = 0x00876400, RamStart = 0x01369000, Length = 0x00003c00 },
					new Region() { RomStart = 0x0087a000, RamStart = 0x0136d000, Length = 0x00001400 },
				};
			}
		}

		public bool TryMapRamToRom(ulong ramAddress, out ulong value) {
			foreach (Region r in Regions) {
				if (r.RamStart <= ramAddress && ramAddress < (r.RamStart + r.Length)) {
					value = ramAddress - (r.RamStart - r.RomStart);
					return true;
				}
			}
			value = 0;
			return false;
		}

		public bool TryMapRomToRam(ulong romAddress, out ulong value) {
			foreach (Region r in Regions) {
				if (r.RomStart <= romAddress && romAddress < (r.RomStart + r.Length)) {
					value = romAddress + (r.RamStart - r.RomStart);
					return true;
				}
			}
			value = 0;
			return false;
		}
	}
}
