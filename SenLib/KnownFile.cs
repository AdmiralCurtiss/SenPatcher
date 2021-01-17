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

		public KnownFile(SHA1 hash, string path, bool important = true, bool writeToBackup = true) {
			Hash = hash;
			AcquisitionMethods = new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile(path, writeToBackup)
			};
			Important = important;
		}
		public KnownFile(SHA1 hash, string path1, string path2, bool important = true, bool writeToBackup = true) {
			Hash = hash;
			AcquisitionMethods = new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile(path1, writeToBackup),
				new KnownFileAcquisitionFromGamefile(path2, writeToBackup)
			};
			Important = important;
		}
		public KnownFile(SHA1 hash, string path1, string path2, string path3, bool important = true, bool writeToBackup = true) {
			Hash = hash;
			AcquisitionMethods = new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromGamefile(path1, writeToBackup),
				new KnownFileAcquisitionFromGamefile(path2, writeToBackup),
				new KnownFileAcquisitionFromGamefile(path3, writeToBackup)
			};
			Important = important;
		}
		public KnownFile(SHA1 hash, DuplicatableStream data, bool important = true, bool writeToBackup = true) {
			Hash = hash;
			AcquisitionMethods = new List<KnownFileAcquisitionMethod>() {
				new KnownFileAcquisitionFromStream(data, writeToBackup)
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
		public bool WriteToBackup { get; private set; }
		public KnownFileAcquisitionFromStream(DuplicatableStream data, bool writeToBackup = true) {
			Data = data.Duplicate();
			WriteToBackup = writeToBackup;
		}
	}

	public class KnownFileAcquisitionFromGamefile : KnownFileAcquisitionMethod {
		public string Path { get; private set; }
		public bool WriteToBackup { get; private set; }
		public KnownFileAcquisitionFromGamefile(string path, bool writeToBackup = true) {
			Path = path;
			WriteToBackup = writeToBackup;
		}
	}

	public class KnownFileAcquisitionFromBpsPatch : KnownFileAcquisitionMethod {
		public SHA1 BasefileHash { get; private set; }
		public DuplicatableStream BpsData { get; private set; }
		public bool WriteToBackup { get; private set; }
		public KnownFileAcquisitionFromBpsPatch(SHA1 basefileHash, DuplicatableStream data, bool writeToBackup = true) {
			BasefileHash = basefileHash;
			BpsData = data.Duplicate();
			WriteToBackup = writeToBackup;
		}
	}
}
