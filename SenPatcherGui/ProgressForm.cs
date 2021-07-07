using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SenPatcherGui {
	//
	// I AM A BAD GUI PROGRAMMER DO NOT USE THIS FOR OTHER STUFF
	// There's a lot of weird threading stuff in here that I'm sure can be done much better.
	//

	public partial class ProgressForm : Form {
		private class ProgressFormReporter : SenLib.ProgressReporter {
			private ProgressForm Parent;

			public ProgressFormReporter(ProgressForm parent) {
				Parent = parent;
			}

			public void Message(string msg, int current = -1, int total = -1, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0) {
				if (current != -1 && total != -1) {
					SenLib.Logging.LogForwarded(string.Format("Progress: {0}/{1}", current, total), memberName, sourceFilePath, sourceLineNumber);
				}
				SenLib.Logging.LogForwarded(msg, memberName, sourceFilePath, sourceLineNumber);
				Parent.PushMessage(msg, current, total);
			}

			public void Error(string msg, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0) {
				SenLib.Logging.LogForwarded(string.Format("ERROR: {0}", msg), memberName, sourceFilePath, sourceLineNumber);
				Parent.PushMessage(string.Format("ERROR: {0}", msg));
			}

			public void Finish(bool success, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0) {
				SenLib.Logging.LogForwarded(string.Format("Task finished with {0}.", success ? "success" : "failure"), memberName, sourceFilePath, sourceLineNumber);
				if (success) {
					Parent.CloseFromPopulationThread();
				} else {
					Parent.PushMessage("");
					Parent.PushMessage("Errors occurred. Please read the messages above, then close this window to proceed.");
				}
			}
		}

		private ProgressFormReporter Progress;
		private Thread PopulationThread;
		private AutoResetEvent ResetEvent = new AutoResetEvent(false);
		private Queue<string> MessageQueue = new Queue<string>();
		private int CurrentProgress = 0;
		private int TotalProgress = 0;
		private object ThreadSyncLock = new object();
		private object GuiUpdateThreadLock = new object();
		private bool ShouldTerminate = false;
		private bool IsAlreadyClosed = false;

		public ProgressForm() {
			InitializeComponent();
			Progress = new ProgressFormReporter(this);
			PopulationThread = new Thread(GuiThreadFunc);
		}

		private void ProgressForm_Shown(object sender, EventArgs e) {
			lock (ThreadSyncLock) {
				PopulationThread.Start();
			}
		}

		private void GuiThreadFunc() {
			int lastProgressCurrent = 0;
			int lastProgressTotal = 0;

			while (true) {
				string msg = null;
				int progressCurrent = 0;
				int progressTotal = 0;
				bool shouldTerminateThread = false;
				lock (GuiUpdateThreadLock) {
					shouldTerminateThread = ShouldTerminate;
					if (!shouldTerminateThread) {
						if (MessageQueue.Count > 0) {
							msg = MessageQueue.Dequeue();
						}

						if (lastProgressCurrent != CurrentProgress || lastProgressTotal != TotalProgress) {
							progressCurrent = CurrentProgress;
							progressTotal = TotalProgress;
						}
					}
				}

				if (shouldTerminateThread) {
					try {
						bool windowAlreadyClosed = false;
						lock (GuiUpdateThreadLock) {
							windowAlreadyClosed = IsDisposed || IsAlreadyClosed;
						}
						if (!windowAlreadyClosed) {
							// i think there's still a potential race condition somewhere around here???
							// i guess it doesn't matter too much, it just means a zombie thread will hang around until program end... but still, ugh
							Invoke(new Action(() => {
								try {
									bool alreadyClosed = false;
									lock (GuiUpdateThreadLock) {
										alreadyClosed = IsAlreadyClosed;
									}
									if (!alreadyClosed) {
										Close();
									}
								} catch (Exception) { }
							}));
						}
					} catch (Exception) { }
					return;
				}

				if (msg != null || progressTotal > 0) {
					try {
						Invoke(new Action(() => {
							if (msg != null) {
								textBox.AppendText(string.Format("{0}{1}", msg, Environment.NewLine));
							}
							if (progressTotal > 0) {
								lastProgressCurrent = progressCurrent;
								lastProgressTotal = progressTotal;
								label.Text = string.Format("Completed {0} out of {1} tasks...", progressCurrent, progressTotal);
							}
						}));
					} catch (Exception) { }
					continue;
				}

				ResetEvent.WaitOne(TimeSpan.FromMilliseconds(100));
			}
		}

		public SenLib.ProgressReporter GetProgressReporter() {
			return Progress;
		}

		internal void PushMessage(string message, int current = -1, int total = -1) {
			lock (GuiUpdateThreadLock) {
				if (current != -1 && total != -1) {
					CurrentProgress = current;
					TotalProgress = total;
				}
				MessageQueue.Enqueue(message);
			}
			ResetEvent.Set();
		}

		internal void CloseFromPopulationThread() {
			lock (GuiUpdateThreadLock) {
				ShouldTerminate = true;
			}
			ResetEvent.Set();
		}

		private void ProgressForm_FormClosing(object sender, FormClosingEventArgs e) {
			lock (GuiUpdateThreadLock) {
				ShouldTerminate = true;
				IsAlreadyClosed = true;
			}
			ResetEvent.Set();
		}
	}
}
