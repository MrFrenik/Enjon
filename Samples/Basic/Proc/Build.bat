set projDir=%1
set projName=%2

pushd %1

echo "Building %projName%..."
cd Build
cmake ../. -G "Visual Studio 16 2019"

echo "Running %projName%..."

popd
exit 0
