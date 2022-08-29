using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class ScriptFunction {
		public string Name;
		public List<ScriptOp> Ops;
	}

	public class ScriptOp {
		public long Position;
		public string StringRep;
		public string DetailedString;
		public byte[] Bytes;
	}

	public static class ScriptParser {
		public static List<ScriptFunction> Parse(Stream s, bool isBook, Dictionary<ushort, string> voiceIds = null, EndianUtils.Endianness? endian = null, int sengame = 1, bool printDetailed = false) {
			EndianUtils.Endianness e = endian.HasValue ? endian.Value : (s.PeekUInt32(EndianUtils.Endianness.LittleEndian) == 0x20 ? EndianUtils.Endianness.LittleEndian : EndianUtils.Endianness.BigEndian);
			uint headerLength = s.ReadUInt32(e);
			if (headerLength != 0x20) {
				throw new Exception("unexpected header length");
			}

			uint namePosition = s.ReadUInt32(e);
			uint functionOffsetsPosition = s.ReadUInt32(e);
			uint functionOffsetsLength = s.ReadUInt32(e);
			uint functionNameOffsetsPosition = s.ReadUInt32(e);
			uint functionCount = s.ReadUInt32(e); // seems redundant between length and count...?
			if (functionCount * 4 != functionOffsetsLength) {
				throw new Exception("inconsistency"); // maybe one of the two means something else then?
			}
			uint functionMetadataEnd = s.ReadUInt32(e);
			uint unknown = s.ReadUInt32(e);

			string name = s.ReadAsciiNulltermFromLocationAndReset(namePosition);
			s.Position = functionOffsetsPosition;
			uint[] functionPositions = new uint[functionCount];
			for (long i = 0; i < functionCount; ++i) {
				functionPositions[i] = s.ReadUInt32(e);
			}
			s.Position = functionNameOffsetsPosition;
			ushort[] functionNamePositions = new ushort[functionCount];
			for (long i = 0; i < functionCount; ++i) {
				functionNamePositions[i] = s.ReadUInt16(e);
			}
			string[] functionNames = new string[functionCount];
			for (long i = 0; i < functionCount; ++i) {
				s.Position = functionNamePositions[i];
				functionNames[i] = s.ReadAsciiNullterm();
			}

			List<ScriptFunction> funcs = new List<ScriptFunction>();
			if (isBook) {
				for (long i = 0; i < functionCount; ++i) {
					bool is99 = functionNames[i].EndsWith("_99");
					if (is99) {
						s.Position = functionPositions[i];
						var r = ParseBookFunction99(s, (i + 1) == functionCount ? s.Length : functionPositions[i + 1], e);
						var ops = new List<ScriptOp>();
						ops.Add(new ScriptOp() { StringRep = r.a.ToString() });
						ops.Add(new ScriptOp() { StringRep = r.b.ToString() });
						funcs.Add(new ScriptFunction() { Name = functionNames[i], Ops = ops });
					} else {
						s.Position = functionPositions[i];
						var ops = ParseBookFunction(s, (i + 1) == functionCount ? s.Length : functionPositions[i + 1], e, voiceIds);
						funcs.Add(new ScriptFunction() { Name = functionNames[i], Ops = ops });
					}
				}
			} else {
				for (long i = 0; i < functionCount; ++i) {
					s.Position = functionPositions[i];
					var ops = ParseFunction(s, (i + 1) == functionCount ? s.Length : functionPositions[i + 1], e, voiceIds, sengame, printDetailed);
					funcs.Add(new ScriptFunction() { Name = functionNames[i], Ops = ops });
				}
			}

			return funcs;
		}

		private static List<ScriptOp> ParseBookFunction(Stream s, long end, EndianUtils.Endianness e, Dictionary<ushort, string> voiceIds) {
			var text = new List<ScriptOp>();
			short dataCounter = s.ReadInt16(e);
			for (int i = 0; i < dataCounter; ++i) {
				s.DiscardBytes(0x26);
			}
			List<byte> sb = new List<byte>();
			List<byte> contentbytes = new List<byte>();
			int stringbytecount = 0;
			ReadString(s, sb, contentbytes, ref stringbytecount, e, voiceIds);
			string str = Encoding.UTF8.GetString(sb.ToArray());
			foreach (string ssplit in str.Split(new string[] { "\\n" }, StringSplitOptions.None)) {
				text.Add(new ScriptOp() { StringRep = ssplit });
			}
			return text;
		}

		private static (ushort a, ushort b) ParseBookFunction99(Stream s, long end, EndianUtils.Endianness e) {
			// two u16s of unknown usage... perhaps page count?
			ushort a = s.ReadUInt16(e);
			ushort b = s.ReadUInt16(e);
			return (a, b);
		}

		private static List<ScriptOp> ParseFunction(Stream s, long end, EndianUtils.Endianness e, Dictionary<ushort, string> voiceIds, int sengame, bool printDetailed) {
			// actually parsing this is more work than i thought it would be, so just guess
			// this will have false positives but that's okay
			int textcommand = (sengame == 1 || sengame == 2) ? 0x1a : 0x24;
			var text = new List<ScriptOp>();
			while (s.Position < end) {
				byte a = s.ReadUInt8();
				if (a == textcommand) {
					List<byte> contentbytes = new List<byte>();
					contentbytes.Add(a);
					long originalPosition = s.Position;
					try {
						// two bytes speaker
						contentbytes.Add(s.ReadUInt8());
						contentbytes.Add(s.ReadUInt8());
						if (!(sengame == 1 || sengame == 2)) {
							contentbytes.Add(s.ReadUInt8());
							contentbytes.Add(s.ReadUInt8());
							contentbytes.Add(s.ReadUInt8());
							contentbytes.Add(s.ReadUInt8());
						}
						List<byte> sb = new List<byte>();
						int stringbytecount = 0;
						ReadString(s, sb, contentbytes, ref stringbytecount, e, voiceIds);
						if (stringbytecount >= 3) {
							string str = Encoding.UTF8.GetString(sb.ToArray());
							string detailedString = null;
							long commandStart = originalPosition - 1;
							long commandEnd = s.Position;
							if (printDetailed) {
								detailedString = PrintDetailedTextCommandForReplacement(s, commandStart, commandEnd);
							}
							text.Add(new ScriptOp() { StringRep = str, Position = commandStart, DetailedString = detailedString, Bytes = contentbytes.ToArray() });
						}

						s.Position = originalPosition;
					} catch (Exception) {
						s.Position = originalPosition;
					}
				}
			}
			return text;
		}

		private static string PrintDetailedTextCommandForReplacement(Stream s, long commandStart, long commandEnd) {
			StringBuilder text = new StringBuilder();

			long p = s.Position;
			long startAddress = commandStart & ~0xfL;
			long endAddress = HyoutaUtils.NumberUtils.Align(commandEnd, 0x10);
			text.AppendLine();
			text.AppendLine(string.Format("patcher.ReplacePartialCommand(0x{0:x}, 0x{1:x}, _location_, _length_, _data_);", commandStart, commandEnd - commandStart));
			text.AppendLine("----------  *0 *1 *2 *3 *4 *5 *6 *7 *8 *9 *a *b *c *d *e *f");
			long currentAddress = startAddress;
			s.Position = commandStart;
			while (currentAddress < endAddress) {
				int[] row = new int[16];
				for (int i = 0; i < 16; ++i) {
					if (currentAddress + i == s.Position && currentAddress + i < commandEnd) {
						row[i] = s.ReadByte();
					} else {
						row[i] = -1;
					}
				}

				StringBuilder sb = new StringBuilder();
				sb.AppendFormat("0x{0:x8} ", currentAddress);
				for (int i = 0; i < 16; ++i) {
					sb.Append(" ").Append(row[i] == -1 ? "  " : row[i].ToString("x2"));
				}
				sb.Append("  ");
				for (int i = 0; i < 16; ++i) {
					sb.Append(row[i] >= 0x20 && row[i] <= 0x7f ? (char)row[i] : ' ');
				}

				text.AppendLine(sb.ToString());
				currentAddress += 16;
			}

			text.AppendLine();
			text.AppendLine();

			s.Position = p;
			return text.ToString();
		}

		private static void ReadString(Stream s, List<byte> sb, List<byte> contentbytes, ref int stringbytecount, EndianUtils.Endianness e, Dictionary<ushort, string> voiceIds) {
			int lastlinestart = sb.Count;
			while (true) {
				byte next = s.ReadUInt8();
				contentbytes.Add(next);
				if (next < 0x20) {
					if (next == 0) {
						break;
					} else if (next == 0x01) {
						sb.Add((byte)'{');
						sb.Add((byte)'n');
						sb.Add((byte)'}');
						lastlinestart = sb.Count;
					} else if (next == 0x02) {
						sb.Add((byte)'{');
						sb.Add((byte)'f');
						sb.Add((byte)'}');
						lastlinestart = sb.Count;
					} else if (next == 0x10) {
						sb.Add((byte)'{');
						ushort v = s.PeekUInt16(e);
						contentbytes.Add(s.ReadUInt8());
						contentbytes.Add(s.ReadUInt8());
						foreach (char c in string.Format("0x10:{0:D5}", v)) {
							sb.Add((byte)c);
						}
						sb.Add((byte)'}');
					} else if (next == 0x11) {
						List<byte> tmp = new List<byte>();
						tmp.Add((byte)'{');
						uint v = s.PeekUInt32(e);
						contentbytes.Add(s.ReadUInt8());
						contentbytes.Add(s.ReadUInt8());
						contentbytes.Add(s.ReadUInt8());
						contentbytes.Add(s.ReadUInt8());
						string voiceclip;
						if (voiceIds != null && voiceIds.TryGetValue((ushort)v, out voiceclip)) {
							foreach (char c in "0x11:") {
								tmp.Add((byte)c);
							}
							foreach (byte b in Encoding.UTF8.GetBytes(voiceclip)) {
								tmp.Add(b);
							}
						} else {
							foreach (char c in string.Format("0x11:{0:D5}", v)) {
								tmp.Add((byte)c);
							}
						}
						tmp.Add((byte)'}');
						sb.InsertRange(lastlinestart, tmp);
						lastlinestart += tmp.Count;
					} else if (next == 0x12) {
						sb.Add((byte)'{');
						uint v = s.PeekUInt32(e);
						contentbytes.Add(s.ReadUInt8());
						contentbytes.Add(s.ReadUInt8());
						contentbytes.Add(s.ReadUInt8());
						contentbytes.Add(s.ReadUInt8());
						foreach (char c in string.Format("0x12:{0:D5}", v)) {
							sb.Add((byte)c);
						}
						sb.Add((byte)'}');
					}
				} else {
					//if (next == 0x23) {
					//	for (int i = 0; i < 2; ++i) {
					//		next = s.ReadUInt8();
					//		if (next == 0) {
					//			break;
					//		}
					//		// there's also some special case with 0x4b here, let's see if we can ignore this...
					//	}
					//} else {
					sb.Add(next);
					++stringbytecount;
					//}
				}
			}
		}

		/*
		private static List<string> ParseFunction(Stream s, long end) {
			List<string> ops = new List<string>();
			while (s.Position < end) {
				byte op = s.ReadUInt8();
				switch (op) {
					case 0x00: {
							// calls something else too
							ops.Add("command 0x00");
							break;
						}
					case 0x01: {
							ops.Add("return");
							break;
						}
					case 0x02: {
							byte flags = s.ReadUInt8(); // ?
							if (flags == 0x0B) {
								string func = s.ReadAsciiNullterm();
								ops.Add(string.Format("call local {0}", func));
							} else if (flags == 0x00) {
								string func = s.ReadAsciiNullterm();
								ops.Add(string.Format("call global {0}", func));
							} else if (flags == 0x00) {
								throw new Exception(string.Format("unknown flag {0:x2} in command 0x02", flags));
							}
							break;
						}
					case 0x03: {
							uint location = s.ReadUInt32();
							ops.Add(string.Format("jmp 0x{0:x8}", location));
							break;
						}
					case 0x05: {
							// this seems to be a *pretty damn wild* conditional jump statement
							// with a lot of options and multiple potential targets
							// we have to parse this in order to know how many bytes this statement consumes...
							HandleCommand05(s, ops);
							break;
						}
					case 0x09: {
							ops.Add("nop");
							break;
						}
					case 0x0a: {
							s.ReadUInt64();
							ops.Add("command 0x0a");
							break;
						}
					case 0x16: {
							s.ReadUInt8();
							ops.Add("command 0x16");
							break;
						}
					case 0x1e: {
							string a = s.ReadAsciiNullterm();
							string b = s.ReadAsciiNullterm();
							ops.Add(string.Format("call other script? ({0}) ({1})", a, b));
							break;
						}
					case 0x59: {
							s.ReadUInt16();
							ops.Add("command 0x59");
							break;
						}
					case 0x86: {
							byte subcommand = s.ReadUInt8();
							if (subcommand == 0x00) {
								s.ReadUInt32();
								ops.Add("command 0x8d/00");
							} else if (subcommand == 0x01) {
								s.ReadUInt32();
								ops.Add("command 0x8d/01");
							} else {
								ops.Add("command 0x8d/??");
							}
							break;
						}
					default:
						throw new Exception(string.Format("unknown command {0:x2}", op));
				}
			}

			return ops;
		}

		private static void HandleCommand05(Stream s, List<string> ops) {
			long extra_data = 0;
			while (true) {
				byte b = s.ReadUInt8();
				switch (b) {
					case 0x00:
						throw new Exception();
						break;
					case 0x01:
						goto exit_0x05;
					case 0x08:
						if (extra_data == 0) {
							extra_data = 1;
						} else {
							extra_data = 0;
						}
						break;
					case 0x1c:
						s.ReadUInt32(); // i think? this piece of code is very confusing
						break;
					case 0x1e:
						s.ReadUInt16();
						break;
					case 0x1f:
					case 0x20:
					case 0x23:
						s.ReadUInt8();
						break;
					case 0x21:
						s.ReadUInt16();
						s.ReadUInt8();
						break;
					default:
						break;
				}
			}
		exit_0x05:
			ops.Add(string.Format("command 0x05"));
		}
		*/

		public static List<ScriptFunction> ParseFull(HyoutaPluginBase.DuplicatableStream bytestream, string voicetablefilename, int sengame, EndianUtils.Endianness endian, TextUtils.GameTextEncoding encoding, bool printDetailed) {
			Dictionary<ushort, string> byIndex = new Dictionary<ushort, string>();
			if (voicetablefilename != null) {
				if (sengame == 1) {
					var tbl = new SenLib.Sen1.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
					foreach (var e in tbl.Entries) {
						var vd = new SenLib.Sen1.FileFixes.VoiceData(e.Data, endian, encoding);
						if (!byIndex.ContainsKey(vd.Index)) {
							byIndex.Add(vd.Index, vd.Name);
						}
					}
				} else if (sengame == 2) {
					var tbl = new SenLib.Sen2.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
					foreach (var e in tbl.Entries) {
						// tbl header is different in CS2 but voice data payload is the same as CS1
						var vd = new SenLib.Sen1.FileFixes.VoiceData(e.Data, endian, encoding);
						if (!byIndex.ContainsKey(vd.Index)) {
							byIndex.Add(vd.Index, vd.Name);
						}
					}
				} else if (sengame == 3) {
					var tbl = new SenLib.Sen3.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
					foreach (var e in tbl.Entries) {
						var vd = new SenLib.Sen3.VoiceDataCS3(e.Data, endian, encoding);
						if (!byIndex.ContainsKey(vd.Index)) {
							byIndex.Add(vd.Index, vd.Name);
						}
					}
				} else if (sengame == 4) {
					var tbl = new SenLib.Sen4.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
					foreach (var e in tbl.Entries) {
						var vd = new SenLib.Sen4.VoiceDataCS4(e.Data, endian, encoding);
						if (!byIndex.ContainsKey(vd.Index)) {
							byIndex.Add(vd.Index, vd.Name);
						}
					}
				}
			}

			return Parse(bytestream, false, byIndex, endian, sengame, printDetailed);
		}
	}
}