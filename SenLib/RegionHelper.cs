using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class RegionHelper {
		public uint Address;
		public uint Remaining;

		public RegionHelper(uint address, uint length) {
			Address = address;
			Remaining = length;
		}

		public void Take(uint bytes) {
			if (bytes > Remaining) {
				throw new Exception("took more than available");
			}

			uint naddr = Address + bytes;
			uint nrem = Remaining - bytes;

			Console.WriteLine("Took 0x{0:X} to 0x{1:X} (0x{4:X} bytes; 0x{2:X} left before, 0x{3:X} left now)", Address, naddr, Remaining, nrem, bytes);

			Address = naddr;
			Remaining = nrem;
		}

		public void TakeToAddress(long address) {
			if (address < (long)Address) {
				throw new Exception("took negative amount?");
			}
			ulong bytes = (ulong)(address - (long)Address);
			if (bytes > uint.MaxValue) {
				throw new Exception("took way too much");
			}
			Take((uint)bytes);
		}
	}
}
