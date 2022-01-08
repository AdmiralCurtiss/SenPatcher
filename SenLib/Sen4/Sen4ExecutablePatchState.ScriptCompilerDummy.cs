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
			RegionScriptCompilerFunctionCallSite1 = SenUtils.InitRegion64("RegionScriptCompilerFunctionCallSite1", jp ? 0x140233ead : 0x14023616d, jp ? 0x140233ed1 : 0x140236191, mapper, bin, 0xcc);
			RegionScriptCompilerFunctionCallSite2 = SenUtils.InitRegion64("RegionScriptCompilerFunctionCallSite2", jp ? 0x1402a774e : 0x1402a9a7e, jp ? 0x1402a777e : 0x1402a9aae, mapper, bin, 0xcc);
			RegionScriptCompilerFunction          = SenUtils.InitRegion64("ScriptCompilerFunction",                jp ? 0x1403e8460 : 0x1403ea710, jp ? 0x1403e8790 : 0x1403eaa40, mapper, bin, 0xcc);
			RegionScriptCompilerFunctionStrings   = SenUtils.InitRegion64("ScriptCompilerFunctionStrings",         jp ? 0x1408a7748 : 0x1408a90a4, jp ? 0x1408a7777 : 0x1408a90d7, mapper, bin, 0);
			SenUtils.JumpOverCode(RegionScriptCompilerFunctionCallSite1, mapper, bin);
			SenUtils.JumpOverCode(RegionScriptCompilerFunctionCallSite2, mapper, bin);
			RegionScriptCompilerFunction.Take(1, "Old function call protection");
		}
	}
}
