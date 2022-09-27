using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen3.FileFixes {
	internal class FileRef {
		public HyoutaUtils.Checksum.SHA1 Checksum;
		public string Path;
	}

	public class voice_opus_ps4_103 : FileMod {
		public string GetDescription() {
			return "Update various voice clips to PS4 version 1.03.";
		}

		private FileRef[] GetFileRefs() {
			return new FileRef[] {
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x47177b904afb4c7ful, 0x6a7b3dcd71d9f336ul, 0xa510479au), Path = "data/voice_us/opus/v00_e0427.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x705db9fda94fcee7ul, 0x9ee44e1c7a81f8adul, 0xfbd9aaddu), Path = "data/voice_us/opus/v00_e1032.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x85d5c7e49da20757ul, 0x219ed878a8c38c17ul, 0x7ce62f9eu), Path = "data/voice_us/opus/v00_e1054.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x78027c66bc6713e2ul, 0x96941f1bc2fe0afaul, 0x16d58652u), Path = "data/voice_us/opus/v00_s0081.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x4544b1a36fcd43c5ul, 0x3354d7b281413c12ul, 0xd6fe0499u), Path = "data/voice_us/opus/v00_s0110.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x381144f8c2504224ul, 0x5630653690900a52ul, 0x424b67a3u), Path = "data/voice_us/opus/v00_s0264.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0xcbadccb5b83c966eul, 0x70828c23337c7577ul, 0x4fc7dfb5u), Path = "data/voice_us/opus/v07_e0026.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x83908f383e62ed46ul, 0xc7f569c8c1d44dd0ul, 0x54bd98ebu), Path = "data/voice_us/opus/v20_e0066.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0xb260dc2e520f9a41ul, 0x5fdc2a18d8c80988ul, 0xa7014b25u), Path = "data/voice_us/opus/v27_e0042.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x300f81a7f4f2b1b4ul, 0x8f089984127d57cful, 0x0849518fu), Path = "data/voice_us/opus/v31_e0070.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0xd5b0d8ee474c452dul, 0xa547e3f6f0ae89d7ul, 0x4e7a447eu), Path = "data/voice_us/opus/v43_e0023.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0xd68debc3a28dd488ul, 0x5a1febbec70206b8ul, 0x37d3446bu), Path = "data/voice_us/opus/v49_e0004.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0xd736113e4beca1bful, 0x4c71b748de04e423ul, 0xdb017434u), Path = "data/voice_us/opus/v52_e0109.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x5091eb68f9ac6480ul, 0xd81e6eb46d2adae9ul, 0x23e808bcu), Path = "data/voice_us/opus/v52_e0119.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0xc76ad49eabeaaba6ul, 0x12c4224d12d0190eul, 0x433804f8u), Path = "data/voice_us/opus/v55_e0018.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x87776fe9380ea836ul, 0x926af327a1b902a8ul, 0x11185d93u), Path = "data/voice_us/opus/v55_e0062.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0xa0f9afb6a537571ful, 0x5da12451744d494dul, 0x70cfa8e3u), Path = "data/voice_us/opus/v56_e0080.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0xae14dafaed074337ul, 0x4fd8eb76d016b80aul, 0x17726b3fu), Path = "data/voice_us/opus/v56_e0092.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x074dc69d2c9b6541ul, 0x55cc88d5e0357c19ul, 0xfd2c0c3du), Path = "data/voice_us/opus/v57_e0020.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x0663cc677885ab2bul, 0x75c95129edda0daaul, 0xd50810bau), Path = "data/voice_us/opus/v64_e0040.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0xe1c6310dc5d14adcul, 0x58b05740a8d02420ul, 0xa3a6c084u), Path = "data/voice_us/opus/v72_e0005.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x756da12dbf0a90a8ul, 0x9d28ee1641d289ddul, 0x79f57887u), Path = "data/voice_us/opus/v72_e0006.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0xe9cdc46863bfe152ul, 0xcd70a0f096ef3eaeul, 0x3d93287eu), Path = "data/voice_us/opus/v72_e0011.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0xef803a8f30d4185cul, 0xbe4eb1f9e837246aul, 0xe53d5d79u), Path = "data/voice_us/opus/v72_e0012.opus" },
				new FileRef() { Checksum = new HyoutaUtils.Checksum.SHA1(0x4090e1aa67d9b5f3ul, 0xf8d610f6e7eaa066ul, 0xcc82698bu), Path = "data/voice_us/opus/v93_e0300.opus" },
			};
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			return new FileModResult[] {
				// Rean t0200, "Even by Your Majesty's imperial command." -> "Even by Your Highness' imperial command."
				new FileModResult("data/voice_us/opus/v00_e0427.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_e0427).CopyToByteArrayStreamAndDispose()),

				// Rean c2440, "I've...heard about the chancellor.{n}But, Your Highness...?" -> "I've...heard about the chancellor.{n}But, Your Majesty...?"
				new FileModResult("data/voice_us/opus/v00_e1032.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_e1032).CopyToByteArrayStreamAndDispose()),

				// Rean c2440, "(Both the chancellor and His Highness{n}understand the situation fully. Yet they...)" -> "(Both the chancellor and His Majesty{n}understand the situation fully. Yet they...)"
				new FileModResult("data/voice_us/opus/v00_e1054.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_e1054).CopyToByteArrayStreamAndDispose()),

				// Rean (post-battle line), "Nothing compared to you, Your Majesty." -> "Nothing compared to you, Your Highness."
				new FileModResult("data/voice_us/opus/v00_s0081.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_s0081).CopyToByteArrayStreamAndDispose()),

				// Rean (Rush callout?), "Your Majesty" -> "Your Highness"
				// this one has the wrong filter but we don't have a better copy...
				new FileModResult("data/voice_us/opus/v00_s0110.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_s0264).CopyToByteArrayStreamAndDispose()),

				// Rean (Rush callout?), "Your Majesty" -> "Your Highness"
				// filter is correct here
				new FileModResult("data/voice_us/opus/v00_s0264.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v00_s0264).CopyToByteArrayStreamAndDispose()),
				
				// Laura f0000, "Heehee." -> "It's been some time, Major Nei--erm, pardon.{n}It's 'Lieutenant Colonel' now, isn't it?" 
				// this is a Laura line from her CS4 voice actor, which is really obviously different than her CS3 one, so let's not use this
				// new FileModResult("data/voice_us/opus/v07_e0026.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v07_e0026).CopyToByteArrayStreamAndDispose()),

				// Elise t0010, "Oh, and, Rean? Here are the letters{n}from Father and Master Yun." -> "Oh, and, Rean? Here are the letters{n}from Father and Master Ka-fai."
				new FileModResult("data/voice_us/opus/v20_e0066.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v20_e0066).CopyToByteArrayStreamAndDispose()),

				// Olivert c0250, "Ah, but it is BECAUSE I am a member{n}of the royal family that I must do this." -> "Ah, but it is BECAUSE I am a member{n}of the Imperial family that I must do this."
				new FileModResult("data/voice_us/opus/v27_e0042.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v27_e0042).CopyToByteArrayStreamAndDispose()),

				// Rufus c2430, "To tell the truth, I was at a party held{n}by the Imperial Household Agency." -> "To tell the truth, I was at a party held{n}by City Hall."
				new FileModResult("data/voice_us/opus/v31_e0070.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v31_e0070).CopyToByteArrayStreamAndDispose()),

				// Carl Regnitz c3210, "Now, we shall begin the award ceremony on{n}behalf of the Imperial Household Agency." -> "Now, we shall present the awards on{n}behalf of City Hall."
				new FileModResult("data/voice_us/opus/v43_e0023.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v43_e0023).CopyToByteArrayStreamAndDispose()),

				// Priscilla c2430, "He was only a young boy when he lost his mother{n}and was adopted into the royal family..." -> "He was only a young boy when he lost his mother{n}and was adopted into the Imperial family..."
				new FileModResult("data/voice_us/opus/v49_e0004.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v49_e0004).CopyToByteArrayStreamAndDispose()),

				// Roselia c3010, "In the beginning, this land housed two{n}of the Sept-Terrion." -> "In the beginning, this land housed two{n}of the Sept-Terrions."
				new FileModResult("data/voice_us/opus/v52_e0109.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v52_e0109).CopyToByteArrayStreamAndDispose()),

				// Roselia c3010, "Exhausting their power in one final strike, the{n}two Sept-Terrion were blown away, left as nothing{n}but empty shells." -> "Exhausting their power in one final strike, the{n}two Sept-Terrions were blown away, left as nothing{n}but empty shells."
				new FileModResult("data/voice_us/opus/v52_e0119.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v52_e0119).CopyToByteArrayStreamAndDispose()),

				// Eugent c2430, "All of these brave people have already been{n}recognized by the Imperial Household Agency, but{n}let us give them yet another round of applause!" -> "All of these brave people have already been{n}recognized by City Hall, but let us give{n}them yet another round of applause!"
				new FileModResult("data/voice_us/opus/v55_e0018.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v55_e0018).CopyToByteArrayStreamAndDispose()),

				// Eugent c2440, "During the War of the Lions, the brothers{n}of the royal family spilled each other's blood{n}and pulled the nation into their feud." -> "During the War of the Lions, the brothers{n}of the Imperial family spilled each other's blood{n}and pulled the nation into their feud."
				new FileModResult("data/voice_us/opus/v55_e0062.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v55_e0062).CopyToByteArrayStreamAndDispose()),

				// Michael r2290, "To be seen off by the royal family is{n}truly more than we deserve..." -> "To be seen off by the Imperial family is{n}truly more than we deserve..."
				new FileModResult("data/voice_us/opus/v56_e0080.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v56_e0080).CopyToByteArrayStreamAndDispose()),

				// Michael r3430, "Zero Artisan Randonneur.{n}Show me what an A-rank bracer is capable of." -> "Zero Artisan Randonneur. Show me what{n}an A-rank level bracer is capable of."
				new FileModResult("data/voice_us/opus/v56_e0092.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v56_e0092).CopyToByteArrayStreamAndDispose()),

				// Alberich m4004, "After the Sept-Terrion of Flame, Ark Rouge, and the{n}Sept-Terrion of Earth, Lost Zem, battled 1,200 years ago..." -> "After the Sept-Terrion of Fire, Ark Rouge, and the{n}Sept-Terrion of Earth, Lost Zem, battled 1,200 years ago..."
				new FileModResult("data/voice_us/opus/v57_e0020.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v57_e0020).CopyToByteArrayStreamAndDispose()),

				// Aurelia t0210, "We seem to have majors from both the{n}RMP and Intelligence Agency here today." -> "We seem to have majors from both the{n}RMP and Intelligence Division here today."
				new FileModResult("data/voice_us/opus/v64_e0040.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v64_e0040).CopyToByteArrayStreamAndDispose()),

				// Rosine t0000, "(Just as Sir Thomas said...)" -> "(Just as Father Thomas said...)"
				new FileModResult("data/voice_us/opus/v72_e0005.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v72_e0005).CopyToByteArrayStreamAndDispose()),
				
				// Rosine t0000, "(...I will need to be ready to{n}move at a moment's notice.)"
				// I'm not sure why this was changed, the new line is just the old one but it fades out slightly earlier
				// new FileModResult("data/voice_us/opus/v72_e0006.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v72_e0006).CopyToByteArrayStreamAndDispose()),
				
				// Rosine c3010, "Sir Lysander! Sir Gaius! Cryptids and Magic Knights{n}have appeared all over the city!" -> "Father Thomas! Father Gaius! Cryptids and Magic{n}Knights have appeared all over the city!"
				new FileModResult("data/voice_us/opus/v72_e0011.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v72_e0011).CopyToByteArrayStreamAndDispose()),

				// Rosine c3000, "Yes, I'll try to get in contact with{n}Sir Hemisphere somehow." -> "Yes, I'll try to get in contact with{n}Father Wazy somehow."
				new FileModResult("data/voice_us/opus/v72_e0012.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v72_e0012).CopyToByteArrayStreamAndDispose()),

				// (random NPC guard) c2430, "Pardon me, Your Highness." -> "Pardon me, Your Majesty."
				new FileModResult("data/voice_us/opus/v93_e0300.opus", DecompressHelper.DecompressFromBuffer(Properties.Resources.v93_e0300).CopyToByteArrayStreamAndDispose()),
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			List<FileModResult> result = new List<FileModResult>();
			foreach (FileRef r in GetFileRefs()) {
				var f = storage.TryGetDuplicate(r.Checksum);
				if (f == null) {
					return null;
				}
				result.Add(new FileModResult(r.Path, f));
			}
			return result.ToArray();
		}
	}
}
