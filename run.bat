@echo off
setlocal
rem Build with the existing WSL toolchain, then run the emulator natively.
rem Set KAKURO_SKIP_BUILD=1 to launch the already-built game.
if "%KAKURO_SKIP_BUILD%"=="1" goto find_emulator
where wsl.exe >nul 2>nul
if errorlevel 1 (
    echo WSL is required to build. Build with make first and set KAKURO_SKIP_BUILD=1.
    goto failed
)
wsl.exe --cd "%~dp0." --exec make
if errorlevel 1 goto failed

:find_emulator
if defined X16_EMU goto emulator_found
set "X16_EMU=%~dp0build\windows-emulator\x16emu.exe"
if exist "%X16_EMU%" goto emulator_found
set "X16_EMU=%~dp0..\x16-emulator\build\x16emu.exe"
if exist "%X16_EMU%" goto emulator_found
set "X16_EMU=%~dp0..\x16-emulator\build\Release\x16emu.exe"
if exist "%X16_EMU%" goto emulator_found
set "X16_EMU=%~dp0..\x16-emulator\x16emu.exe"
if exist "%X16_EMU%" goto emulator_found
set "X16_EMU=%~dp0..\emulator\x16emu.exe"
if exist "%X16_EMU%" (
    echo Using the existing Windows emulator. For the new emulator, run build-emulator-windows.bat.
)

:emulator_found
if not exist "%X16_EMU%" (
    echo Windows emulator not found: "%X16_EMU%"
    echo Run build-emulator-windows.bat or set X16_EMU to an x16emu.exe path.
    goto failed
)
if not defined X16_ROM set "X16_ROM=%~dp0..\emulator\rom.bin"
if not exist "%X16_ROM%" (
    echo ROM not found: "%X16_ROM%". Set X16_ROM to your rom.bin path.
    goto failed
)
pushd "%~dp0src"
if not exist KAKURO.PRG (
    echo KAKURO.PRG is missing. Build with make first.
    popd
    goto failed
)
echo Running "%X16_EMU%" natively on Windows.
"%X16_EMU%" -rom "%X16_ROM%" -prg KAKURO.PRG -run -scale 2 %*
set "LAUNCH_RESULT=%ERRORLEVEL%"
popd
if not "%LAUNCH_RESULT%"=="0" goto failed
exit /b 0

:failed
echo Launch failed.
pause
exit /b 1
