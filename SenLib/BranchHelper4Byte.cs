using HyoutaPluginBase;
using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class BranchHelper4Byte : IDisposable {
		private Stream Binary;
		private IRomMapper Mapper;
		private ulong? Target;
		private List<ulong> Sources;
		private bool Disposed;

		public BranchHelper4Byte(Stream binary, IRomMapper mapper) {
			Binary = binary;
			Mapper = mapper;
			Target = null;
			Sources = null;
			Disposed = false;
		}

		public void SetTarget(ulong target) {
			if (Target != null) {
				throw new Exception("target already set");
			}
			Target = target;
			if (Sources != null) {
				foreach (ulong s in Sources) {
					Commit(s);
				}
				Sources = null;
			}
		}

		public void AddSource(ulong source) {
			if (Target == null) {
				if (Sources == null) {
					Sources = new List<ulong>();
				}
				Sources.Add(source);
			} else {
				Commit(source);
			}
		}

		public void WriteJump5Byte(byte style) {
			ulong source = (ulong)(Binary.Position + 5);
			Binary.WriteUInt8(style);
			Binary.WriteInt32(-5, EndianUtils.Endianness.LittleEndian); // self-jump until set
			AddSource(Mapper.MapRomToRam(source - 4));
			Binary.Position = (long)source;
		}

		public void WriteJump6Byte(ushort style) {
			ulong source = (ulong)(Binary.Position + 6);
			Binary.WriteUInt16(style, EndianUtils.Endianness.BigEndian);
			Binary.WriteInt32(-6, EndianUtils.Endianness.LittleEndian); // self-jump until set
			AddSource(Mapper.MapRomToRam(source - 4));
			Binary.Position = (long)source;
		}

		private void Commit(ulong source) {
			long diff = (long)(Target.Value - (source + 4));
			if (diff < int.MinValue || diff > int.MaxValue) {
				throw new Exception("too far apart");
			}

			long p = Binary.Position;
			ulong s = Mapper.MapRamToRom(source);
			Binary.Position = (long)s;
			Binary.WriteInt32((int)diff, EndianUtils.Endianness.LittleEndian);

			Binary.Position = p;
		}

		public void Dispose() {
			Dispose(true);
			GC.SuppressFinalize(this);
		}

		protected virtual void Dispose(bool disposing) {
			if (Disposed) return;
			if (disposing) {
				if (Sources != null) {
					throw new Exception("Added a source but never set a target.");
				}
			}
			Disposed = true;
		}
	}
}
