echo off
rem BuildMergeModules.Bat - rebuilds the DSM Merge Modules

if exist "%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" goto 32bitWindows
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" goto 64bitWindows

echo - MSVC 2008 not found. Please installe it and try again
exit /b 1

:32bitWindows
set VCBUILD="%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" 
goto CheckVersion

:64bitWindows
set VCBUILD="%ProgramFiles(x86)%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" 
goto CheckVersion

:CheckVersion
echo off
if not exist ".\merge_module\TWAINDSM32.vdproj" goto error3
if not exist ".\merge_module\TWAINDSM64.vdproj" goto error4
setlocal
findstr "\"Version\"" ".\merge_module\TWAINDSM32.vdproj" > temp.txt
set /p XXX= < temp.txt
del temp.txt
for /f "tokens=* delims= " %%a in ("%XXX%") do set XXX=%%a
set YYY=%XXX:~15,7%
echo 32 bit Merge module version %YYY%
findstr "\"Version\"" ".\merge_module\TWAINDSM64.vdproj" > temp.txt
set /p XXX= < temp.txt
del temp.txt
for /f "tokens=* delims= " %%a in ("%XXX%") do set XXX=%%a
set YYY=%XXX:~15,7%
echo 64 bit Merge module version %YYY%
endlocal

echo Are the DSM versions shown above correct (Y/N)?
set /p DSMVerAnswer=
if %DSMVerAnswer% == Y goto BuildDSM
if %DSMVerAnswer% == y goto BuildDSM

echo Build aborted
pause
exit /b 1
 
:BuildDSM
echo off
echo Build started ...
if exist "./buildMSM_x64.log" del "./buildMSM_x64.log"
if exist "./buildMSM_x86.log" del "./buildMSM_x86.log"

%VCBUILD% ".\merge_module\TWAINDSM merge.sln" /rebuild Release /project TWAINDSM32 /log ./buildMSM_x86.log /out ./buildMSM_x86.log
if %errorlevel% neq 0 goto error1
echo - 32 bit DSM Merge Module build succeeded
echo - Merge module is located in %CD%\pub\bin\twain32
del "./buildMSM_x86.log"

%VCBUILD% ".\merge_module\TWAINDSM merge.sln" /rebuild Release /project TWAINDSM64 /log ./buildMSM_x86.log /out ./buildMSM_x64.log
if %errorlevel% neq 0 goto error2
echo - 64 bit DSM Merge Module build succeeded
echo - Merge module is located in %CD%\pub\bin\twain64
del "./buildMSM_x64.log"

pause
exit /b 0

:error1
echo 32 bit DSM Merge Module build failed. See build_x86.log for details
pause
exit /b 1

:error2
echo 64 bit DSM Merge Module build failed. See build_x64.log for details
pause
exit /b 1


:error3
echo TWAINDSM32.vdproj file does not exist.
pause
exit /b 1

:error4
echo TWAINDSM64.vdproj file does not exist.
pause
exit /b 1