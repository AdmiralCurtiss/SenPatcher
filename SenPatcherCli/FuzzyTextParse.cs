using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli {
	public class FuzzyTextParse {
		public FuzzyTextParse(Stream s) {
			while (s.Position < s.Length) {
				byte a = s.ReadUInt8();
				if (a == 0x1a) {
					long p = s.Position;
					s.ReadUInt16();
					byte b = s.ReadUInt8();
					if (b == 0x11) {
						long vidpos = s.Position;
						uint voiceid = s.ReadUInt32(EndianUtils.Endianness.LittleEndian);
						string text = s.ReadAsciiNullterm();
						Console.WriteLine("0x{2:x}: {0} => {1}", voiceid, text, vidpos);
					} else {
						s.Position = p;
					}
				}
			}

			return;
		}
	}
}
