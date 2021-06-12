using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Checksum;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3 {
	public static class Sen3KnownFiles {
		public static KnownFile[] Files { get { return GetFiles(); } }

		private static object Mutex = new object();
		private static KnownFile[] Files_ = null;
		private static KnownFile[] GetFiles() {
			lock (Mutex) {
				if (Files_ == null) {
					Files_ = InitKnownFiles();
				}
				return Files_;
			}
		}
		private static KnownFile[] InitKnownFiles() {
			return new KnownFile[] {
			new KnownFile(new SHA1(0x72920fa16072f4b7ul, 0x51e6a5be3fb4df50ul, 0xb21da506u), "bin/x64/ed8_3_PC.exe"),
			new KnownFile(new SHA1(0x2e9da161efc30b9aul, 0xf020903e99cfa9a2ul, 0x287ff1a2u), "bin/x64/ed8_3_PC_JP.exe"),
			new KnownFile(new SHA1(0x21de3b088a5ddad7ul, 0xed1fdb8e40061497ul, 0xc248ca65u), "Sen3Launcher.exe"),
			};
		}
	}
}
