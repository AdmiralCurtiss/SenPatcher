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
			new KnownFile(new SHA1(0x6c33cf8b1cf93950ul, 0xd802f5ac9de1b213ul, 0xc0b45033u), "data/movie_us/webm/insa_05.webm"),
			new KnownFile(new SHA1(0x97b37cb0d324014dul, 0x8db6b2965f1836eful, 0xfdb1ce01u), "data/movie_us/webm/insa_08.webm", "data/movie_us/webm/insa_09.webm"),
			new KnownFile(new SHA1(0x2c6088c4f1e8847eul, 0x49f5f5f48b410fe2ul, 0xaec3ef54u), "data/scripts/battle/dat_en/alchr022.dat"),
			new KnownFile(new SHA1(0x4531fd2f226f5c64ul, 0x1e09fc35e118d1c1ul, 0xbb2a4144u), "data/scripts/book/dat_en/book07.dat"),
			new KnownFile(new SHA1(0x2b0b345b51b705b7ul, 0x1ddc5f7989836dc3ul, 0x139f62f5u), "data/scripts/scena/dat_en/a0417.dat"),
			new KnownFile(new SHA1(0x8a6e007a75ff9d1bul, 0x06cc96f67d383ff7ul, 0xde6d2eb6u), "data/scripts/scena/dat_en/c0200.dat"),
			new KnownFile(new SHA1(0xe9ef91d4cf274a0ful, 0xb055303c3d29dcd1ul, 0xaf99fd49u), "data/scripts/scena/dat_en/c0250.dat"),
			new KnownFile(new SHA1(0x4d4183d8d45f846cul, 0x7659a7db1d8c39c7ul, 0x58ffb50au), "data/scripts/scena/dat_en/c0420.dat"),
			new KnownFile(new SHA1(0x2a1b99c4395efd23ul, 0x0bc10942ae1c06f4ul, 0x36f4e504u), "data/scripts/scena/dat_en/c0430.dat"),
			new KnownFile(new SHA1(0xa5da1241a9c17b2ful, 0xbb2fc9c93dc691d5ul, 0x1471ec8au), "data/scripts/scena/dat_en/c0820.dat"),
			new KnownFile(new SHA1(0xbb6a5b0a31ebe05cul, 0xeb06ae8c0b773e2ful, 0x0b8823afu), "data/scripts/scena/dat_en/c0830.dat"),
			new KnownFile(new SHA1(0x2fae040a29bafe3cul, 0xf202c2d5eddadef2ul, 0x424523d8u), "data/scripts/scena/dat_en/c2430.dat"),
			new KnownFile(new SHA1(0x37161993d10c6582ul, 0x1c11026fedb1af52ul, 0x90ceff4du), "data/scripts/scena/dat_en/c2440.dat"),
			new KnownFile(new SHA1(0xd89a6f84a665a650ul, 0xf32269931c874bf3ul, 0xd1cb0b1bu), "data/scripts/scena/dat_en/c2610.dat"),
			new KnownFile(new SHA1(0xaf0504554a605f3eul, 0x154b967683fed546ul, 0x17d7a0ebu), "data/scripts/scena/dat_en/c2620.dat"),
			new KnownFile(new SHA1(0x6c8a21013585ce0ful, 0x1ddf378cf8963aecul, 0x5f283a75u), "data/scripts/scena/dat_en/c3000.dat"),
			new KnownFile(new SHA1(0x648885e17c3f2371ul, 0xf9a71b07990e054dul, 0xb09242fbu), "data/scripts/scena/dat_en/c3010.dat"),
			new KnownFile(new SHA1(0x34c651df94044c96ul, 0xf600a24ba99eafa2ul, 0xf04d08d1u), "data/scripts/scena/dat_en/c3210.dat"),
			new KnownFile(new SHA1(0x043cfd82b96630f5ul, 0x6a838aadce36591dul, 0x2001196fu), "data/scripts/scena/dat_en/c3610.dat"),
			new KnownFile(new SHA1(0x24b0dc7a0f3f7c94ul, 0x5f199de834a1df5eul, 0x8a9ec78bu), "data/scripts/scena/dat_en/f0000.dat"),
			new KnownFile(new SHA1(0x1c5d1ffde19b3fe0ul, 0xb14ce4b4d8c29ebaul, 0xe850b8cbu), "data/scripts/scena/dat_en/f0010.dat"),
			new KnownFile(new SHA1(0xbb3220cb9f85e554ul, 0xfd1196688bcc1ea5ul, 0x78a3d234u), "data/scripts/scena/dat_en/f2000.dat"),
			new KnownFile(new SHA1(0x289204f27043cf2ful, 0xdc6272bffd1ec085ul, 0x7f3230d3u), "data/scripts/scena/dat_en/m0000.dat"),
			new KnownFile(new SHA1(0xb0f3a274fd1e5452ul, 0x8118b4a68706e7c3ul, 0x30285fefu), "data/scripts/scena/dat_en/m0100.dat"),
			new KnownFile(new SHA1(0x7103fd901920a786ul, 0x37b5abe65b071220ul, 0xd0962783u), "data/scripts/scena/dat_en/m0300.dat"),
			new KnownFile(new SHA1(0x09161c59396b4aa8ul, 0x932f628fa9709ca8ul, 0x899fe58cu), "data/scripts/scena/dat_en/m0600.dat"),
			new KnownFile(new SHA1(0x03bec649d56e0487ul, 0x72de4fd1ce013005ul, 0xb651d784u), "data/scripts/scena/dat_en/m3000.dat"),
			new KnownFile(new SHA1(0x2eff0eb2d6a770b4ul, 0x2069380f42d7daecul, 0xdd33a96du), "data/scripts/scena/dat_en/m3420.dat"),
			new KnownFile(new SHA1(0x8a880ba00b221cdcul, 0x176eb66cabb23573ul, 0x750251d5u), "data/scripts/scena/dat_en/m3430.dat"),
			new KnownFile(new SHA1(0x5941f93f60a4ab76ul, 0x661ae37e51103b8ful, 0x1cf35011u), "data/scripts/scena/dat_en/m4004.dat"),
			new KnownFile(new SHA1(0x098c038fe41d7629ul, 0xd13428c3484f0d89ul, 0x9113937fu), "data/scripts/scena/dat_en/r0000.dat"),
			new KnownFile(new SHA1(0xb636a0c274714e93ul, 0xb74db3871e7019bful, 0x6ca39a7cu), "data/scripts/scena/dat_en/r0210.dat"),
			new KnownFile(new SHA1(0x0c5fbbb90cb3459eul, 0x0d2f5b9769eb5f7dul, 0x9d87992cu), "data/scripts/scena/dat_en/r2290.dat"),
			new KnownFile(new SHA1(0x9bde3d287daabc7dul, 0xf76207d20b04da2aul, 0xd1c7120au), "data/scripts/scena/dat_en/r3000.dat"),
			new KnownFile(new SHA1(0x48c59f32ff001518ul, 0xdcef78c47b5f2050ul, 0xf3e0de55u), "data/scripts/scena/dat_en/r3200.dat"),
			new KnownFile(new SHA1(0x3efbd8764d61274aul, 0x8750342972e75143ul, 0x131d7721u), "data/scripts/scena/dat_en/r3430.dat"),
			new KnownFile(new SHA1(0xd6dcc55f71cf2e61ul, 0x93c6a33b53b8879cul, 0x0d4d5958u), "data/scripts/scena/dat_en/r4200.dat"),
			new KnownFile(new SHA1(0x871c9c02460dde4aul, 0xcbb7712111af384eul, 0xd76a3bdcu), "data/scripts/scena/dat_en/r4290.dat"),
			new KnownFile(new SHA1(0x895e048dd4b006b1ul, 0xbda8d2434de9edfbul, 0x20142ef9u), "data/scripts/scena/dat_en/t0000.dat"),
			new KnownFile(new SHA1(0x53812a2a94c94c7bul, 0xc9068c4e43d8973eul, 0xb32d5034u), "data/scripts/scena/dat_en/t0010.dat"),
			new KnownFile(new SHA1(0xdacda5af52ca6ab3ul, 0xefee4f9b51606a5eul, 0x3d676a7eu), "data/scripts/scena/dat_en/t0080.dat"),
			new KnownFile(new SHA1(0xf36378e7f72b2dd7ul, 0xe49e8b363b3916bbul, 0x388ecbc1u), "data/scripts/scena/dat_en/t0200.dat"),
			new KnownFile(new SHA1(0xc45eb8312b11f6a8ul, 0x7476165fe69d6467ul, 0x89d5e48bu), "data/scripts/scena/dat_en/t0210.dat"),
			new KnownFile(new SHA1(0x1ba8784695ed8630ul, 0x9b72a3104c7b0b81ul, 0xb67b503bu), "data/scripts/scena/dat_en/t0250.dat"),
			new KnownFile(new SHA1(0xf00fc1a818c84469ul, 0xfd34cfb593d03ad4ul, 0x24393aceu), "data/scripts/scena/dat_en/t0260.dat"),
			new KnownFile(new SHA1(0x4d0f1d1d3b57eb70ul, 0xde562a52f4367495ul, 0x426d1896u), "data/scripts/scena/dat_en/t0410.dat"),
			new KnownFile(new SHA1(0xd0fd8367295246abul, 0x1eafede393303d34ul, 0xf7f5bcc5u), "data/scripts/scena/dat_en/t3000.dat"),
			new KnownFile(new SHA1(0x11f838467868bad6ul, 0x1623e965fbc3c060ul, 0x7d2a3356u), "data/scripts/scena/dat_en/t3200.dat"),
			new KnownFile(new SHA1(0x4cdac7c910724aa2ul, 0x7bfe5ada8fc25f9bul, 0x9118833bu), "data/scripts/scena/dat_en/t3220.dat"),
			new KnownFile(new SHA1(0xd48fd0f978975b3dul, 0x4854d72475ba6700ul, 0x76961db8u), "data/scripts/scena/dat_en/t3400.dat"),
			new KnownFile(new SHA1(0x7d6eddb306de371ful, 0xd7fcfd55b5993016ul, 0x83e18853u), "data/scripts/scena/dat_en/t3510.dat"),
			new KnownFile(new SHA1(0xf20f8c36a44c88feul, 0xcc44155250b42fa0ul, 0x259cd699u), "data/scripts/scena/dat_en/t3600.dat"),
			new KnownFile(new SHA1(0x4c2781204c47f245ul, 0x7e831b6a50413068ul, 0xb498c5b3u), "data/scripts/scena/dat_en/v0010.dat"),
			new KnownFile(new SHA1(0xb1f43611bbf1130ful, 0xf552405bc50d2f75ul, 0x637e49e1u), "data/scripts/scena/dat_en/v0050.dat"),
			new KnownFile(new SHA1(0x4c1c3cd56be5eb1aul, 0x950812709685b2f0ul, 0x29096f2du), "data/scripts/talk/dat_en/tk_ada.dat"),
			new KnownFile(new SHA1(0x65aa739d1b7a5f3ful, 0xf91e7fcf4840a5dbul, 0x26bfcdfbu), "data/scripts/talk/dat_en/tk_linde.dat"),
			new KnownFile(new SHA1(0x6ed1ba8558121db3ul, 0x18927df877171223ul, 0x3f41fcceu), "data/scripts/talk/dat_en/tk_patrick.dat"),
			new KnownFile(new SHA1(0x095615a115f3653bul, 0x2ecb59a16a756f8cul, 0x3f12b68du), "data/scripts/talk/dat_en/tk_stark.dat"),
			new KnownFile(new SHA1(0x5f392e50537f7ef7ul, 0x79cb276c8d04c4ebul, 0x7b7198d4u), "data/scripts/talk/dat_en/tk_tovar.dat"),
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
