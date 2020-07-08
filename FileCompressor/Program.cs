using HyoutaUtils;
using SevenZip;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.NetworkInformation;
using System.Text;
using System.Threading.Tasks;

namespace FileCompressor {
	class Program {
		public enum Prefilter {
			None,
			Delta2LE_0x30Lead,
			Delta4LE_0x30Lead,
			Delta2LE_Deinterleaved_0x30Lead,
		}

		public enum Exhaustion {
			Standard,
			SemiExhaustive,
			Exhaustive,
		}

		static void Main(string[] args) {
			List<string> infiles = new List<string>();
			Prefilter filter = Prefilter.None;
			Exhaustion exhaustion = Exhaustion.Standard;

			for (int i = 0; i < args.Length; ++i) {
				if (args[i] == "--filter") {
					++i;
					filter = (Prefilter)Enum.Parse(typeof(Prefilter), args[i], true);
				} else if (args[i] == "--exhaustion") {
					++i;
					exhaustion = (Exhaustion)Enum.Parse(typeof(Exhaustion), args[i], true);
				} else {
					infiles.Add(args[i]);
				}
			}

			if (infiles.Count == 0) {
				Console.WriteLine("Usage: FileCompressor [options] files");
				Console.WriteLine("Options:");
				Console.WriteLine("  --filter");
				foreach (object e in Enum.GetValues(typeof(Prefilter))) {
					Console.WriteLine("      " + e.ToString());
				}
				Console.WriteLine("  --exhaustion");
				foreach (object e in Enum.GetValues(typeof(Exhaustion))) {
					Console.WriteLine("      " + e.ToString());
				}
			}

			foreach (string infile in infiles) {
				string outfile = infile + "_" + filter.ToString() + "_" + exhaustion.ToString() + ".bin";
				Console.WriteLine("Generating: " + outfile);

				Stream ms;
				using (var fs = new FileStream(infile, FileMode.Open, FileAccess.Read, FileShare.Read)) {
					ms = fs.CopyToMemory();
					fs.Close();
				}

				using (var compressed = Compress(ms, filter, exhaustion)) {
					using (FileStream os = new FileStream(outfile, FileMode.Create)) {
						compressed.Position = 0;
						StreamUtils.CopyStream(compressed, os);
					}
				}
			}
		}

