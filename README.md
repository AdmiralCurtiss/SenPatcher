SenPatcher
==========

SenPatcher is a collection of fixes and customizations for several PC ports of games developed by Falcom, specifically:
- The XSEED PC release of The Legend of Heroes: Trails of Cold Steel
- The XSEED PC release of The Legend of Heroes: Trails of Cold Steel II
- The NISA PC release of The Legend of Heroes: Trails of Cold Steel III
- The NISA PC release of The Legend of Heroes: Trails of Cold Steel IV
- The NISA PC release of The Legend of Heroes: Trails into Reverie

It also acts as a mod loader so file-based mods can be distributed and installed without modifying the base game files.

Release binaries: https://github.com/AdmiralCurtiss/SenPatcher/releases

Source code: https://github.com/AdmiralCurtiss/SenPatcher

----


Installation Guide
==================


Prerequisites
-------------

- If you have installed a version of SenPatcher before v1.0, please restore your game before continuing.
  - The `SenPatcherGui.exe` provided in the release binary package should offer to automatically restore the game for you when you open the relevant game.
  - Alternatively, verify the game files in Steam or GoG Galaxy. It will detect and redownload any patched files and replace them with unpatched ones.
- Make sure your game is updated to a supported version (CS1 v1.6, CS2 v1.4.1 or v1.4.2, CS3 v1.06, CS4 v1.2.1, Reverie v1.1.4). Other versions of the games will not work.


Windows (using the GUI)
-----------------------

- Extract the entire zip file of the release binary package you downloaded anywhere you like.
- Launch `SenPatcherGui.exe`.
- Click the 'Patch game' button for the game you'd like to patch.
- Navigate to the relevant game folder and open the file requested by the dialog.
  - In Steam, you can find this folder by selecting the game in the Library, opening its Properties, and going to 'Installed Files' -> 'Browse...'
  - In GOG Galaxy, you can find this folder by right-clicking the game and selecting 'Manage installation' -> 'Show folder'
- Adjust the active patches as desired. If you're unsure, just leave the defaults.
  - Note that some base functionality, such as the mod loader, is always active and cannot be disabled.
  - You can always come back here and adjust it again later by just following this procedure again.
- Hit the big 'Patch!' button at the bottom.
- Run the game as usual.

You can verify that SenPatcher is running correctly by checking the lower-right corner of the title screen. If SenPatcher is successfully running, it will display its version information as well as information about loaded mods next to the game's version string.

To uninstall, launch SenPatcherGui.exe again, click 'Patch game' for the relevant game, and use the 'Remove Patches / Restore Original' button at the top.


Windows (manual)
----------------

- Place the contents of the folder for the game you want to patch into your game install directory.
  - The provided layout in the release binary package for each game should be correct, but to be specific:
  - `senpatcher_settings.ini` should end up in the root directory of the game (next to the `data` directory).
  - The `.dll` file's name and location changes depending on the game:
    - For CS1 it's `DINPUT8.dll` in the root of the game folder, next to `ed8.exe`.
    - For CS2 it's `DINPUT8.dll` in `bin/Win32`, next to `ed8_2_PC_US.exe`.
    - For CS3 it's `DINPUT8.dll` in `bin/x64`, next to `ed8_3_PC.exe`.
    - For CS4 it's `DINPUT8.dll` in `bin/Win64`, next to `ed8_4_PC.exe`.
    - For Reverie it's `DSOUND.dll` in `bin/Win64`, next to `hnk.exe`.
- Adjust the active patches as desired by modifiying `senpatcher_settings.ini`.
- Run the game as usual.

To uninstall, just delete `DINPUT8.dll` from the folder with the game executable.


Steam Deck
----------

- Switch to Desktop mode.
- Place the contents of the folder for the game you want to patch into your game install directory.
  - You can find this folder by selecting the game in the Library, opening its Properties, and going to 'Installed Files' -> 'Browse...'
  - The provided layout in the release binary package for each game should be correct, but to be specific:
  - `senpatcher_settings.ini` should end up in the root directory of the game (next to the `data` directory).
  - The `.dll` file's name and location changes depending on the game:
    - For CS1 it's `DINPUT8.dll` in the root of the game folder, next to `ed8.exe`.
    - For CS2 it's `DINPUT8.dll` in `bin/Win32`, next to `ed8_2_PC_US.exe`.
    - For CS3 it's `DINPUT8.dll` in `bin/x64`, next to `ed8_3_PC.exe`.
    - For CS4 it's `DINPUT8.dll` in `bin/Win64`, next to `ed8_4_PC.exe`.
    - For Reverie it's `DSOUND.dll` in `bin/Win64`, next to `hnk.exe`.
- Right-click the game in the Steam game list and select 'Properties'.
- In the General tab, add the following text (without the backticks!) into the advanced Launch Options field at the bottom:
  - For CS1-4: `WINEDLLOVERRIDES=DINPUT8=n,b %command%`
  - For Reverie: `WINEDLLOVERRIDES=DSOUND=n,b %command%`
- Adjust the active patches as desired by modifiying `senpatcher_settings.ini`.
- You can switch back to Gaming mode now and run the game as usual.

To uninstall, just delete `DINPUT8.dll` or `DSOUND.dll` from the folder with the game executable.


Feature Set for Trails of Cold Steel
====================================

