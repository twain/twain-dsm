echo off
rem BuildWindowsDSM.Bat - rebuilds the DSM
if exist ".\src\new.h" del ".\src\new.h"

if "%VS90COMNTOOLS%"=="" goto MSVSerror 
echo %VS90COMNTOOLS% |findstr "Tools\\" >temp.txt
If %ERRORLEVEL% NEQ 0 goto :MSVSerror
del temp.txt
		
set VCBUILD=..\IDE\devenv.exe
set VCBUILD="%VS90COMNTOOLS%%VCBUILD%"
	
if not exist "./src/resource.h" goto error3
findstr "TWNDSM_VERSION_NUM" ".\src\resource.h"
echo Is the DSM version shown above correct (Y/N)?
set /p DSMVerAnswer=
if %DSMVerAnswer% == Y goto BuildDSM
if %DSMVerAnswer% == y goto BuildDSM

SETLOCAL EnableDelayedExpansion
:ReplaceVer
set /p ver="Type the right version (X,X,X,X): "
echo "%ver%" |findstr /R "[0-9],[0-9],[0-9],[0-9]" 	
If %ERRORLEVEL% EQU 0 goto :l11
echo "Wrong version format. Use X,X,X,X"
goto ReplaceVer

:l11
for /f "tokens=1,2,3,4 delims=, " %%a in ("%ver%") do set v1=%%a&set v2=%%b&set v3=%%c&set v4=%%d
set ver=%v1%, %v2%, %v3%, %v4%

for /f "tokens=* delims=" %%a in (.\src\resource.h) do  (
set Temp=%%a
call :replace
)
ENDLOCAL EnableDelayedExpansion

del ".\src\resource.h"
move /y ".\src\new.h" ".\src\resource.h" >>  temp.txt
if exist temp.txt del temp.txt

goto BuildDSM


:replace

echo "%Temp%" |findstr "TWNDSM_VERSION_NUM" >temp.txt
If %ERRORLEVEL% EQU 0 goto :l1
echo "%Temp%" |findstr "TWNDSM_VERSION_STR" >temp.txt
If %ERRORLEVEL% EQU 0 goto :l2
(echo !Temp!)>>".\src\new.h" 
goto :l3
:l1
set repNum=#define TWNDSM_VERSION_NUM      
set repNum=%repNum%%ver%
(echo !repNum!)>>".\src\new.h"
goto l3
:l2
set repVer=#define TWNDSM_VERSION_STR      
set repVer=%repVer%"%ver%"
(echo !repVer!)>>".\src\new.h"
goto l3
:l3

exit /b


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

:MSVSerror
if exist temp.txt del temp.txt
echo - MSVC 2008 not found. Please install it and try again
pause
exit /b 1