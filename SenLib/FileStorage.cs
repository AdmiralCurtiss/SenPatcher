using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Checksum;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class FileStorage {
		private struct FileData {
			public DuplicatableStream Data;
		}

		private Dictionary<SHA1, FileData> Map = new Dictionary<SHA1, FileData>();

		public void Add(DuplicatableStream stream) {
			using (var s = stream.Duplicate()) {
				Add(HyoutaUtils.ChecksumUtils.CalculateSHA1ForEntireStream(s), s);
			}
		}

		public void Add(SHA1 hash, DuplicatableStream stream) {
			if (Map.ContainsKey(hash)) {
				using (var newdata = stream.Duplicate())
				using (var olddata = Map[hash].Data.Duplicate()) {
					if (!StreamUtils.IsIdentical(newdata, olddata)) {
						throw new Exception("Storage already contains a different file for the given hash.");
					}
				}
				return;
			}

			FileData fd = new FileData() { Data = stream.Duplicate() };
			Map.Add(hash, fd);
		}

		public DuplicatableStream TryGetDuplicate(SHA1 hash) {
			FileData fd;
			if (Map.TryGetValue(hash, out fd)) {
				return fd.Data.Duplicate();
			}
			return null;
		}

		public void WriteToHyoutaArchive(System.IO.Stream target) {
			List<HyoutaUtils.HyoutaArchive.HyoutaArchiveFileInfo> files = new List<HyoutaUtils.HyoutaArchive.HyoutaArchiveFileInfo>();
			foreach (var kvp in Map) {
				HyoutaUtils.HyoutaArchive.HyoutaArchiveFileInfo file = new HyoutaUtils.HyoutaArchive.HyoutaArchiveFileInfo();
				file.Data = kvp.Value.Data.Duplicate();
				file.sha1 = kvp.Key;
				files.Add(file);
			}
			HyoutaUtils.HyoutaArchive.HyoutaArchiveChunk.Pack(target, files, 0, EndianUtils.Endianness.LittleEndian, null, new HyoutaUtils.HyoutaArchive.Compression.DeflateSharpCompressionInfo(0));
		}

		public class InitReturnValue {
			public FileStorage Storage;
			public List<(KnownFile file, List<string> errors)> Errors;
			public bool NewFileFound;
		}

		public static InitReturnValue InitializeFromKnownFiles(string basePath, KnownFile[] knownFiles, HyoutaUtils.HyoutaArchive.HyoutaArchiveContainer existingBackupArchive) {
			FileStorage storage = new FileStorage();
			List<(KnownFile file, List<string> errors)> errors = new List<(KnownFile file, List<string> errors)>();

			HashSet<SHA1> expectedHashes = new HashSet<SHA1>();
			foreach (KnownFile knownFile in knownFiles) {
				expectedHashes.Add(knownFile.Hash);
			}

			Dictionary<SHA1, DuplicatableStream> existingBackupArchiveFiles = new Dictionary<SHA1, DuplicatableStream>();
			if (existingBackupArchive != null) {
				for (long i = 0; i < existingBackupArchive.Filecount; ++i) {
					using (var file = existingBackupArchive.GetChildByIndex(i).AsFile)
					using (var filestream = file.DataStream.Duplicate()) {
						var hash = ChecksumUtils.CalculateSHA1ForEntireStream(filestream);
						if (expectedHashes.Contains(hash)) {
							existingBackupArchiveFiles.Add(hash, filestream.CopyToByteArrayStreamAndDispose());
						}
					}
				}
			}

			bool newFileFound = false;
			foreach (KnownFile knownFile in knownFiles) {
				DuplicatableStream existingStream;
				if (existingBackupArchiveFiles.TryGetValue(knownFile.Hash, out existingStream)) {
					storage.Add(knownFile.Hash, existingStream);
					continue;
				}

				List<string> localErrors = new List<string>();
				bool success = false;
				foreach (KnownFileAcquisitionMethod acquisitionMethod in knownFile.AcquisitionMethods) {
					try {
						if (acquisitionMethod is KnownFileAcquisitionFromStream) {
							using (var stream = (acquisitionMethod as KnownFileAcquisitionFromStream).Data.Duplicate()) {
								if (ChecksumUtils.CalculateSHA1ForEntireStream(stream) == knownFile.Hash) {
									storage.Add(knownFile.Hash, stream);
									success = true;
									newFileFound = true;
									break;
								} else {
									// this should never happen...
									localErrors.Add("Internal stream seems to be corrupted.");
									continue;
								}
							}
						} else if (acquisitionMethod is KnownFileAcquisitionFromGamefile) {
							string filename = (acquisitionMethod as KnownFileAcquisitionFromGamefile).Path;
							using (var stream = new HyoutaUtils.Streams.DuplicatableFileStream(System.IO.Path.Combine(basePath, filename))) {
								if (ChecksumUtils.CalculateSHA1ForEntireStream(stream) == knownFile.Hash) {
									storage.Add(knownFile.Hash, stream.CopyToByteArrayStreamAndDispose());
									success = true;
									newFileFound = true;
									break;
								} else {
									localErrors.Add(string.Format("File {0} does not match expected hash.", filename));
									continue;
								}
							}
						} else if (acquisitionMethod is KnownFileAcquisitionFromBpsPatch) {
							var method = acquisitionMethod as KnownFileAcquisitionFromBpsPatch;
							using (var basestream = storage.TryGetDuplicate(method.BasefileHash)) {
								if (basestream != null) {
									using (var bps = method.BpsData.Duplicate())
									using (var target = new System.IO.MemoryStream()) {
										HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(basestream, bps, target);
										if (ChecksumUtils.CalculateSHA1ForEntireStream(target) == knownFile.Hash) {
											storage.Add(knownFile.Hash, target.CopyToByteArrayStreamAndDispose());
											success = true;
											newFileFound = true;
											break;
										} else {
											// this is very unlikely, this means the basefile hase a matching hash but is not the expected file
											localErrors.Add("Patch did not apply correctly.");
											continue;
										}
									}
								} else {
									localErrors.Add(string.Format("Couldn't find base file with hash {0}.", method.BasefileHash));
									continue;
								}
							}
						} else {
							// shouldn't happen
							localErrors.Add("Internal error.");
							continue;
						}
					} catch (Exception ex) {
						// something happened, go to the next method and keep the exception message for now
						localErrors.Add(ex.Message);
					}
				}

				if (!success && knownFile.Important) {
					if (localErrors.Count == 0) {
						localErrors.Add(string.Format("Failed to find unmodified copy of game file with hash {0}.", knownFile.Hash));
					}
					errors.Add((knownFile, localErrors));
				}
			}

			return new InitReturnValue() { Storage = storage, Errors = errors, NewFileFound = newFileFound };
		}
	}
}
