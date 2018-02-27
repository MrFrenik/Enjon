set projDir=%1
set projName=%2

pushd %1

echo "Building %projName%..."
cmake -G"Visual Studio 14 2015"

echo "Running %projName%..."
start %projName%.sln

popd
exit 0
