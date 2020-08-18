using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class SenScriptPatcher {
		private const byte JumpCommand = 0x03;
		private const byte NopCommand = 0x09;

		private Stream Bin;

		public SenScriptPatcher(Stream s) {
			Bin = s;
		}

		public void ReplaceCommand(long originalLocation, long originalLength, byte[] newCommand) {
			long nextCommandLocation = originalLocation + originalLength;

			if (newCommand.LongLength <= originalLength) {
				// this is simple, just put the new command and fill the rest with NOP
				Bin.Position = originalLocation;
				Bin.Write(newCommand);
				long rest = nextCommandLocation - Bin.Position;
				for (long i = 0; i < rest; ++i) {
					Bin.WriteByte(NopCommand);
				}
				return;
			}

			// new command is longer than the old one
			// need to write a jump, write the new command in free space at the bottom of the file, and then jump back

			if (originalLength < 5) {
				throw new Exception("Not enough space to write jump command.");
			}

			// write new command at and of file
			Bin.Position = Bin.Length;
			Bin.WriteAlign(4);
			uint newCommandLocation = (uint)Bin.Position;
			Bin.Write(newCommand);

			// then a jump back to the actual script
			Bin.WriteUInt8(JumpCommand);
			Bin.WriteUInt32((uint)nextCommandLocation, EndianUtils.Endianness.LittleEndian);

			// jump to the new command back at the original command
			Bin.Position = originalLocation;
			Bin.WriteUInt8(JumpCommand);
			Bin.WriteUInt32(newCommandLocation, EndianUtils.Endianness.LittleEndian);

			// and to be clean, dummy out the rest of the old command
			for (long i = 0; i < originalLength - 5; ++i) {
				Bin.WriteUInt8(NopCommand);
			}
		}

		public void ReplacePartialCommand(long commandLocation, long commandLength, long replacementLocation, long replacementLength, byte[] replacementData) {
			long keepByteCountStart = replacementLocation - commandLocation;
			long keepByteCountEnd = (commandLocation + commandLength) - (replacementLocation + replacementLength);

			MemoryStream newCommand = new MemoryStream();
			if (keepByteCountStart > 0) {
				newCommand.Write(Bin.ReadBytesFromLocationAndReset(commandLocation, keepByteCountStart));
			}
			if (replacementData != null && replacementData.LongLength > 0) {
				newCommand.Write(replacementData);
			}
			if (keepByteCountEnd > 0) {
				newCommand.Write(Bin.ReadBytesFromLocationAndReset((commandLocation + commandLength) - keepByteCountEnd, keepByteCountEnd));
			}

			ReplaceCommand(commandLocation, commandLength, newCommand.CopyToByteArrayAndDispose());
		}

		public void RemovePartialCommand(long commandLocation, long commandLength, long removeLocation, long removeLength) {
			ReplacePartialCommand(commandLocation, commandLength, removeLocation, removeLength, null);
		}

		public void ExtendPartialCommand(long commandLocation, long commandLength, long extendLocation, byte[] extendData) {
			ReplacePartialCommand(commandLocation, commandLength, extendLocation, 0, extendData);
		}
	}
}
