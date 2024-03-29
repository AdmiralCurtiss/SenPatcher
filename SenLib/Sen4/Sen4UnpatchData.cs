using HyoutaUtils.Checksum;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4 {
	public static class Sen4UnpatchData {
		public static UnpatchFile[] GetFiles() {
			return new UnpatchFile[] {
				new UnpatchFile(new SHA1(0x21ac0c6c99dcf57cul, 0x0f75b00d44d42d8dul, 0xae99fe3fu), "data/scripts/scena/dat_en/t3600.dat"),
				new UnpatchFile(new SHA1(0xcdc0b7a54ba0420aul, 0x0c7c314d2a021f69ul, 0x49f5cb2eu), "data/scripts/scena/dat_en/m9031.dat"),
				new UnpatchFile(new SHA1(0xd1cd26a05828553bul, 0xb7bf03e370717226ul, 0xf28353a0u), "data/scripts/scena/dat_en/f4200.dat"),
				new UnpatchFile(new SHA1(0x4e83e0152b272f6eul, 0x7739c89a07c1b0c5ul, 0xe2499e8au), "data/text/dat_en/t_item_en.tbl"),
				new UnpatchFile(new SHA1(0xa6a6031210ad60feul, 0xe7cf1949e4c31be2ul, 0xdb4fc413u), "data/text/dat_en/t_itemhelp.tbl"),
				new UnpatchFile(new SHA1(0x2c71852245a5d7a1ul, 0x0c5c7e687e4a095aul, 0xc1f54b60u), "data/text/dat_en/t_magic.tbl"),
				new UnpatchFile(new SHA1(0x40ae4f525021b550ul, 0xb77e045a00841a42ul, 0xbc460a77u), "data/text/dat_en/t_mstqrt.tbl"),
				new UnpatchFile(new SHA1(0xc755b53e8ba8618eul, 0x9e55d4e7c83cccb7ul, 0xe7298fbbu), "data/text/dat_en/t_notecook.tbl"),
				new UnpatchFile(new SHA1(0xed242395ee9b5aa1ul, 0x5f11bf83138b161bul, 0xb116d20au), "data/text/dat_en/t_text.tbl"),
				new UnpatchFile(new SHA1(0x0d424434330d4368ul, 0xc1ae187cae83e4dbul, 0xa296ccf1u), "bin/Win64/ed8_4_PC.exe"),
				new UnpatchFile(new SHA1(0x79e81f7a977e9180ul, 0x41b66fb27dc8805dul, 0x035e83ffu), "bin/Win64/ed8_4_PC_JP.exe"),
			};
		}
	}
}
