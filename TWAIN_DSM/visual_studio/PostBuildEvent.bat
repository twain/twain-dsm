@echo off
set ProjectDir=%1
set Pub=%ProjectDir%\..\pub
set OutDir=%ProjectDir%%2
set Dest=%3

::
:: Create the folders if we don't have them...
::
mkdir "%Pub%\include\twain" > NUL 2>&1
mkdir "%Pub%\bin\twain32" > NUL 2>&1
mkdir "%Pub%\bin\twain64" > NUL 2>&1

::
:: Copy the header file...
::
echo copy "%ProjectDir%\..\src\twain.h" to "%Pub%\include\twain"
xcopy "%ProjectDir%\..\src\twain.h" "%Pub%\include\twain" /r /y /q

::
:: Copy the binary...
::
if exist "%OutDir%twaindsm.dll" (
	echo copy "%OutDir%\twaindsm.dll" to "%Pub%\bin\%Dest%"
	xcopy "%OutDir%twaindsm.dll" "%Pub%\bin\%Dest%" /r /y /q
)
