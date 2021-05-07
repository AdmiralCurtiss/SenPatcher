using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using HyoutaUtils;

namespace SenPatcherCli {
	public class wav_samples {
		public short[] Samples_L;
		public short[] Samples_R;

		// this is not a generic wav parser, this just works for diffing two already similar files
		public wav_samples(string filename) {
			using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(filename)) {
				var e = EndianUtils.Endianness.LittleEndian;
				uint riff = fs.ReadUInt32(e);
				uint rifflength = fs.ReadUInt32(e);
				uint wave = fs.ReadUInt32(e);

				uint fmt = fs.ReadUInt32(e);
				uint fmt_length = fs.ReadUInt32(e);
				ushort format = fs.ReadUInt16(e);
				ushort channels = fs.ReadUInt16(e);
				uint samplerate = fs.ReadUInt32(e);
				uint bytes_per_second = fs.ReadUInt32(e);
				ushort block_align = fs.ReadUInt16(e);
				ushort bits_per_sample = fs.ReadUInt16(e);
				uint data = fs.ReadUInt32(e);
				uint data_length = fs.ReadUInt32(e);
				if (channels == 1) {
					long sample_count = data_length / 2;
					short[] samples = new short[sample_count];
					for (long i = 0; i < sample_count; ++i) {
						samples[i] = fs.ReadInt16(e);
					}
					Samples_L = samples;
				} else if (channels == 2) {
					long sample_count = data_length / 4;
					short[] samples_left = new short[sample_count];
					short[] samples_right = new short[sample_count];
					for (long i = 0; i < sample_count; ++i) {
						samples_left[i] = fs.ReadInt16(e);
						samples_right[i] = fs.ReadInt16(e);
					}
					Samples_L = samples_left;
					Samples_R = samples_right;
				}
			}
		}

		public static void FullDiff(string a_path, string a_identifier, string b_path, string b_identifier, string outpath) {
			SortedSet<string> names = new SortedSet<string>();
			foreach (var file in new DirectoryInfo(a_path).EnumerateFiles()) {
				names.Add(file.Name);
			}
			foreach (var file in new DirectoryInfo(b_path).EnumerateFiles()) {
				names.Add(file.Name);
			}

			StringBuilder sb = new StringBuilder();

			foreach (string filename in names) {
				sb.Append(filename);
				sb.Append(": ");
				string p_a = Path.Combine(a_path, filename);
				string p_b = Path.Combine(b_path, filename);
				bool exists_a = File.Exists(p_a);
				bool exists_b = File.Exists(p_b);
				if (exists_a && exists_b) {
					wav_samples a = new wav_samples(p_a);
					wav_samples b = new wav_samples(p_b);
					//using (var fs = new FileStream(p_a + ".raw", FileMode.Create)) {
					//	foreach (short s in a.Samples_L) {
					//		fs.WriteUInt16((ushort)s, EndianUtils.Endianness.LittleEndian);
					//	}
					//}

					if (a.Samples_L != null && a.Samples_R != null) {
						long diff_channels = DoDiff(a.Samples_L, a.Samples_R, 0);
						if (diff_channels != 0) {
							sb.Append($"diff between channels on ${a_identifier}; ");
						}
					}
					if (b.Samples_L != null && b.Samples_R != null) {
						long diff_channels = DoDiff(b.Samples_L, b.Samples_R, 0);
						if (diff_channels != 0) {
							sb.Append($"diff between channels on ${b_identifier}; ");
						}
					}

					if (a.Samples_L != null && b.Samples_L != null) {
						long diff_ab = DoDiff(a.Samples_L, b.Samples_L, 0);
						if (diff_ab < 0) {
							sb.Append("different length between versions!");
						} else {
							long average_diff_whole = diff_ab / a.Samples_L.LongLength;
							long average_diff_frac_int = diff_ab % a.Samples_L.LongLength;
							double average_diff_frac = ((double)average_diff_frac_int) / ((double)a.Samples_L.LongLength);
							double average_diff = ((double)average_diff_whole) + average_diff_frac;

							sb.Append("average sample diff: ").Append(average_diff);
							if (average_diff_whole >= 500) {
								sb.Append(" (unusual!)");
							}
						}
					} else {
						sb.Append("cannot compare, at least one file is missing first channel; ");
					}
				} else if (exists_a && !exists_b) {
					sb.Append($"exists only on ${a_identifier}");
				} else if (!exists_a && exists_b) {
					sb.Append($"exists only on ${b_identifier}");
				} else {
					sb.Append("does not exist at all????");
				}
				sb.AppendLine();
			}

			File.WriteAllText(outpath, sb.ToString(), Encoding.UTF8);
		}

		public static long DoDiff(short[] buffer_a, short[] buffer_b, long max_distance) {
			if (buffer_a.LongLength != buffer_b.LongLength) {
				return -1;
			}

			long diff_count = 0;
			long max_dist = 0;
			long total_diff = 0;
			for (long i = 0; i < buffer_a.LongLength; ++i) {
				int a = buffer_a[i];
				int b = buffer_b[i];
				int dist = Math.Abs(a - b);
				if (dist > max_distance) {
					++diff_count;
					max_dist = Math.Max(max_dist, (long)dist);
					total_diff += dist;
				}
			}
			return total_diff;
		}
	}
}
