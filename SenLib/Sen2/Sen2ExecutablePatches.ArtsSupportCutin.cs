using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static partial class Sen2ExecutablePatches {
		public static void PatchFixArtsSupportCutin(Stream bin, Sen2ExecutablePatchState state) {
			bool jp = state.IsJp;
			var be = EndianUtils.Endianness.BigEndian;

			// force link abilities to always trigger
			//bin.Position = state.Mapper.MapRamToRom(jp ? 0x62030a : 0x61fe5a);
			//bin.WriteUInt24(0xc6c001, EndianUtils.Endianness.BigEndian);

			long addressInjectPos = jp ? 0x560be7 : 0x560917;
			long flagOffInjectPos = jp ? 0x468091 : 0x468251;
			long flagOnInjectPos = jp ? 0x433650 : 0x433aa0;
			long texcoordInjectPos = jp ? 0x4a22a9 : 0x4a25b9;
			long addressFlag = state.AddressOfScriptCompilerFlag;

			// turn on flag when arts support starts
			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(flagOnInjectPos);
				byte[] instr = bin.ReadBytes(10);
				bin.Position = state.Mapper.MapRamToRom(flagOnInjectPos);
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				for (int i = 0; i < 5; ++i) {
					bin.WriteUInt8(0x90); // nop
				}
				jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				long newRegionStartRam = state.RegionScriptCompilerFunction23.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);

				bin.Write(instr);
				bin.WriteUInt16(0xc605, be);     // mov byte ptr[addressFlag],2
				bin.WriteInt32((int)(addressFlag));
				bin.WriteInt8(2);
				jumpBack.WriteJump5Byte(0xe9);   // jmp jumpBack

				state.RegionScriptCompilerFunction23.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Arts Support cutin fixes: Flag on");
			}

			// turn off flag during this function that seems to be called after all link actions
			// this may need further adjustments
			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(flagOffInjectPos);
				ulong instr = bin.PeekUInt40();
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				long newRegionStartRam = state.RegionScriptCompilerFunction23.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);

				bin.WriteUInt40(instr);
				bin.WriteUInt16(0xc605, be);     // mov byte ptr[addressFlag],0
				bin.WriteInt32((int)(addressFlag));
				bin.WriteInt8(0);
				jumpBack.WriteJump5Byte(0xe9);   // jmp jumpBack

				state.RegionScriptCompilerFunction23.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Arts Support cutin fixes: Flag off");
			}

			// fix texcoords when running at not 1280x720
			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper))
			using (var skip_modification = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(texcoordInjectPos);
				ulong instr = bin.PeekUInt48();
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				bin.WriteUInt8(0x90); // nop
				jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				long newRegionStartRam = state.Region50a.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);

				// eax, edx is free
				// xmm0, xmm1, xmm2 need to be fixed up
				bin.WriteUInt8(0xba);                  // mov edx,addressFlag
				bin.WriteInt32((int)(addressFlag));
				bin.WriteUInt16(0x8a02, be);           // mov al,byte ptr[edx]
				bin.WriteUInt16(0x3c02, be);           // cmp al,2
				skip_modification.WriteJump(0x75);     // jne skip_modification

				bin.WriteUInt24(0xc60201, be);         // mov byte ptr[edx],1
				bin.WriteUInt40(0xb8cdcc4c3d, be);     // mov eax,float(0.05)
				bin.WriteUInt32(0x660f6ec0, be);       // movd xmm0,eax
				bin.WriteUInt40(0xb80000803f, be);     // mov eax,float(1.0)
				bin.WriteUInt32(0x660f6ec8, be);       // movd xmm1,eax
				bin.WriteUInt40(0xb8cdcc4c3e, be);     // mov eax,float(0.2)
				bin.WriteUInt32(0x660f6ed0, be);       // movd xmm2,eax

				skip_modification.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				bin.WriteUInt48(instr);
				jumpBack.WriteJump5Byte(0xe9);   // jmp jumpBack

				state.Region50a.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Arts Support cutin fixes: Modify texcoords");
			}

			// modify matrix during arts support
			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper))
			using (var skip_modification = new BranchHelper1Byte(bin, state.Mapper)) {
				// reshuffle instructions so we have a cleaner injection point
				bin.Position = state.Mapper.MapRamToRom(addressInjectPos);
				uint instr1 = bin.ReadUInt32();
				ulong instr2 = bin.ReadUInt56();
				byte instr3 = bin.ReadUInt8();
				bin.Position = state.Mapper.MapRamToRom(addressInjectPos);
				bin.WriteUInt56(instr2);
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				long newRegionStartRam = state.RegionScriptCompilerFunction23.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);

				// ebx, ecx, edx, xmm0-2 is free
				// eax contains address to matrix
				bin.WriteUInt16(0x8a0d, be);           // mov cl,byte ptr[addressFlag]
				bin.WriteInt32((int)(addressFlag));
				bin.WriteUInt16(0x84c9, be);           // test cl,cl
				skip_modification.WriteJump(0x74);     // jz skip_modification

				// fix visibility
				bin.WriteUInt40(0xf30f10402c, be);     // movss xmm0,dword ptr[eax+2ch]
				bin.WriteUInt40(0xba0000503f, be);     // mov edx,float(0.8125)
				bin.WriteUInt32(0x660f6eca, be);       // movd xmm1,edx
				bin.WriteUInt32(0xf30f59c1, be);       // mulss xmm0,xmm1
				bin.WriteUInt40(0xf30f11402c, be);     // movss dword ptr[eax+2ch],xmm0

				// modify y position
				bin.WriteUInt40(0xf30f104024, be);     // movss xmm0,dword ptr[eax+24h]
				bin.WriteUInt40(0xbacdcc4c3e, be);     // mov edx,float(0.2)
				bin.WriteUInt32(0x660f6eca, be);       // movd xmm1,edx
				bin.WriteUInt32(0xf30f58c1, be);       // addss xmm0,xmm1
				bin.WriteUInt40(0xf30f114024, be);     // movss dword ptr[eax+24h],xmm0

				skip_modification.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				bin.WriteUInt32(instr1);
				bin.WriteUInt8(instr3);
				jumpBack.WriteJump5Byte(0xe9);   // jmp jumpBack

				state.RegionScriptCompilerFunction23.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Arts Support cutin fixes: Modify matrix");
			}
		}
	}
}
