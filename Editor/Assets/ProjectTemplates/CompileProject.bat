@echo off

set projDir=%1
set projName=%2
set buildConfig=%3
set vsPath=%4

if not defined DevEnvDir (
	pushd %vsPath%
	call VC\vcvarsall.bat x86
	popd
)

echo Compiling %projName%...

@echo off
devenv %projDir%Build/%projName%.sln /Build %buildConfig%

@echo off
popd

exit
