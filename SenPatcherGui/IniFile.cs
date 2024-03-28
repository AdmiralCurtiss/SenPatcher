using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SenPatcherGui {
	public class Value {
		internal string Name;
		internal string Data;
		internal List<string> Comment;
	}

	public class Section {
		internal string Name;
		internal List<Value> Values = new List<Value>();

		internal Section() { }

		internal void FillLines(List<string> lines) {
			foreach (var v in Values) {
				if (v.Comment != null) {
					foreach (string line in v.Comment) {
						lines.Add("; " + line);
					}
				}
				lines.Add(string.Format("{0}={1}", v.Name, v.Data));
				lines.Add("");
			}
		}

		internal void Insert(string name, string data, List<string> comment, bool overwrite) {
			foreach (var v in Values) {
				if (string.Compare(name, v.Name, true, CultureInfo.InvariantCulture) == 0) {
					if (overwrite) {
						v.Data = data;
						if (comment != null && comment.Count > 0) {
							v.Comment = comment;
						}
					}
					return;
				}
			}
			Values.Add(new Value { Name = name, Data = data, Comment = comment });
		}

		internal bool TryGet(string name, out string value) {
			foreach (var v in Values) {
				if (string.Compare(name, v.Name, true, CultureInfo.InvariantCulture) == 0) {
					value = v.Data;
					return true;
				}
			}
			value = null;
			return false;
		}
	}

	public class IniFile {
		private List<Section> Sections = new List<Section>();

		public IniFile() {
		}

		public void LoadIniFromString(string iniString, bool overwriteExistingValues) {
			string[] lines = iniString.Split('\n');
			string currentSection = null;
			List<string> currentComment = new List<string>();
			foreach (string rawLine in lines) {
				string line = rawLine.Trim();
				if (line.StartsWith(";")) {
					currentComment.Add(line.Substring(1).Trim());
					continue;
				}

				if (line.StartsWith("[") && line.EndsWith("]")) {
					currentSection = line.Substring(1, line.Length - 2);
					continue;
				}

				if (line.Contains("=")) {
					string[] split = line.Split(new char[] { '=' }, 2);
					string key = split[0].TrimEnd();
					string value = split[1].TrimStart();
					MakeOrGetSection(currentSection).Insert(key, value, currentComment, overwriteExistingValues);
					currentComment = new List<string>();
				}
			}
		}

		private Section MakeOrGetSection(string name) {
			Section sec = GetSection(name);
			if (sec == null) {
				sec = new Section() { Name = name };
				Sections.Add(sec);
			}
			return sec;
		}

		private Section GetSection(string name) {
			if (name == null) {
				foreach (var s in Sections) {
					if (s.Name == null) {
						return s;
					}
				}
			} else {
				foreach (var s in Sections) {
					if (string.Compare(name, s.Name, true, CultureInfo.InvariantCulture) == 0) {
						return s;
					}
				}
			}
			return null;
		}

		public void WriteToFile(string path) {
			List<string> lines = new List<string>();

			Section nullSection = GetSection(null);
			if (nullSection != null) {
				nullSection.FillLines(lines);
			}

			foreach (Section section in Sections) {
				if (section.Name != null) {
					lines.Add(string.Format("[{0}]", section.Name));
					lines.Add("");
					section.FillLines(lines);
				}
			}

			System.IO.File.WriteAllLines(path, lines);
		}

		public bool GetBool(string sectionName, string keyName, bool defaultValue) {
			Section section = GetSection(sectionName);
			if (section != null) {
				string value;
				if (section.TryGet(keyName, out value)) {
					if (string.Compare("false", value, true, CultureInfo.InvariantCulture) == 0) {
						return false;
					}
					if (string.Compare("true", value, true, CultureInfo.InvariantCulture) == 0) {
						return true;
					}
					if (value == "0") {
						return false;
					}
					if (value == "1") {
						return true;
					}
				}
			}

			return defaultValue;
		}

		public int GetInt(string sectionName, string keyName, int defaultValue) {
			Section section = GetSection(sectionName);
			if (section != null) {
				string value;
				if (section.TryGet(keyName, out value)) {
					int r;
					if (int.TryParse(value, NumberStyles.None, CultureInfo.InvariantCulture, out r)) {
						return r;
					}
				}
			}

			return defaultValue;
		}

		public string GetString(string sectionName, string keyName, string defaultValue) {
			Section section = GetSection(sectionName);
			if (section != null) {
				string value;
				if (section.TryGet(keyName, out value)) {
					return value;
				}
			}

			return defaultValue;
		}

		public void SetBool(string sectionName, string keyName, bool boolValue) {
			SetString(sectionName, keyName, boolValue ? "true" : "false");
		}

		public void SetInt(string sectionName, string keyName, int intValue) {
			SetString(sectionName, keyName, intValue.ToString());
		}

		public void SetString(string sectionName, string keyName, string stringValue) {
			MakeOrGetSection(sectionName).Insert(keyName, stringValue, null, true);
		}
	}
}
