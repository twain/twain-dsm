@echo off
set ProjectDir=%1

::
:: Create the folder if we don't have it...
::
if not exist "%ProjectDir%..\pub\include\twain" (
	mkdir "%ProjectDir%..\pub"
	mkdir "%ProjectDir%..\pub\include"
	mkdir "%ProjectDir%\..\pub\include\twain"
)

::
:: Copy the header file...
::
echo copy "twain.h" to "..\pub\include\twain"
xcopy "%ProjectDir%..\src\twain.h" "%ProjectDir%..\pub\include\twain" /r /y /q