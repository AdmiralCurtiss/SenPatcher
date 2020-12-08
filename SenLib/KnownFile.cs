using HyoutaPluginBase;
using HyoutaUtils.Checksum;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class KnownFile {
		public SHA1 Hash { get; private set; }
		public List<KnownFileAcquisitionMethod> AcquisitionMethods { get; private set; }
		public bool Important { get; private set; }

		public KnownFile(SHA1 hash, string path, bool important = true) {
			Hash = hash;
			AcquisitionMethods = new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile(path)
			};
			Important = important;
		}
		public KnownFile(SHA1 hash, string path1, string path2, bool important = true) {
			Hash = hash;
			AcquisitionMethods = new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile(path1),
				new KnownFileAcquisitionFromGamefile(path2)
			};
			Important = important;
		}
		public KnownFile(SHA1 hash, string path1, string path2, string path3, bool important = true) {
			Hash = hash;
			AcquisitionMethods = new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile(path1),
				new KnownFileAcquisitionFromGamefile(path2),
				new KnownFileAcquisitionFromGamefile(path3)
			};
			Important = important;
		}
		public KnownFile(SHA1 hash, DuplicatableStream data, bool important = true) {
			Hash = hash;
			AcquisitionMethods = new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromStream(data)
			};
			Important = important;
		}
		public KnownFile(SHA1 hash, KnownFileAcquisitionMethod method, bool important = true) {
			Hash = hash;
			AcquisitionMethods = new List<KnownFileAcquisitionMethod>() { method };
			Important = important;
		}
		public KnownFile(SHA1 hash, List<KnownFileAcquisitionMethod> methods, bool important = true) {
			Hash = hash;
			AcquisitionMethods = methods;
			Important = important;
		}
	}

	public interface KnownFileAcquisitionMethod { }

	public class KnownFileAcquisitionFromStream : KnownFileAcquisitionMethod {
		public DuplicatableStream Data { get; private set; }
		public KnownFileAcquisitionFromStream(DuplicatableStream data) {
			Data = data.Duplicate();
		}
	}

	public class KnownFileAcquisitionFromGamefile : KnownFileAcquisitionMethod {
		public string Path { get; private set; }
		public KnownFileAcquisitionFromGamefile(string path) {
			Path = path;
		}
	}

	public class KnownFileAcquisitionFromBpsPatch : KnownFileAcquisitionMethod {
		public SHA1 BasefileHash { get; private set; }
		public DuplicatableStream BpsData { get; private set; }
		public KnownFileAcquisitionFromBpsPatch(SHA1 basefileHash, DuplicatableStream data) {
			BasefileHash = basefileHash;
			BpsData = data.Duplicate();
		}
	}
}
