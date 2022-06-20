using HyoutaPluginBase;
using HyoutaPluginBase.FileContainer;
using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class Pka : HyoutaPluginBase.FileContainer.IContainer {
		private DuplicatableStream Stream;
		private List<PkaPkgToHashData> Pkgs;
		private List<PkaHashToFileData> Files;

		public int PkgCount => Pkgs.Count;

		public Pka(DuplicatableStream stream, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian) {
			// From what I can tell, the idea of this format is deduplication of files that exist
			// identically in multiple PKGs. To accomplish this, two structures are set up:
			// First, a table that maps each PKG name (as would be opened by the PS4 version)
			// to data of which files are within that PKG (filename and SHA256 hash of file).
			// This is pre-sorted by PKG filename, likely to facilitate fast binary search lookup.
			// Second, a table that maps SHA256 hashes to the typical PKG file data (offset,
			// compressed/uncompressed size, flags). This is also pre-sorted, by numeric value of
			// the SHA256.
			// When a game then requests to open a PKG, one can quickly build a facsimile of the the PKG
			// by first finding the data of which files would be in that PKG from the first table, then
			// finding the actual file data via the SHA256 in the second table.

			Stream = stream.Duplicate();
			Stream.ReStart();
			uint magic = Stream.ReadUInt32(e);

			uint pkgCount = Stream.ReadUInt32(e);
			Pkgs = new List<PkaPkgToHashData>((int)pkgCount);
			for (uint i = 0; i < pkgCount; ++i) {
				Pkgs.Add(new PkaPkgToHashData(Stream));
			}

			uint fileCount = Stream.ReadUInt32(e);
			Files = new List<PkaHashToFileData>((int)fileCount);
			for (uint i = 0; i < fileCount; ++i) {
				Files.Add(new PkaHashToFileData(Stream));
			}
		}

		public string GetPkgName(int index) {
			return Pkgs[index].PkgName;
		}

		public bool IsFile => false;

		public bool IsContainer => true;

		public IFile AsFile => null;

		public IContainer AsContainer => this;

		public void Dispose() {
			Stream.Dispose();
		}

		public INode GetChildByIndex(long index) {
			return new Pkg(BuildPkg((int)index));
		}

		public INode GetChildByName(string name) {
			// not efficient, I think these are sorted
			for (int i = 0; i < Pkgs.Count; ++i) {
				if (Pkgs[i].PkgName == name) {
					return GetChildByIndex(i);
				}
			}
			return null;
		}

		public IEnumerable<string> GetChildNames() {
			for (int i = 0; i < Pkgs.Count; ++i) {
				yield return Pkgs[i].PkgName;
			}
		}

		public DuplicatableStream BuildPkg(int index, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian) {
			uint offset;
			List<(ulong offset, uint size)> files;
			var ms = BuildPkgInternal(index, e, out offset, out files);
			List<DuplicatableStream> streams = new List<DuplicatableStream>(files.Count + 1);
			streams.Add(ms.CopyToByteArrayStreamAndDispose());
			for (int i = 0; i < files.Count; ++i) {
				streams.Add(new HyoutaUtils.Streams.PartialStream(Stream, (long)files[i].offset, files[i].size));
			}
			return HyoutaUtils.Streams.ConcatenatedStream.CreateConcatenatedStream(streams);
		}

		public System.IO.MemoryStream BuildPkgToMemory(int index, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian) {
			uint offset;
			List<(ulong offset, uint size)> files;
			var ms = BuildPkgInternal(index, e, out offset, out files);
			ms.SetLength(offset);
			for (int i = 0; i < files.Count; ++i) {
				Stream.Position = (long)files[i].offset;
				StreamUtils.CopyStream(Stream, ms, files[i].size);
			}
			ms.Position = 0;
			return ms;
		}

		private System.IO.MemoryStream BuildPkgInternal(int index, EndianUtils.Endianness e, out uint offset, out List<(ulong offset, uint size)> files) {
			PkaPkgToHashData pkgHashData = Pkgs[index];

			offset = 8 + (uint)pkgHashData.Files.Count * 0x50;
			var header = new System.IO.MemoryStream((int)offset);
			header.WriteUInt32(0, e);
			header.WriteInt32(pkgHashData.Files.Count, e);
			files = new List<(ulong offset, uint size)>(pkgHashData.Files.Count);
			for (int i = 0; i < pkgHashData.Files.Count; ++i) {
				header.WriteString(Encoding.UTF8, pkgHashData.Files[i].Filename, 0x40);
				var file = FindFile(pkgHashData.Files[i].Hash);
				if (file == null) {
					throw new Exception("Invalid PKA.");
				}
				header.WriteUInt32(file.UncompressedSize, e);
				header.WriteUInt32(file.CompressedSize, e);
				header.WriteUInt32(offset, e);
				header.WriteUInt32(file.Flags, e);
				offset += file.CompressedSize;
				files.Add((file.Offset, file.CompressedSize));
			}
			return header;
		}

		private PkaHashToFileData FindFile(HyoutaUtils.Checksum.SHA256 hash) {
			return FindFile(Files, 0, Files.Count - 1, hash);
		}

		private static PkaHashToFileData FindFile(List<PkaHashToFileData> files, int left, int right, HyoutaUtils.Checksum.SHA256 hash) {
			if (left > right) {
				return null;
			}

			int mid = (int)(((uint)left + (uint)right) / 2);
			PkaHashToFileData c = files[mid];
			int cmp = HyoutaUtils.Checksum.SHA256.CompareSha256(hash, c.Hash);
			if (cmp < 0) {
				return FindFile(files, left, mid - 1, hash);
			} else if (cmp > 0) {
				return FindFile(files, mid + 1, right, hash);
			} else {
				return c;
			}
		}
	}

	internal class PkaPkgToHashData {
		public string PkgName;
		public List<PkaFileHashData> Files;

		public PkaPkgToHashData(DuplicatableStream stream, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian) {
			PkgName = stream.ReadSizedString(32, TextUtils.GameTextEncoding.UTF8).TrimNull();
			uint fileCount = stream.ReadUInt32(e);
			Files = new List<PkaFileHashData>((int)fileCount);
			for (uint i = 0; i < fileCount; ++i) {
				Files.Add(new PkaFileHashData(stream, e));
			}
		}
	}

	internal class PkaFileHashData {
		public string Filename;
		public HyoutaUtils.Checksum.SHA256 Hash;

		public PkaFileHashData(DuplicatableStream stream, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian) {
			Filename = stream.ReadSizedString(0x40, TextUtils.GameTextEncoding.UTF8).TrimNull();
			Hash = new HyoutaUtils.Checksum.SHA256(stream.ReadBytes(0x20));
		}
	}

	internal class PkaHashToFileData {
		public HyoutaUtils.Checksum.SHA256 Hash;
		public ulong Offset;
		public uint CompressedSize;
		public uint UncompressedSize;
		public uint Flags;

		public PkaHashToFileData(DuplicatableStream stream, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian) {
			Hash = new HyoutaUtils.Checksum.SHA256(stream.ReadBytes(0x20));
			Offset = stream.ReadUInt64(e);
			CompressedSize = stream.ReadUInt32(e);
			UncompressedSize = stream.ReadUInt32(e);
			Flags = stream.ReadUInt32(e);
		}
	}
}
