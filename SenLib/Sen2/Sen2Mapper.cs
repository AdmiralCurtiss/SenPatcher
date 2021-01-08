using HyoutaPluginBase;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public class Sen2Mapper : IRomMapper {
		private struct Region {
			public ulong RomStart;
			public ulong RamStart;
			public ulong Length;
		}

		private Region[] Regions;

		public Sen2Mapper(bool jp) {
			if (jp) {
				Regions = new Region[] {
					new Region() { RomStart = 0x00000000, RamStart = 0x00400000, Length = 0x00000400 },
					new Region() { RomStart = 0x00000400, RamStart = 0x00401000, Length = 0x004e7200 },
					new Region() { RomStart = 0x004e7600, RamStart = 0x008e9000, Length = 0x000ccc00 },
					new Region() { RomStart = 0x005b4200, RamStart = 0x009b6000, Length = 0x0003ce00 },
					new Region() { RomStart = 0x005f1000, RamStart = 0x01175000, Length = 0x0002a800 },
				};
			} else {
				Regions = new Region[] {
					new Region() { RomStart = 0x00000000, RamStart = 0x00400000, Length = 0x00000400 },
					new Region() { RomStart = 0x00000400, RamStart = 0x00401000, Length = 0x004e8400 },
					new Region() { RomStart = 0x004e8800, RamStart = 0x008ea000, Length = 0x000cfe00 },
					new Region() { RomStart = 0x005b8600, RamStart = 0x009ba000, Length = 0x0003ce00 },
					new Region() { RomStart = 0x005f5400, RamStart = 0x0117a000, Length = 0x0002a800 },
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
