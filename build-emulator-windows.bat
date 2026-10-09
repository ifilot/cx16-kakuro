@echo off
setlocal
rem Build the neighboring agent-capable emulator using MSYS2 UCRT64.
if not defined MSYS2_ROOT set "MSYS2_ROOT=C:\msys64"
set "TOOLCHAIN=%MSYS2_ROOT%\ucrt64"
if not exist "%TOOLCHAIN%\bin\cmake.exe" goto missing_tools
if not exist "%TOOLCHAIN%\bin\ninja.exe" goto missing_tools
if not exist "%TOOLCHAIN%\bin\gcc.exe" goto missing_tools
if not exist "%TOOLCHAIN%\bin\python.exe" goto missing_tools
if not exist "%TOOLCHAIN%\bin\SDL2.dll" goto missing_tools
if not exist "%TOOLCHAIN%\bin\zlib1.dll" goto missing_tools
set "PATH=%TOOLCHAIN%\bin;%MSYS2_ROOT%\usr\bin;%PATH%"
set "OUTPUT=%~dp0build\windows-emulator"
"%TOOLCHAIN%\bin\cmake.exe" -S "%~dp0..\x16-emulator" -B "%OUTPUT%" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_PREFIX_PATH="%TOOLCHAIN%" -DPython3_EXECUTABLE="%TOOLCHAIN%\bin\python.exe" -DENABLE_FLUIDSYNTH=OFF -DBUILD_TESTING=OFF
if errorlevel 1 goto failed
"%TOOLCHAIN%\bin\cmake.exe" --build "%OUTPUT%" --target x16emu --parallel
if errorlevel 1 goto failed
for %%D in (SDL2.dll zlib1.dll libwinpthread-1.dll libstdc++-6.dll libgcc_s_seh-1.dll) do (
    copy /y "%TOOLCHAIN%\bin\%%D" "%OUTPUT%\" >nul
    if errorlevel 1 goto failed
)
echo Built "%OUTPUT%\x16emu.exe". Run run.bat to play.
pause
exit /b 0

:missing_tools
echo Install these packages from an MSYS2 UCRT64 terminal:
echo pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-python mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-zlib
echo If MSYS2 is elsewhere, set MSYS2_ROOT to its installation directory.
:failed
echo Windows emulator build failed.
pause
exit /b 1
