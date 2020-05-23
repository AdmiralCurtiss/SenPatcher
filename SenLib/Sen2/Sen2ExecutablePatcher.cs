using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public class Sen2ExecutablePatcher {
		// the branch at ram 0x57C807 or 0x57C80D seems responsible for the weird behavior 
		// where the same music track doesn't restart if it is still fading out (and the wrong one gets queued instead)
		// this probably has side-effects if we just always jump but investigate this further...
	}
}
