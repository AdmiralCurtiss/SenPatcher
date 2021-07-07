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
			Logging.Log("Initializing CS3 file data.");
			return new KnownFile[] {
			new KnownFile(new SHA1(0x9c78d23b2b80fa5aul, 0x0042caf00dc67244ul, 0xd6265d25u), DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_e0441).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
			new KnownFile(new SHA1(0xac35607991b8d7b2ul, 0xe1e1b9cbd4deb578ul, 0x45697915u), DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_s0027).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
			new KnownFile(new SHA1(0x4d7ba4fffd80658cul, 0x9db5b475cac0033dul, 0xf4f00d96u), DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_s0028).CopyToByteArrayStreamAndDispose(), writeToBackup: false),
			new KnownFile(new SHA1(0x27648dfed57f4085ul, 0x7139008f5f9e3bb8ul, 0x347cbb90u), "data/asset/D3D11_us/I_CVIS0061.pkg"),
			new KnownFile(new SHA1(0x3721996e74336af0ul, 0x133f096b1f717e83ul, 0xb9bdc075u), "data/asset/D3D11_us/I_CVIS1008.pkg"),
			new KnownFile(new SHA1(0x05f162efc3d880e9ul, 0x4398ae4e10108aa0ul, 0x18f30feeu), "data/asset/D3D11_us/I_JMP009.pkg"),
			new KnownFile(new SHA1(0x660380324204ba12ul, 0xb6b100333cff246ful, 0xe6e18674u), "data/text/dat/t_item.tbl"),
			new KnownFile(new SHA1(0x5deee9b833b2bb93ul, 0xb0a326f586943f3dul, 0x2e2424b9u), "data/text/dat_en/t_item_en.tbl", "data/text/dat_fr/t_item_en.tbl"),
			new KnownFile(new SHA1(0x78602fdfed633bb3ul, 0xe41303f135d93674ul, 0x40c25c38u), "data/text/dat_en/t_item_fr.tbl", "data/text/dat_fr/t_item_fr.tbl"),
			new KnownFile(new SHA1(0x1ee95c9c9cd1ff0aul, 0xb3367f142bb02a18ul, 0x0a4036efu), "data/text/dat/t_itemhelp.tbl"),
			new KnownFile(new SHA1(0xcb9135407b8264acul, 0x813e921329374a84ul, 0x4f55036bu), "data/text/dat_en/t_itemhelp.tbl"),
			new KnownFile(new SHA1(0xc7df5e2a0b4365fbul, 0xf5a19b4710e50d7dul, 0x3758fbd7u), "data/text/dat_fr/t_itemhelp.tbl"),
			new KnownFile(new SHA1(0xf575ce0fc4798812ul, 0x2ec8107110238ad3ul, 0xc902529eu), "data/text/dat/t_magic.tbl"),
			new KnownFile(new SHA1(0xc0b07b04d56268a7ul, 0xc42471d6671dc5cbul, 0x6243286fu), "data/text/dat_en/t_magic.tbl"),
			new KnownFile(new SHA1(0xf0460f7c90178135ul, 0xeae01a737c98f614ul, 0xa2227b38u), "data/text/dat_fr/t_magic.tbl"),
			new KnownFile(new SHA1(0x86c4d62ac6efc4abul, 0x5dbfdc7f5bf36c4eul, 0xaef64c3bu), "data/text/dat/t_mstqrt.tbl"),
			new KnownFile(new SHA1(0x494b68fcbc463581ul, 0xf79f7dd1f6444df9ul, 0xc4ad6204u), "data/text/dat_en/t_mstqrt.tbl"),
			new KnownFile(new SHA1(0xb8022803ece579b5ul, 0x342e5143dc91ae47ul, 0x06fd7f7cu), "data/text/dat_fr/t_mstqrt.tbl"),
			new KnownFile(new SHA1(0x0b0b536b6e6d29ecul, 0x059cdde6900b4614ul, 0x789d84a9u), "data/text/dat/t_notecook.tbl"),
			new KnownFile(new SHA1(0x9155e5158bb3e333ul, 0x654dd761072a04e0ul, 0xa8be6728u), "data/text/dat_en/t_notecook.tbl"),
			new KnownFile(new SHA1(0x9e6bf6a3bb2bf3a5ul, 0xa06fbe4aa54e451bul, 0xd61bc20du), "data/text/dat_fr/t_notecook.tbl"),
			new KnownFile(new SHA1(0x5bf0cb69ea8f50fcul, 0xf4ea4854581e2f46ul, 0x0e539634u), "data/text/dat/t_text.tbl"),
			new KnownFile(new SHA1(0xa2720e94f597640dul, 0xecd1d978b6b8f731ul, 0x147578a6u), "data/text/dat_en/t_text.tbl"),
			new KnownFile(new SHA1(0xec2b652621dd2b31ul, 0x9beabce7c923f2e8ul, 0x67bd1abfu), "data/text/dat_fr/t_text.tbl"),
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
