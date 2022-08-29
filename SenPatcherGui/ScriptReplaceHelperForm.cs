using HyoutaUtils;
using SenLib;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SenPatcherGui {
	public partial class ScriptReplaceHelperForm : Form {
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

		private void buttonGo_Click(object sender, EventArgs e) {
			string inputfilename = textBoxFile.Text;
			string voicetablefilename = null;
			int sengame = comboBoxGame.SelectedIndex + 1;
			EndianUtils.Endianness endian = comboBoxEndian.SelectedIndex != 0 ? EndianUtils.Endianness.BigEndian : EndianUtils.Endianness.LittleEndian;
			TextUtils.GameTextEncoding encoding = comboBoxEndian.SelectedIndex != 0 ? TextUtils.GameTextEncoding.ShiftJIS : TextUtils.GameTextEncoding.UTF8;
			List<byte> rawsearch = Encode(textBoxSearch.Text, encoding);
			List<byte> search = Encode(textBoxSearch.Text, encoding);
			List<byte> replace = Encode(textBoxReplace.Text, encoding);
			int offset = StripCommonPrefix(search, replace);
			StripCommonPostfix(search, replace);

			StringBuilder sb = new StringBuilder();
			using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(inputfilename)) {
				var bytestream = fs.CopyToByteArrayStreamAndDispose();
				var sha1 = ChecksumUtils.CalculateSHA1ForEntireStream(bytestream);
				var funcs = ScriptParser.ParseFull(bytestream, voicetablefilename, sengame, endian, encoding, printDetailed: false);
				foreach (var func in funcs) {
					foreach (var op in func.Ops) {
						for (int i = 0; i < op.Bytes.Length - rawsearch.Count - 1; ++i) {
							if (op.Bytes[i] == rawsearch[0] && ArrayUtils.IsByteArrayPartEqual(op.Bytes, i, rawsearch, 0, rawsearch.Count)) {
								sb.AppendLine(
									string.Format(
											"patcher.ReplacePartialCommand(0x{0:x}, 0x{1:x}, 0x{2:x}, 0x{3:x}, {4});",
											op.Position,
											op.Bytes.Length,
											op.Position + i + offset,
											search.Count,
											NewByteArrayString(replace)
										)
									);
							}
						}
					}
				}
			}
			textBoxOutput.Text = sb.ToString();
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
	}
}
