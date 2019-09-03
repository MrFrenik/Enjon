mkdir Build

cd Generator
mkdir Build
echo "Building Reflection..."
cd Build
cmake ../. -G"Visual Studio 16 2019" -A Win32
echo "Running Reflection..."

cd ..
cd ..
echo "Building Enjon Editor..."
cd Editor

Proc\WriteResourceFile.exe

mkdir Build
cd Build
cmake ../. -G"Visual Studio 16 2019" -A Win32

echo "Building Enjon..."
cd ..
cd ..
cd Build
cmake ../. -G"Visual Studio 16 2019" -A Win32

echo "Running Enjon..."
call Enjon.sln
