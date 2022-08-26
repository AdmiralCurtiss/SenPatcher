using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public class Sen2SystemData {
		public const long FileLength = 120;


		// possibly a game identifier, as this is the seventh mainline Kiseki game?
		public uint Always7;

		public byte Unknown1;

		// this value clearly means something but I'm not sure what
		public byte Unknown2;

		// 0x0 - 0xD
		public byte TitleScreenVariant;

		public byte Unknown4;

		// bitfield for the Memories section in the main menu
		// note: if a character's Ending flag is set, the dorm event is also unlocked implicitly
		// bit 0 = Memories menu, clear locked, set unlocked
		// bit 1 = Alisa Ending, clear locked, set unlocked
		// bit 2 = Alisa Dorm Event, clear locked, set unlocked
		// bit 3 = Elliot Ending, clear locked, set unlocked
		// bit 4 = Elliot Dorm Event, clear locked, set unlocked
		// bit 5 = Laura Ending, clear locked, set unlocked
		// bit 6 = Laura Dorm Event, clear locked, set unlocked
		// bit 7 = Machias Ending, clear locked, set unlocked
		// bit 8 = Machias Dorm Event, clear locked, set unlocked
		// bit 9 = Emma Ending, clear locked, set unlocked
		// bit 10 = Emma Dorm Event, clear locked, set unlocked
		// bit 11 = Jusis Ending, clear locked, set unlocked
		// bit 12 = Jusis Dorm Event, clear locked, set unlocked
		// bit 13 = Fie Ending, clear locked, set unlocked
		// bit 14 = Fie Dorm Event, clear locked, set unlocked
		// bit 15 = Gaius Ending, clear locked, set unlocked
		// bit 16 = Gaius Dorm Event, clear locked, set unlocked
		// bit 17 = Millium Ending, clear locked, set unlocked
		// bit 18 = Millium Dorm Event, clear locked, set unlocked
		// bit 18 = Sara Ending, clear locked, set unlocked
		// bit 20 = Sara Dorm Event, clear locked, set unlocked
		// bit 21 = Towa Ending, clear locked, set unlocked
		// bit 22 = Towa Dorm Event, clear locked, set unlocked
		// bit 23 = Alfin Ending, clear locked, set unlocked
		// bit 24 = Alfin Dorm Event, clear locked, set unlocked
		// bit 25 = Toval Farewell, clear locked, set unlocked
		// bit 26 = Claire Farewell, clear locked, set unlocked
		// bit 27 = Sharon Farewell, clear locked, set unlocked
		// bits 28-31 seem unused
		public uint MemoriesBitfield;

		public byte Unknown9;

		public byte Unknown10;

		public byte Unknown11;

		public byte Unknown12;

		// 0x0 to 0xA
		public ushort BgmVolume;
		public ushort SeVolume;
		public ushort VoiceVolume;

		// 0 == normal, 1 == inverted
		public ushort VerticalCamera;
		public ushort HorizontalCamera;

		// 0 == main notebook
		// 1 == quest notebook
		// 2 == battle notebook
		// 3 == character notebook
		// 4 == recipe book
		// 5 == fishing book
		// 6 == books
		// 7 == active voice notes
		// 8 == help
		public ushort L2ButtonMapping;
		public ushort R2ButtonMapping;

		// 0 == Zoom In
		// 1 == Zoom Out
		// 2 == Zoom In/Out
		// 3 == Reset Camera
		// 4 == Turn Character
		// 5 == Change Leader (Next)
		// 6 == Change Leader (Previous)
		// 7 == Dash
		// 8 == Walk
		public ushort DPadUpButtonMapping; // Dash/Walk not allowed
		public ushort DPadDownButtonMapping; // Dash/Walk not allowed
		public ushort DPadLeftButtonMapping; // Dash/Walk not allowed
		public ushort DPadRightButtonMapping; // Dash/Walk not allowed
		public ushort L1ButtonMapping; // Turn Character not allowed
		public ushort R1ButtonMapping;
		public ushort CircleButtonMapping; // Only Dash/Walk allowed

		// 0 == rotate, 1 == fixed
		public ushort Minimap;

		// 0 == on, 1 == off
		public ushort Vibration;

		// from 0 to 100 (0x64) in increments of 2; default 100, which fills whole screen
		// in PC version this is generates artifacts so probably should be left alone
		public ushort ScreenWidth;
		public ushort ScreenHeight;

		// from 0 to 100 (0x64) in increments of 2; default 50 (0x32), less is darker, more is brighter
		public ushort ScreenBrightness;

		// 0 == wait, 1 == free
		public ushort ActiveVoice;

		public ulong Unknown13;
		public ulong Unknown14;
		public ulong Unknown15;
		public ulong Unknown16;
		public ulong Unknown17;
		public ulong Unknown18;
		public ulong Unknown19;
		public ulong Unknown20;

		public Sen2SystemData(Stream s, EndianUtils.Endianness endian) {
			Always7 = s.ReadUInt32(endian);
			Unknown1 = s.ReadUInt8();
			Unknown2 = s.ReadUInt8();
			TitleScreenVariant = s.ReadUInt8();
			Unknown4 = s.ReadUInt8();
			MemoriesBitfield = s.ReadUInt32(endian);
			Unknown9 = s.ReadUInt8();
			Unknown10 = s.ReadUInt8();
			Unknown11 = s.ReadUInt8();
			Unknown12 = s.ReadUInt8();
			BgmVolume = s.ReadUInt16(endian);
			SeVolume = s.ReadUInt16(endian);
			VoiceVolume = s.ReadUInt16(endian);
			VerticalCamera = s.ReadUInt16(endian);
			HorizontalCamera = s.ReadUInt16(endian);
			L2ButtonMapping = s.ReadUInt16(endian);
			R2ButtonMapping = s.ReadUInt16(endian);
			DPadUpButtonMapping = s.ReadUInt16(endian);
			DPadDownButtonMapping = s.ReadUInt16(endian);
			DPadLeftButtonMapping = s.ReadUInt16(endian);
			DPadRightButtonMapping = s.ReadUInt16(endian);
			L1ButtonMapping = s.ReadUInt16(endian);
			R1ButtonMapping = s.ReadUInt16(endian);
			CircleButtonMapping = s.ReadUInt16(endian);
			Minimap = s.ReadUInt16(endian);
			Vibration = s.ReadUInt16(endian);
			ScreenWidth = s.ReadUInt16(endian);
			ScreenHeight = s.ReadUInt16(endian);
			ScreenBrightness = s.ReadUInt16(endian);
			ActiveVoice = s.ReadUInt16(endian);
			Unknown13 = s.ReadUInt64(endian);
			Unknown14 = s.ReadUInt64(endian);
			Unknown15 = s.ReadUInt64(endian);
			Unknown16 = s.ReadUInt64(endian);
			Unknown17 = s.ReadUInt64(endian);
			Unknown18 = s.ReadUInt64(endian);
			Unknown19 = s.ReadUInt64(endian);
			Unknown20 = s.ReadUInt64(endian);
		}

		public void SerializeToStream(Stream s, EndianUtils.Endianness endian) {
			s.WriteUInt32(Always7, endian);
			s.WriteUInt8(Unknown1);
			s.WriteUInt8(Unknown2);
			s.WriteUInt8(TitleScreenVariant);
			s.WriteUInt8(Unknown4);
			s.WriteUInt32(MemoriesBitfield, endian);
			s.WriteUInt8(Unknown9);
			s.WriteUInt8(Unknown10);
			s.WriteUInt8(Unknown11);
			s.WriteUInt8(Unknown12);
			s.WriteUInt16(BgmVolume, endian);
			s.WriteUInt16(SeVolume, endian);
			s.WriteUInt16(VoiceVolume, endian);
			s.WriteUInt16(VerticalCamera, endian);
			s.WriteUInt16(HorizontalCamera, endian);
			s.WriteUInt16(L2ButtonMapping, endian);
			s.WriteUInt16(R2ButtonMapping, endian);
			s.WriteUInt16(DPadUpButtonMapping, endian);
			s.WriteUInt16(DPadDownButtonMapping, endian);
			s.WriteUInt16(DPadLeftButtonMapping, endian);
			s.WriteUInt16(DPadRightButtonMapping, endian);
			s.WriteUInt16(L1ButtonMapping, endian);
			s.WriteUInt16(R1ButtonMapping, endian);
			s.WriteUInt16(CircleButtonMapping, endian);
			s.WriteUInt16(Minimap, endian);
			s.WriteUInt16(Vibration, endian);
			s.WriteUInt16(ScreenWidth, endian);
			s.WriteUInt16(ScreenHeight, endian);
			s.WriteUInt16(ScreenBrightness, endian);
			s.WriteUInt16(ActiveVoice, endian);
			s.WriteUInt64(Unknown13, endian);
			s.WriteUInt64(Unknown14, endian);
			s.WriteUInt64(Unknown15, endian);
			s.WriteUInt64(Unknown16, endian);
			s.WriteUInt64(Unknown17, endian);
			s.WriteUInt64(Unknown18, endian);
			s.WriteUInt64(Unknown19, endian);
			s.WriteUInt64(Unknown20, endian);
		}
	}
}
