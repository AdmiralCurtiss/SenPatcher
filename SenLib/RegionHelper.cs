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

			Address += bytes;
			Remaining -= bytes;
		}
	}
}
