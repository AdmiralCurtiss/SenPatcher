using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public partial class Sen2ExecutablePatchState {
		// global u32 (or maybe u8?) that stores whether '-compile' was passed as a command line option
		// we reuse this flag as an indicator whether arts support is active so we should modify the view matrix for it
		public long AddressOfScriptCompilerFlag { get { return IsJp ? 0x10f2f88 : 0x10f7808; } }

		// string '-compile', no longer used, maybe repurpose
		public long AddressStartScriptCompilerCommandLineString { get { return IsJp ? 0x8f7b80 : 0x8fb244; } }
		public long AddressEndScriptCompilerCommandLineString { get { return IsJp ? 0x8f7b8c : 0x8fb250; } }

		// instructions that parse the '-compile' flag
		// this is interleaved with instructions for other stuff so we have to be a bit creative here...
		public long AddressStartOfScriptCompilerCliParsingCode { get { return IsJp ? 0x57c285 : 0x57c5f5; } }
		public long AddressEnd1OfScriptCompilerCliParsingCode { get { return IsJp ? 0x57c2a0 : 0x57c610; } }
		public long AddressEnd2OfScriptCompilerCliParsingCode { get { return IsJp ? 0x57c2af : 0x57c61f; } }

		// place where the flag is checked
		public long AddressStartOfScriptCompilerFlagUsageCode { get { return IsJp ? 0x554f77 : 0x554c07; } }
		public long AddressEndOfScriptCompilerFlagUsageCode { get { return IsJp ? 0x554f9d : 0x554c2d; } }

		// different place that calls the same function that is conditional on a parameter that is never passed as nonzero
		// since we're removing the function dummy this out just in case
		public long AddressStartOfCompilerFuncCallCode { get { return IsJp ? 0x500b96 : 0x500a46; } }
		public long AddressEndOfCompilerFuncCallCode { get { return IsJp ? 0x500bb0 : 0x500a60; } }

		// the function we're removing; this calls the script compiler as an external program
		public long AddressStartOfScriptCompilerFunction1 { get { return IsJp ? 0x5e0b60 : 0x5e0bb0; } }
		public long AddressEndOfScriptCompilerFunction1 { get { return IsJp ? 0x5e0d60 : 0x5e0db0; } }

		// two functions right next to eachother that are only called by the function above
		public long AddressStartOfScriptCompilerFunction23 { get { return IsJp ? 0x5e0e10 : 0x5e0e60; } }
		public long AddressEndOfScriptCompilerFunction23 { get { return IsJp ? 0x5e0f60 : 0x5e0fb0; } }

		public long AddressStartOfScriptCompilerFunctionStrings { get { return IsJp ? 0x8fb51c : 0x8ff0a4; } }
		public long AddressEndOfScriptCompilerFunctionStrings { get { return IsJp ? 0x8fb5a8 : 0x8ff130; } }

		// push address of version string displayed on the title screen
		public long PushAddressVersionString { get { return IsJp ? 0x62a74c : 0x62a2dc; } }

		// push address of version string displayed written to TTY on game boot
		public long PushAddressVersionStringTTY { get { return IsJp ? 0x578911 : 0x578c81; } }

		// push address of version string passed to the crash reporter
		public long PushAddressVersionCrashRpt { get { return IsJp ? 0x425e88 : 0x426098; } }

		public RegionHelper RegionScriptCompilerCommandLineString = null;
		public RegionHelper RegionScriptCompilerCliParsingCode = null;
		public RegionHelper RegionScriptCompilerFlagUsageCode = null;
		public RegionHelper RegionScriptCompilerFuncCallCode = null;
		public RegionHelper RegionScriptCompilerFunction1 = null;
		public RegionHelper RegionScriptCompilerFunction23 = null;
		public RegionHelper RegionScriptCompilerFunctionStrings = null;

		private void InitCodeSpaceScriptCompilerDummyIfNeeded(Stream bin) {
			if (RegionScriptCompilerCommandLineString != null) {
				return;
			}

			RegionScriptCompilerCommandLineString = SenUtils.InitRegion("ScriptCompilerCommandLineString", AddressStartScriptCompilerCommandLineString, AddressEndScriptCompilerCommandLineString, Mapper, bin, 0);
			{
				// deinterleave the parsing code
				bin.Position = Mapper.MapRamToRom(AddressEnd1OfScriptCompilerCliParsingCode);
				byte[] arr = bin.ReadUInt8Array(9);
				bin.Position = Mapper.MapRamToRom(AddressEnd2OfScriptCompilerCliParsingCode) - 9;
				bin.Write(arr);
				bin.Position = Mapper.MapRamToRom(AddressEnd2OfScriptCompilerCliParsingCode) + 4;
				bin.WriteByte(8);

				// then overwrite it
				RegionScriptCompilerCliParsingCode = SenUtils.InitRegion("ScriptCompilerCliParsingCode", AddressStartOfScriptCompilerCliParsingCode, AddressEnd2OfScriptCompilerCliParsingCode - 9, Mapper, bin, 0xcc);
				SenUtils.JumpOverCode(RegionScriptCompilerCliParsingCode, Mapper, bin);
			}
			RegionScriptCompilerFlagUsageCode = SenUtils.InitRegion("ScriptCompilerFlagUsage", AddressStartOfScriptCompilerFlagUsageCode, AddressEndOfScriptCompilerFlagUsageCode, Mapper, bin, 0xcc);
			SenUtils.JumpOverCode(RegionScriptCompilerFlagUsageCode, Mapper, bin);
			RegionScriptCompilerFuncCallCode = SenUtils.InitRegion("ScriptCompilerFuncCallCode", AddressStartOfCompilerFuncCallCode, AddressEndOfCompilerFuncCallCode, Mapper, bin, 0xcc);
			SenUtils.JumpOverCode(RegionScriptCompilerFuncCallCode, Mapper, bin);
			RegionScriptCompilerFunction1 = SenUtils.InitRegion("ScriptCompilerFunction1", AddressStartOfScriptCompilerFunction1, AddressEndOfScriptCompilerFunction1, Mapper, bin, 0xcc);
			RegionScriptCompilerFunction23 = SenUtils.InitRegion("ScriptCompilerFunction23", AddressStartOfScriptCompilerFunction23, AddressEndOfScriptCompilerFunction23, Mapper, bin, 0xcc);
			RegionScriptCompilerFunctionStrings = SenUtils.InitRegion("ScriptCompilerStrings", AddressStartOfScriptCompilerFunctionStrings, AddressEndOfScriptCompilerFunctionStrings, Mapper, bin, 0);
		}
	}
}
