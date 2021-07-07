using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public static class DecompressHelper {
		public static Stream DecompressFromBuffer(byte[] buffer) {
			return DecompressFromStream(new DuplicatableByteArrayStream(buffer));
		}

		public static Stream DecompressFromStream(Stream instream) {
			Logging.Log(string.Format("Decompressing {0} bytes", instream.Length));
			instream.Position = 0;
			int filter = instream.ReadUInt8();

			if (filter == 0) {
				uint crc = instream.ReadUInt32(EndianUtils.Endianness.LittleEndian);
				byte[] properties = instream.ReadBytes(5);
				long length = instream.ReadInt64(EndianUtils.Endianness.LittleEndian);
				Stream outstream = new MemoryStream((int)length);

				SevenZip.Compression.LZMA.Decoder decoder = new SevenZip.Compression.LZMA.Decoder();
				decoder.SetDecoderProperties(properties);
				long compressedSize = instream.Length - instream.Position;
				decoder.Code(instream, outstream, compressedSize, length, null);

				if (!SevenZip.CRC.VerifyDigest(crc, outstream.CopyToByteArray(), 0, (uint)outstream.Length)) {
					throw new Exception("crc mismatch");
				}

				outstream.Position = 0;
				return outstream;
			}

			if (filter == 1) {
				uint crc = instream.ReadUInt32(EndianUtils.Endianness.LittleEndian);
				byte[] properties = instream.ReadBytes(5);
				long length = instream.ReadInt64(EndianUtils.Endianness.LittleEndian);
				Stream outstream = new MemoryStream((int)length);

				SevenZip.Compression.LZMA.Decoder decoder = new SevenZip.Compression.LZMA.Decoder();
				decoder.SetDecoderProperties(properties);
				long compressedSize = instream.Length - instream.Position;
				decoder.Code(instream, outstream, compressedSize, length, null);

				MemoryStream defilteredstream = new MemoryStream((int)outstream.Length);
				outstream.Position = 0;
				StreamUtils.CopyStream(outstream, defilteredstream, 0x30);

				ushort last = 0;
				while (outstream.Position < outstream.Length) {
					ushort diff = outstream.ReadUInt16(EndianUtils.Endianness.LittleEndian);
					ushort curr = (ushort)(last - diff);
					defilteredstream.WriteUInt16(curr, EndianUtils.Endianness.LittleEndian);
					last = curr;
				}

				if (!SevenZip.CRC.VerifyDigest(crc, defilteredstream.CopyToByteArray(), 0, (uint)defilteredstream.Length)) {
					throw new Exception("crc mismatch");
				}

				defilteredstream.Position = 0;
				return defilteredstream;
			}

			if (filter == 2) {
				uint crc = instream.ReadUInt32(EndianUtils.Endianness.LittleEndian);
				byte[] properties = instream.ReadBytes(5);
				long length = instream.ReadInt64(EndianUtils.Endianness.LittleEndian);
				Stream outstream = new MemoryStream((int)length);

				SevenZip.Compression.LZMA.Decoder decoder = new SevenZip.Compression.LZMA.Decoder();
				decoder.SetDecoderProperties(properties);
				long compressedSize = instream.Length - instream.Position;
				decoder.Code(instream, outstream, compressedSize, length, null);

				MemoryStream defilteredstream = new MemoryStream((int)outstream.Length);
				outstream.Position = 0;
				StreamUtils.CopyStream(outstream, defilteredstream, 0x30);

				uint last = 0;
				while (outstream.Position < outstream.Length) {
					uint diff = outstream.ReadUInt32(EndianUtils.Endianness.LittleEndian);
					uint curr = (uint)(last - diff);
					defilteredstream.WriteUInt32(curr, EndianUtils.Endianness.LittleEndian);
					last = curr;
				}

				if (!SevenZip.CRC.VerifyDigest(crc, defilteredstream.CopyToByteArray(), 0, (uint)defilteredstream.Length)) {
					throw new Exception("crc mismatch");
				}

				defilteredstream.Position = 0;
				return defilteredstream;
			}

			if (filter == 3) {
				uint crc = instream.ReadUInt32(EndianUtils.Endianness.LittleEndian);
				byte[] properties = instream.ReadBytes(5);
				long length = instream.ReadInt64(EndianUtils.Endianness.LittleEndian);
				Stream outstream = new MemoryStream((int)length);

				SevenZip.Compression.LZMA.Decoder decoder = new SevenZip.Compression.LZMA.Decoder();
				decoder.SetDecoderProperties(properties);
				long compressedSize = instream.Length - instream.Position;
				decoder.Code(instream, outstream, compressedSize, length, null);

				MemoryStream defilteredstream = new MemoryStream((int)outstream.Length);
				outstream.Position = 0;
				StreamUtils.CopyStream(outstream, defilteredstream, 0x30);

				long pos = defilteredstream.Position;
				ushort last = 0;
				while (defilteredstream.Position < outstream.Length) {
					ushort diff = outstream.ReadUInt16(EndianUtils.Endianness.LittleEndian);
					ushort curr = (ushort)(last - diff);
					defilteredstream.WriteUInt16(curr, EndianUtils.Endianness.LittleEndian);
					defilteredstream.WriteUInt16(0, EndianUtils.Endianness.LittleEndian);
					last = curr;
				}
				defilteredstream.Position = pos;
				last = 0;
				while (defilteredstream.Position < outstream.Length) {
					ushort diff = outstream.ReadUInt16(EndianUtils.Endianness.LittleEndian);
					ushort curr = (ushort)(last - diff);
					defilteredstream.Position = defilteredstream.Position + 2;
					defilteredstream.WriteUInt16(curr, EndianUtils.Endianness.LittleEndian);
					last = curr;
				}

				if (!SevenZip.CRC.VerifyDigest(crc, defilteredstream.CopyToByteArray(), 0, (uint)defilteredstream.Length)) {
					throw new Exception("crc mismatch");
				}

				defilteredstream.Position = 0;
				return defilteredstream;
			}

			throw new Exception("unimplemented filter");
		}
	}
}
