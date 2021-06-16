using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3 {
	public partial class Sen3ExecutablePatchState {
		public RegionHelper64 RegionScriptCompilerFunctionCallSite1 = null;
		public RegionHelper64 RegionScriptCompilerFunctionCallSite2 = null;
		public RegionHelper64 RegionScriptCompilerFunction = null;
		public RegionHelper64 RegionScriptCompilerFunctionStrings1 = null;
		public RegionHelper64 RegionScriptCompilerFunctionStrings2 = null;

		private void CtorCodeSpaceScriptCompilerDummy(bool jp, HyoutaPluginBase.IRomMapper mapper, Stream bin) {
			if (jp) return; // TODO

			RegionScriptCompilerFunctionCallSite1 = SenUtils.InitRegion64("RegionScriptCompilerFunctionCallSite1", jp ? 0 : 0x14025e4ee, jp ? 0 : 0x14025e512, Mapper, bin, 0xcc);
			RegionScriptCompilerFunctionCallSite2 = SenUtils.InitRegion64("RegionScriptCompilerFunctionCallSite2", jp ? 0 : 0x1402c65a0, jp ? 0 : 0x1402c65c6, Mapper, bin, 0xcc);
			RegionScriptCompilerFunction = SenUtils.InitRegion64("ScriptCompilerFunction", jp ? 0 : 0x1403c76f0, jp ? 0 : 0x1403c7ac0, mapper, bin, 0xcc);
			RegionScriptCompilerFunctionStrings1 = SenUtils.InitRegion64("ScriptCompilerFunctionStrings1", jp ? 0 : 0x1408215f0, jp ? 0 : 0x140821647, mapper, bin, 0);
			RegionScriptCompilerFunctionStrings2 = SenUtils.InitRegion64("ScriptCompilerFunctionStrings2", jp ? 0 : 0x1408213a8, jp ? 0 : 0x1408213dc, mapper, bin, 0);
			SenUtils.JumpOverCode(RegionScriptCompilerFunctionCallSite1, Mapper, bin);
			SenUtils.JumpOverCode(RegionScriptCompilerFunctionCallSite2, Mapper, bin);
		}
	}
}
