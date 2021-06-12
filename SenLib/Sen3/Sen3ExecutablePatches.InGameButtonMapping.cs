using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3 {
	public static partial class Sen3ExecutablePatches {
		public static void FixInGameButtonMappingValidity(Stream bin, Sen3ExecutablePatchState state) {
			bool jp = state.IsJp;
			EndianUtils.Endianness be = EndianUtils.Endianness.BigEndian;
			EndianUtils.Endianness le = EndianUtils.Endianness.LittleEndian;

			var region = new RegionHelper64(jp ? 0x140447157 : 0x140452f97, 0x44, "In-game Button Mapping Validity");
			bin.Position = state.Mapper.MapRamToRom(region.Address);
			for (uint i = 0; i < region.Remaining; ++i) {
				bin.WriteUInt8(0xcc); // int 3
			}

			using (var done = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(region.Address);
				done.SetTarget((ulong)(region.Address + region.Remaining));

				// input:
				//   ax as the button, same order as in the config menu (0-9)
				//   bx as the action, same order as in the config menu (0-12)
				//   rax, rcx, rdx may be clobbered
				// output:
				//   esi = 0 if option should be enabled, esi = 3 if option should be disabled

				bin.WriteUInt56(0x488d0d00000000, be); // lea rcx,[lookup_table]
				long lookup_table_inject_address = bin.Position;
				bin.WriteUInt32(0x480fb7c0, be);       // movzx rax,ax
				bin.WriteUInt32(0x488d0c41, be);       // lea rcx,[rcx+rax*2]
				bin.WriteUInt24(0x668b01, be);         // mov ax,word ptr[rcx]
				bin.WriteUInt24(0x668bcb, be);         // mov cx,bx
				bin.WriteUInt24(0x66d3e8, be);         // shr ax,cl
				bin.WriteUInt32(0x6683e001, be);       // and ax,1
				bin.WriteUInt24(0x668bc8, be);         // mov cx,ax
				bin.WriteUInt24(0x66d1e0, be);         // shl ax,1
				bin.WriteUInt24(0x660bc1, be);         // or ax,cx
				bin.WriteUInt24(0x0fb7f0, be);         // movzx esi,ax
				done.WriteJump(0xeb);                  // jmp done

				bin.WriteAlign(4, 0xcc);
				long lookup_table_address = bin.Position;
				bin.WriteUInt16(0x0c00, le); // bitmask for disallowed options for D-Pad Up
				bin.WriteUInt16(0x0c00, le); // bitmask for disallowed options for D-Pad Left
				bin.WriteUInt16(0x0c00, le); // bitmask for disallowed options for D-Pad Right
				bin.WriteUInt16(0x0c00, le); // bitmask for disallowed options for D-Pad L1
				bin.WriteUInt16(0x0c00, le); // bitmask for disallowed options for D-Pad R1
				bin.WriteUInt16(0x0c00, le); // bitmask for disallowed options for D-Pad R3
				bin.WriteUInt16(0x0c02, le); // bitmask for disallowed options for D-Pad Circle
				bin.WriteUInt16(0x0c00, le); // bitmask for disallowed options for D-Pad Select
				bin.WriteUInt16(0x03ff, le); // bitmask for disallowed options for D-Pad Select (Battle)
				bin.WriteUInt16(0x03ff, le); // bitmask for disallowed options for D-Pad Start (Battle)

				region.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Button Mapping fix");

				bin.Position = lookup_table_inject_address - 4;
				bin.WriteUInt32((uint)(lookup_table_address - lookup_table_inject_address), le);
			}
		}
	}
}
