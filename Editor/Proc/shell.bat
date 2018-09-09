@echo off
IF %1.==. GOTO Error1
set arg1=%1
if not defined DevEnvDir (
	call "E:/Programs/MicrosoftVisualStudio14.0/VC/vcvarsall.bat" %arg1%
)
GOTO End

:Error1
	echo Error: Platform needed (x86 / x64)

:End
