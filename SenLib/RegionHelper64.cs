using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class RegionHelper64 {
		public long Address;
		public uint Remaining;
		private string Identifier;

		public RegionHelper64(long address, uint length, string identifier) {
			Address = address;
			Remaining = length;
			Identifier = identifier;
		}

		public void Take(uint bytes, string reason) {
			if (bytes > Remaining) {
				throw new Exception(Identifier + ": took more than available for " + reason);
			}

			long naddr = Address + bytes;
			uint nrem = Remaining - bytes;

			Console.WriteLine("{5}: Took 0x{0:X} to 0x{1:X} (0x{4:X} bytes; 0x{2:X} left before, 0x{3:X} left now) [{6}]", Address, naddr, Remaining, nrem, bytes, Identifier, reason);

			Address = naddr;
			Remaining = nrem;
		}

		public void TakeToAddress(long address, string reason) {
			if (address < Address) {
				throw new Exception(Identifier + ": took negative amount for " + reason);
			}
			ulong bytes = (ulong)(address - Address);
			if (bytes > uint.MaxValue) {
				throw new Exception(Identifier + ": took way too much for " + reason);
			}
			Take((uint)bytes, reason);
		}

		public void PrintStatistics() {
			Console.WriteLine("{0}: Currently at 0x{1:X}, 0x{2:X} bytes free.", Identifier, Address, Remaining);
		}
	}
}
