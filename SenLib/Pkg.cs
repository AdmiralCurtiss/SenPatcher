using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class Pkg {
		public uint Unknown; // seems like a unique index or something, at least in CS1/2?
		public List<PkgFile> Files;

		public Pkg(DuplicatableStream stream, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian) {
			var s = stream.Duplicate();
			s.Position = 0;
			Unknown = s.ReadUInt32(e);
			uint fileCount = s.ReadUInt32(e);

			Files = new List<PkgFile>((int)fileCount);
			for (long i = 0; i < fileCount; ++i) {
				Files.Add(new PkgFile(s, e));
			}
		}

		public void WriteToStream(Stream s, EndianUtils.Endianness e) {
			long pos = s.Position;
			s.WriteUInt32(Unknown, e);
			s.WriteUInt32((uint)Files.Count, e);
			for (int i = 0; i < Files.Count; ++i) {
				var f = Files[i];
				s.WriteUTF8(f.Filename, 0x40, false);
				s.WriteUInt32(f.UncompressedSize, e);
				s.WriteUInt32(0, e); // compressed size, fill in later
				s.WriteUInt32(0, e); // file position, fill in later
				s.WriteUInt32(f.Flags, e);
			}

			for (int i = 0; i < Files.Count; ++i) {
				var f = Files[i];
				long currentFileStart = s.Position;
				using (DuplicatableStream fs = f.Data.Duplicate()) {
					uint p = (uint)(currentFileStart - pos);
					uint size = (uint)fs.Length;
					s.Position = pos + 8 + 0x50 * i + 0x44;
					s.WriteUInt32(size, e);
					s.WriteUInt32(p, e);
					s.Position = currentFileStart;
					StreamUtils.CopyStream(fs, s);
				}
			}
		}
	}

	public class PkgFile {
		public string Filename;
		public uint UncompressedSize;
		public uint Flags;
		public DuplicatableStream Data;

		internal PkgFile(DuplicatableStream s, EndianUtils.Endianness e) {
			Filename = s.ReadSizedString(0x40, TextUtils.GameTextEncoding.UTF8).TrimNull();
			UncompressedSize = s.ReadUInt32(e);
			uint compressedSize = s.ReadUInt32(e);
			uint position = s.ReadUInt32(e);
			Flags = s.ReadUInt32(e);
			Data = new PartialStream(s, position, compressedSize);
		}
	}
}
