using HyoutaUtils.Checksum;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class UnpatchFile {
		public SHA1 Hash { get; private set; }
		public SHA1? AltHash { get; private set; }
		public string Path { get; private set; }

		public UnpatchFile(SHA1 hash, string path) {
			Hash = hash;
			AltHash = null;
			Path = path;
		}

		public UnpatchFile(SHA1 hash, SHA1 altHash, string path) {
			Hash = hash;
			AltHash = altHash;
			Path = path;
		}
	}
}
