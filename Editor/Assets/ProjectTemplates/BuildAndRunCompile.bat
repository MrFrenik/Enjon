set projName=%1
set projDir=%2
set vsCall="%3"

@echo off
pushd "%projDir%"

echo "Building %projName%..."
@echo off
cmake -G"Visual Studio 14 2015"
echo "Running %projName%..."
@echo off

echo %vsCall%\VC\vcvarsall.bat x86

if not defined DevEnvDir (
	pushd "%vsCall%"
	call VC\vcvarsall.bat x86
	popd
)

devenv %projName%.sln /Build Release

@echo off
popd
exit 0
