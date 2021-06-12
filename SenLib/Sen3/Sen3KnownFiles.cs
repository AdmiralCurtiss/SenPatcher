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
			new KnownFile(new SHA1(0x9c78d23b2b80fa5aul, 0x0042caf00dc67244ul, 0xd6265d25u), DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_e0441).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
			new KnownFile(new SHA1(0xac35607991b8d7b2ul, 0xe1e1b9cbd4deb578ul, 0x45697915u), DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_s0027).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
			new KnownFile(new SHA1(0x4d7ba4fffd80658cul, 0x9db5b475cac0033dul, 0xf4f00d96u), DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_s0028).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
			new KnownFile(new SHA1(0x07fb5a4afb6719fful, 0x611f4641e90ad652ul, 0x23d0fae3u), "data/voice_us/opus/v00_e0441.opus"),
			new KnownFile(new SHA1(0x28b5f314748c04e6ul, 0x0a250a6147ed3b8cul, 0x7b3694c8u), "data/voice_us/opus/v00_s0027.opus"),
			new KnownFile(new SHA1(0x2efce24a4badc67bul, 0x00111edf1a0bf398ul, 0x5c1f26aau), "data/voice_us/opus/v00_s0028.opus"),
			new KnownFile(new SHA1(0x72920fa16072f4b7ul, 0x51e6a5be3fb4df50ul, 0xb21da506u), "bin/x64/ed8_3_PC.exe"),
			new KnownFile(new SHA1(0x2e9da161efc30b9aul, 0xf020903e99cfa9a2ul, 0x287ff1a2u), "bin/x64/ed8_3_PC_JP.exe"),
			new KnownFile(new SHA1(0x21de3b088a5ddad7ul, 0xed1fdb8e40061497ul, 0xc248ca65u), "Sen3Launcher.exe"),
			};
		}
	}
}
