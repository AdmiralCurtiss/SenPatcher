using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public static partial class Sen1ExecutablePatches {
		public static void PatchLanguageAppropriateVoiceTables(Stream binary, Sen1ExecutablePatchState state) {
			state.InitCodeSpaceIfNeeded(binary);

			var mapper = state.Mapper;
			var regionStrings = state.RegionScriptCompilerFunctionStrings;
			var regionCode = state.RegionScriptCompilerFunction;
			bool jp = state.IsJp;

			using (BranchHelper4Byte jump_from_function = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte back_to_function = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte get_pc_config = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte pc_config__get_voice_language = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper1Byte depends_on_voice_lang = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper1Byte exit_inject = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper1Byte use_english_string = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper1Byte use_text_lang_string = new BranchHelper1Byte(binary, mapper)) {
				EndianUtils.Endianness be = EndianUtils.Endianness.BigEndian;
				EndianUtils.Endianness le = EndianUtils.Endianness.LittleEndian;
				Stream _ = binary;

				get_pc_config.SetTarget(jp ? 0x7bdef0u : 0x7bf7c0u);
				pc_config__get_voice_language.SetTarget(jp ? 0x7bd3f0u : 0x7becc0u);

				// inject into the asset-from-text-subfolder loader function
				long address_of_inject = jp ? 0x56f6d8 : 0x5708d8;
				_.Position = mapper.MapRamToRom(address_of_inject);
				ulong push_local_formatting_string = _.PeekUInt40(be);
				uint address_local_formatting_string = ((uint)(push_local_formatting_string & 0xffffffffu)).SwapEndian();
				jump_from_function.SetTarget(regionCode.Address);
				jump_from_function.WriteJump5Byte(0xe9);
				back_to_function.SetTarget(mapper.MapRomToRam((ulong)_.Position));
				ulong push_en_formatting_string;
				ulong push_jp_formatting_string;
				uint t_vctiming_address;
				uint t_voice_address;
				if (jp) {
					push_jp_formatting_string = push_local_formatting_string;
					t_vctiming_address = 0xb41c68;
					t_voice_address = 0xb41bdc;

					// english formatting string doesn't exist in the japanese executable, generate and inject it
					byte[] formatting_string_jp = _.ReadBytesFromLocationAndReset(mapper.MapRamToRom(address_local_formatting_string), 0x13);
					byte[] extra = _.ReadBytesFromLocationAndReset(0x73fd32, 0x3);
					byte[] formatting_string_en = new byte[formatting_string_jp.Length + 3];
					ArrayUtils.CopyByteArrayPart(formatting_string_jp, 0, formatting_string_en, 0, 0xb);
					ArrayUtils.CopyByteArrayPart(extra, 0, formatting_string_en, 0xb, 3);
					ArrayUtils.CopyByteArrayPart(formatting_string_jp, 0xb, formatting_string_en, 0xe, 0x8);

					uint nonlocal_string_address = regionStrings.Address;
					_.Position = mapper.MapRamToRom(regionStrings.Address);
					_.Write(formatting_string_en);
					regionStrings.TakeToAddress(mapper.MapRomToRam(_.Position), "Voice Tables: EN Formatting String");

					push_en_formatting_string = 0x6800000000 | (ulong)(nonlocal_string_address.SwapEndian());
				} else {
					push_en_formatting_string = push_local_formatting_string;
					t_vctiming_address = 0xb43f30;
					t_voice_address = 0xb43ea4;

					// japanese formatting string doesn't exist in the english executable, generate and inject it
					byte[] formatting_string_en = _.ReadBytesFromLocationAndReset(mapper.MapRamToRom(address_local_formatting_string), 0x16);
					byte[] formatting_string_jp = new byte[formatting_string_en.Length - 3];
					ArrayUtils.CopyByteArrayPart(formatting_string_en, 0, formatting_string_jp, 0, 0xb);
					ArrayUtils.CopyByteArrayPart(formatting_string_en, 0xe, formatting_string_jp, 0xb, 0x8);

					uint nonlocal_string_address = regionStrings.Address;
					_.Position = mapper.MapRamToRom(regionStrings.Address);
					_.Write(formatting_string_jp);
					regionStrings.TakeToAddress(mapper.MapRomToRam(_.Position), "Voice Tables: JP Formatting String");

					push_jp_formatting_string = 0x6800000000 | (ulong)(nonlocal_string_address.SwapEndian());
				}

				// assemble logic to select the voice-language-matching voice tables
				_.Position = mapper.MapRamToRom(regionCode.Address);
				_.WriteUInt16(0x81ff, be);                             // cmp  edi,(t_vctiming)
				_.WriteUInt32(t_vctiming_address, le);
				depends_on_voice_lang.WriteJump(0x74);                 // je   depends_on_voice_lang
				_.WriteUInt16(0x81ff, be);                             // cmp  edi,(t_voice)
				_.WriteUInt32(t_voice_address, le);
				depends_on_voice_lang.WriteJump(0x74);                 // je   depends_on_voice_lang
				use_text_lang_string.WriteJump(0xeb);                  // jmp  use_text_lang_string

				depends_on_voice_lang.SetTarget(mapper.MapRomToRam((ulong)_.Position));
				get_pc_config.WriteJump5Byte(0xe8);                    // call get_pc_config
				_.WriteUInt16(0x8bc8, be);                             // mov  ecx,eax
				pc_config__get_voice_language.WriteJump5Byte(0xe8);    // call pc_config__get_voice_language
				_.WriteUInt16(0x84c0, be);                             // test al,al
				use_english_string.WriteJump(0x74);                    // jz   use_english_string
				if (jp) {
					use_text_lang_string.SetTarget(mapper.MapRomToRam((ulong)_.Position));
				}
				_.WriteUInt40(push_jp_formatting_string, be);          // push (jp_formatting_string)
				exit_inject.WriteJump(0xeb);                           // jmp  exit_inject

				use_english_string.SetTarget(mapper.MapRomToRam((ulong)_.Position));
				if (!jp) {
					use_text_lang_string.SetTarget(mapper.MapRomToRam((ulong)_.Position));
				}
				_.WriteUInt40(push_en_formatting_string, be);          // push (en_formatting_string)

				exit_inject.SetTarget(mapper.MapRomToRam((ulong)_.Position));
				back_to_function.WriteJump5Byte(0xe9);                 // jmp  back_to_function

				regionCode.TakeToAddress(mapper.MapRomToRam(_.Position), "Voice Tables: Formatting String Select Code");
			}
		}
	}
}
