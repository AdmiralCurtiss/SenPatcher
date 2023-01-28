using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Streams;
using SenLib;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli {
	internal class TblFileInfo {
		public string Filename;
		public EndianUtils.Endianness Endian;
		public TextUtils.GameTextEncoding Encoding;
		public TblFileInfo(string filename, EndianUtils.Endianness endian, TextUtils.GameTextEncoding encoding) {
			Filename = filename;
			Endian = endian;
			Encoding = encoding;
		}
	}

	public class ScriptData {
		public byte Type;
		public uint Opcode;
		public uint VTblPtr;
		public uint Unknown0;
		public uint Unknown1;
		public string Filename;
		public string Functionname;
		public uint Unknown2;
		public uint Unknown3;
		public uint FilePtr;
		public uint Position;
		public uint Unknown4;
		public uint Unknown5;
		public int Unknown6;
		public uint Unknown7;
		public uint Unknown8;
		public uint Unknown9;

		public ScriptData(System.IO.Stream s) {
			Type = s.ReadUInt8();
			Opcode = s.ReadUInt32();
			VTblPtr = s.ReadUInt32();
			Unknown0 = s.ReadUInt32();
			Unknown1 = s.ReadUInt32();
			Filename = s.ReadSizedString(0x20, TextUtils.GameTextEncoding.ASCII).TrimNull();
			Functionname = s.ReadSizedString(0x30, TextUtils.GameTextEncoding.ASCII).TrimNull();
			Unknown2 = s.ReadUInt32();
			Unknown3 = s.ReadUInt32();
			FilePtr = s.ReadUInt32();
			Position = s.ReadUInt32();
			Unknown4 = s.ReadUInt32();
			Unknown5 = s.ReadUInt32();
			Unknown6 = s.ReadInt32();
			Unknown7 = s.ReadUInt32();
			Unknown8 = s.ReadUInt32();
			Unknown9 = s.ReadUInt32();
		}

		public override string ToString() {
			bool isEnd = (Type & 0x80) != 0;
			return "[" + Filename + "/" + Functionname + "] " + (isEnd ? "!" : "?") + " 0x" + Position.ToString("x8");
		}
	}

	public class Meta {
		public uint Count;
	}
	public class Meta2 {
		public byte[] Data;
		public ScriptData A;
		public ScriptData B;
	}
	public class Stats {
		public Dictionary<uint, Meta> MetadataPerLength = new Dictionary<uint, Meta>();
		public List<Meta2> Meta2s = new List<Meta2>();
	}

	public static class Playground {
		public static void Run() {
			//foreach (string s in new string[] {
			//	  @"d:\SteamLibrary\steamapps\common\The Legend of Heroes Trails of Cold Steel IV\data\text\dat_en\t_item_en.tbl",
			//	  @"d:\SteamLibrary\steamapps\common\The Legend of Heroes Trails of Cold Steel IV\data\text\dat_en\t_mstqrt.tbl",
			//}) {
			//	Sen4.TblDumper.Dump(s + ".txt", s);
			//}
			//return;

			//t_voice_tbl.CheckVoiceTable(
			//	@"c:\_tmp_a\_cs1-voicetiming\pc_t_voice_jp.tbl",
			//	Path.Combine(SenCommonPaths.Sen1SteamDir, "data/voice/wav"),
			//	@"c:\_tmp_a\_cs1-voicetiming\pc_t_voice_jp.txt"
			//);
			//t_voice_tbl.CheckVoiceTable(
			//	@"c:\_tmp_a\_cs1-voicetiming\pc_t_voice_us__patched.tbl",
			//	Path.Combine(SenCommonPaths.Sen1SteamDir, "data/voice/wav"),
			//	@"c:\_tmp_a\_cs1-voicetiming\pc_t_voice_us__patched.txt"
			//);
			//t_voice_tbl.CheckVoiceTable(
			//	@"c:\_tmp_a\_cs1-voicetiming\ps4_t_voice.tbl",
			//	@"c:\_tmp_a\CS1\data\voice_us",
			//	@"c:\_tmp_a\_cs1-voicetiming\ps4_t_voice.txt"
			//);

			//wav_samples.FullDiff(
			//	@"c:\_tmp_a\CS1_voice-compare\pc_us\",
			//	"PC",
			//	@"c:\_tmp_a\CS1_voice-compare\ps4_us\",
			//	"PS4",
			//	@"c:\_tmp_a\CS1_voice-compare\diff_pc_ps4_us.txt"
			//);

			//wav_samples.FullDiff(
			//	@"c:\_tmp_a\CS2_voice-compare\pc_us\",
			//	"PC",
			//	@"c:\_tmp_a\CS2_voice-compare\ps4_us\",
			//	"PS4",
			//	@"c:\_tmp_a\CS2_voice-compare\diff_pc_ps4_us.txt"
			//);

			//VoiceTiming vctiming;
			//using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(@"c:\_tmp_a\_cs1-voicetiming\_ps3-us\t_vctiming.tbl")) {
			//	vctiming = new VoiceTiming(fs, EndianUtils.Endianness.BigEndian);
			//	vctiming.WriteToStream(fs, EndianUtils.Endianness.LittleEndian);
			//}

			//var a = new wav_samples(@"c:\__ed8\___se_converted\_cs3\v00_s0027.wav");
			//using (var fs = new FileStream(@"c:\__ed8\___se_converted\_cs3\v00_s0027.wav_L", FileMode.Create)) {
			//	foreach (short s in a.Samples_L) {
			//		fs.WriteUInt16((ushort)s, EndianUtils.Endianness.LittleEndian);
			//	}
			//}
			//using (var fs = new FileStream(@"c:\__ed8\___se_converted\_cs3\v00_s0027.wav_R", FileMode.Create)) {
			//	foreach (short s in a.Samples_R) {
			//		fs.WriteUInt16((ushort)s, EndianUtils.Endianness.LittleEndian);
			//	}
			//}

			//var a = new wav_samples(@"c:\__ed8\___se_converted\_cs3\v00_s0028.wav");
			//var a = new wav_samples(@"c:\__ed8\___se_converted\_cs3\v00_s0027.wav");

			//cs1_vctiming_generator.GenerateEnFiles(true);
			//cs1_vctiming_generator.GenerateEnFiles(false);

			//SenPatcherCli.Sen3.TblDumper.InjectItemsIntoSaveFile(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_psv3\save107.dat"), @"c:\_\t_item_en_u.tbl");

			//foreach (string s in new string[] {
			//	@"d:\SteamLibrary\steamapps\common\The Legend of Heroes Trails of Cold Steel III\data\text\dat_en\t_item_en.tbl",
			//	@"d:\SteamLibrary\steamapps\common\The Legend of Heroes Trails of Cold Steel III\data\text\dat_en\t_quest.tbl",
			//	@"c:\__ed8\__\t_mstqrt.tbl",
			//	@"c:\_\t_itemhelp_u.tbl",
			//	@"c:\_\t_itemhelp_j.tbl",
			//	@"c:\_\t_magic_u.tbl",
			//	@"c:\_\t_magic_j.tbl",
			//	@"c:\_\t_notecook_u.tbl",
			//	@"c:\_\t_notecook_j.tbl",
			//	@"c:\_\t_text_u.tbl",
			//	@"c:\_\t_text_j.tbl",
			//	@"c:\_\t_mstqrt.tbl",
			//	@"c:\_\t_mstqrt_fr.tbl",
			//}) {
			//	TblDumper.Dump(s + ".txt", s);
			//}

			//foreach (TblFileInfo tfi in new TblFileInfo[] {
			//	//new TblFileInfo(@"c:\__ed8\__script-compare\EnDavio_1.0\text\dat_us\", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8),
			//	//new TblFileInfo(@"c:\__ed8\__script-compare\EnDavio_1.0\text\dat\", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.ShiftJIS),
			//	//new TblFileInfo(@"c:\__ed8\__script-compare\EnDavio_1.1\text\dat_us\", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8),
			//	//new TblFileInfo(@"c:\__ed8\__script-compare\EnDavio_1.1\text\dat\", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.ShiftJIS),
			//	new TblFileInfo(@"c:\__ed8\__script-compare\pc_1.6\text\", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8),
			//	new TblFileInfo(@"c:\__ed8\__script-compare\pc_1.6\text_jp\", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.ShiftJIS),
			//	new TblFileInfo(@"c:\__ed8\__script-compare\ps3_1.0\text\", EndianUtils.Endianness.BigEndian, TextUtils.GameTextEncoding.UTF8),
			//	new TblFileInfo(@"c:\__ed8\__script-compare\ps3_patched\text\", EndianUtils.Endianness.BigEndian, TextUtils.GameTextEncoding.UTF8),
			//	new TblFileInfo(@"c:\__ed8\__script-compare\ps4\text\", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8),
			//}) {
			//	new SenPatcherCli.Sen1.TblDumper(
			//		new HyoutaUtils.Streams.DuplicatableFileStream(Path.Combine(tfi.Filename, "t_item.tbl")),
			//		new HyoutaUtils.Streams.DuplicatableFileStream(Path.Combine(tfi.Filename, "t_magic.tbl")),
			//		new HyoutaUtils.Streams.DuplicatableFileStream(Path.Combine(tfi.Filename, "t_notecook.tbl")),
			//		tfi.Endian, tfi.Encoding
			//	).Dump(tfi.Filename);
			//}

			//SenPatcherCli.Sen1.TblDumper.InjectItemsIntoSaveFile(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8\save229.dat"), @"c:\__ed8\__script-compare\pc_1.6\text\t_item.tbl");


			//foreach (TblFileInfo tfi in new TblFileInfo[] {
			//	new TblFileInfo(@"c:\__ed8\__script-compare_cs2\pc_1.4.2\text\t_item.tbl", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8),
			//	new TblFileInfo(@"c:\__ed8\__script-compare_cs2\pc_1.4.2\text\t_magic.tbl", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8),
			//	new TblFileInfo(@"c:\__ed8\__script-compare_cs2\pc_1.4.2\text\t_notecook.tbl", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8),
			//	new TblFileInfo(@"c:\__ed8\__script-compare_cs2\ps3\text\t_item.tbl", EndianUtils.Endianness.BigEndian, TextUtils.GameTextEncoding.UTF8),
			//	new TblFileInfo(@"c:\__ed8\__script-compare_cs2\ps3\text\t_magic.tbl", EndianUtils.Endianness.BigEndian, TextUtils.GameTextEncoding.UTF8),
			//	new TblFileInfo(@"c:\__ed8\__script-compare_cs2\ps3\text\t_notecook.tbl", EndianUtils.Endianness.BigEndian, TextUtils.GameTextEncoding.UTF8),
			//	new TblFileInfo(@"c:\__ed8\__script-compare_cs2\ps4\text\t_item.tbl", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8),
			//	new TblFileInfo(@"c:\__ed8\__script-compare_cs2\ps4\text\t_magic.tbl", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8),
			//	new TblFileInfo(@"c:\__ed8\__script-compare_cs2\ps4\text\t_notecook.tbl", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8),
			//}) {
			//	SenPatcherCli.Sen2.TblDumper.Dump(tfi.Filename + ".txt", tfi.Filename, tfi.Endian, tfi.Encoding);
			//}

			//SenPatcherCli.Sen2.TblDumper.InjectItemsIntoSaveFile(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_2\save063.dat"), @"c:\__ed8\__script-compare_cs2\pc_1.4.2\text\t_item.tbl");

			//{
			//	var tbl = new SenLib.Sen1.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(@"d:\SteamLibrary\steamapps\common\Trails of Cold Steel\data\text\dat_us\t_item.tbl"), EndianUtils.Endianness.LittleEndian);
			//
			//	FileStream fs = new FileStream(@"d:\SteamLibrary\steamapps\common\Trails of Cold Steel\data\text\dat_us\t_item.tbl.new", FileMode.Create);
			//	tbl.WriteToStream(fs, EndianUtils.Endianness.LittleEndian);
			//	fs.Close();
			//}

			//return;

			//var jkjslkjk = File.ReadAllLines(@"d:\_____cs2\bin\Win32\cs2_scena_script_tbl.txt");
			//for (int i = 0; i < jkjslkjk.Length; ++i) {
			//	jkjslkjk[i] = "0x" + i.ToString("x2") + " -- " + jkjslkjk[i];
			//}
			//File.WriteAllLines(@"d:\_____cs2\bin\Win32\cs2_scena_script_opcode_func_list.txt", jkjslkjk);
			//return;

			//string path = @"d:\_____cs2\bin\Win32\karel_imperial_villa.txt.bin";
			//var s = new DuplicatableFileStream(path).CopyToByteArrayStreamAndDispose();
			//List<ScriptData> datas = new List<ScriptData>();
			//while (s.Position < s.Length) {
			//	datas.Add(new ScriptData(s));
			//}
			//
			//List<Stats> stats_ui = new List<Stats>();
			//List<Stats> stats_scenario = new List<Stats>();
			//List<Stats> stats_ed8 = new List<Stats>();
			//for (int i = 0; i < 256; ++i) {
			//	stats_ui.Add(new Stats());
			//	stats_scenario.Add(new Stats());
			//	stats_ed8.Add(new Stats());
			//}
			//
			//for (int i = 1; i < datas.Count; ++i) {
			//	ScriptData a = datas[i - 1];
			//	ScriptData b = datas[i];
			//
			//	if (a.Type == (b.Type & 0x7f) && a.Opcode == b.Opcode && a.Filename == b.Filename && a.Functionname == b.Functionname && a.Position < b.Position) {
			//		List<Stats> stats;
			//		if (a.Type == 0) {
			//			stats = stats_ed8;
			//			continue;
			//		} else if (a.Type == 1) {
			//			stats = stats_scenario;
			//		} else if (a.Type == 2) {
			//			stats = stats_ui;
			//			continue;
			//		} else {
			//			continue;
			//		}
			//
			//		uint distance = b.Position - a.Position;
			//
			//		byte[] data = null;
			//		using (var correspondingFile = FindTblFile(a.Filename)) {
			//			if (correspondingFile != null) {
			//				correspondingFile.Position = a.Position;
			//				data = correspondingFile.ReadBytes(distance);
			//			}
			//		}
			//
			//		var stat = stats[(int)a.Opcode];
			//		if (!stat.MetadataPerLength.ContainsKey(distance)) {
			//			stat.MetadataPerLength.Add(distance, new Meta());
			//		}
			//		stat.MetadataPerLength[distance].Count += 1;
			//		stat.Meta2s.Add(new Meta2() { Data = data, A = a, B = b });
			//	}
			//}
			//
			//StringBuilder sb = new StringBuilder();
			//var current_stats = stats_scenario;
			//for (int i = 0; i < current_stats.Count; ++i) {
			//	foreach (var kvp in current_stats[i].MetadataPerLength) {
			//		uint distance = kvp.Key;
			//		uint count = kvp.Value.Count;
			//		sb.AppendFormat("Opcode {0:x2}: Length {1} found {2} times.", i, distance, count);
			//		sb.AppendLine();
			//	}
			//	foreach (var m2 in current_stats[i].Meta2s) {
			//		sb.Append("{");
			//		if (m2.Data != null) {
			//			foreach (byte b in m2.Data) {
			//				sb.AppendFormat("{0:x2} ", b);
			//			}
			//			sb.Remove(sb.Length - 1, 1);
			//		}
			//		sb.Append("}");
			//		sb.AppendFormat(" {0} 0x{1:x8} {2}", m2.A.Filename, m2.A.Position, m2.A.Functionname);
			//		sb.AppendLine();
			//	}
			//	sb.AppendLine();
			//	sb.AppendLine();
			//	sb.AppendLine();
			//}
			//
			//File.WriteAllText(@"d:\_____cs2\bin\Win32\karel_imperial_villa.txt.bin.txt", sb.ToString());

			//foreach (TblFileInfo tfi in new TblFileInfo[] {
			//	new TblFileInfo(@"d:\SteamLibrary\steamapps\common\Trails of Cold Steel\data\text\dat\", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.ShiftJIS),
			//	new TblFileInfo(@"d:\SteamLibrary\steamapps\common\Trails of Cold Steel\data\text\dat_us\", EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8),
			//}) {
			//	new SenPatcherCli.Sen1.TblDumper(
			//		new HyoutaUtils.Streams.DuplicatableFileStream(Path.Combine(tfi.Filename, "t_item.tbl")),
			//		new HyoutaUtils.Streams.DuplicatableFileStream(Path.Combine(tfi.Filename, "t_magic.tbl")),
			//		new HyoutaUtils.Streams.DuplicatableFileStream(Path.Combine(tfi.Filename, "t_notecook.tbl")),
			//		tfi.Endian, tfi.Encoding
			//	).Dump(tfi.Filename);
			//}

			//foreach (string s in new string[] {
			//	@"d:\SteamLibrary\steamapps\common\Trails of Cold Steel II\data\text\dat_us\t_item.tbl",
			//	@"d:\SteamLibrary\steamapps\common\Trails of Cold Steel II\data\text\dat_us\t_magic.tbl",
			//	@"d:\SteamLibrary\steamapps\common\Trails of Cold Steel II\data\text\dat_us\t_notecook.tbl",
			//	@"d:\SteamLibrary\steamapps\common\Trails of Cold Steel II\data\text\dat_us\t_voice.tbl",
			//}) {
			//	SenPatcherCli.Sen2.TblDumper.Dump(s + ".txt", s, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			//}
			//SenPatcherCli.Sen2.TblDumper.Dump(@"c:\__ed8\__script-compare_cs2\ps3\text\t_voice.tbl.txt", @"c:\__ed8\__script-compare_cs2\ps3\text\t_voice.tbl", EndianUtils.Endianness.BigEndian, TextUtils.GameTextEncoding.UTF8);

			// sort saves by playtime
			//var sdslot = new DuplicatableFileStream(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_psv4\sdslot.dat")).CopyToMemoryAndDispose();
			//var list = new List<(int index, long playtime, byte[] sd, byte[] save, byte[] thumb)>();
			//for (int idx = 0; idx < 54; ++idx) {
			//	sdslot.Position = idx * 0x2d0;
			//	var sd = sdslot.ReadBytes(0x2d0);
			//	sdslot.Position = idx * 0x2d0 + 0xc0;
			//	string[] playtimeStrings = sdslot.ReadUTF8Nullterm().Split('\n').Where(x => x.StartsWith("Play Time")).First().Split(' ').Last().Split(':');
			//	long playtime = long.Parse(playtimeStrings[0], System.Globalization.NumberStyles.Integer) * 60 * 60 + long.Parse(playtimeStrings[1], System.Globalization.NumberStyles.Integer) * 60 + long.Parse(playtimeStrings[2], System.Globalization.NumberStyles.Integer);
			//	var save = File.ReadAllBytes(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_psv4\save" + idx.ToString("D3") + ".dat"));
			//	var thumb = File.ReadAllBytes(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_psv4\thumb" + idx.ToString("D3") + ".bmp"));
			//	list.Add((idx, playtime, sd, save, thumb));
			//}
			//var sorted = list.OrderBy(x => x.playtime).ToArray();
			//for (int idx = 0; idx < 54; ++idx) {
			//	var item = sorted[idx];
			//	if (idx != item.index) {
			//		sdslot.Position = idx * 0x2d0;
			//		sdslot.Write(item.sd);
			//		File.WriteAllBytes(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_psv4\save" + idx.ToString("D3") + ".dat"), item.save);
			//		File.WriteAllBytes(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_psv4\thumb" + idx.ToString("D3") + ".bmp"), item.thumb);
			//	}
			//}
			//System.IO.File.WriteAllBytes(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_psv4\sdslot.dat"), sdslot.CopyToByteArray());

			// rewrite save timestamps
			//var sdslot = new DuplicatableFileStream(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_psv4\sdslot.dat")).CopyToMemoryAndDispose();
			//var list = new List<(int index, long playtime, byte[] sd, byte[] save, byte[] thumb)>();
			//for (int idx = 0; idx < 54; ++idx) {
			//	sdslot.Position = idx * 0x2d0;
			//	string s1 = sdslot.ReadUTF8Nullterm();
			//	sdslot.Position = idx * 0x2d0 + 0x40;
			//	string s2 = sdslot.ReadUTF8Nullterm();
			//	sdslot.Position = idx * 0x2d0 + 0xc0;
			//	string s3 = sdslot.ReadUTF8Nullterm();
			//
			//	Console.WriteLine(s1);
			//	Console.WriteLine("-----------------");
			//	Console.WriteLine(s2);
			//	Console.WriteLine("-----------------");
			//	Console.WriteLine(s3);
			//	Console.WriteLine("-----------------");
			//
			//	Console.Write("Year: 2020");
			//	int year = 2020;
			//	Console.Write("Month: ");
			//	int month = ReadInt();
			//	Console.Write("Day: ");
			//	int day = ReadInt();
			//	Console.Write("Hour: ");
			//	int hour = ReadInt();
			//	Console.Write("Minute: ");
			//	int minute = ReadInt();
			//
			//	int dow = 0;
			//	switch (new DateTime(year, month, day, hour, minute, 0).DayOfWeek) {
			//		case DayOfWeek.Sunday: dow = 0; break;
			//		case DayOfWeek.Monday: dow = 1; break;
			//		case DayOfWeek.Tuesday: dow = 2; break;
			//		case DayOfWeek.Wednesday: dow = 3; break;
			//		case DayOfWeek.Thursday: dow = 4; break;
			//		case DayOfWeek.Friday: dow = 5; break;
			//		case DayOfWeek.Saturday: dow = 6; break;
			//	}
			//
			//	sdslot.Position = idx * 0x2d0 + 0x2c0;
			//
			//	sdslot.WriteUInt16((ushort)year);
			//	sdslot.WriteUInt16((ushort)month);
			//	sdslot.WriteUInt16((ushort)dow);
			//	sdslot.WriteUInt16((ushort)day);
			//	sdslot.WriteUInt16((ushort)hour);
			//	sdslot.WriteUInt16((ushort)minute);
			//	sdslot.WriteUInt16((ushort)0); // sec
			//	sdslot.WriteUInt16((ushort)0); // ms
			//
			//	Console.WriteLine("=================");
			//}
			//System.IO.File.WriteAllBytes(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_psv4\sdslot.dat"), sdslot.CopyToByteArray());

			// import param.sfo into sdslot
			//for (int idx = 0; idx < 55; ++idx) {
			//	string patha = @"c:\__ps4\save" + idx.ToString("D3");
			//	string f = Path.Combine(patha, "savedata0", "sce_sys", "param.sfo");
			//	if (File.Exists(f)) {
			//		var s = new DuplicatableFileStream(f);
			//		s.Position = 0x16c;
			//		string s1 = s.ReadUTF8Nullterm();
			//		s.Position = 0x570;
			//		string s2 = s.ReadUTF8Nullterm();
			//		s.Position = 0xa1c;
			//		string s3 = s.ReadUTF8Nullterm();
			//
			//		Console.WriteLine(s1);
			//		Console.WriteLine("-----------------");
			//		Console.WriteLine(s2);
			//		Console.WriteLine("-----------------");
			//		Console.WriteLine(s3);
			//		Console.WriteLine("=================");
			//
			//		sdslot.Position = idx * 0x2d0;
			//		sdslot.WriteUTF8(s2, 0x40, true);
			//		sdslot.Position = idx * 0x2d0 + 0x40;
			//		sdslot.WriteUTF8(s3, 0x80, true);
			//		sdslot.Position = idx * 0x2d0 + 0xc0;
			//		sdslot.WriteUTF8(s1, 0x200, true);
			//	}
			//}
			//System.IO.File.WriteAllBytes(Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_psv4\sdslot.dat"), sdslot.CopyToByteArray());

			// convert saves
			//for (int idx = 0; idx < 55; ++idx) {
			//	using (var fs = new FileStream(@"c:\__ps4\save" + idx.ToString("D3") + @"\savedata0\user.dat", FileMode.Open, FileAccess.ReadWrite)) {
			//		try {
			//			var decomp = SenLib.PkgFile.DecompressType1(fs, EndianUtils.Endianness.LittleEndian);
			//			System.IO.File.WriteAllBytes(@"c:\__ps4\save" + idx.ToString("D3") + @"\savedata0\user.dat.dec", decomp);
			//			var output = SenLib.Sen4.Save.ConvertPs4ToPc(new HyoutaUtils.Streams.DuplicatableByteArrayStream(decomp)).CopyToByteArrayAndDispose();
			//			System.IO.File.WriteAllBytes(@"c:\__ps4\save" + idx.ToString("D3") + @"\savedata0\save" + idx.ToString("D3") + ".dat", output);
			//		} catch (Exception ex) {
			//		}
			//	}
			//}

			return;
		}

		private static DuplicatableStream FindTblFile(string basename) {
			string scenadir = @"d:\_____cs2\data\scripts\scena\dat_us\";
			string talkdir = @"d:\_____cs2\data\scripts\talk\dat_us\";
			string anidir = @"d:\_____cs2\data\scripts\ani\dat_us\";
			string battledir = @"d:\_____cs2\data\scripts\battle\dat_us\";

			string filename = basename + ".dat";
			if (File.Exists(Path.Combine(scenadir, filename))) {
				return new DuplicatableFileStream(Path.Combine(scenadir, filename));
			}
			if (File.Exists(Path.Combine(talkdir, filename))) {
				return new DuplicatableFileStream(Path.Combine(talkdir, filename));
			}
			if (File.Exists(Path.Combine(anidir, filename))) {
				return new DuplicatableFileStream(Path.Combine(anidir, filename));
			}
			if (File.Exists(Path.Combine(battledir, filename))) {
				return new DuplicatableFileStream(Path.Combine(battledir, filename));
			}
			return null;
		}
	}
}
