rm -rf Build
mkdir Build

cd Generator
rm -rf Build
mkdir Build
echo "Building Reflection..."
cd Build
cmake ../. -G"Visual Studio 16 2019"
echo "Running Reflection..."

cd ..
cd ..
echo "Building Enjon Editor..."
cd Editor

Proc\WriteResourceFile.exe

rm -rf Build
mkdir Build
cd Build
cmake ../. -G"Visual Studio 16 2019"

echo "Building Enjon..."
cd ..
cd ..
cd Build
cmake ../. -G"Visual Studio 16 2019"

echo "Running Enjon..."
call Enjon.sln
