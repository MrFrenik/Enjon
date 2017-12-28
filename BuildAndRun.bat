mkdir Build

cd Generator
mkdir Build
echo "Building Reflection..."
cd Build
cmake ../. -G"Visual Studio 14 2015"
echo "Running Reflection..."

cd ..
cd ..
echo "Building Enjon Editor..."
cd Editor
mkdir Build
cd Build
cmake ../. -G"Visual Studio 14 2015"

echo "Building Enjon..."
cd ..
cd ..
cd Build
cmake ../. -G"Visual Studio 14 2015"

echo "Running Enjon..."
call Enjon.sln
