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
			public DuplicatableStream Data { get; private set; }
			public bool WriteToBackup;

			public FileData(DuplicatableStream data, bool writeToBackup) {
				this.Data = data;
				this.WriteToBackup = writeToBackup;
			}
		}

		private Dictionary<SHA1, FileData> Map = new Dictionary<SHA1, FileData>();

		public void Add(DuplicatableStream stream, bool writeToBackup) {
			using (var s = stream.Duplicate()) {
				Add(HyoutaUtils.ChecksumUtils.CalculateSHA1ForEntireStream(s), s, writeToBackup);
			}
		}

		public void Add(SHA1 hash, DuplicatableStream stream, bool writeToBackup) {
			FileData ofd;
			if (Map.TryGetValue(hash, out ofd)) {
				using (var newdata = stream.Duplicate())
				using (var olddata = ofd.Data.Duplicate()) {
					if (!StreamUtils.IsIdentical(newdata, olddata)) {
						throw new Exception("Storage already contains a different file for the given hash.");
					}
				}
				if (writeToBackup) {
					ofd.WriteToBackup = true;
				}
				return;
			}

			FileData fd = new FileData(stream.Duplicate(), writeToBackup);
			Map.Add(hash, fd);
		}

		public DuplicatableStream TryGetDuplicate(SHA1 hash) {
			FileData fd;
			if (Map.TryGetValue(hash, out fd)) {
				return fd.Data.Duplicate();
			}
			return null;
		}

		public bool Contains(SHA1 hash) {
			return Map.ContainsKey(hash);
		}

		public void WriteToHyoutaArchive(System.IO.Stream target) {
			List<HyoutaUtils.HyoutaArchive.HyoutaArchiveFileInfo> files = new List<HyoutaUtils.HyoutaArchive.HyoutaArchiveFileInfo>();
			foreach (var kvp in Map) {
				if (!kvp.Value.WriteToBackup) {
					continue;
				}

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
			public bool ShouldWriteBackupArchive;
		}

		public static InitReturnValue InitializeFromKnownFiles(string basePath, KnownFile[] knownFiles, HyoutaUtils.HyoutaArchive.HyoutaArchiveContainer existingBackupArchive) {
			FileStorage storage = new FileStorage();
			List<(KnownFile file, List<string> errors)> errors = new List<(KnownFile file, List<string> errors)>();

			if (existingBackupArchive != null) {
				SenLib.Logging.Log(string.Format("Existing backup archive found, contains {0} files.", existingBackupArchive.Filecount));
				for (long i = 0; i < existingBackupArchive.Filecount; ++i) {
					using (var file = existingBackupArchive.GetChildByIndex(i).AsFile)
					using (var filestream = file.DataStream.Duplicate()) {
						var hash = ChecksumUtils.CalculateSHA1ForEntireStream(filestream);
						SenLib.Logging.Log(string.Format("Already contains file: {0}", hash.ToString()));
						storage.Add(hash, filestream.CopyToByteArrayStreamAndDispose(), true);
					}
				}
			}

			SenLib.Logging.Log("Looking for known files...");
			bool shouldWriteBackupArchive = false;
			foreach (KnownFile knownFile in knownFiles) {
				if (storage.Contains(knownFile.Hash)) {
					SenLib.Logging.Log(string.Format("File {0} already in archive, no need to search.", knownFile.Hash.ToString()));
					continue;
				}

				SenLib.Logging.Log(string.Format("File {0} not in archive...", knownFile.Hash.ToString()));
				List<string> localErrors = new List<string>();
				bool success = false;
				foreach (KnownFileAcquisitionMethod acquisitionMethod in knownFile.AcquisitionMethods) {
					try {
						if (acquisitionMethod is KnownFileAcquisitionFromStream) {
							SenLib.Logging.Log("Trying acquisition from stream...");
							var method = acquisitionMethod as KnownFileAcquisitionFromStream;
							using (var stream = method.Data.Duplicate()) {
								if (ChecksumUtils.CalculateSHA1ForEntireStream(stream) == knownFile.Hash) {
									SenLib.Logging.Log(string.Format("Acquired {0} from stream!", knownFile.Hash.ToString()));
									storage.Add(knownFile.Hash, stream, method.WriteToBackup);
									success = true;
									if (method.WriteToBackup) {
										SenLib.Logging.Log("Marking file to write to backup.");
										shouldWriteBackupArchive = true;
									}
									break;
								} else {
									// this should never happen...
									SenLib.Logging.Log("Internal stream seems to be corrupted.");
									localErrors.Add("Internal stream seems to be corrupted.");
									continue;
								}
							}
						} else if (acquisitionMethod is KnownFileAcquisitionFromGamefile) {
							var method = acquisitionMethod as KnownFileAcquisitionFromGamefile;
							string filename = method.Path;
							string path = System.IO.Path.Combine(basePath, filename);
							SenLib.Logging.Log(string.Format("Trying acquisition from file at {0}...", path));
							if (System.IO.File.Exists(path)) {
								using (var stream = new HyoutaUtils.Streams.DuplicatableFileStream(path)) {
									DuplicatableStream usedStream = stream;
									if (usedStream.Length <= 100 * 1024 * 1024) {
										usedStream = usedStream.CopyToByteArrayStreamAndDispose();
									}
									SHA1 filesha = ChecksumUtils.CalculateSHA1ForEntireStream(usedStream);
									if (filesha == knownFile.Hash) {
										SenLib.Logging.Log(string.Format("Acquired {0} from file!", knownFile.Hash.ToString()));
										storage.Add(knownFile.Hash, usedStream.CopyToByteArrayStreamAndDispose(), method.WriteToBackup);
										success = true;
										if (method.WriteToBackup) {
											SenLib.Logging.Log("Marking file to write to backup.");
											shouldWriteBackupArchive = true;
										}
										break;
									} else {
										string error = string.Format("File {0} does not match expected hash (should be {1} but is {2}).", filename, knownFile.Hash.ToString(), filesha.ToString());
										localErrors.Add(error);
										SenLib.Logging.Log(error);
										continue;
									}
								}
							} else {
								string error = string.Format("File {0} does not exist.", filename);
								localErrors.Add(error);
								SenLib.Logging.Log(error);
								continue;
							}
						} else if (acquisitionMethod is KnownFileAcquisitionFromBpsPatch) {
							var method = acquisitionMethod as KnownFileAcquisitionFromBpsPatch;
							SenLib.Logging.Log(string.Format("Trying acquisition via patch from {0}...", method.BasefileHash));
							using (var basestream = storage.TryGetDuplicate(method.BasefileHash)) {
								if (basestream != null) {
									using (var bps = method.BpsData.Duplicate())
									using (var target = new System.IO.MemoryStream()) {
										HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(basestream, bps, target);
										if (ChecksumUtils.CalculateSHA1ForEntireStream(target) == knownFile.Hash) {
											SenLib.Logging.Log(string.Format("Acquired {0} via patch!", knownFile.Hash.ToString()));
											storage.Add(knownFile.Hash, target.CopyToByteArrayStreamAndDispose(), method.WriteToBackup);
											success = true;
											if (method.WriteToBackup) {
												SenLib.Logging.Log("Marking file to write to backup.");
												shouldWriteBackupArchive = true;
											}
											break;
										} else {
											// this is very unlikely, this means the basefile hase a matching hash but is not the expected file
											localErrors.Add("Patch did not apply correctly.");
											SenLib.Logging.Log("Patch did not apply correctly.");
											continue;
										}
									}
								} else {
									string error = string.Format("Couldn't find base file with hash {0}.", method.BasefileHash);
									localErrors.Add(error);
									SenLib.Logging.Log(error);
									continue;
								}
							}
						} else if (acquisitionMethod is KnownFileAcquisitionMethodFunction) {
							var method = acquisitionMethod as KnownFileAcquisitionMethodFunction;
							SenLib.Logging.Log("Trying acquisition via custom function...");
							if (method.Exec(storage, localErrors, ref shouldWriteBackupArchive)) {
								success = true;
								break;
							} else {
								continue;
							}
						} else {
							// shouldn't happen
							SenLib.Logging.Log("Unknown acquisition type.");
							localErrors.Add("Internal error.");
							continue;
						}
					} catch (Exception ex) {
						// something happened, go to the next method and keep the exception message for now
						SenLib.Logging.Log(ex.ToString());
						localErrors.Add(ex.Message);
					}
				}

				if (!success && knownFile.Important) {
					string error = string.Format("Failed to find unmodified copy of game file with hash {0}.", knownFile.Hash);
					SenLib.Logging.Log(error);
					if (localErrors.Count == 0) {
						localErrors.Add(error);
					}
					errors.Add((knownFile, localErrors));
				}
			}

			return new InitReturnValue() { Storage = storage, Errors = errors, ShouldWriteBackupArchive = shouldWriteBackupArchive };
		}
	}
}