		public static Stream Compress(Stream instream, Prefilter filter, Exhaustion exhaustion) {
			MemoryStream outstream = new MemoryStream((int)instream.Length);
			uint crc = CRC.CalculateDigest(instream.CopyToByteArray(), 0, (uint)instream.Length);

			if (filter == Prefilter.None) {
				MemoryStream memorystream = new MemoryStream((int)instream.Length);
				instream.Position = 0;
				StreamUtils.CopyStream(instream, memorystream);

				outstream.WriteUInt8(0); // filter ID
				outstream.WriteUInt32(crc, EndianUtils.Endianness.LittleEndian);
				CompressInternalLzmaSemiOptimized(memorystream, outstream, exhaustion);
				return outstream;
			}

			if (filter == Prefilter.Delta2LE_0x30Lead) {
				if ((instream.Length % 2) != 0 || instream.Length < 0x30) {
					throw new Exception("unsupported length for " + filter.ToString());
				}
				MemoryStream filteredstream = new MemoryStream((int)instream.Length);
				instream.Position = 0;
				StreamUtils.CopyStream(instream, filteredstream, 0x30);

				ushort last = 0;
				while (instream.Position < instream.Length) {
					ushort curr = instream.ReadUInt16(EndianUtils.Endianness.LittleEndian);
					ushort diff = (ushort)(last - curr);
					filteredstream.WriteUInt16(diff, EndianUtils.Endianness.LittleEndian);
					last = curr;
				}

				outstream.WriteUInt8(1); // filter ID
				outstream.WriteUInt32(crc, EndianUtils.Endianness.LittleEndian);
				filteredstream.Position = 0;
				CompressInternalLzmaSemiOptimized(filteredstream, outstream, exhaustion);
				return outstream;
			}

			if (filter == Prefilter.Delta4LE_0x30Lead) {
				if ((instream.Length % 4) != 0 || instream.Length < 0x30) {
					throw new Exception("unsupported length for " + filter.ToString());
				}
				MemoryStream filteredstream = new MemoryStream((int)instream.Length);
				instream.Position = 0;
				StreamUtils.CopyStream(instream, filteredstream, 0x30);

				uint last = 0;
				while (instream.Position < instream.Length) {
					uint curr = instream.ReadUInt32(EndianUtils.Endianness.LittleEndian);
					uint diff = (uint)(last - curr);
					filteredstream.WriteUInt32(diff, EndianUtils.Endianness.LittleEndian);
					last = curr;
				}

				outstream.WriteUInt8(2); // filter ID
				outstream.WriteUInt32(crc, EndianUtils.Endianness.LittleEndian);
				filteredstream.Position = 0;
				CompressInternalLzmaSemiOptimized(filteredstream, outstream, exhaustion);
				return outstream;
			}

			if (filter == Prefilter.Delta2LE_Deinterleaved_0x30Lead) {
				if ((instream.Length % 4) != 0 || instream.Length < 0x30) {
					throw new Exception("unsupported length for " + filter.ToString());
				}
				MemoryStream filteredstream = new MemoryStream((int)instream.Length);
				instream.Position = 0;
				StreamUtils.CopyStream(instream, filteredstream, 0x30);

				long pos = instream.Position;
				ushort last = 0;
				while (instream.Position < instream.Length) {
					ushort curr = instream.ReadUInt16(EndianUtils.Endianness.LittleEndian);
					ushort diff = (ushort)(last - curr);
					filteredstream.WriteUInt16(diff, EndianUtils.Endianness.LittleEndian);
					last = curr;
					instream.ReadUInt16(EndianUtils.Endianness.LittleEndian);
				}
				instream.Position = pos;
				last = 0;
				while (instream.Position < instream.Length) {
					instream.ReadUInt16(EndianUtils.Endianness.LittleEndian);
					ushort curr = instream.ReadUInt16(EndianUtils.Endianness.LittleEndian);
					ushort diff = (ushort)(last - curr);
					filteredstream.WriteUInt16(diff, EndianUtils.Endianness.LittleEndian);
					last = curr;
				}

				outstream.WriteUInt8(3); // filter ID
				outstream.WriteUInt32(crc, EndianUtils.Endianness.LittleEndian);
				filteredstream.Position = 0;
				CompressInternalLzmaSemiOptimized(filteredstream, outstream, exhaustion);
				return outstream;
			}

			throw new Exception("unexpected filter");
		}

