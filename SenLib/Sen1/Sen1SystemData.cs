using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public class Sen1SystemData {
		// possibly a game identifier, as this is the sixth mainline Kiseki game?
		public uint Always6;

		public byte Unknown1;

		// this value clearly means something but I'm not sure what
		public byte Unknown2;

		// 0x00 - 0x15 ish, not all of them seem used
		public byte TitleScreenVariant;

		public byte Unknown4;

		public byte Unknown5;

		public byte Unknown6;

		public byte Unknown7;

		public byte Unknown8;

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
		public ushort DPadUpButtonMapping;
		public ushort DPadDownButtonMapping;
		public ushort DPadLeftButtonMapping;
		public ushort DPadRightButtonMapping;
		public ushort L1ButtonMapping; // Turn Character not allowed

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

		public ushort Unused;


		public Sen1SystemData(Stream s, EndianUtils.Endianness endian) {
			Always6 = s.ReadUInt32(endian);
			Unknown1 = s.ReadUInt8();
			Unknown2 = s.ReadUInt8();
			TitleScreenVariant = s.ReadUInt8();
			Unknown4 = s.ReadUInt8();
			Unknown5 = s.ReadUInt8();
			Unknown6 = s.ReadUInt8();
			Unknown7 = s.ReadUInt8();
			Unknown8 = s.ReadUInt8();
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
			Minimap = s.ReadUInt16(endian);
			Vibration = s.ReadUInt16(endian);
			ScreenWidth = s.ReadUInt16(endian);
			ScreenHeight = s.ReadUInt16(endian);
			ScreenBrightness = s.ReadUInt16(endian);
			Unused = s.ReadUInt16(endian);
		}

		public void SerializeToStream(Stream s, EndianUtils.Endianness endian) {
			s.WriteUInt32(Always6, endian);
			s.WriteUInt8(Unknown1);
			s.WriteUInt8(Unknown2);
			s.WriteUInt8(TitleScreenVariant);
			s.WriteUInt8(Unknown4);
			s.WriteUInt8(Unknown5);
			s.WriteUInt8(Unknown6);
			s.WriteUInt8(Unknown7);
			s.WriteUInt8(Unknown8);
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
			s.WriteUInt16(Minimap, endian);
			s.WriteUInt16(Vibration, endian);
			s.WriteUInt16(ScreenWidth, endian);
			s.WriteUInt16(ScreenHeight, endian);
			s.WriteUInt16(ScreenBrightness, endian);
			s.WriteUInt16(Unused, endian);
		}
	}
}
