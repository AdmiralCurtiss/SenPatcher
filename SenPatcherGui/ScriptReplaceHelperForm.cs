using HyoutaUtils;
using HyoutaUtils.Checksum;
using SenLib;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SenPatcherGui {
	public partial class ScriptReplaceHelperForm : Form {
		private List<ScriptFile> Files = new List<ScriptFile>();

		public ScriptReplaceHelperForm() {
			InitializeComponent();

			comboBoxGame.Items.Add("CS1");
			comboBoxGame.Items.Add("CS2");
			comboBoxGame.Items.Add("CS3");
			comboBoxGame.Items.Add("CS4");
			comboBoxGame.SelectedIndex = 2;
			comboBoxEncoding.Items.Add("UTF8");
			comboBoxEncoding.Items.Add("ShiftJIS");
			comboBoxEncoding.SelectedIndex = 0;
			comboBoxEndian.Items.Add("Little Endian");
			comboBoxEndian.Items.Add("Big Endian");
			comboBoxEndian.SelectedIndex = 0;
		}

		private void SearchIn(List<SearchMatch> matches, List<ScriptFunction> funcs, List<byte> rawsearch, List<byte> search, List<byte> replace, int offset) {
			foreach (var func in funcs) {
				foreach (var op in func.Ops) {
					for (int i = 0; i < op.Bytes.Length - rawsearch.Count - 1; ++i) {
						if (op.Bytes[i] == rawsearch[0] && ArrayUtils.IsByteArrayPartEqual(op.Bytes, i, rawsearch, 0, rawsearch.Count)) {
							matches.Add(new SearchMatch() { Op = op, Offset = offset + i });
						}
					}
				}
			}
		}

		private void buttonGo_Click(object sender, EventArgs e) {
			TextUtils.GameTextEncoding encoding = comboBoxEndian.SelectedIndex != 0 ? TextUtils.GameTextEncoding.ShiftJIS : TextUtils.GameTextEncoding.UTF8;
			List<byte> rawsearch = Encode(textBoxSearch.Text, encoding);
			List<byte> search = Encode(textBoxSearch.Text, encoding);
			List<byte> replace = Encode(textBoxReplace.Text, encoding);
			int offset = StripCommonPrefix(search, replace);
			StripCommonPostfix(search, replace);

			if (search.Count == 0 && replace.Count == 0) {
				textBoxOutput.Text = "[identical search/replace]";
				return;
			}

			if (Files.Count == 0) {
				string inputfilename = textBoxFile.Text;
				RunSearch(new List<ScriptFile>() { GetScriptFile(inputfilename) }, encoding, rawsearch, search, replace, offset);
			} else {
				RunSearch(Files, encoding, rawsearch, search, replace, offset);
			}
		}

		private void RunSearch(List<ScriptFile> scriptFiles, TextUtils.GameTextEncoding encoding, List<byte> rawsearch, List<byte> search, List<byte> replace, int offset) {
			int sengame = comboBoxGame.SelectedIndex + 1;
			StringBuilder sb = new StringBuilder();
			List<SearchMatch> matches = new List<SearchMatch>();
			foreach (ScriptFile scriptFile in scriptFiles) {
				var inputfilename = scriptFile.Filename;
				var sha1 = scriptFile.Hash;
				var funcs = scriptFile.Functions;

				matches.Clear();
				SearchIn(matches, funcs, rawsearch, search, replace, offset);
				if (matches.Count == 0) {
					continue;
				}

				sb.AppendLine("using HyoutaUtils;");
				sb.AppendLine("using HyoutaUtils.Streams;");
				sb.AppendLine("using System;");
				sb.AppendLine("using System.Collections.Generic;");
				sb.AppendLine("using System.Linq;");
				sb.AppendLine("using System.Text;");
				sb.AppendLine("using System.Threading.Tasks;");
				sb.AppendLine();
				sb.AppendFormat("namespace SenLib.Sen{0}.FileFixes {1}", sengame, "{");
				sb.AppendLine();
				sb.AppendFormat("\tclass {0} : FileMod {1}", Path.GetFileName(inputfilename).Replace(".", "_"), "{");
				sb.AppendLine();
				sb.AppendLine("\t\tpublic string GetDescription() {");
				sb.AppendLine("\t\t\t//return \"\";");
				sb.AppendLine("\t\t}");
				sb.AppendLine();
				sb.AppendLine("\t\tpublic IEnumerable<FileModResult> TryApply(FileStorage storage) {");
				sb.AppendFormat("\t\t\tvar file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1({0}));", GetChecksumString(sha1));
				sb.AppendLine();
				sb.AppendLine("\t\t\tif (file == null) {");
				sb.AppendLine("\t\t\t\treturn null;");
				sb.AppendLine("\t\t\t}");
				sb.AppendLine();
				sb.AppendLine("\t\t\t// UNTESTED");
				sb.AppendLine("\t\t\tvar bin = file.CopyToMemory();");
				sb.AppendLine("\t\t\tvar patcher = new SenScriptPatcher(bin);");
				sb.AppendLine();

				foreach (var match in matches) {
					var op = match.Op;
					if (search.Count == 0) {
						sb.AppendFormat(
							"\t\t\t//patcher.ExtendPartialCommand(0x{0:x}, 0x{1:x}, 0x{2:x}, {3});",
							op.Position,
							op.Bytes.Length,
							op.Position + match.Offset,
							NewByteArrayString(replace)
						);
						sb.AppendLine();
					} else if (replace.Count == 0) {
						sb.AppendFormat(
							"\t\t\t//patcher.RemovePartialCommand(0x{0:x}, 0x{1:x}, 0x{2:x}, 0x{3:x});",
							op.Position,
							op.Bytes.Length,
							op.Position + match.Offset,
							search.Count
						);
						sb.AppendLine();
					} else {
						sb.AppendFormat(
							"\t\t\t//patcher.ReplacePartialCommand(0x{0:x}, 0x{1:x}, 0x{2:x}, 0x{3:x}, {4});",
							op.Position,
							op.Bytes.Length,
							op.Position + match.Offset,
							search.Count,
							NewByteArrayString(replace)
						);
						sb.AppendLine();
					}
					if (search.Count == replace.Count) {
						sb.AppendFormat("\t\t\t//bin.Position = 0x{0:x};", op.Position + match.Offset);
						sb.AppendLine();
						sb.AppendFormat("\t\t\t//bin.Write({0});", NewByteArrayString(replace));
						sb.AppendLine();
					}
				}

				sb.AppendLine();
				sb.AppendFormat("\t\t\treturn new FileModResult[] {1} new FileModResult(\"{0}\", bin) {2};", GetRelativePath(inputfilename), "{", "}");
				sb.AppendLine();
				sb.AppendLine("\t\t}");
				sb.AppendLine();
				sb.AppendLine("\t\tpublic IEnumerable<FileModResult> TryRevert(FileStorage storage) {");
				sb.AppendFormat("\t\t\tvar file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1({0}));", GetChecksumString(sha1));
				sb.AppendLine();
				sb.AppendLine("\t\t\tif (file == null) {");
				sb.AppendLine("\t\t\t\treturn null;");
				sb.AppendLine("\t\t\t}");
				sb.AppendFormat("\t\t\treturn new FileModResult[] {1} new FileModResult(\"{0}\", file) {2};", GetRelativePath(inputfilename), "{", "}");
				sb.AppendLine();
				sb.AppendLine("\t\t}");
				sb.AppendLine("\t}");
				sb.AppendLine("}");
				sb.AppendLine();
				sb.AppendLine();
				sb.AppendLine();
				sb.AppendLine("====================================================");
			}
			textBoxOutput.Text = sb.ToString();
		}

		private object GetRelativePath(string inputfilename) {
			string fw = inputfilename.Replace("\\", "/");
			int dataidx = fw.LastIndexOf("/data/");
			if (dataidx != -1) {
				fw = fw.Substring(dataidx + 1);
			}
			return fw;
		}

		private object GetChecksumString(HyoutaUtils.Checksum.SHA1 sha1) {
			var sha1bytes = sha1.Value;
			StringBuilder sb = new StringBuilder();
			sb.Append("0x");
			for (int bidx = 0; bidx < 8; ++bidx) {
				sb.Append(sha1bytes[bidx].ToString("x2"));
			}
			sb.Append("ul, 0x");
			for (int bidx = 8; bidx < 16; ++bidx) {
				sb.Append(sha1bytes[bidx].ToString("x2"));
			}
			sb.Append("ul, 0x");
			for (int bidx = 16; bidx < 20; ++bidx) {
				sb.Append(sha1bytes[bidx].ToString("x2"));
			}
			sb.Append("u");
			return sb.ToString();
		}

		private string NewByteArrayString(List<byte> replace) {
			StringBuilder sb = new StringBuilder();
			sb.Append("new byte[] {");
			for (int i = 0; i < replace.Count; ++i) {
				if (i != 0) {
					sb.Append(", ");
				}
				sb.AppendFormat("0x{0:x2}", replace[i]);
			}
			sb.Append("}");
			return sb.ToString();
		}

		private int StripCommonPrefix(List<byte> search, List<byte> replace) {
			int i = 0;
			for (; i < Math.Min(search.Count, replace.Count); ++i) {
				if (search[i] != replace[i]) {
					break;
				}
			}
			search.RemoveRange(0, i);
			replace.RemoveRange(0, i);
			return i;
		}

		private int StripCommonPostfix(List<byte> search, List<byte> replace) {
			int i = 0;
			for (; i < Math.Min(search.Count, replace.Count); ++i) {
				if (search[search.Count - i - 1] != replace[replace.Count - i - 1]) {
					break;
				}
			}
			search.RemoveRange(search.Count - i, i);
			replace.RemoveRange(replace.Count - i, i);
			return i;
		}

		private static List<byte> Encode(string text, TextUtils.GameTextEncoding encoding) {
			MemoryStream ms = new MemoryStream();
			ms.WriteNulltermString(text, encoding);
			var inputbytes = ms.CopyToByteArrayAndDispose();

			List<byte> bytes = new List<byte>();
			for (int i = 0; i < inputbytes.Length - 1; ++i) {
				byte c = inputbytes[i];
				if (c == '{' && inputbytes[i + 1] == 'n' && inputbytes[i + 2] == '}') {
					bytes.Add(1);
					i += 2;
				} else if (c == '{' && inputbytes[i + 1] == 'f' && inputbytes[i + 2] == '}') {
					bytes.Add(2);
					i += 2;
				} else {
					bytes.Add(c);
				}
			}
			return bytes;
		}

		private void buttonLoadFiles_Click(object sender, EventArgs e) {
			Files.Clear();

			int sengame = comboBoxGame.SelectedIndex + 1;
			string folder = textBoxFile.Text;
			if (folder == "") {
				switch (sengame) {
					case 1:
						folder = GamePaths.GetDefaultPathCS1();
						break;
					case 2:
						folder = GamePaths.GetDefaultPathCS2();
						break;
					case 3:
						folder = GamePaths.GetDefaultPathCS3();
						break;
					case 4:
						folder = GamePaths.GetDefaultPathCS4();
						break;
					default:
						return;
				}
			}
			LoadFolder(Path.Combine(folder, @"data\scripts\scena\dat_en"));
			LoadFolder(Path.Combine(folder, @"data\scripts\talk\dat_en"));
		}

		private void LoadFolder(string folder) {
			try {
				foreach (string inputfilename in Directory.GetFiles(folder)) {
					Files.Add(GetScriptFile(inputfilename));
				}
			} catch (Exception ex) { }
		}

		private ScriptFile GetScriptFile(string inputfilename) {
			string voicetablefilename = null;
			int sengame = comboBoxGame.SelectedIndex + 1;
			EndianUtils.Endianness endian = comboBoxEndian.SelectedIndex != 0 ? EndianUtils.Endianness.BigEndian : EndianUtils.Endianness.LittleEndian;
			TextUtils.GameTextEncoding encoding = comboBoxEndian.SelectedIndex != 0 ? TextUtils.GameTextEncoding.ShiftJIS : TextUtils.GameTextEncoding.UTF8;
			using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(inputfilename)) {
				var bytestream = fs.CopyToByteArrayStreamAndDispose();
				var sha1 = ChecksumUtils.CalculateSHA1ForEntireStream(bytestream);
				var funcs = ScriptParser.ParseFull(bytestream, voicetablefilename, sengame, endian, encoding, printDetailed: false);
				return new ScriptFile() { Filename = inputfilename, Hash = sha1, Functions = funcs };
			}
		}
	}

	public struct SearchMatch {
		public ScriptOp Op;
		public int Offset;
	}

	public class ScriptFile {
		public string Filename;
		public HyoutaUtils.Checksum.SHA1 Hash;
		public List<ScriptFunction> Functions;
	}

}
