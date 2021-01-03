using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public class Sen2ExecutableCodeSpaceLocations {
		public uint Region50aAddress { get; private set; }
		public uint Region50aTarget { get; private set; }
		public uint Region50aLength { get; private set; }

		public uint Region50bAddress { get; private set; }
		public uint Region50bTarget { get; private set; }
		public uint Region50bLength { get; private set; }

		public uint Region51Address { get; private set; }
		public uint Region51Target { get; private set; }
		public uint Region51Length { get; private set; }

		public uint Region60Address { get; private set; }
		public uint Region60Target { get; private set; }
		public uint Region60Length { get; private set; }

		public uint Region41Address { get; private set; }
		public uint Region41Target { get; private set; }
		public uint Region41Length { get; private set; }

		public uint Region32Address { get; private set; }
		public uint Region32Target { get; private set; }
		public uint Region32Length { get; private set; }

		public uint RegionDAddress { get; private set; }
		public uint RegionDLength { get; private set; }

		public Sen2ExecutableCodeSpaceLocations(bool jp) {
			if (jp) {
				Region50aAddress = 0x420bbb;
				Region50aTarget = 0x42195b;
				Region50aLength = 0x50;
				Region50bAddress = 0x42117b;
				Region50bTarget = 0x42195b;
				Region50bLength = 0x50;
				Region51Address = 0x4213ba;
				Region51Target = 0x42159b;
				Region51Length = 0x51;
				Region60Address = 0x420ccb;
				Region60Target = 0x42158b;
				Region60Length = 0x60;

				Region41Address = 0x421aba; // command 0x2
				Region41Target = 0x42196b; // command 0x6
				Region41Length = 0x41;
				Region32Address = 0x41fca9; // command 0x0
				Region32Target = 0x420ed1; // command 0x8
				Region32Length = 0x32;

				RegionDAddress = 0x41fbe3;
				RegionDLength = 0xd;
			} else {
				Region50aAddress = 0x420c4b; // command 0x9
				Region50aTarget = 0x4219cb; // command 0x6
				Region50aLength = 0x50;
				Region50bAddress = 0x4211fb; // command 0xB
				Region50bTarget = 0x4219cb; // command 0x6
				Region50bLength = 0x50;
				Region51Address = 0x42143a; // command 0x4
				Region51Target = 0x42161b; // command 0xA
				Region51Length = 0x51;
				Region60Address = 0x420d5b; // command 0x7
				Region60Target = 0x42160b; // command 0xA
				Region60Length = 0x60;

				Region41Address = 0x421b2a; // command 0x2
				Region41Target = 0x4219db; // command 0x6
				Region41Length = 0x41;
				Region32Address = 0x41fcc9; // command 0x0
				Region32Target = 0x420f51; // command 0x8
				Region32Length = 0x32;

				RegionDAddress = 0x41fc03;
				RegionDLength = 0xd;
			}
		}
	}
}
