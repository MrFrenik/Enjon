mkdir Build
cd Generator
mkdir Build
echo "Building Reflection..."
cd Build
cmake ../.
echo "Running Reflection..."

echo "Building Enjon..."
cd ..
cd ..
cd Build
cmake ../.

echo "Running Enjon..."
call Enjon.sln
