Savage Wind with the FPS boost (0.3.1-alpha.3), no cheats

1. Extract this "Savage Wind FPS" folder anywhere, for example next to your Savage Wind folder.
2. If your Savage Wind folder is not in Downloads\Games\SavageWind-0.3.1-alpha.3-windows\Savage Wind,
   open each .bat file in Notepad and fix the GAME_FOLDER line.
3. Double-click play-fps-boost.bat. Press F10 in game for the FPS counter.

What's faster: a speedup from the Savage Wind developers' next release. The CPU builds the next frame
while the GPU draws this one, and smaller constant buffer copies are made. Frame rate, VSync and the
upscaler are in the game's Options. The first minutes may stutter while shaders are rebuilt for this exe.

Save backups: every start of a .bat file first copies your saves (the "save" folder) and
savagewind.ini to save-backups\<date and time> in this folder, and keeps the 10 newest. If a save
goes wrong, close the game and copy a backup's "save" folder back into your Savage Wind folder.

Test launchers (white streaks across the screen in rain): test-overlap-off.bat, test-cbuf-off.bat and
test-speedup-off.bat each turn part of the speedup off.

Your Savage Wind folder is used as it is: nothing there is replaced. To play without the boost, start
the original savagewind.exe as usual.
