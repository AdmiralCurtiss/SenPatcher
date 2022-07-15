using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HyoutaUtils.Checksum {
	public struct SHA256 : IEquatable<SHA256> {
		public byte[] Value {
			get {
				return new byte[] {
					(byte)((Part1 >> 56) & 0xff),
					(byte)((Part1 >> 48) & 0xff),
					(byte)((Part1 >> 40) & 0xff),
					(byte)((Part1 >> 32) & 0xff),
					(byte)((Part1 >> 24) & 0xff),
					(byte)((Part1 >> 16) & 0xff),
					(byte)((Part1 >> 8) & 0xff),
					(byte)(Part1 & 0xff),
					(byte)((Part2 >> 56) & 0xff),
					(byte)((Part2 >> 48) & 0xff),
					(byte)((Part2 >> 40) & 0xff),
					(byte)((Part2 >> 32) & 0xff),
					(byte)((Part2 >> 24) & 0xff),
					(byte)((Part2 >> 16) & 0xff),
					(byte)((Part2 >> 8) & 0xff),
					(byte)(Part2 & 0xff),
					(byte)((Part3 >> 56) & 0xff),
					(byte)((Part3 >> 48) & 0xff),
					(byte)((Part3 >> 40) & 0xff),
					(byte)((Part3 >> 32) & 0xff),
					(byte)((Part3 >> 24) & 0xff),
					(byte)((Part3 >> 16) & 0xff),
					(byte)((Part3 >> 8) & 0xff),
					(byte)(Part3 & 0xff),
					(byte)((Part4 >> 56) & 0xff),
					(byte)((Part4 >> 48) & 0xff),
					(byte)((Part4 >> 40) & 0xff),
					(byte)((Part4 >> 32) & 0xff),
					(byte)((Part4 >> 24) & 0xff),
					(byte)((Part4 >> 16) & 0xff),
					(byte)((Part4 >> 8) & 0xff),
					(byte)(Part4 & 0xff),
				};
			}
		}

		private ulong Part1;
		private ulong Part2;
		private ulong Part3;
		private ulong Part4;

		public SHA256(byte[] value) {
			if (value.Length != 32) {
				throw new Exception("incorrect length for SHA256 (must be 32 bytes)");
			}
			Part1 = (((ulong)(value[7])))
				  | (((ulong)(value[6])) << 8)
				  | (((ulong)(value[5])) << 16)
				  | (((ulong)(value[4])) << 24)
				  | (((ulong)(value[3])) << 32)
				  | (((ulong)(value[2])) << 40)
				  | (((ulong)(value[1])) << 48)
				  | (((ulong)(value[0])) << 56);
			Part2 = (((ulong)(value[15])))
				  | (((ulong)(value[14])) << 8)
				  | (((ulong)(value[13])) << 16)
				  | (((ulong)(value[12])) << 24)
				  | (((ulong)(value[11])) << 32)
				  | (((ulong)(value[10])) << 40)
				  | (((ulong)(value[9])) << 48)
				  | (((ulong)(value[8])) << 56);
			Part3 = (((ulong)(value[23])))
				  | (((ulong)(value[22])) << 8)
				  | (((ulong)(value[21])) << 16)
				  | (((ulong)(value[20])) << 24)
				  | (((ulong)(value[19])) << 32)
				  | (((ulong)(value[18])) << 40)
				  | (((ulong)(value[17])) << 48)
				  | (((ulong)(value[16])) << 56);
			Part4 = (((ulong)(value[31])))
				  | (((ulong)(value[30])) << 8)
				  | (((ulong)(value[29])) << 16)
				  | (((ulong)(value[28])) << 24)
				  | (((ulong)(value[27])) << 32)
				  | (((ulong)(value[26])) << 40)
				  | (((ulong)(value[25])) << 48)
				  | (((ulong)(value[24])) << 56);
		}

		public SHA256(ulong part1, ulong part2, ulong part3, ulong part4) {
			Part1 = part1;
			Part2 = part2;
			Part3 = part3;
			Part4 = part4;
		}

		public SHA256(string sha256) {
			if (sha256.Length != 64) {
				throw new Exception("incorrect length for SHA256 string");
			}

			Part1 = (((ulong)SHA1.Decode4Bytes(sha256, 0)) << 32) | ((ulong)SHA1.Decode4Bytes(sha256, 8));
			Part2 = (((ulong)SHA1.Decode4Bytes(sha256, 16)) << 32) | ((ulong)SHA1.Decode4Bytes(sha256, 24));
			Part3 = (((ulong)SHA1.Decode4Bytes(sha256, 32)) << 32) | ((ulong)SHA1.Decode4Bytes(sha256, 40));
			Part4 = (((ulong)SHA1.Decode4Bytes(sha256, 48)) << 32) | ((ulong)SHA1.Decode4Bytes(sha256, 56));
		}

		public bool Equals(SHA256 other) {
			return this == other;
		}

		public override bool Equals(object obj) {
			return obj is SHA256 && Equals((SHA256)obj);
		}

		public static bool operator ==(SHA256 lhs, SHA256 rhs) {
			return lhs.Part1 == rhs.Part1 && lhs.Part2 == rhs.Part2 && lhs.Part3 == rhs.Part3 && lhs.Part4 == rhs.Part4;
		}

		public static bool operator !=(SHA256 lhs, SHA256 rhs) {
			return !(lhs == rhs);
		}

		public static int CompareSha256(SHA256 lhs, SHA256 rhs) {
			if (lhs.Part1 < rhs.Part1) { return -1; }
			if (lhs.Part1 > rhs.Part1) { return 1; }
			if (lhs.Part2 < rhs.Part2) { return -1; }
			if (lhs.Part2 > rhs.Part2) { return 1; }
			if (lhs.Part3 < rhs.Part3) { return -1; }
			if (lhs.Part3 > rhs.Part3) { return 1; }
			if (lhs.Part4 < rhs.Part4) { return -1; }
			if (lhs.Part4 > rhs.Part4) { return 1; }
			return 0;
		}

		public override int GetHashCode() {
			return (int)(Part4 & 0xFFFFFFFF);
		}

		public override string ToString() {
			return string.Format("{0:x16}{1:x16}{2:x16}{3:x16}", Part1, Part2, Part3, Part4);
		}
	}
}
