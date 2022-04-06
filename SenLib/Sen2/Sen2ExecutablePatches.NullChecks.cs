using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static partial class Sen2ExecutablePatches {
		public static void PatchAddNullCheckBattleScopeCrashMaybe(Stream bin, Sen2ExecutablePatchState state) {
			bool jp = state.IsJp;
			var be = EndianUtils.Endianness.BigEndian;

			// Multiple reported crashes at this address while using a Battle Scope.
			// Not sure if this actually fixes anything, but let's try to add a null check here in case it does.
			uint checkAddress = jp ? 0x52e53eu : 0x52e1ceu;

			// These constructs in the area also deref this pointer, so check those too just in case...
			uint checkAddress2 = jp ? 0x52e500u : 0x52e190u;
			uint checkAddress3 = jp ? 0x52e513u : 0x52e1a3u;

			long exitFunction;
			{
				var region = state.RegionForBattleScopeNullCheckAddress;

				bin.Position = state.Mapper.MapRamToRom(checkAddress);
				uint overwrittenInstruction1 = bin.ReadUInt24(be); // this is the instruction that crashes when eax == 0
				ushort overwrittenInstruction2 = bin.ReadUInt16(be); // this should be executed regardless
				long continueTarget = state.Mapper.MapRomToRam(bin.Position);
				bin.ReadUInt8();
				sbyte jumpOffset = bin.ReadInt8();
				exitFunction = state.Mapper.MapRomToRam(bin.Position) + jumpOffset;

				using (BranchHelper4Byte jmp_to_code = new BranchHelper4Byte(bin, state.Mapper))
				using (BranchHelper4Byte jmp_back = new BranchHelper4Byte(bin, state.Mapper))
				using (BranchHelper1Byte skip_deref = new BranchHelper1Byte(bin, state.Mapper)) {
					jmp_back.SetTarget((ulong)continueTarget);
					jmp_to_code.SetTarget(region.Address);

					bin.Position = state.Mapper.MapRamToRom(checkAddress);
					jmp_to_code.WriteJump5Byte(0xe9); // jmp to our code

					bin.Position = state.Mapper.MapRamToRom(region.Address);
					bin.WriteUInt16(0x85c0, be);   // test eax,eax
					skip_deref.WriteJump(0x74);    // jz skip_deref
					bin.WriteUInt24(overwrittenInstruction1, be);
					skip_deref.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
					bin.WriteUInt16(overwrittenInstruction2, be);
					jmp_back.WriteJump5Byte(0xe9); // jmp back
					region.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Battle Scope crash: Null check 1");

					// why this works: by skipping overwrittenInstruction1, the zero flag keeps the state of the test eax,eax
					// this causes the next jge in the original code to return the function early, without us having to explicitly do that
				}
			}

			{
				var region = state.Region50a;

				bin.Position = state.Mapper.MapRamToRom(checkAddress2);
				uint overwrittenInstruction2a = bin.ReadUInt24(be);
				uint overwrittenInstruction2b = bin.ReadUInt24(be);
				ushort overwrittenInstruction2c = bin.ReadUInt16(be);
				bin.Position -= 3;
				long continueTarget2 = state.Mapper.MapRomToRam(bin.Position);
				bin.WriteUInt24(overwrittenInstruction2b, be); // swap instructions here to save a byte
				bin.Position = state.Mapper.MapRamToRom(checkAddress3);
				uint overwrittenInstruction3a = bin.ReadUInt24(be);
				ushort overwrittenInstruction3b = bin.ReadUInt16(be);
				long continueTarget3 = state.Mapper.MapRomToRam(bin.Position);

				using (BranchHelper4Byte jmp_to_code_2 = new BranchHelper4Byte(bin, state.Mapper))
				using (BranchHelper4Byte jmp_back_2 = new BranchHelper4Byte(bin, state.Mapper))
				using (BranchHelper4Byte jmp_to_code_3 = new BranchHelper4Byte(bin, state.Mapper))
				using (BranchHelper4Byte jmp_back_3 = new BranchHelper4Byte(bin, state.Mapper))
				using (BranchHelper4Byte exit_function = new BranchHelper4Byte(bin, state.Mapper))
				using (BranchHelper1Byte skip_deref = new BranchHelper1Byte(bin, state.Mapper)) {
					jmp_back_2.SetTarget((ulong)continueTarget2);
					jmp_back_3.SetTarget((ulong)continueTarget3);
					exit_function.SetTarget((ulong)exitFunction);

					bin.Position = state.Mapper.MapRamToRom(checkAddress2);
					jmp_to_code_2.WriteJump5Byte(0xe9); // jmp to our code
					bin.Position = state.Mapper.MapRamToRom(checkAddress3);
					jmp_to_code_3.WriteJump5Byte(0xe9); // jmp to our code

					bin.Position = state.Mapper.MapRamToRom(region.Address);
					jmp_to_code_2.SetTarget(region.Address);
					bin.WriteUInt16(0x85c0, be);     // test eax,eax
					skip_deref.WriteJump(0x74);      // jz skip_deref
					bin.WriteUInt24(overwrittenInstruction2a, be);
					bin.WriteUInt16(overwrittenInstruction2c, be);
					jmp_back_2.WriteJump5Byte(0xe9); // jmp back

					jmp_to_code_3.SetTarget((ulong)state.Mapper.MapRomToRam(bin.Position));
					bin.WriteUInt16(0x85c0, be);     // test eax,eax
					skip_deref.WriteJump(0x74);      // jz skip_deref
					bin.WriteUInt24(overwrittenInstruction3a, be);
					bin.WriteUInt16(overwrittenInstruction3b, be);
					jmp_back_3.WriteJump5Byte(0xe9); // jmp back

					skip_deref.SetTarget((ulong)state.Mapper.MapRomToRam(bin.Position));
					exit_function.WriteJump5Byte(0xe9); // jmp back

					region.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Battle Scope crash: Null check 2");
				}
			}

			// After the above patches and manually forcing the relevant pointer to 0, this function crashed, so let's patch this too.
			// tbqh I don't think this is the right approach, but I don't know what actually causes the situation in the first place, so...
			uint checkAddress4 = jp ? 0x52b037u : 0x52acc7u;

			{
				var region = state.Region50a;

				bin.Position = state.Mapper.MapRamToRom(checkAddress4);
				ushort overwrittenInstruction4a = bin.ReadUInt16(be);
				uint overwrittenInstruction4b = bin.ReadUInt24(be);
				long continueTarget4 = state.Mapper.MapRomToRam(bin.Position);
				bin.Position += 9;
				int jumpOffset = bin.ReadInt32();
				long exitFunction4 = state.Mapper.MapRomToRam(bin.Position) + jumpOffset;

				using (BranchHelper4Byte jmp_to_code = new BranchHelper4Byte(bin, state.Mapper))
				using (BranchHelper4Byte jmp_back = new BranchHelper4Byte(bin, state.Mapper))
				using (BranchHelper4Byte exit_function = new BranchHelper4Byte(bin, state.Mapper)) {
					jmp_back.SetTarget((ulong)continueTarget4);
					exit_function.SetTarget((ulong)exitFunction4);

					bin.Position = state.Mapper.MapRamToRom(checkAddress4);
					jmp_to_code.WriteJump5Byte(0xe9); // jmp to our code

					bin.Position = state.Mapper.MapRamToRom(region.Address);
					jmp_to_code.SetTarget(region.Address);
					bin.WriteUInt16(overwrittenInstruction4a, be);
					bin.WriteUInt24(overwrittenInstruction4b, be);
					bin.WriteUInt16(0x83be, be);     // cmp dword ptr[esi+03ech],0
					bin.WriteUInt32(0x3ec);
					bin.WriteUInt8(0);
					exit_function.WriteJump6Byte(0x0f84); // je exit_function
					jmp_back.WriteJump5Byte(0xe9);        // jmp back

					region.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Battle Scope crash: Null check 3");
				}
			}
		}
	}
}
