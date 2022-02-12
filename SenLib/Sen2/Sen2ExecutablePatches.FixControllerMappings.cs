using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static partial class Sen2ExecutablePatches {
		public static void PatchFixControllerMappings(Stream bin, Sen2ExecutablePatchState state) {
			bool jp = state.IsJp;
			var be = EndianUtils.Endianness.BigEndian;

			long addressStructMemAlloc = (jp ? 0x6ac97c : 0x6ad9ac) + 1;
			long addressInjectPos = jp ? 0x6ac02f : 0x6acf8f;
			long addressMapLookupCode = jp ? 0x6ad70b : 0x6ae73b;
			long lengthMapLookupCodeForDelete = 0x36;
			long addressMapLookupPops = addressMapLookupCode + (0x6ae76d - 0x6ae73b);
			long addressMapLookupSuccessPush = addressMapLookupCode + (0x6ae77a - 0x6ae73b);
			long addressMapLookupFailMov = addressMapLookupCode + (0x6ae771 - 0x6ae73b);

			// increase struct allocation by 0x20 bytes
			bin.Position = state.Mapper.MapRamToRom(addressStructMemAlloc);
			byte allocLengthOld = bin.PeekUInt8();
			byte allocLengthNew = (byte)(allocLengthOld + 0x20);
			bin.WriteUInt8(allocLengthNew);

			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper))
			using (var begin_loop_1 = new BranchHelper1Byte(bin, state.Mapper))
			using (var begin_loop_2 = new BranchHelper1Byte(bin, state.Mapper))
			using (var lookup_1 = new BranchHelper1Byte(bin, state.Mapper))
			using (var lookup_2 = new BranchHelper1Byte(bin, state.Mapper))
			using (var lookup_3 = new BranchHelper1Byte(bin, state.Mapper))
			using (var lookup_4 = new BranchHelper1Byte(bin, state.Mapper))
			using (var lookup_5 = new BranchHelper1Byte(bin, state.Mapper))
			using (var lookup_6 = new BranchHelper1Byte(bin, state.Mapper))
			using (var lookup_fail = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(addressInjectPos);
				ulong overwrittenInstructions = bin.PeekUInt40(be);
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				jumpBack.SetTarget((ulong)(addressInjectPos + 5));

				long newRegionStartRam = state.RegionScriptCompilerFunction23.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);

				// initialize the lookup table so every button points at itself
				bin.WriteUInt24(0x8b75c8, be);       // mov esi,dword ptr[ebp-38h]
				bin.WriteUInt24(0x8d7e04, be);       // lea edi,[esi+4h]
				bin.WriteUInt16(0x33c0, be);         // xor eax,eax
				begin_loop_1.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt24(0x8d0c07, be);       // lea ecx,[edi+eax]
				bin.WriteUInt16(0x8801, be);         // mov byte ptr[ecx],al
				bin.WriteUInt8(0x40);                // inc eax
				bin.WriteUInt24(0x83f820, be);       // cmp eax,20h
				begin_loop_1.WriteJump(0x72);        // jb begin_loop_1

				// look up each key in the (presumably) std::map<int, int>
				// and write it into the lookup table in the other direction
				bin.WriteUInt16(0x33c0, be);         // xor eax,eax
				bin.WriteUInt16(0x8b36, be);         // mov esi,dword ptr[esi]
				bin.WriteUInt24(0x8b7608, be);       // mov esi,dword ptr[esi+8h]
				begin_loop_2.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt24(0x8b4e04, be);       // mov ecx,dword ptr[esi+04h]
				bin.WriteUInt32(0x80790d00, be);     // cmp byte ptr[ecx+0dh],0
				bin.WriteUInt16(0x8bd6, be);         // mov edx,esi
				lookup_4.WriteJump(0x75);            // jnz lookup_4
				lookup_1.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt24(0x394110, be);       // cmp dword ptr[ecx+10h],eax
				lookup_2.WriteJump(0x7d);            // jge lookup_2
				bin.WriteUInt24(0x8b4908, be);       // mov ecx,dword ptr[ecx+08h]
				lookup_3.WriteJump(0xeb);            // jmp lookup_3
				lookup_2.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt16(0x8bd1, be);         // mov edx,ecx
				bin.WriteUInt16(0x8b09, be);         // mov ecx,dword ptr[ecx]
				lookup_3.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt32(0x80790d00, be);     // cmp byte ptr[ecx+0dh],0
				lookup_1.WriteJump(0x74);            // jz  lookup_1
				lookup_4.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt16(0x3bd6, be);         // cmp edx,esi
				lookup_5.WriteJump(0x74);            // jz  lookup_5
				bin.WriteUInt16(0x8bca, be);         // mov ecx,edx
				bin.WriteUInt24(0x3b4210, be);       // cmp eax,dword ptr[edx+10h]
				lookup_6.WriteJump(0x7d);            // jge lookup_6
				lookup_5.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt16(0x8bce, be);         // mov ecx,esi
				lookup_6.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt16(0x8bd1, be);         // mov edx,ecx
				bin.WriteUInt16(0x3bd6, be);         // cmp edx,esi
				bin.WriteUInt16(0x8bc8, be);         // mov ecx,eax
				lookup_fail.WriteJump(0x74);         // jz  lookup_fail
				bin.WriteUInt24(0x8b4a14, be);       // mov ecx,dword ptr[edx+14h]
				lookup_fail.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt24(0x88040f, be);       // mov byte ptr[edi+ecx],al
				bin.WriteUInt8(0x40);                // inc eax
				bin.WriteUInt24(0x83f820, be);       // cmp eax,20h
				begin_loop_2.WriteJump(0x72);        // jb begin_loop_2


				bin.WriteUInt40(overwrittenInstructions, be);
				jumpBack.WriteJump5Byte(0xe9);   // jmp jumpBack

				state.RegionScriptCompilerFunction23.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Fix controller button mappings: Init");
			}

			// two pops need to be executed either way so prepare that
			bin.Position = state.Mapper.MapRamToRom(addressMapLookupPops);
			ushort pops = bin.ReadUInt16(be);
			bin.Position = state.Mapper.MapRamToRom(addressMapLookupFailMov - 2);
			bin.WriteUInt16(pops, be);
			bin.Position = state.Mapper.MapRamToRom(addressMapLookupSuccessPush);
			bin.WriteUInt16(pops, be);
			bin.WriteUInt8(0x51); // push ecx

			// clear out old lookup code
			bin.Position = state.Mapper.MapRamToRom(addressMapLookupCode);
			for (long i = 0; i < lengthMapLookupCodeForDelete; ++i) {
				bin.WriteUInt8(0x90); // nop
			}

			// replace with new lookup code
			using (var lookup_success = new BranchHelper1Byte(bin, state.Mapper))
			using (var lookup_fail = new BranchHelper1Byte(bin, state.Mapper)) {
				// input is in edx
				// on success: result should be in ecx
				// on fail: result doesn't matter, restores itself
				lookup_success.SetTarget((ulong)(addressMapLookupSuccessPush));
				lookup_fail.SetTarget((ulong)(addressMapLookupFailMov - 2));

				bin.Position = state.Mapper.MapRamToRom(addressMapLookupCode);
				bin.WriteUInt24(0x83fa20, be);     // cmp edx,20h
				lookup_fail.WriteJump(0x73);       // jnb lookup_fail
				bin.WriteUInt24(0x8b4dfc, be);     // mov ecx,dword ptr[ebp-4h]
				bin.WriteUInt40(0x0fb64c1104, be); // movzx ecx,byte ptr[ecx+edx+4h]
				bin.WriteUInt24(0x83f920, be);     // cmp ecx,20h
				lookup_fail.WriteJump(0x73);       // jnb lookup_fail
				lookup_success.WriteJump(0xeb);    // jmp lookup_success
			}

			if (jp) {
				// swap the config <-> game mapping for circle and cross to match the English executable
				bin.SwapBytes(state.Mapper.MapRamToRom(0x6ac8b2 + 1), state.Mapper.MapRamToRom(0x6ac8ab + 1));
				bin.SwapBytes(state.Mapper.MapRamToRom(0x6ac6ee + 1), state.Mapper.MapRamToRom(0x6ac6f5 + 1));

				// swap actions 4/5 on mouse config and controller config readin
				foreach (long addr in new long[] { 0x6abdba, 0x6abfd5 }) {
					using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
					using (var replacedCall = new BranchHelper4Byte(bin, state.Mapper))
					using (var check5 = new BranchHelper1Byte(bin, state.Mapper))
					using (var checkdone = new BranchHelper1Byte(bin, state.Mapper))
					using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper)) {
						bin.Position = state.Mapper.MapRamToRom(addr + 1);
						long calladdr = bin.ReadInt32() + addr + 5;
						replacedCall.SetTarget((ulong)calladdr);

						bin.Position = state.Mapper.MapRamToRom(addr);
						jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
						jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

						long newRegionStartRam = state.RegionScriptCompilerFunction23.Address;
						long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
						bin.Position = newRegionStartRom;
						jumpToNewCode.SetTarget((ulong)newRegionStartRam);

						replacedCall.WriteJump5Byte(0xe8);   // call (replaced function call)
						bin.WriteUInt24(0x8d4dcc, be);       // lea ecx,[ebp-34h]
						bin.WriteUInt16(0x8b11, be);         // mov edx,dword ptr[ecx]
						bin.WriteUInt24(0x83fa04, be);       // cmp edx,4
						check5.WriteJump(0x75);              // jne check5
						bin.WriteUInt48(0xc70105000000, be); // mov dword ptr[ecx],5
						bin.WriteUInt16(0xeb0b, be);         // jmp checkdone
						check5.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
						bin.WriteUInt24(0x83fa05, be);       // cmp edx,5
						checkdone.WriteJump(0x75);           // jne checkdone
						bin.WriteUInt48(0xc70104000000, be); // mov dword ptr[ecx],4
						checkdone.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
						jumpBack.WriteJump5Byte(0xe9);       // jmp jumpBack

						state.RegionScriptCompilerFunction23.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Fix controller button mappings: Swap O/X at 0x" + addr.ToString("X8"));
					}
				}

				// this is the function that handles the button prompt (re)mapping
				//bin.Position = state.Mapper.MapRamToRom(0x6964D5);
				//bin.WriteUInt8(0xcc);

				// this inits the struct responsible for telling which button to use for the textbox text advance prompts
				// the JP version incorrectly uses the cancel button instead of the confirm button, so swap that around
				bin.Position = state.Mapper.MapRamToRom(0x699777);
				bin.WriteUInt8(4);
			}
		}
	}
}
