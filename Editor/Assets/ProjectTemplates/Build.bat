set projDir=%1
set projName=%2

pushd %1

echo "Building %projName%..."
cd Build
cmake ../. -G #CMAKE_FLAGS

echo "Running %projName%..."

popd
exit 0
