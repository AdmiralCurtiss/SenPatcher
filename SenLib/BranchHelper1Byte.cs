using HyoutaPluginBase;
using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class BranchHelper1Byte : IDisposable {
		private Stream Binary;
		private IRomMapper Mapper;
		private ulong? Target;
		private List<ulong> Sources;
		private bool Disposed;

		public BranchHelper1Byte(Stream binary, IRomMapper mapper) {
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

		public void WriteJump(byte style) {
			ulong source = (ulong)(Binary.Position + 2);
			Binary.WriteUInt8(style);
			Binary.WriteInt8(-2); // self-jump until set
			AddSource(Mapper.MapRomToRam(source - 1));
			Binary.Position = (long)source;
		}

		private void Commit(ulong source) {
			long diff = (long)(Target.Value - (source + 1));
			if (diff < sbyte.MinValue || diff > sbyte.MaxValue) {
				throw new Exception("too far apart");
			}

			long p = Binary.Position;
			ulong s = Mapper.MapRamToRom(source);
			Binary.Position = (long)s;
			Binary.WriteInt8((sbyte)diff);

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
