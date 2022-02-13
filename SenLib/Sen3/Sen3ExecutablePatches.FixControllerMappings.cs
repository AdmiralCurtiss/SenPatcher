using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3 {
	public static partial class Sen3ExecutablePatches {
		public static void PatchFixControllerMappings(Stream bin, Sen3ExecutablePatchState state) {
			bool jp = state.IsJp;
			var be = EndianUtils.Endianness.BigEndian;

			long addressStructMemAlloc = (jp ? 0x14012dd59 : 0x1401312d9) + 2;
			long addressInjectPos = jp ? 0x14012d85c : 0x140130dde;
			long addressMapLookupCode = jp ? 0x14012e056 : 0x1401315f2;
			long lengthMapLookupCodeForDelete = jp ? 0x45 : 0x3f;
			long addressMapLookupSuccessForDelete = jp ? 0x14012e0a8 : 0x14013163e;
			long lengthMapLookupSuccessForDelete = jp ? 4 : 3;

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
			using (var lookup_fail = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(addressInjectPos);
				ulong overwrittenInstructions = bin.PeekUInt64(be);
				bin.WriteUInt8(0x90); // nop
				bin.WriteUInt8(0x90); // nop
				bin.WriteUInt8(0x90); // nop
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				jumpBack.SetTarget((ulong)(addressInjectPos + 8));

				long newRegionStartRam = state.RegionScriptCompilerFunction.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);

				// initialize the lookup table so every button points at itself
				bin.WriteUInt32(0x4d8d6500u | ((uint)allocLengthOld), be); // lea r12,[r13+allocLengthOld]
				bin.WriteUInt24(0x4d33c9, be);   // xor r9,r9
				begin_loop_1.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt32(0x4f8d3c0c, be); // lea r15,[r12+r9]
				bin.WriteUInt24(0x45880f, be);   // mov byte ptr[r15],r9b
				bin.WriteUInt24(0x49ffc1, be);   // inc r9
				bin.WriteUInt32(0x4983f920, be); // cmp r9,20h
				begin_loop_1.WriteJump(0x72);    // jb begin_loop_1

				// look up each key in the (presumably) std::map<int, int>
				// and write it into the lookup table in the other direction
				bin.WriteUInt24(0x4d33c9, be);   // xor r9,r9
				begin_loop_2.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt24(0x498bdd, be);   // mov rbx,r13
				bin.WriteUInt24(0x488b03, be);   // mov rax,qword ptr [rbx]
				bin.WriteUInt32(0x488b5010, be); // mov rdx,qword ptr [rax + 0x10]
				bin.WriteUInt24(0x488bc2, be);   // mov rax,rdx
				bin.WriteUInt32(0x488b4a08, be); // mov rcx,qword ptr [rdx + 0x8]
				bin.WriteUInt32(0x80791900, be); // cmp byte ptr [rcx + 0x19],0x0
				lookup_4.WriteJump(0x75);        // jnz lookup_4
				lookup_1.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt32(0x4439491c, be); // cmp dword ptr [rcx + 0x1c],r9d
				lookup_2.WriteJump(0x7d);        // jge lookup_2
				bin.WriteUInt32(0x488b4910, be); // mov rcx,qword ptr [rcx + 0x10]
				lookup_3.WriteJump(0xeb);        // jmp lookup_3
				lookup_2.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt24(0x488bc1, be);   // mov rax,rcx
				bin.WriteUInt24(0x488b09, be);   // mov rcx,qword ptr [rcx]
				lookup_3.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt32(0x80791900, be); // cmp byte ptr [rcx + 0x19],0x0
				lookup_1.WriteJump(0x74);        // jz  lookup_1
				bin.WriteUInt24(0x483bc2, be);   // cmp rax,rdx
				lookup_4.WriteJump(0x74);        // jz  lookup_4
				bin.WriteUInt32(0x443b481c, be); // cmp r9d,dword ptr [rax + 0x1c]
				lookup_5.WriteJump(0x7d);        // jge lookup_5
				lookup_4.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt24(0x488bc2, be);   // mov rax,rdx
				lookup_5.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt24(0x483bc2, be);   // cmp rax,rdx
				bin.WriteUInt24(0x418bc9, be);   // mov ecx,r9d
				lookup_fail.WriteJump(0x74);     // jz  lookup_fail
				bin.WriteUInt24(0x8b4820, be);   // mov ecx,dword ptr[rax + 20h]
				lookup_fail.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				// r9d now contains the UNMAPPED value
				// ecx now contains the MAPPED value
				bin.WriteUInt32(0x45880c0c, be); // mov byte ptr[r12+rcx],r9b
				bin.WriteUInt24(0x49ffc1, be);   // inc r9
				bin.WriteUInt32(0x4983f920, be); // cmp r9,20h
				begin_loop_2.WriteJump(0x72);    // jb begin_loop_2

				bin.WriteUInt64(overwrittenInstructions, be);
				bin.WriteUInt24(0x498bc5, be);   // mov rax,r13 ; restore clobbered rax
				jumpBack.WriteJump5Byte(0xe9);   // jmp jumpBack

				state.RegionScriptCompilerFunction.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Fix controller button mappings: Init");
			}

			// clear out old lookup code
			bin.Position = state.Mapper.MapRamToRom(addressMapLookupCode);
			for (long i = 0; i < lengthMapLookupCodeForDelete; ++i) {
				bin.WriteUInt8(0x90); // nop
			}
			bin.Position = state.Mapper.MapRamToRom(addressMapLookupSuccessForDelete);
			for (long i = 0; i < lengthMapLookupSuccessForDelete; ++i) {
				bin.WriteUInt8(0x90); // nop
			}

			// replace with new lookup code
			using (var lookup_success = new BranchHelper1Byte(bin, state.Mapper))
			using (var lookup_fail = new BranchHelper1Byte(bin, state.Mapper)) {
				// on success: result should be in ecx (on US) or eax (on JP)
				// on fail: result doesn't matter, restores itself
				lookup_success.SetTarget((ulong)(addressMapLookupSuccessForDelete + lengthMapLookupSuccessForDelete));
				lookup_fail.SetTarget((ulong)(addressMapLookupCode + lengthMapLookupCodeForDelete));

				bin.Position = state.Mapper.MapRamToRom(addressMapLookupCode);
				if (!jp) {
					bin.WriteUInt24(0x418bc1, be);     // mov eax,r9d
				}
				bin.WriteUInt24(0x83f820, be);         // cmp eax,20h
				lookup_fail.WriteJump(0x73);           // jnb lookup_fail
				if (!jp) {
					bin.WriteUInt40(0x0fb64c0300u | ((ulong)allocLengthOld), be); // movzx ecx,byte ptr[rbx+rax+allocLengthOld]
					bin.WriteUInt24(0x83f920, be);                                // cmp ecx,20h
				} else {
					bin.WriteUInt40(0x0fb6440300u | ((ulong)allocLengthOld), be); // movzx eax,byte ptr[rbx+rax+allocLengthOld]
					bin.WriteUInt24(0x83f820, be);                                // cmp eax,20h
				}
				lookup_fail.WriteJump(0x73);           // jnb lookup_fail
				lookup_success.WriteJump(0xeb);        // jmp lookup_success
			}

			if (jp) {
				// swap actions 4/5 on controller config readin so we get identical mapping behavior between JP/US builds
				using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
				using (var replacedCall = new BranchHelper4Byte(bin, state.Mapper))
				using (var check5 = new BranchHelper1Byte(bin, state.Mapper))
				using (var checkdone = new BranchHelper1Byte(bin, state.Mapper))
				using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper)) {
					long addr = 0x14012d75d;
					bin.Position = state.Mapper.MapRamToRom(addr + 1);
					long calladdr = bin.ReadInt32() + addr + 5;
					replacedCall.SetTarget((ulong)calladdr);

					bin.Position = state.Mapper.MapRamToRom(addr);
					jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
					jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

					long newRegionStartRam = state.RegionScriptCompilerFunction.Address;
					long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
					bin.Position = newRegionStartRom;
					jumpToNewCode.SetTarget((ulong)newRegionStartRam);

					replacedCall.WriteJump5Byte(0xe8);   // call (replaced function call)
					bin.WriteUInt40(0x488d4c2450, be);   // lea rcx,[rsp+50h]
					bin.WriteUInt16(0x8b11, be);         // mov edx,dword ptr[rcx]
					bin.WriteUInt24(0x83fa04, be);       // cmp edx,4
					check5.WriteJump(0x75);              // jne check5
					bin.WriteUInt48(0xc70105000000, be); // mov dword ptr[rcx],5
					bin.WriteUInt16(0xeb0b, be);         // jmp checkdone
					check5.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
					bin.WriteUInt24(0x83fa05, be);       // cmp edx,5
					checkdone.WriteJump(0x75);           // jne checkdone
					bin.WriteUInt48(0xc70104000000, be); // mov dword ptr[rcx],4
					checkdone.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
					jumpBack.WriteJump5Byte(0xe9);       // jmp jumpBack

					state.RegionScriptCompilerFunction.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Fix controller button mappings: Swap O/X");
				}
			}
		}
	}
}
