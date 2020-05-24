using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static partial class Sen2ExecutablePatches {
		public static void PatchMusicFadeTiming(Stream binary) {
			// TODO: needs to be done for JP as well
			var mapper = new Sen2Mapper();

			RegionHelper region50a = null;
			RegionHelper region50b = null;
			RegionHelper region60 = null;
			RegionHelper region51 = null;
			RegionHelper regionEntryPoint = null;
			RegionHelper region80 = null;

			using (BranchHelper4Byte entryPoint = new BranchHelper4Byte(binary, mapper)) {
				// make some free space to actually put our expanded code in
				// note that there's a few more functions around here that the same logic could be applied to for even more space if we need it
				// (though I think I found the biggest blocks already, hard to tell)
				using (var b = new BranchHelper4Byte(binary, mapper)) {
					// inject a jmp 0x4219cb, as that target has the same function tail as this one, so we can get us 0x50 bytes of free space
					long source = 0x420c4b;
					binary.Position = (long)mapper.MapRamToRom((ulong)source);
					b.SetTarget(0x4219cb);
					b.WriteJump5Byte(0xe9);
					for (int i = 0; i < 0x50; ++i) {
						binary.WriteUInt8(0xcc);
					}
					region50a = new RegionHelper((uint)source, 0x50);
				}
				using (var b = new BranchHelper4Byte(binary, mapper)) {
					// same here
					long source = 0x4211fb;
					binary.Position = (long)mapper.MapRamToRom((ulong)source);
					b.SetTarget(0x4219cb);
					b.WriteJump5Byte(0xe9);
					for (int i = 0; i < 0x50; ++i) {
						binary.WriteUInt8(0xcc);
					}
					region50b = new RegionHelper((uint)source, 0x50);
				}
				using (var b = new BranchHelper4Byte(binary, mapper)) {
					// same logic, different target, this gives us 0x60 bytes
					long source = 0x420d5b;
					binary.Position = (long)mapper.MapRamToRom((ulong)source);
					b.SetTarget(0x42160b);
					b.WriteJump5Byte(0xe9);
					for (int i = 0; i < 0x60; ++i) {
						binary.WriteUInt8(0xcc);
					}
					region60 = new RegionHelper((uint)source, 0x60);
				}
				using (var b = new BranchHelper4Byte(binary, mapper)) {
					// and one more for 0x51 bytes
					long source = 0x42143a;
					binary.Position = (long)mapper.MapRamToRom((ulong)source);
					b.SetTarget(0x42161b);
					b.WriteJump5Byte(0xe9);
					for (int i = 0; i < 0x51; ++i) {
						binary.WriteUInt8(0xcc);
					}
					region51 = new RegionHelper((uint)source, 0x51);
				}
				{
					// this is the entry point of the function we're replacing, clear it out and just stick a jmp there to the new code
					binary.Position = (long)mapper.MapRamToRom((ulong)0x41dc30);
					entryPoint.WriteJump5Byte(0xe9);
					for (int i = 5; i < 0x10; ++i) {
						binary.WriteUInt8(0xcc);
					}
					regionEntryPoint = new RegionHelper(0x41dc30, 0x10);
				}
				{
					// this is the body of the function we're replacing, clear it out
					// don't ask me why this is split up like this
					binary.Position = (long)mapper.MapRamToRom((ulong)0x421f10);
					for (int i = 0; i < 0x80; ++i) {
						binary.WriteUInt8(0xcc);
					}
					region80 = new RegionHelper(0x421f10, 0x80);
				}


			}
		}
	}
}
