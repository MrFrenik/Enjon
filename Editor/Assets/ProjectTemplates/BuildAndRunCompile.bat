set projName=%1
set projDir=%2
set vsCall="%3"

@echo off
pushd "%projDir%"

echo "Building %projName%..."
@echo off
cmake -G #CMAKE_FLAGS
echo "Running %projName%..."
@echo off

MSBuild %projName%.sln /t:Build /p:Configuration=Release

@echo off
popd
exit 0
