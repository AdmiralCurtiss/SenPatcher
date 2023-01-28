using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4 {
	public enum SaveType {
		SystemData,
		GameData,
	}

	public class Save {
		public static SaveType? IdentifySaveFile(Stream stream) {
			if (stream.Length == 1184 || stream.Length == 1463976) {
				stream.Position = 0;
				uint type = stream.ReadUInt32(EndianUtils.Endianness.LittleEndian);
				uint unknown = stream.ReadUInt32(EndianUtils.Endianness.LittleEndian);
				uint length = stream.ReadUInt32(EndianUtils.Endianness.LittleEndian);
				if (type == 1 && unknown == 2 && length == stream.Length && length == 1184) {
					return SaveType.SystemData;
				}
				if (type == 3 && unknown == 2 && length == stream.Length && length == 1463976) {
					return SaveType.GameData;
				}
			}
			return null;
		}

		public static bool FixSaveChecksum(Stream stream) {
			var t = IdentifySaveFile(stream);
			if (t == null) {
				return false;
			}

			stream.Position = 0x10;
			uint crc32 = (uint)(stream.Length - 0x10);
			crc32 = CRC32Algorithm.crc_update(crc32, stream, (ulong)(stream.Length - 0x10));

			stream.Position = 0xc;
			stream.WriteUInt32(crc32, EndianUtils.Endianness.LittleEndian);

			return true;
		}

		private static void Skip4(Stream input) {
			uint v = input.ReadUInt32();
			if (v != 0) {
				Console.WriteLine("Warning: trashed nonzero uint {0:x8} at {1:x8}", v, input.Position - 4);
			}
		}

		public static Stream ConvertPs4ToPc(Stream input) {
			// this is complete guesswork based on comparing a PC and PS4 save saved immediately after hitting New Game
			// might break everything
			MemoryStream output = new MemoryStream();
			StreamUtils.CopyStream(input, output, 8);
			input.ReadUInt32(); // old filesize
			output.WriteUInt32(1463976, EndianUtils.Endianness.LittleEndian);
			StreamUtils.CopyStream(input, output, 0x152dc - 12);
			Skip4(input);
			StreamUtils.CopyStream(input, output, 0xc8);
			for (int i = 0; i < 144; ++i) {
				Skip4(input);
				StreamUtils.CopyStream(input, output, 0x18);
				Skip4(input);
				Skip4(input);
				Skip4(input);
				StreamUtils.CopyStream(input, output, 0x3a8);
			}
			StreamUtils.CopyStream(input, output, 0x7be20 - 0x36fa4);
			Skip4(input);
			Skip4(input);
			Skip4(input);
			StreamUtils.CopyStream(input, output, input.Length - input.Position - 0x8);

			output.Position = 0;
			FixSaveChecksum(output);
			return output;
		}
	}
}
