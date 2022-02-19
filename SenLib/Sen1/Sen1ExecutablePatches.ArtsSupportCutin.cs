using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public static partial class Sen1ExecutablePatches {
		public static void PatchFixArtsSupportCutin(Stream bin, Sen1ExecutablePatchState state) {
			bool jp = state.IsJp;
			var be = EndianUtils.Endianness.BigEndian;

			// force link abilities to always trigger
			//bin.Position = state.Mapper.MapRamToRom(jp ? 0x686907 : 0x688407);
			//bin.WriteUInt24(0xc6c001, EndianUtils.Endianness.BigEndian);

			long addressInjectPos = jp ? 0x5d15ad : 0x5d277d;
			long flagOffInjectPos = jp ? 0x4b4f5a : 0x4b67aa;
			long flagOnInjectPos = jp ? 0x490565 : 0x491db5;
			long texcoordInjectPos1 = jp ? 0x504e69 : 0x506599;
			long texcoordInjectPos2 = jp ? 0x504ff3 : 0x506723;
			long configStructPos = jp ? 0x1302a30 : 0x1304dd0;
			long addressFlag = state.AddressOfScriptCompilerFlag;

			// turn on flag when arts support starts
			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(flagOnInjectPos);
				ulong instr = bin.PeekUInt40();
				bin.Position = state.Mapper.MapRamToRom(flagOnInjectPos);
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				long newRegionStartRam = state.RegionScriptCompilerFunction.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);

				bin.WriteUInt40(instr);
				bin.WriteUInt16(0xc605, be);     // mov byte ptr[addressFlag],2
				bin.WriteInt32((int)(addressFlag));
				bin.WriteInt8(2);
				jumpBack.WriteJump5Byte(0xe9);   // jmp jumpBack

				state.RegionScriptCompilerFunction.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Arts Support cutin fixes: Flag on");
			}

			// turn off flag during this function that seems to be called after all link actions
			// this may need further adjustments
			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(flagOffInjectPos);
				ulong instr = bin.PeekUInt40();
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				long newRegionStartRam = state.RegionScriptCompilerFunction.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);

				bin.WriteUInt40(instr);
				bin.WriteUInt16(0xc605, be);     // mov byte ptr[addressFlag],0
				bin.WriteInt32((int)(addressFlag));
				bin.WriteInt8(0);
				jumpBack.WriteJump5Byte(0xe9);   // jmp jumpBack

				state.RegionScriptCompilerFunction.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Arts Support cutin fixes: Flag off");
			}

			// fix texcoords when running at not 1280x720
			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper))
			using (var skip_modification = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(texcoordInjectPos1);
				ulong instr = bin.PeekUInt40();
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				long newRegionStartRam = state.RegionScriptCompilerFunction.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);

				// eax, edx okay to use
				bin.WriteUInt8(0xba);                  // mov edx,addressFlag
				bin.WriteInt32((int)(addressFlag));
				bin.WriteUInt16(0x8a02, be);           // mov al,byte ptr[edx]
				bin.WriteUInt16(0x3c02, be);           // cmp al,2
				skip_modification.WriteJump(0x75);     // jne skip_modification

				bin.WriteUInt40(0xb80000a044, be);     // mov eax,float(1280.0)
				bin.WriteUInt48(0x898554ffffff, be);   // mov dword ptr[ebp-0ach],eax
				bin.WriteUInt40(0xb800003444, be);     // mov eax,float(720.0)
				bin.WriteUInt48(0x898558ffffff, be);   // mov dword ptr[ebp-0a8h],eax

				skip_modification.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				bin.WriteUInt40(instr);
				jumpBack.WriteJump5Byte(0xe9);   // jmp jumpBack

				state.RegionScriptCompilerFunction.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Arts Support cutin fixes: Modify texcoords (1)");
			}

			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper))
			using (var skip_modification = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(texcoordInjectPos2);
				uint instr1 = bin.ReadUInt24();
				byte instr2 = bin.ReadUInt8();
				ushort instr3 = bin.ReadUInt16();
				bin.Position = state.Mapper.MapRamToRom(texcoordInjectPos2);
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				bin.WriteUInt8(0x90);
				jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				long newRegionStartRam = state.RegionScriptCompilerFunction.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);
				bin.WriteUInt24(instr1);
				bin.WriteUInt8(instr2);
				bin.WriteUInt16(instr3);

				// eax, edx okay to use
				bin.WriteUInt8(0xba);                  // mov edx,addressFlag
				bin.WriteInt32((int)(addressFlag));
				bin.WriteUInt16(0x8a02, be);           // mov al,byte ptr[edx]
				bin.WriteUInt16(0x3c02, be);           // cmp al,2
				skip_modification.WriteJump(0x75);     // jne skip_modification

				bin.WriteUInt24(0xc60201, be);         // mov byte ptr[edx],1

				bin.WriteUInt8(0xba);                  // mov edx,configStructPos
				bin.WriteInt32((int)(configStructPos));
				bin.WriteUInt16(0x8b12, be);           // mov edx,dword ptr[edx]
				bin.WriteUInt16(0x85d2, be);           // test edx,edx
				skip_modification.WriteJump(0x74);     // jz skip_modification

				bin.WriteUInt56(0xc745d80000a044, be); // mov dword ptr[ebp-0ach+84h],44a00000h
				bin.WriteUInt24(0xd945d8, be);         // fld dword ptr[ebp-0ach+84h]
				bin.WriteUInt24(0xdb4214, be);         // fild dword ptr[edx+14h]
				bin.WriteUInt16(0xdec9, be);           // fmulp ; 1280*height
				bin.WriteUInt24(0xd94588, be);         // fld dword ptr[ebp-0ach+34h]
				bin.WriteUInt24(0xdb4210, be);         // fild dword ptr[edx+10h]
				bin.WriteUInt16(0xdec9, be);           // fmulp	; 720*width
				bin.WriteUInt16(0xdef9, be);           // fdivp ; (1280*height)/(720*width) = multiplication factor for texcoords
				bin.WriteUInt16(0xd9c0, be);           // fld st(0)
				bin.WriteUInt48(0xd9856cffffff, be);   // fld dword ptr[ebp-0ach+18h]
				bin.WriteUInt16(0xdec9, be);           // fmulp
				bin.WriteUInt48(0xd99d6cffffff, be);   // fstp dword ptr[ebp-0ach+18h]
				bin.WriteUInt24(0xd94590, be);         // fld dword ptr[ebp-0ach+3ch]
				bin.WriteUInt16(0xdec9, be);           // fmulp
				bin.WriteUInt24(0xd95590, be);         // fst dword ptr[ebp-0ach+3ch]
				bin.WriteUInt24(0xd95dd8, be);         // fstp dword ptr[ebp-0ach+84h]

				skip_modification.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				jumpBack.WriteJump5Byte(0xe9);   // jmp jumpBack

				state.RegionScriptCompilerFunction.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Arts Support cutin fixes: Modify texcoords (2)");
			}

			// modify camera x when aspect ratio != 16:9
			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper))
			using (var skip_modification = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(addressInjectPos);
				ulong instr = bin.PeekUInt48();
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				bin.WriteUInt8(0x90); // nop
				jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				long newRegionStartRam = state.RegionScriptCompilerFunction.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);

				// ecx, edx is free
				bin.WriteUInt16(0x8a0d, be);             // mov cl,byte ptr[addressFlag]
				bin.WriteInt32((int)(addressFlag));
				bin.WriteUInt16(0x84c9, be);             // test cl,cl
				skip_modification.WriteJump(0x74);       // jz skip_modification

				bin.WriteUInt8(0xba);                    // mov edx,configStructPos
				bin.WriteInt32((int)(configStructPos));
				bin.WriteUInt16(0x8b12, be);             // mov edx,dword ptr[edx]
				bin.WriteUInt16(0x85d2, be);             // test edx,edx
				skip_modification.WriteJump(0x74);       // jz skip_modification

				bin.WriteUInt24(0xdb4210, be);           // fild dword ptr[edx+10h]
				bin.WriteUInt24(0xdb4214, be);           // fild dword ptr[edx+14h]
				bin.WriteUInt16(0xdef9, be);             // fdivp ; width/height

				bin.WriteUInt64(0xc74424fc398ee33f, be); // mov dword ptr[esp-4h],float(16/9)
				bin.WriteUInt32(0xd94424fc, be);         // fld dword ptr[esp-4h]
				bin.WriteUInt16(0xdee9, be);             // fsubp
				bin.WriteUInt64(0xc74424fcc1ca01bf, be); // mov dword ptr[esp-4h],float(-0.507)
				bin.WriteUInt32(0xd94424fc, be);         // fld dword ptr[esp-4h]
				bin.WriteUInt16(0xdec9, be);             // fmulp

				bin.WriteUInt48(0xd9857cfeffff, be);     // fld dword ptr[ebp-1a4h+20h]
				bin.WriteUInt16(0xdec1, be);             // faddp
				bin.WriteUInt48(0xd99d7cfeffff, be);     // fstp dword ptr[ebp-1a4h+20h]

				skip_modification.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));

				bin.WriteUInt48(instr);
				jumpBack.WriteJump5Byte(0xe9);   // jmp jumpBack

				state.RegionScriptCompilerFunction.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Arts Support cutin fixes: Modify matrix");
			}
		}
	}
}
