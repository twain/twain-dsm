echo off
rem BuildMergeModules.Bat - rebuilds the DSM Merge Modules

if exist "%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" goto 32bitWindows
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" goto 64bitWindows

echo - MSVC 2008 not found. Please installe it and try again
exit /b 1

:32bitWindows
set VCBUILD="%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" 
goto BuildDSM

:64bitWindows
set VCBUILD="%ProgramFiles(x86)%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" 
goto BuildDSM

 
:BuildDSM
echo off
echo Build started ...
if exist "./buildMSM_x64.log" del "./buildMSM_x64.log"
if exist "./buildMSM_x86.log" del "./buildMSM_x86.log"

%VCBUILD% ".\merge_module\TWAINDSM merge.sln" /rebuild Release /project TWAINDSM32 /log ./buildMSM_x86.log /out ./buildMSM_x86.log
if %errorlevel% neq 0 goto error1
echo - 32 bit DSM Merge Module build succeeded
del "./buildMSM_x86.log"

%VCBUILD% ".\merge_module\TWAINDSM merge.sln" /rebuild Release /project TWAINDSM64 /log ./buildMSM_x86.log /out ./buildMSM_x64.log
if %errorlevel% neq 0 goto error2
echo - 64 bit DSM Merge Module build succeeded
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