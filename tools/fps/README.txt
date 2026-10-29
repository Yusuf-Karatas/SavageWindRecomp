Savage Wind with the FPS boost (0.3.1-alpha.3), no cheats

1. Extract this "Savage Wind FPS" folder anywhere, for example next to your Savage Wind folder.
2. If your Savage Wind folder is not in Downloads\Games\SavageWind-0.3.1-alpha.3-windows\Savage Wind,
   open each .bat file in Notepad and fix the GAME_FOLDER line.
3. Double-click play-fps-boost.bat. Press F10 in game for the FPS counter.

What's faster: a speedup from the Savage Wind developers' next release. The CPU builds the next frame
while the GPU draws this one, plus fixes from the same release. Frame rate, VSync and the
upscaler are in the game's Options. The first minutes may stutter while shaders are rebuilt for this exe.

Save backups: every start of a .bat file first copies your saves (the "save" folder) and
savagewind.ini to save-backups\<date and time> in this folder, and keeps the 10 newest. If a save
goes wrong, close the game and copy a backup's "save" folder back into your Savage Wind folder.

Test launchers: test-overlap-off.bat turns the CPU/GPU overlap (the bigger half of the speedup) off.
test-cbuf-on.bat turns the smaller constant buffer copies back on: they are off in this build, the likely cause of
dust and smoke vanishing at the end of their effect instead of fading.

Your Savage Wind folder is used as it is: nothing there is replaced. To play without the boost, start
the original savagewind.exe as usual.
