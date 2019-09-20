@echo on
setlocal enabledelayedexpansion

set projDir=%1
pushd %1
@set vsw=%projDir%/vswhere

echo "vsws:"

rem for /f "usebackq tokens=*" %%i in (`vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
	@echo on  
	echo "%%i" %*
	popd
	exit /b !errorlevel!
)

popd
exit 0
