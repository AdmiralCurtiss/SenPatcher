using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Checksum;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4 {
	public static class Sen4KnownFiles {
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
			Logging.Log("Initializing CS4 file data.");
			return new KnownFile[] {
				new KnownFile(new SHA1(0xb80489ae03c9c909ul, 0x53e7285b920df660ul, 0x148768efu), "data/text/dat_en/t_text.tbl"),
				new KnownFile(new SHA1(0xc8669cb40e015c16ul, 0xec16776bd9a30945ul, 0xb5545a72u), "bin/Win64/ed8_4_PC.exe"),
				new KnownFile(new SHA1(0xf3952664b8da2607ul, 0x23677c91980ef9bcul, 0xf5c3113au), "bin/Win64/ed8_4_PC_JP.exe"),
				new KnownFile(new SHA1(0x5f480136aa4c3b53ul, 0xadd422bf75b63350ul, 0xfa58d202u), "Sen4Launcher.exe"),
			};
		}
	}
}
