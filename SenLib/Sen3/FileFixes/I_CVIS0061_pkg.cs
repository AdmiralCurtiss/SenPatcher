using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class I_CVIS0061_pkg : FileMod {
		public string GetDescription() {
			return "Fix McBurn nametag.";
		}

		public static System.IO.MemoryStream PatchSingleTexturePkg(HyoutaPluginBase.DuplicatableStream file, System.IO.Stream patch) {
			var pkg = new Pkg(file, HyoutaUtils.EndianUtils.Endianness.LittleEndian);
			byte[] compressedOriginal = pkg.Files[1].Data.Duplicate().CopyToByteArrayAndDispose();
			byte[] decompressedOriginal = new byte[pkg.Files[1].UncompressedSize];
			int result = lz4_sharp.lz4.LZ4_decompress_safe(compressedOriginal, 0, decompressedOriginal, 0, compressedOriginal.Length, decompressedOriginal.Length);
			if (result != decompressedOriginal.Length) {
				return null; // something broke
			}

			var patched = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(new DuplicatableByteArrayStream(decompressedOriginal), patch);
			byte[] modified = patched.CopyToByteArrayAndDispose();

			// TODO: original file was LZ4HC compressed, replicate that
			byte[] modifiedCompressed = new byte[lz4_sharp.lz4.LZ4_compressBound(modified.Length)];
			int modifiedCompressedSize = lz4_sharp.lz4.LZ4_compress_default(modified, 0, modifiedCompressed, 0, modified.Length, modifiedCompressed.Length);

			pkg.Files[1].Data = new PartialStream(new DuplicatableByteArrayStream(modifiedCompressed), 0, modifiedCompressedSize);
			pkg.Files[1].UncompressedSize = (uint)modified.Length;
			var ms = new System.IO.MemoryStream();
			pkg.WriteToStream(ms, EndianUtils.Endianness.LittleEndian);

			return ms;
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x27648dfed57f4085ul, 0x7139008f5f9e3bb8ul, 0x347cbb90u));
			if (file == null) {
				return null;
			}

			var result = PatchSingleTexturePkg(file, DecompressHelper.DecompressFromBuffer(Properties.Resources.cvis0061));
			return new FileModResult[] { new FileModResult("data/asset/D3D11_us/I_CVIS0061.pkg", result) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x27648dfed57f4085ul, 0x7139008f5f9e3bb8ul, 0x347cbb90u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/asset/D3D11_us/I_CVIS0061.pkg", file) };
		}
	}
}
