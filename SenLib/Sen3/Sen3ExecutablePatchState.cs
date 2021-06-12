using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3 {
	public partial class Sen3ExecutablePatchState {
		public bool IsJp { get; private set; }

		public HyoutaPluginBase.IRomMapper Mapper { get; private set; }

		public Sen3ExecutablePatchState(bool jp, HyoutaPluginBase.IRomMapper mapper) {
			IsJp = jp;
			Mapper = mapper;
		}
	}
}
