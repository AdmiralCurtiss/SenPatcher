SenPatcher
==========

Fixes and customizations for:
- The XSEED PC release of The Legend of Heroes: Trails of Cold Steel
- The XSEED PC release of The Legend of Heroes: Trails of Cold Steel II
- The NISA PC release of The Legend of Heroes: Trails of Cold Steel III
- The NISA PC release of The Legend of Heroes: Trails of Cold Steel IV

https://github.com/AdmiralCurtiss/SenPatcher


----

Prerequisites
-------------

- If you have installed a version of SenPatcher before v1.0, please restore your game before continuing.
  - Launch the old version of SenPatcher again and click the "Remove Patches / Restore Original" button.
  - Alternatively, verify the game files in Steam or GoG Galaxy. It will detect and redownload any patches files and replace them with unpatched ones.
- Make sure your game is updated to a supported version (CS1 v1.6, CS2 v1.4.1 or v1.4.2, CS3 v1.06, CS4 v1.2.1). Other versions of the games will not work.

Windows
-------

- Place the contents of the folder for the game you want to patch into your game install directory.
  - The DINPUT8.dll should end up in the same folder as the game executable, ie. the executables starting with 'ed8'.
- Adjust configuration in senpatcher_settings.ini to taste.
- Run the game as usual.
- To uninstall just delete DINPUT8.dll from the folder with the game executable.

Steam Deck
----------

- Switch to Desktop mode.
- Place the contents of the folder for the game you want to patch into your game install directory.
  - The DINPUT8.dll should end up in the same folder as the game executable, ie. the executables starting with 'ed8'.
- Right-click the game in the Steam game list and select 'Properties'.
- In the General tab, add the following text (without the backticks!) into the advanced Launch Options field at the bottom: `WINEDLLOVERRIDES=DINPUT8=n,b %command%`
- Adjust configuration in senpatcher_settings.ini to taste.
- You can switch back to Game mode now.
- To uninstall just delete DINPUT8.dll from the folder with the game executable.

----

For modders:

This version of SenPatcher has *file mod support*. This allows seamless replacement of files as seen by the game without actually replacing them in the file system.

This comes in two flavors:

For development, you can create a `dev` directory in the base game directory and put your replacement files in there. Files in this folder will have priority over everything else, so eg. if you want to replace the t_magic table in CS4, you can place your modified copy at `The Legend of Heroes Trails of Cold Steel IV\dev\data\text\dat_en\t_magic.tbl`. This should work for all game files *except* the FMVs and the big `assets.pka`. Note that the `dev` directory must exist when the game is booted in order for this replacement to take effect.

Once you're ready to release your mod into the world, you can pack it into a convenient archive file for distribution. In order to do that, take the included `p3a.exe` and pack the `dev` directory with it, so for example `p3a.exe pack --compression lz4 "The Legend of Heroes Trails of Cold Steel IV\dev" mymod.p3a`. The generated `mymod.p3a` can be placed in the `mods` directory that is created by SenPatcher, where it will act exactly like the freestanding `dev` directory, replacing the files as seen by the game. (Yes, this is the same file format used by the PH3 Daybreak PC port. You may choose to use their tool instead if you prefer it.)

If there are multiple copies of a single file across multiple mods, only the highest priority one will be seen by the game. The priority list is:

- Files in the `dev` directory (highest priority)
- Files contained in the p3a archives in the `mods` directory. The `order.txt` in there can be used to change which mods have priority over which other mods, higher in the list has priority.
- Base game files (lowest priority)

----


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
* Option to allow changing difficulty to Nightmare mid-playthrough.
* Option to disable the mouse being captured by the game and its movement being used for the game camera.
* Option to fix a bug that applies complicated button mappings incorrectly, causing the actual button actions and the displayed button prompts to mismatch.
* Option to force XInput for controllers (this supposedly fixes newer Xbox controllers)
* Option to keep the game running while in background.

Note: Compatible with version 1.06 of the game only.

Feature Set for Trails of Cold Steel IV
=======================================

* Option to allow changing difficulty to Nightmare mid-playthrough.
* Option to disable the mouse being captured by the game and its movement being used for the game camera.
* Option to keep the game running while in background.
* Option to force the controller button Confirm/Cancel layout to English/Xbox or Japanese/Nintendo style.

Note: Compatible with version 1.2.1 of the game only.