		public static void CompressInternalLzmaSemiOptimized(Stream instream, Stream outstream, Exhaustion exhaustion) {
			int[] dictionary;
			int[] posStateBits;
			int[] litContextBits;
			int[] litPosBits;
			int[] numFastBytes;

			{
				List<int> dicts = new List<int>();
				for (int i = 0; i < 30; ++i) {
					int dictsize = 1 << i;
					dicts.Add(i);
					if (dictsize >= instream.Length) {
						break;
					}
				}
				dictionary = dicts.ToArray();
			}

			switch (exhaustion) {
				case Exhaustion.Standard:
					posStateBits = new int[] { 0, 1, 2, 4 };
					litContextBits = new int[] { 0, 1, 2, 3, 4, 8 };
					litPosBits = new int[] { 0, 1, 2, 4 };
					numFastBytes = new int[] { 16, 64, 128, 273 };
					break;
				case Exhaustion.SemiExhaustive:
					posStateBits = new int[] { 0, 1, 2, 3, 4 };
					litContextBits = new int[] { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
					litPosBits = new int[] { 0, 1, 2, 3, 4 };
					numFastBytes = new int[] { 5, 16, 32, 64, 96, 128, 192, 273 };
					break;
				case Exhaustion.Exhaustive: {
					posStateBits = new int[] { 0, 1, 2, 3, 4 };
					litContextBits = new int[] { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
					litPosBits = new int[] { 0, 1, 2, 3, 4 };
					List<int> tmpfb = new List<int>();
					for (int i = 5; i <= 273; ++i) {
						tmpfb.Add(i);
					}
					numFastBytes = tmpfb.ToArray();
					break;
				}
				default:
					throw new Exception("invalid exhaustion");
			}

			string mf = "bt4";

			Stream best = null;
			long bestlen = long.MaxValue;
			int bestd = 0;
			int bestpb = 0;
			int bestlc = 0;
			int bestlp = 0;
			int bestfb = 0;

			foreach (int d in dictionary) {
				foreach (int pb in posStateBits) {
					Console.WriteLine($"Reached d{d} pb{pb}.");
					foreach (int lc in litContextBits) {
						foreach (int lp in litPosBits) {
							foreach (int fb in numFastBytes) {
								MemoryStream tmp = new MemoryStream((int)instream.Length);
								instream.Position = 0;
								CompressInternalLzma(instream, tmp, 1 << d, pb, lc, lp, fb, mf);
								long tmplen = tmp.Length;
								if (best == null || bestlen > tmplen) {
									best = tmp;
									bestlen = tmplen;
									bestd = d;
									bestpb = pb;
									bestlc = lc;
									bestlp = lp;
									bestfb = fb;
									Console.WriteLine($"New best compression with {bestlen} bytes: d{d}, pb{pb}, lc{lc}, lp{lp}, fb{fb}");
								}
							}
						}
					}
				}
			}

			if (exhaustion != Exhaustion.Exhaustive) {
				// one final exhaustive fastbytes loop
				Console.WriteLine("Exhausting fastbytes with current best values...");
				for (int fbb = 5; fbb <= 273; ++fbb) {
					MemoryStream tmp = new MemoryStream((int)instream.Length);
					instream.Position = 0;
					CompressInternalLzma(instream, tmp, 1 << bestd, bestpb, bestlc, bestlp, fbb, mf);
					long tmplen = tmp.Length;
					if (best == null || bestlen > tmplen) {
						best = tmp;
						bestlen = tmplen;
						bestfb = fbb;
						Console.WriteLine($"New best compression with {bestlen} bytes: d{bestd}, pb{bestpb}, lc{bestlc}, lp{bestlp}, fb{fbb}");
					}
				}
			}

			best.Position = 0;
			StreamUtils.CopyStream(best, outstream);
		}

		public static void CompressInternalLzma(Stream inStream, Stream outStream, int dictionary, int posStateBits, int litContextBits, int litPosBits, int numFastBytes, string mf) {
			// somewhat copied from LzmaAlone
			int algorithm = 2;
			bool eos = false;

			CoderPropID[] propIDs = {
				CoderPropID.DictionarySize,
				CoderPropID.PosStateBits,
				CoderPropID.LitContextBits,
				CoderPropID.LitPosBits,
				CoderPropID.Algorithm,
				CoderPropID.NumFastBytes,
				CoderPropID.MatchFinder,
				CoderPropID.EndMarker
			};
			object[] properties = {
				dictionary,
				posStateBits,
				litContextBits,
				litPosBits,
				algorithm,
				numFastBytes,
				mf,
				eos
			};

			SevenZip.Compression.LZMA.Encoder encoder = new SevenZip.Compression.LZMA.Encoder();
			encoder.SetCoderProperties(propIDs, properties);
			encoder.WriteCoderProperties(outStream);
			outStream.WriteInt64(inStream.Length, EndianUtils.Endianness.LittleEndian);
			encoder.Code(inStream, outStream, -1, -1, null);
		}
	}
}
