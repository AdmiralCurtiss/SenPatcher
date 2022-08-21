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
	}
}
