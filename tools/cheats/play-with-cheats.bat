@echo off
rem Starts this savagewind.exe (with the Cheats tab) on your existing Savage Wind folder: its game files,
rem game code, saves and settings. Nothing in that folder is replaced.
rem Set GAME_FOLDER to your Savage Wind folder (the one with the original savagewind.exe).
set "GAME_FOLDER=%USERPROFILE%\Downloads\Games\SavageWind-0.3.1-alpha.3-windows\Savage Wind"
if not exist "%GAME_FOLDER%\savagewind.exe" (
  echo Savage Wind folder not found: %GAME_FOLDER%
  echo Open this file in Notepad and fix the GAME_FOLDER line.
  pause
  exit /b 1
)
set "SAVAGEWIND_DATA_ROOT=%GAME_FOLDER%"
start "" "%~dp0savagewind.exe"
