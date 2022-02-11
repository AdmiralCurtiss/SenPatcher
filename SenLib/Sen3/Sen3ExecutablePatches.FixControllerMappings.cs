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

			if (jp) {
				// JP version has different register allocation in these functions, need to adjust more stuff...
				// seems to also have the circle/cross remapping issues CS2 has, based on the button prompts that show up
				return;
			}

			long addressStructMemAlloc = (jp ? 0x14012dd59 : 0x1401312d9) + 2;
			long addressInjectPos = jp ? 0x14012d85c : 0x140130dde;
			long addressMapLookupCode = jp ? 0x14012e056 : 0x1401315f2;
			long lengthMapLookupCodeForCopy = jp ? 0x43 : 0x3d;
			long lengthMapLookupCodeForDelete = lengthMapLookupCodeForCopy + 2;
			long addressMapLookupSuccessForDelete = jp ? 0x14012e0a8 : 0x14013163e;
			long lengthMapLookupSuccessForDelete = jp ? 4 : 3;

			// grab the map lookup code
			bin.Position = state.Mapper.MapRamToRom(addressMapLookupCode);
			byte[] lookupCode = bin.ReadBytes(lengthMapLookupCodeForCopy);

			// increase struct allocation by 0x20 bytes
			bin.Position = state.Mapper.MapRamToRom(addressStructMemAlloc);
			byte allocLengthOld = bin.PeekUInt8();
			byte allocLengthNew = (byte)(allocLengthOld + 0x20);
			bin.WriteUInt8(allocLengthNew);

			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper))
			using (var begin_loop_1 = new BranchHelper1Byte(bin, state.Mapper))
			using (var begin_loop_2 = new BranchHelper1Byte(bin, state.Mapper))
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

				bin.WriteUInt24(0x4d33c9, be);   // xor r9,r9
				begin_loop_2.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt24(0x498bdd, be);   // mov rbx,r13
				bin.Write(lookupCode);           // performs most of a lookup for r9d in rbx
				bin.WriteUInt24(0x418bc9, be);   // mov ecx,r9d
				lookup_fail.WriteJump(0x74);     // jz lookup_fail
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
				// on success: result should be in ecx
				// on fail: result doesn't matter, restores itself
				lookup_success.SetTarget((ulong)(addressMapLookupSuccessForDelete + lengthMapLookupSuccessForDelete));
				lookup_fail.SetTarget((ulong)(addressMapLookupCode + lengthMapLookupCodeForDelete));

				bin.Position = state.Mapper.MapRamToRom(addressMapLookupCode);
				bin.WriteUInt24(0x418bc1, be);     // mov eax,r9d
				bin.WriteUInt24(0x83f820, be);     // cmp eax,20h
				lookup_fail.WriteJump(0x73);       // jnb lookup_fail
				bin.WriteUInt40(0x0fb64c0300u | ((ulong)allocLengthOld), be); // movzx ecx,byte ptr[rbx+rax+allocLengthOld]
				bin.WriteUInt24(0x83f920, be);     // cmp ecx,20h
				lookup_fail.WriteJump(0x73);       // jnb lookup_fail
				lookup_success.WriteJump(0xeb);    // jmp lookup_success
			}
		}
	}
}