* Option to remove the forced animation skip in Turbo mode.
* Option to fix the internal Thor master quartz HD texture ID to allow usage of the HD texture pack for it.
* Option to fix various minor script and asset file errors, such as missing sound effects, incorrect voice clips, text typos, etc.
* Option to remap the button used for triggering Turbo mode and to re-enable the 2nd Notebook shortcut even when Turbo is on.
* Option to disable the mouse being captured by the game and its movement being used for the game camera.
* Option to keep the game running while in background.
* Option to fix a bug that makes the game load the English lipflap data when playing with Japanese voices and vice-versa.
* Option to fix a bug that displays the Arts Support cut-in incorrectly at resolutions other than 1280x720.
* Option to fix a bug that causes incorrect music playback when you switch areas too quickly.
* Option to use CS3 font spacing when using the high-res font from CS3.
* Option to force XInput for controllers (this supposedly fixes newer Xbox controllers)
* Option to edit the vestigial System Data button mapping from the PS3 version.

Note: Compatible with version 1.6 of the game only.


Feature Set for Trails of Cold Steel II
=======================================

* Option to remove the forced animation skip in Turbo mode.
* Option to fix two audio-related bugs, which cause missing or incorrect BGM/SFX and overlong track transitions.
* Option to fix a few minor script errors.
* Option to disable the mouse being captured by the game and its movement being used for the game camera.
* Option to keep the game running while in background.
* Option to fix a bug that makes the game load the English lipflap data when playing with Japanese voices and vice-versa.
* Option to fix a bug that applies complicated button mappings incorrectly, causing the actual button actions and the displayed button prompts to mismatch.
* Option to fix a bug that displays the Arts Support cut-in incorrectly.
* Option to use CS3 font spacing when using the high-res font from CS3.
* Option to force XInput for controllers (this supposedly fixes newer Xbox controllers)
* Experimental option to fix the occasional crash when analyzing enemies.
* Option to edit the vestigial System Data button mapping from the PS3 version.

Note: Compatible with versions 1.4, 1.4.1, and 1.4.2 of the game only.


Feature Set for Trails of Cold Steel III
========================================

* Option to fix various minor script and asset file errors, such as incorrect textures, incorrect voice clips, item description errors, broken Master Quartz effects, etc.
* Option to fix the in-game button remapping options being incorrectly enabled/disabled, disallowing various sensible mappings.
* Option to fix a bug that causes incorrect music playback when you switch areas too quickly.
* Option to allow changing difficulty to Nightmare mid-playthrough.
* Option to disable the mouse being captured by the game and its movement being used for the game camera.
* Option to fix a bug that applies complicated button mappings incorrectly, causing the actual button actions and the displayed button prompts to mismatch.
* Option to force XInput for controllers (this supposedly fixes newer Xbox controllers)
* Option to keep the game running while in background.

Note: Compatible with version 1.06 of the game only.

Feature Set for Trails of Cold Steel IV
=======================================

* Option to allow changing difficulty to Nightmare mid-playthrough.
* Option to fix a bug that causes incorrect music playback when you switch areas too quickly.
* Option to disable the mouse being captured by the game and its movement being used for the game camera.
* Option to keep the game running while in background.
* Option to force the controller button Confirm/Cancel layout to English/Xbox or Japanese/Nintendo style.

Note: Compatible with version 1.2.1 of the game only.

Feature Set for Trails into Reverie
===================================

* Option to fix a bug that causes incorrect music playback when you switch areas too quickly.
* Option to disable the mouse being captured by the game and its movement being used for the game camera.
* Option to remove the 30 FPS limit when the game is in the background.

Note: Compatible with version 1.1.4 of the game only.


Information for people wanting to create file-based mods
========================================================

Starting with version 1.0, SenPatcher has support for seamless replacement of files as seen by the game without actually modifying the game's assets directly.

This feature comes in two flavors:

For development, you can create a `dev` directory in the base game directory and put your replacement files in there. Files in this folder will have priority over everything else, so eg. if you want to replace the t_magic table in CS4, you can place your modified copy at `The Legend of Heroes Trails of Cold Steel IV\dev\data\text\dat_en\t_magic.tbl`. This should work for all game files *except* the FMVs and the big `assets.pka`. Note that the `dev` directory must exist when the game is booted in order for this replacement to take effect. You will see a notice on the title screen (next to the version info) if the `dev` folder is active and being scanned.

Once you're ready to release your mod into the world, you can pack it into a convenient archive file for distribution. In order to do that, take the included `sentools.exe` and pack the `dev` directory with it, so for example `sentools.exe P3A.Pack -c lz4 "The Legend of Heroes Trails of Cold Steel IV\dev" mymod.p3a`. The generated `mymod.p3a` can be placed in the `mods` directory that is created by SenPatcher, where it will act exactly like the freestanding `dev` directory, replacing the files as seen by the game. (Yes, this is the same file format used by the PH3 Daybreak PC port. You may choose to use their tool instead if you prefer it.)

If there are multiple copies of a single file across multiple mods, only the highest priority one will be seen by the game. The priority list is:

- Files in the `dev` directory (highest priority)
- Files contained in the p3a archives in the `mods` directory.
  - The `order.txt` in there can be used to change which mods have priority over which other mods, closer to the top of the list has higher priority.
  - If there is a p3a archive in the `mods` directory that is not in `order.txt`, SenPatcher will update `order.txt` automatically to include it below any previously existing mods but above its own asset fixes.
- Base game files (lowest priority)
