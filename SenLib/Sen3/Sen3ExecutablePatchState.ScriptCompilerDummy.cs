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
			// 1.05
			//long scriptCompilerFunctionCall1S = jp ? 0x140257bdd : 0x14025e4ee;
			//long scriptCompilerFunctionCall1E = jp ? 0x140257c01 : 0x14025e512;
			//long scriptCompilerFunctionCall2S = jp ? 0x1402bee9e : 0x1402c65a0;
			//long scriptCompilerFunctionCall2E = jp ? 0x1402beec4 : 0x1402c65c6;
			//long scriptCompilerFunctionS = jp ? 0x1403bd450 : 0x1403c76f0;
			//long scriptCompilerFunctionE = jp ? 0x1403bd800 : 0x1403c7ac0;
			//long scriptCompilerFunctionStrings1S = jp ? 0x14080d578 : 0x1408215f0;
			//long scriptCompilerFunctionStrings1E = jp ? 0x14080d5cf : 0x140821647;
			//long scriptCompilerFunctionStrings2S = jp ? 0x14080d3d0 : 0x1408213a8;
			//long scriptCompilerFunctionStrings2E = jp ? 0x14080d3e9 : 0x1408213dc;

			// 1.06
			long scriptCompilerFunctionCall1S = jp ? 0x140257b9d : 0x14025e4ae;
			long scriptCompilerFunctionCall1E = jp ? 0x140257bc1 : 0x14025e4d2;
			long scriptCompilerFunctionCall2S = jp ? 0x1402bf0cf : 0x1402c67d0;
			long scriptCompilerFunctionCall2E = jp ? 0x1402bf0f5 : 0x1402c67f6;
			long scriptCompilerFunctionS = jp ? 0x1403bd6b0 : 0x1403c7950;
			long scriptCompilerFunctionE = jp ? 0x1403bda60 : 0x1403c7d20;
			long scriptCompilerFunctionStrings1S = jp ? 0x14080d438 : 0x1408214d0;
			long scriptCompilerFunctionStrings1E = jp ? 0x14080d48f : 0x140821527;
			long scriptCompilerFunctionStrings2S = jp ? 0x14080d290 : 0x140821280;
			long scriptCompilerFunctionStrings2E = jp ? 0x14080d2a9 : 0x1408212b4;


			RegionScriptCompilerFunctionCallSite1 = SenUtils.InitRegion64("RegionScriptCompilerFunctionCallSite1", scriptCompilerFunctionCall1S, scriptCompilerFunctionCall1E, Mapper, bin, 0xcc);
			RegionScriptCompilerFunctionCallSite2 = SenUtils.InitRegion64("RegionScriptCompilerFunctionCallSite2", scriptCompilerFunctionCall2S, scriptCompilerFunctionCall2E, Mapper, bin, 0xcc);
			RegionScriptCompilerFunction = SenUtils.InitRegion64("ScriptCompilerFunction", scriptCompilerFunctionS, scriptCompilerFunctionE, mapper, bin, 0xcc);
			RegionScriptCompilerFunctionStrings1 = SenUtils.InitRegion64("ScriptCompilerFunctionStrings1", scriptCompilerFunctionStrings1S, scriptCompilerFunctionStrings1E, mapper, bin, 0);
			RegionScriptCompilerFunctionStrings2 = SenUtils.InitRegion64("ScriptCompilerFunctionStrings2", scriptCompilerFunctionStrings2S, scriptCompilerFunctionStrings2E, mapper, bin, 0);
			SenUtils.JumpOverCode(RegionScriptCompilerFunctionCallSite1, Mapper, bin);
			SenUtils.JumpOverCode(RegionScriptCompilerFunctionCallSite2, Mapper, bin);
			RegionScriptCompilerFunction.Take(1, "Old function call protection");
		}
	}
}
