using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4 {
	public partial class Sen4ExecutablePatchState {
		public bool IsJp { get; private set; }

		public HyoutaPluginBase.IRomMapper Mapper { get; private set; }

		public Sen4ExecutablePatchState(bool jp, HyoutaPluginBase.IRomMapper mapper, Stream bin) {
			IsJp = jp;
			Mapper = mapper;

			CtorCodeSpaceScriptCompilerDummy(jp, mapper, bin);
		}
	}
}
