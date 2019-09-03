set projDir=%1
set projName=%2

pushd %1

echo "Building %projName%..."
cd Build
cmake ../. -G "#CMAKE_FLAGS" -A Win32

echo "Running %projName%..."
start %projName%.sln

popd
exit 0
