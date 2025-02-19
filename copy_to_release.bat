rd /Q /S SenPatcher-release
mkdir SenPatcher-release
mkdir "SenPatcher-release\Trails of Cold Steel"
mkdir "SenPatcher-release\Trails of Cold Steel II"
mkdir "SenPatcher-release\Trails of Cold Steel II\bin"
mkdir "SenPatcher-release\Trails of Cold Steel II\bin\Win32"
mkdir "SenPatcher-release\Tokyo Xanadu eX+"
mkdir "SenPatcher-release\The Legend of Heroes Trails of Cold Steel III"
mkdir "SenPatcher-release\The Legend of Heroes Trails of Cold Steel III\bin"
mkdir "SenPatcher-release\The Legend of Heroes Trails of Cold Steel III\bin\x64"
mkdir "SenPatcher-release\The Legend of Heroes Trails of Cold Steel IV"
mkdir "SenPatcher-release\The Legend of Heroes Trails of Cold Steel IV\bin"
mkdir "SenPatcher-release\The Legend of Heroes Trails of Cold Steel IV\bin\Win64"
mkdir "SenPatcher-release\The Legend of Heroes Trails into Reverie"
mkdir "SenPatcher-release\The Legend of Heroes Trails into Reverie\bin"
mkdir "SenPatcher-release\The Legend of Heroes Trails into Reverie\bin\Win64"
copy /b README.md SenPatcher-release\README.txt
copy /b LICENSE SenPatcher-release\LICENSE.txt
copy /b native\build\release64\sentools.exe SenPatcher-release\SenPatcher.exe
copy /b native\build\release32\cs1hook.dll "SenPatcher-release\Trails of Cold Steel\DINPUT8.dll"
copy /b native\build\release32\cs2hook.dll "SenPatcher-release\Trails of Cold Steel II\bin\Win32\DINPUT8.dll"
copy /b native\build\release32\txhook.dll "SenPatcher-release\Tokyo Xanadu eX+\DINPUT8.dll"
copy /b native\build\release64\cs3hook.dll "SenPatcher-release\The Legend of Heroes Trails of Cold Steel III\bin\x64\DINPUT8.dll"
copy /b native\build\release64\cs4hook.dll "SenPatcher-release\The Legend of Heroes Trails of Cold Steel IV\bin\Win64\DINPUT8.dll"
copy /b native\build\release64\reveriehook.dll "SenPatcher-release\The Legend of Heroes Trails into Reverie\bin\Win64\DSOUND.dll"
copy /b SenLib\Sen1\senpatcher_settings.ini "SenPatcher-release\Trails of Cold Steel\senpatcher_settings.ini"
copy /b SenLib\Sen2\senpatcher_settings.ini "SenPatcher-release\Trails of Cold Steel II\senpatcher_settings.ini"
copy /b SenLib\TX\senpatcher_settings.ini "SenPatcher-release\Tokyo Xanadu eX+\senpatcher_settings.ini"
copy /b SenLib\Sen3\senpatcher_settings.ini "SenPatcher-release\The Legend of Heroes Trails of Cold Steel III\senpatcher_settings.ini"
copy /b SenLib\Sen4\senpatcher_settings.ini "SenPatcher-release\The Legend of Heroes Trails of Cold Steel IV\senpatcher_settings.ini"
copy /b SenLib\Sen5\senpatcher_settings.ini "SenPatcher-release\The Legend of Heroes Trails into Reverie\senpatcher_settings.ini"
echo Looking for SenTools?>SenPatcher-release\sentools.txt
echo.>>SenPatcher-release\sentools.txt
echo SenTools is now the same executable as SenPatcher.exe.>>SenPatcher-release\sentools.txt
echo Just copy or rename it to sentools.exe, or use it as-is.>>SenPatcher-release\sentools.txt
echo Run the file from a command prompt to get the CLI interface.>>SenPatcher-release\sentools.txt
echo.>>SenPatcher-release\sentools.txt
native\build\release64\sentools.exe --help>>SenPatcher-release\sentools.txt
