@echo off
set start=%time%

SETLOCAL EnableDelayedExpansion

IF NOT DEFINED DevEnvDir (
	call C:\"Program Files (x86)"\"Microsoft Visual Studio"\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat x64
)

set CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -DTETRIS_INTERNAL=1 -DTETRIS_SLOW=1 -FC -Z7 
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2> NUL
echo WAITING FOR PDB > lock.tmp

:: tetris_console.exe
:: cl %CommonCompilerFlags% /EHsc ..\code\tetris_console.cpp /link %CommonLinkerFlags% User32.lib

:: tetris.dll
cl %CommonCompilerFlags% ..\code\tetris.cpp /LD /link -incremental:no -opt:ref -PDB:tetris_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
if !errorlevel! neq 0 goto CLEANUPERROR

:: Skip trying to build tetris.exe if currently running
FOR /F %%x IN ('tasklist /NH /FI "IMAGENAME eq win32_tetris.exe"') DO IF %%x == win32_tetris.exe goto FOUND

cl %CommonCompilerFlags% ..\code\win32_tetris.cpp /link %CommonLinkerFlags%
if !errorlevel! neq 0 goto CLEANUPERROR

:FOUND
set end=%time%
set options="tokens=1-4 delims=:.,"
for /f %options% %%a in ("%start%") do set start_h=%%a&set /a start_m=100%%b %% 100&set /a start_s=100%%c %% 100&set /a start_ms=100%%d %% 100
for /f %options% %%a in ("%end%") do set end_h=%%a&set /a end_m=100%%b %% 100&set /a end_s=100%%c %% 100&set /a end_ms=100%%d %% 100

set /a hours=%end_h%-%start_h%
set /a mins=%end_m%-%start_m%
set /a secs=%end_s%-%start_s%
set /a ms=%end_ms%-%start_ms%
if %ms% lss 0 set /a secs = %secs% - 1 & set /a ms = 100%ms%
if %secs% lss 0 set /a mins = %mins% - 1 & set /a secs = 60%secs%
if %mins% lss 0 set /a hours = %hours% - 1 & set /a mins = 60%mins%
if %hours% lss 0 set /a hours = 24%hours%
if 1%ms% lss 100 set ms=0%ms%

set /a totalsecs = %hours%*3600 + %mins%*60 + %secs%
echo Build Duration : %mins%:%secs%.%ms% (%totalsecs%.%ms%s total)

del /q *.obj
del lock.tmp
popd
goto:eof

:CLEANUPERROR
del lock.tmp
popd
exit /b 1