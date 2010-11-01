echo off
rem BuildMergeModules.Bat - rebuilds the DSM Merge Modules

if "%VS90COMNTOOLS%"=="" goto MSVSerror 
echo %VS90COMNTOOLS% |findstr "Tools\\" > temp.txt
If %ERRORLEVEL% NEQ 0 goto :MSVSerror
del temp.txt

set VCBUILD=..\IDE\devenv.exe
set VCBUILD="%VS90COMNTOOLS%%VCBUILD%"


if not exist ".\merge_module\TWAINDSM32.vdproj" goto error3
if not exist ".\merge_module\TWAINDSM64.vdproj" goto error4
setlocal
findstr "\"Version\"" ".\merge_module\TWAINDSM32.vdproj" > temp.txt
set /p XXX= < temp.txt
del temp.txt
for /f "tokens=* delims= " %%a in ("%XXX%") do set XXX=%%a
set YYY=%XXX:~15,7%
echo 32 bit Merge module version %YYY%
endlocal

echo Is the DSM version shown above correct (Y/N)?
set /p DSMVerAnswer=
if %DSMVerAnswer% == Y goto Check64
if %DSMVerAnswer% == y goto Check64

if exist ".\merge_module\new.vdproj" del ".\merge_module\new.vdproj"
SETLOCAL EnableDelayedExpansion
:ReplaceVer
set /p ver="Type the right version (X.X.X.X): "
echo "%ver%" |findstr "[0-9]\.[0-9]\.[0-9]\.[0-9]" >temp.txt
If %ERRORLEVEL% EQU 0 goto :l11
echo "Wrong version format. Use X.X.X.X"
goto ReplaceVer

:l11
set rep=        "Version" = "8:
set rep=%rep%%ver%
set ver="
set rep=%rep%%ver%



for /f "tokens=* delims=" %%a in (.\merge_module\TWAINDSM32.vdproj) do  (
set Temp=%%a
call :replace
)
ENDLOCAL EnableDelayedExpansion

del ".\merge_module\TWAINDSM32.vdproj"
move /y ".\merge_module\new.vdproj" ".\merge_module\TWAINDSM32.vdproj" >>  temp.txt
if exist temp.txt del temp.txt


goto Check64


:replace

echo "%Temp%" |findstr ":[0-9]\.[0-9]\.[0-9]\.[0-9]" >temp.txt
If %ERRORLEVEL% EQU 0 goto :l1
(echo !Temp!)>>".\merge_module\new.vdproj" 
goto :l2
:l1
(echo !rep!)>>".\merge_module\new.vdproj"
:l2

exit /b

 
:Check64
setlocal
findstr "\"Version\"" ".\merge_module\TWAINDSM64.vdproj" > temp.txt
set /p XXX= < temp.txt
del temp.txt
for /f "tokens=* delims= " %%a in ("%XXX%") do set XXX=%%a
set YYY=%XXX:~15,7%
echo 64 bit Merge module version %YYY%
endlocal

echo Is the DSM version shown above correct (Y/N)?
set /p DSMVerAnswer=
if %DSMVerAnswer% == Y goto BuildDSM
if %DSMVerAnswer% == y goto BuildDSM

if exist ".\merge_module\new.vdproj" del ".\merge_module\new.vdproj"
SETLOCAL EnableDelayedExpansion
:ReplaceVer_64
set /p ver="Type the right version (X.X.X.X): "
echo "%ver%" |findstr "[0-9]\.[0-9]\.[0-9]\.[0-9]" >temp.txt
If %ERRORLEVEL% EQU 0 goto :l11
echo "Wrong version format. Use X.X.X.X"
goto ReplaceVer_64

:l11
set rep=        "Version" = "8:
set rep=%rep%%ver%
set ver="
set rep=%rep%%ver%



for /f "tokens=* delims=" %%a in (.\merge_module\TWAINDSM64.vdproj) do  (
set Temp=%%a
call :replace_64
)
ENDLOCAL EnableDelayedExpansion

del ".\merge_module\TWAINDSM64.vdproj"
move /y ".\merge_module\new.vdproj" ".\merge_module\TWAINDSM64.vdproj" >>  temp.txt
if exist temp.txt del temp.txt

goto BuildDSM


:replace_64

echo "%Temp%" |findstr ":[0-9]\.[0-9]\.[0-9]\.[0-9]" >temp.txt
If %ERRORLEVEL% EQU 0 goto :l1_64
(echo !Temp!)>>".\merge_module\new.vdproj" 
goto :l2_64
:l1_64
(echo !rep!)>>".\merge_module\new.vdproj"
:l2_64
exit /b
 
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

:MSVSerror
if exist temp.txt del temp.txt
echo - MSVC 2008 not found. Please install it and try again
pause
exit /b 1