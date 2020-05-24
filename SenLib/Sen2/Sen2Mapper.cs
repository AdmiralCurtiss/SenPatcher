using HyoutaPluginBase;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public class Sen2Mapper : IRomMapper {
		public bool TryMapRamToRom(ulong ramAddress, out ulong value) {
			value = ramAddress - 0x400c00;
			return true;
		}

		public bool TryMapRomToRam(ulong romAddress, out ulong value) {
			value = romAddress + 0x400c00;
			return true;
		}
	}
}
