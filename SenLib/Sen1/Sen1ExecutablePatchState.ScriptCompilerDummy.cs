using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public partial class Sen1ExecutablePatchState {
		// global u32 (or maybe u8?) that stores whether '-compile' was passed as a command line option
		// we remove this flag so we could reuse this for something else, but right now we don't
		public long AddressOfScriptCompilerFlag { get; private set; }

		// string '-compile', no longer used, maybe repurpose; 8 bytes, 1 nullterminator, then 3 bytes padding
		public long AddressStartScriptCompilerCommandLineString { get; private set; }
		public long AddressEndScriptCompilerCommandLineString { get; private set; }

		// instructions that parse the '-compile' flag
		public long AddressStartOfScriptCompilerCliParsingCode { get; private set; }
		public long AddressEndOfScriptCompilerCliParsingCode { get; private set; }

		// instructions of one of the two uses of the flag
		public long AddressStartOfScriptCompilerFlagUsage1 { get; private set; }
		public long AddressEndOfScriptCompilerFlagUsage1 { get; private set; }

		// instructions of the other of the two uses of the flag
		public long AddressStartOfScriptCompilerFlagUsage2 { get; private set; }
		public long AddressEndOfScriptCompilerFlagUsage2 { get; private set; }

		// the actual script compiler function, we can reuse all of this space for new code
		public long AddressThunkScriptCompilerFunction { get; private set; }
		public long AddressStartScriptCompilerFunction { get; private set; }
		public long AddressEndScriptCompilerFunction { get; private set; }

		// string used by the script compiler function, can be repurposed
		public long AddressStartOfScriptCompilerFunctionStrings { get; private set; }
		public long AddressEndOfScriptCompilerFunctionStrings { get; private set; }

		private void CtorCodeSpaceScriptCompilerDummy(bool jp) {
			AddressOfScriptCompilerFlag = jp ? 0x12e8958 : 0x12ead00;
			AddressStartScriptCompilerCommandLineString = jp ? 0xb44bd8 : 0xb47028;
			AddressEndScriptCompilerCommandLineString = jp ? 0xb44be4 : 0xb47034;
			AddressStartOfScriptCompilerCliParsingCode = jp ? 0x5de1aa : 0x5df38a;
			AddressEndOfScriptCompilerCliParsingCode = jp ? 0x5de1c6 : 0x5df3a6;
			AddressStartOfScriptCompilerFlagUsage1 = jp ? 0x56f6b2 : 0x5708b2;
			AddressEndOfScriptCompilerFlagUsage1 = jp ? 0x56f6c3 : 0x5708c3;
			AddressStartOfScriptCompilerFlagUsage2 = jp ? 0x5a7203 : 0x5a8373;
			AddressEndOfScriptCompilerFlagUsage2 = jp ? 0x5a7229 : 0x5a8399;
			AddressThunkScriptCompilerFunction = jp ? 0x40dd3c : 0x40dd5a;
			AddressStartScriptCompilerFunction = jp ? 0x56e240 : 0x56f470;
			AddressEndScriptCompilerFunction = jp ? 0x56e3b0 : 0x56f5e0;
			AddressStartOfScriptCompilerFunctionStrings = jp ? 0xb41b24 : 0xb43de8;
			AddressEndOfScriptCompilerFunctionStrings = jp ? 0xb41b8c : 0xb43e54;
		}

		public RegionHelper RegionScriptCompilerCommandLineString = null;
		public RegionHelper RegionScriptCompilerCliParsingCode = null;
		public RegionHelper RegionScriptCompilerFlagUsage1 = null;
		public RegionHelper RegionScriptCompilerFlagUsage2 = null;
		public RegionHelper RegionScriptCompilerFunction = null;
		public RegionHelper RegionScriptCompilerFunctionStrings = null;

		private void InitCodeSpaceScriptCompilerDummyIfNeeded(Stream bin) {
			if (RegionScriptCompilerCommandLineString != null) {
				return;
			}

			RegionScriptCompilerCommandLineString = SenUtils.InitRegion("ScriptCompilerCommandLineString", AddressStartScriptCompilerCommandLineString, AddressEndScriptCompilerCommandLineString, Mapper, bin, 0);
			RegionScriptCompilerCliParsingCode = SenUtils.InitRegion("ScriptCompilerCliParsingCode", AddressStartOfScriptCompilerCliParsingCode, AddressEndOfScriptCompilerCliParsingCode, Mapper, bin, 0xcc);
			RegionScriptCompilerFlagUsage1 = SenUtils.InitRegion("ScriptCompilerFlagUsage1", AddressStartOfScriptCompilerFlagUsage1, AddressEndOfScriptCompilerFlagUsage1, Mapper, bin, 0xcc);
			RegionScriptCompilerFlagUsage2 = SenUtils.InitRegion("ScriptCompilerFlagUsage2", AddressStartOfScriptCompilerFlagUsage2, AddressEndOfScriptCompilerFlagUsage2, Mapper, bin, 0xcc);
			RegionScriptCompilerFunction = SenUtils.InitRegion("ScriptCompilerFunction", AddressStartScriptCompilerFunction, AddressEndScriptCompilerFunction, Mapper, bin, 0xcc);
			RegionScriptCompilerFunctionStrings = SenUtils.InitRegion("ScriptCompilerFunctionStrings", AddressStartOfScriptCompilerFunctionStrings, AddressEndOfScriptCompilerFunctionStrings, Mapper, bin, 0);
			SenUtils.JumpOverCode(RegionScriptCompilerCliParsingCode, Mapper, bin);
			SenUtils.JumpOverCode(RegionScriptCompilerFlagUsage1, Mapper, bin);
			SenUtils.JumpOverCode(RegionScriptCompilerFlagUsage2, Mapper, bin);
		}
	}
}
