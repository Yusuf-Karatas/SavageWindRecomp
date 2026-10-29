@echo off
rem Starts this savagewind.exe (with the FPS boost) on your existing Savage Wind folder: its game files,
rem game code, saves and settings. Nothing in that folder is replaced.
rem Set GAME_FOLDER to your Savage Wind folder (the one with the original savagewind.exe).
set "GAME_FOLDER=%USERPROFILE%\Downloads\Games\SavageWind-0.3.1-alpha.3-windows\Savage Wind"
if not exist "%GAME_FOLDER%\savagewind.exe" (
  echo Savage Wind folder not found: %GAME_FOLDER%
  echo Open this file in Notepad and fix the GAME_FOLDER line.
  pause
  exit /b 1
)
rem Back up the saves and settings first, to save-backups\<date and time> next to this file. To recover, copy a
rem backup's "save" folder (and savagewind.ini) back into your Savage Wind folder. The 10 newest backups are kept.
for /f %%i in ('powershell -NoProfile -Command "Get-Date -Format yyyy-MM-dd_HH-mm-ss"') do set "STAMP=%%i"
set "BACKUP=%~dp0save-backups\%STAMP%"
if exist "%GAME_FOLDER%\save" (
  robocopy "%GAME_FOLDER%\save" "%BACKUP%\save" /E /NFL /NDL /NJH /NJS /NP >nul
  if errorlevel 8 (
    echo Could not back up your saves to %BACKUP%. The game was not started.
    pause
    exit /b 1
  )
)
if exist "%GAME_FOLDER%\savagewind.ini" copy /y "%GAME_FOLDER%\savagewind.ini" "%BACKUP%\" >nul
powershell -NoProfile -Command "Get-ChildItem -Directory '%~dp0save-backups' | Sort-Object Name -Descending | Select-Object -Skip 10 | Remove-Item -Recurse -Force" >nul 2>&1
set "SAVAGEWIND_DATA_ROOT=%GAME_FOLDER%"
rem Test launcher: both halves of the speedup off (for the white lines in rain).
set "SAVAGEWIND_GPU_OVERLAP=0"
set "SAVAGEWIND_NO_CBUF_REACH=1"
start "" "%~dp0savagewind.exe"
