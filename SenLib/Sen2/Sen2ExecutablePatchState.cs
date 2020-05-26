using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public class Sen2ExecutablePatchState {
		public RegionHelper Region50a = null;
		public RegionHelper Region50b = null;
		public RegionHelper Region60 = null;
		public RegionHelper Region51 = null;

		public void InitCodeSpaceIfNeeded(Stream binary) {
			if (Region50a != null) {
				// already initialized
				return;
			}

			var mapper = new Sen2Mapper();
			// make some free space to actually put our expanded code in
			// note that there's a few more functions around here that the same logic could be applied to for even more space if we need it
			// (though I think I found the biggest blocks already, hard to tell)
			using (var b = new BranchHelper4Byte(binary, mapper)) {
				// inject a jmp 0x4219cb, as that target has the same function tail as this one, so we can get us 0x50 bytes of free space
				long source = 0x420c4b;
				binary.Position = (long)mapper.MapRamToRom((ulong)source);
				b.SetTarget(0x4219cb);
				b.WriteJump5Byte(0xe9);
				Region50a = new RegionHelper((uint)mapper.MapRomToRam((ulong)binary.Position), 0x50);
				for (int i = 0; i < 0x50; ++i) {
					binary.WriteUInt8(0xcc);
				}
			}
			using (var b = new BranchHelper4Byte(binary, mapper)) {
				// same here
				long source = 0x4211fb;
				binary.Position = (long)mapper.MapRamToRom((ulong)source);
				b.SetTarget(0x4219cb);
				b.WriteJump5Byte(0xe9);
				Region50b = new RegionHelper((uint)mapper.MapRomToRam((ulong)binary.Position), 0x50);
				for (int i = 0; i < 0x50; ++i) {
					binary.WriteUInt8(0xcc);
				}
			}
			using (var b = new BranchHelper4Byte(binary, mapper)) {
				// same logic, different target, this gives us 0x60 bytes
				long source = 0x420d5b;
				binary.Position = (long)mapper.MapRamToRom((ulong)source);
				b.SetTarget(0x42160b);
				b.WriteJump5Byte(0xe9);
				Region60 = new RegionHelper((uint)mapper.MapRomToRam((ulong)binary.Position), 0x60);
				for (int i = 0; i < 0x60; ++i) {
					binary.WriteUInt8(0xcc);
				}
			}
			using (var b = new BranchHelper4Byte(binary, mapper)) {
				// and one more for 0x51 bytes
				long source = 0x42143a;
				binary.Position = (long)mapper.MapRamToRom((ulong)source);
				b.SetTarget(0x42161b);
				b.WriteJump5Byte(0xe9);
				Region51 = new RegionHelper((uint)mapper.MapRomToRam((ulong)binary.Position), 0x51);
				for (int i = 0; i < 0x51; ++i) {
					binary.WriteUInt8(0xcc);
				}
			}
		}
	}
}
