proj_root_dir=$(pwd)

# Generate reflection for Enjon
./Generator/Build/Reflection --target-path ${proj_root_dir}/ 
cd Build

# Compile Enjon library
make -j 
cd .. 

# Generate reflection for Enjon Editor
./Generator/Build/Reflection --target-path ${proj_root_dir}/Editor/
cd Editor/Build/

# Compile Enjon Editor
make -j 
cd .. && cd ..  

# Moving into correct folders ( this needs to depend on configuration )
cp Build/EnjonEditor Build/Release/EnjonEditor
cp Build/EnjonEditor Build/Debug/EnjonEditor
rm Build/EnjonEditor

