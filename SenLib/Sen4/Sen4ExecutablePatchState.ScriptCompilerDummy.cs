using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4 {
	public partial class Sen4ExecutablePatchState {
		public RegionHelper64 RegionScriptCompilerFunctionCallSite1 = null;
		public RegionHelper64 RegionScriptCompilerFunctionCallSite2 = null;
		public RegionHelper64 RegionScriptCompilerFunction = null;
		public RegionHelper64 RegionScriptCompilerFunctionStrings = null;

		private void CtorCodeSpaceScriptCompilerDummy(bool jp, HyoutaPluginBase.IRomMapper mapper, Stream bin) {
			// 1.2.0
			//long scriptCompilerFunctionCall1S = jp ? 0x140233ead : 0x14023616d;
			//long scriptCompilerFunctionCall1E = jp ? 0x140233ed1 : 0x140236191;
			//long scriptCompilerFunctionCall2S = jp ? 0x1402a774e : 0x1402a9a7e;
			//long scriptCompilerFunctionCall2E = jp ? 0x1402a777e : 0x1402a9aae;
			//long scriptCompilerFunctionS = jp ? 0x1403e8460 : 0x1403ea710;
			//long scriptCompilerFunctionE = jp ? 0x1403e8790 : 0x1403eaa40;
			//long scriptCompilerFunctionStringsS = jp ? 0x1408a7748 : 0x1408a90a4;
			//long scriptCompilerFunctionStringsE = jp ? 0x1408a7777 : 0x1408a90d7;

			// 1.2.1
			long scriptCompilerFunctionCall1S = jp ? 0x140233e9d : 0x14023615d;
			long scriptCompilerFunctionCall1E = jp ? 0x140233ec1 : 0x140236181;
			long scriptCompilerFunctionCall2S = jp ? 0x1402a78ee : 0x1402a9c1e;
			long scriptCompilerFunctionCall2E = jp ? 0x1402a791e : 0x1402a9c4e;
			long scriptCompilerFunctionS = jp ? 0x1403e8820 : 0x1403eaac0;
			long scriptCompilerFunctionE = jp ? 0x1403e8b50 : 0x1403eadf0;
			long scriptCompilerFunctionStringsS = jp ? 0x1408a7918 : 0x1408aa264;
			long scriptCompilerFunctionStringsE = jp ? 0x1408a7947 : 0x1408aa297;

			RegionScriptCompilerFunctionCallSite1 = SenUtils.InitRegion64("RegionScriptCompilerFunctionCallSite1", scriptCompilerFunctionCall1S, scriptCompilerFunctionCall1E, mapper, bin, 0xcc);
			RegionScriptCompilerFunctionCallSite2 = SenUtils.InitRegion64("RegionScriptCompilerFunctionCallSite2", scriptCompilerFunctionCall2S, scriptCompilerFunctionCall2E, mapper, bin, 0xcc);
			RegionScriptCompilerFunction          = SenUtils.InitRegion64("ScriptCompilerFunction", scriptCompilerFunctionS, scriptCompilerFunctionE, mapper, bin, 0xcc);
			RegionScriptCompilerFunctionStrings   = SenUtils.InitRegion64("ScriptCompilerFunctionStrings", scriptCompilerFunctionStringsS, scriptCompilerFunctionStringsE, mapper, bin, 0);
			SenUtils.JumpOverCode(RegionScriptCompilerFunctionCallSite1, mapper, bin);
			SenUtils.JumpOverCode(RegionScriptCompilerFunctionCallSite2, mapper, bin);
			RegionScriptCompilerFunction.Take(1, "Old function call protection");
		}
	}
}
