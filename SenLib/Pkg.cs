using HyoutaPluginBase;
using HyoutaPluginBase.FileContainer;
using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class Pkg : HyoutaPluginBase.FileContainer.IContainer {
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

		public bool IsFile => false;

		public bool IsContainer => true;

		public IFile AsFile => null;

		public IContainer AsContainer => this;

		public void Dispose() {
			foreach (PkgFile f in Files) {
				f.Dispose();
			}
		}

		public INode GetChildByIndex(long index) {
			return Files[(int)index];
		}

		public INode GetChildByName(string name) {
			foreach (PkgFile f in Files) {
				if (f != null && f.Filename == name) {
					return f;
				}
			}
			return null;
		}

		public IEnumerable<string> GetChildNames() {
			foreach (PkgFile f in Files) {
				if (f != null && f.Filename != null) {
					yield return f.Filename;
				}
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

	public class PkgFile : HyoutaPluginBase.FileContainer.IFile {
		// individual file unpacking logic can be found at 0x41ad80 in CS2

		public string Filename;
		public uint UncompressedSize;
		public uint Flags;
		public DuplicatableStream Data;
		public EndianUtils.Endianness Endian;

		internal PkgFile(DuplicatableStream s, EndianUtils.Endianness e) {
			Endian = e;
			Filename = s.ReadSizedString(0x40, TextUtils.GameTextEncoding.UTF8).TrimNull();
			UncompressedSize = s.ReadUInt32(e); // 0x40
			uint compressedSize = s.ReadUInt32(e); // 0x44
			uint position = s.ReadUInt32(e); // 0x48
			Flags = s.ReadUInt32(e); // 0x4c
			Data = new PartialStream(s, position, compressedSize);
		}

		private static DuplicatableStream DecompressInternal(uint type, DuplicatableStream stream, long uncompressedSize, EndianUtils.Endianness e) {
			switch (type) {
				case 0: {
					// no compression
					return stream.Position != 0 ? new PartialStream(stream, stream.Position, stream.Length - stream.Position) : stream;
				}
				case 1: {
					// decompression algorithm derived from 0x41aa50 in CS2
					// very simple, can only copy verbatim bytes or reference already written bytes
					byte[] target = new byte[uncompressedSize];
					uint targetSize = stream.ReadUInt32(e); // seems to be unused
					uint sourceSize = stream.ReadUInt32(e);
					int backrefByte = stream.ReadInt32(e); // this is in fact read and compared as a 32-bit int
					uint targetPosition = 0;
					uint sourcePosition = 12;
					while (sourcePosition < sourceSize) {
						byte sourceByte = stream.ReadUInt8();
						++sourcePosition;
						if (sourceByte == backrefByte) {
							int backrefOffset = stream.ReadUInt8();
							++sourcePosition;
							if (backrefOffset == backrefByte) {
								target[targetPosition] = (byte)backrefByte;
								++targetPosition;
							} else {
								if (backrefByte < backrefOffset) {
									--backrefOffset;
								}
								uint backrefLength = stream.ReadUInt8();
								++sourcePosition;
								for (uint i = 0; i < backrefLength; ++i) {
									target[targetPosition + i] = target[targetPosition - backrefOffset + i];
								}
								targetPosition += backrefLength;
							}
						} else {
							target[targetPosition] = sourceByte;
							++targetPosition;
						}
					}
					if (targetSize != targetPosition) {
						Console.WriteLine("WARNING: Decompressed file is different filesize than expected. (type 1)");
					}
					return new DuplicatableByteArrayStream(target);
				}
				case 4: {
					// introduced in CS3 PC, I think, it's just LZ4
					byte[] compressed = stream.ReadBytes(stream.Length - stream.Position);
					byte[] decompressed = new byte[uncompressedSize];
					int result = lz4_sharp.lz4.LZ4_decompress_safe(compressed, 0, decompressed, 0, compressed.Length, decompressed.Length);
					if (result != decompressed.Length) {
						throw new Exception("LZ4 decompression failed.");
					}
					return new DuplicatableByteArrayStream(decompressed);
				}
				default:
					throw new Exception("Unsupported compression type '" + type + "'.");
			}
		}

		private DuplicatableStream Decompress() {
			bool hasChecksum = (Flags & 2u) != 0;
			using var stream = Data.Duplicate();
			stream.ReStart();
			uint checksum = hasChecksum ? stream.ReadUInt32(Endian) : 0;
			if (hasChecksum) {
				// verify checksum here? see 0x41ad40 in CS2, seems to be some kind of CRC
			}
			using var decompressed = DecompressInternal(Flags & ~2u, stream, UncompressedSize, Endian);
			if (decompressed.Length != UncompressedSize) {
				Console.WriteLine("WARNING: Decompressed file is different filesize than expected. (header)");
			}
			return decompressed.Duplicate();
		}

		public DuplicatableStream DataStream => Decompress();

		public bool IsFile => true;

		public bool IsContainer => false;

		public IFile AsFile => this;

		public IContainer AsContainer => null;

		public void Dispose() {
			if (Data != null) {
				Data.Dispose();
				Data = null;
			}
		}
	}
}
