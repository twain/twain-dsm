echo off
rem BuildWindowsDSM.Bat - rebuilds the DSM

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
if not exist "./src/resource.h" goto error3
findstr "TWNDSM_VERSION_NUM" ".\src\resource.h"
echo Is the DSM version shown above correct (Y/N)?
set /p DSMVerAnswer=
if %DSMVerAnswer% == Y goto BuildDSM
if %DSMVerAnswer% == y goto BuildDSM

echo Build aborted
pause
exit /b 1

:BuildDSM
echo Build started ...
if exist "./buildDSM_x64.log" del "./buildDSM_x64.log"
if exist "./buildDSM_x86.log" del "./buildDSM_x86.log"

%VCBUILD% ".\visual_studio\TWAIN_DSM_VS2008.sln" /rebuild Release /ProjectConfig "Release|Win32" /project TWAIN_DSM_VS2008 /log ./buildDSM_x86.log /out ./buildDSM_x86.log
if %errorlevel% neq 0 goto error1
echo - 32 bit DSM build succeeded
echo - DSM file is located in %CD%\pub\bin\twain32
del "./buildDSM_x86.log"

%VCBUILD% ".\visual_studio\TWAIN_DSM_VS2008.sln" /rebuild Release /ProjectConfig "Release|x64" /project TWAIN_DSM_VS2008 /log ./buildDSM_x86.log /out ./buildDSM_x64.log
if %errorlevel% neq 0 goto error2
echo - 64 bit DSM build succeeded
echo - DSM file is located in %CD%\pub\bin\twain64
del "./buildDSM_x64.log"
pause
exit /b 0

:error1
echo 32 bit DSM build failed. See build_x86.log for details
pause
exit /b 1

:error2
echo 64 bit DSM build failed. See build_x64.log for details
pause
exit /b 1


:error3
echo resource.h file does not exist.
pause
exit /b 1