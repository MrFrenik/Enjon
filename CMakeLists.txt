cmake_minimum_required(VERSION 2.6) 

# Declare project file
project(Enjon)

# set top level directory
set (ENJON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
set (REFLECTION_DIR "${ENJON_DIR}/GENERATOR/")
set (ENJON_BUILD_DIR "${ENJON_DIR}/Build/")
set (ENJON_RELEASE_DIR "${ENJON_BUILD_DIR}/Release/")
set (ENJON_RELDEBUGINFO_DIR "${ENJON_BUILD_DIR}/RelWithDebInfo/")
set (ENJON_DEBUG_DIR "${ENJON_BUILD_DIR}/Debug/")
set (ENJON_GENERATED_DIR "${ENJON_BUILD_DIR}/Generator/Linked/")
set (PROJECT_DIR "${ENJON_DIR}")

# Set build directory for solution
set (EXECUTABLE_OUTPUT_PATH ${ENJON_BUILD_DIR})

# Turn on ability to create folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_BUILD_TYPE Release)
	
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")	

# Set compiler flags and build options
if (MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /MD /W1 -DWINDOWS_IGNORE_PACKING_MISMATCH -DB3_USE_CLEW")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MP /MD /W1 -DWINDOWS_IGNORE_PACKING_MISMATCH -DB3_USE_CLEW")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP /MDd /W1 -DWINDOWS_IGNORE_PACKING_MISMATCH -DB3_USE_CLEW /bigobj")
else()
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 -O3")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")
endif()
set(CMAKE_CXX_STANDARD 11)

message("cxx Flags:" ${CMAKE_CXX_FLAGS})

# Add additional sub-directories
if (MSVC)
	add_subdirectory( Generator )
	add_subdirectory( Editor )
	add_subdirectory( Sandbox )
endif()
#add_subdirectory( TestSuite )

# Add include directories
include_directories(${ENJON_DIR}/Include
					${ENJON_DIR}/ThirdParty/Build/Include
					${ENJON_DIR}ThirdParty/Build/Include/Bullet
					${ENJON_DIR}ThirdParty/Build/Include/zmq/include
)

# Output generation file
if (NOT IS_DIRECTORY ${ENJON_GENERATED_DIR})
	file(MAKE_DIRECTORY ${ENJON_GENERATED_DIR})
endif()

if (NOT IS_DIRECTORY ${ENJON_BUILD_DIR}/Generator/Intermediate/)
	file(MAKE_DIRECTORY ${ENJON_BUILD_DIR}/Generator/Intermediate)
endif()

if ( NOT EXISTS ${ENJON_GENERATED_DIR}/Enjon_Generated.cpp )
	file(WRITE ${ENJON_GENERATED_DIR}/Enjon_Generated.cpp "// Enjon_Generated.cpp\n") 
endif()

# Add lib directories
link_directories(${ENJON_DIR}/ThirdParty/Build/Libraries/Windows/Release)
link_directories(${ENJON_DIR}/ThirdParty/Build/Libraries/Windows/Debug)
link_directories(${ENJON_DIR}/ThirdParty/Build/Libraries/OSX/Release)

set(ENJON_SRC "${ENJON_DIR}/Source")
set(ENJON_INC "${ENJON_DIR}/Include")

file(GLOB_RECURSE SRC_BASE
	"${ENJON_INC}/Base/*.h"
	"${ENJON_INC}/Base/*.inl"
	"${ENJON_SRC}/Base/*.cpp")

file(GLOB_RECURSE SRC_ENTITY
	"${ENJON_INC}/Entity/*.h"
	"${ENJON_SRC}/Entity/*.cpp"
	"${ENJON_INC}/Entity/*.inl")

file(GLOB_RECURSE SRC_MATH
	"${ENJON_INC}/Math/*.h"
	"${ENJON_SRC}/Math/*.cpp"
	"${ENJON_INC}/Math/*.inl")

file(GLOB_RECURSE SRC_PHYSICS
	"${ENJON_INC}/Physics/*.h"
	"${ENJON_SRC}/Physics/*.cpp"
	"${ENJON_INC}/Physics/*.inl")

file(GLOB_RECURSE SRC_UI
	"${ENJON_INC}/ImGui/*.h"
	"${ENJON_SRC}/ImGui/*.cpp"
	"${ENJON_INC}/ImGui/*.inl"
	"${ENJON_INC}/GUI/*.h"
	"${ENJON_INC}/GUI/*.inl"
	"${ENJON_SRC}/GUI/*.cpp"
)

file(GLOB_RECURSE SRC_GRAPHICS
	"${ENJON_INC}/Graphics/*.h"
	"${ENJON_SRC}/Graphics/*.cpp"
	"${ENJON_INC}/Graphics/*.inl")

file(GLOB_RECURSE SRC_INPUT
	"${ENJON_INC}/IO/*.h"
	"${ENJON_SRC}/IO/*.cpp"
	"${ENJON_INC}/IO/*.inl")

file(GLOB_RECURSE SRC_SYSTEM
	"${ENJON_INC}/System/*.h"
	"${ENJON_SRC}/System/*.cpp"
	"${ENJON_INC}/System/*.inl")

file(GLOB_RECURSE SRC_RESOURCE
	"${ENJON_INC}/Resource/*.h"
	"${ENJON_SRC}/Resource/*.cpp"
	"${ENJON_INC}/Resource/*.inl")

file(GLOB_RECURSE SRC_SHADER_GRAPH
	"${ENJON_INC}/Graphics/ShaderGraph/*.h"
	"${ENJON_SRC}/Graphics/ShaderGraph/*.cpp"
	"${ENJON_INC}/Graphics/ShaderGraph/*.inl")

file(GLOB_RECURSE SRC_UTILS
	"${ENJON_INC}/Utils/*.h"
	"${ENJON_SRC}/Utils/*.cpp"
	"${ENJON_INC}/Utils/*.inl")

file(GLOB_RECURSE SRC_ASSET
	"${ENJON_INC}/Asset/*.h"
	"${ENJON_SRC}/Asset/*.cpp"
	"${ENJON_INC}/Asset/*.inl")

file(GLOB_RECURSE SRC_SERIALIZE
	"${ENJON_INC}/Serialize/*.h"
	"${ENJON_SRC}/Serialize/*.cpp"
	"${ENJON_INC}/Serialize/*.inl")

file(GLOB_RECURSE SRC_SCENE
	"${ENJON_INC}/Scene/*.h"
	"${ENJON_SRC}/Scene/*.cpp"
	"${ENJON_INC}/Scene/*.inl") 

set(ENJON_BULLET_DIR ${ENJON_DIR}/ThirdParty/Build/Include/Bullet)

file(GLOB_RECURSE SRC_BULLET 
	"${ENJON_BULLET_DIR}/*.h"
	"${ENJON_BULLET_DIR}/*.cpp"

	"${ENJON_BULLET_DIR}/Bullet3Collision/BroadPhaseCollision/*.cpp"
	"${ENJON_BULLET_DIR}/Bullet3Collision/BroadPhaseCollision/*.h"
	"${ENJON_BULLET_DIR}/Bullet3Collision/BroadPhaseCollision/shared/*.h"
	"${ENJON_BULLET_DIR}/Bullet3Collision/BroadPhaseCollision/shared/*.cpp"

	"${ENJON_BULLET_DIR}/Bullet3Collision/BroadPhaseCollision/NarrowPhaseCollision/*.h"
	"${ENJON_BULLET_DIR}/Bullet3Collision/BroadPhaseCollision/NarrowPhaseCollision/*.cpp"
	"${ENJON_BULLET_DIR}/Bullet3Collision/BroadPhaseCollision/NarrowPhaseCollision/shared/*.cpp"
	"${ENJON_BULLET_DIR}/Bullet3Collision/BroadPhaseCollision/NarrowPhaseCollision/shared/*.h"
	"${ENJON_BULLET_DIR}/Bullet3Collision/BroadPhaseCollision/NarrowPhaseCollision/shared/*.h"

	"${ENJON_BULLET_DIR}/Bullet3Common/shared/*.h"
	"${ENJON_BULLET_DIR}/Bullet3Common/shared/*.cpp"
	"${ENJON_BULLET_DIR}/Bullet3Common/*.cpp"
	"${ENJON_BULLET_DIR}/Bullet3Common/*.h"

	"${ENJON_BULLET_DIR}/Bullet3Dynamics/*.h"
	"${ENJON_BULLET_DIR}/Bullet3Dynamics/*.cpp"

	"${ENJON_BULLET_DIR}/Bullet3Dynamics/ConstraintSolver/*.cpp"
	"${ENJON_BULLET_DIR}/Bullet3Dynamics/ConstraintSolver/*.h"

	"${ENJON_BULLET_DIR}/Bullet3Dynamics/shared/*.h"
	"${ENJON_BULLET_DIR}/Bullet3Dynamics/shared/*.cpp"

	"${ENJON_BULLET_DIR}/Bullet3Geometry/*.cpp"
	"${ENJON_BULLET_DIR}/Bullet3Geometry/*.h"

	 "${ENJON_BULLET_DIR}/BulletCollision/*.h"
	 "${ENJON_BULLET_DIR}/BulletCollision/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletCollision/BroadPhaseCollision/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletCollision/BroadPhaseCollision/*.h"
	 "${ENJON_BULLET_DIR}/BulletCollision/CollisionDispatch/*.h"
	 "${ENJON_BULLET_DIR}/BulletCollision/CollisionDispatch/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletCollision/CollisionShapes/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletCollision/CollisionShapes/*.h"
	 "${ENJON_BULLET_DIR}/BulletCollision/Gimpact/*.h"
	 "${ENJON_BULLET_DIR}/BulletCollision/Gimpact/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletCollision/NarrowPhaseCollision/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletCollision/NarrowPhaseCollision/*.h"

	 "${ENJON_BULLET_DIR}/BulletDynamics/*.h"
	 "${ENJON_BULLET_DIR}/BulletDynamics/Character/*.h"
	 "${ENJON_BULLET_DIR}/BulletDynamics/Character/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletDynamics/ConstraintSolver/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletDynamics/ConstraintSolver/*.h"
	 "${ENJON_BULLET_DIR}/BulletDynamics/Dynamics/*.h"
	 "${ENJON_BULLET_DIR}/BulletDynamics/Dynamics/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletDynamics/Featherstone/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletDynamics/Featherstone/*.h"
	 "${ENJON_BULLET_DIR}/BulletDynamics/MLCPSolvers/*.h"
	 "${ENJON_BULLET_DIR}/BulletDynamics/MLCPSolvers/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletDynamics/Vehicle/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletDynamics/Vehicle/*.h"

	 "${ENJON_BULLET_DIR}/BulletInverseDynamics/*.h"
	 "${ENJON_BULLET_DIR}/BulletInverseDynamics/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletInverseDynamics/details/*.cpp"
	 "${ENJON_BULLET_DIR}/BulletInverseDynamics/details/*.h"

	 "${ENJON_BULLET_DIR}/BulletSoftBody/*.h"
	 "${ENJON_BULLET_DIR}/BulletSoftBody/*.cpp"

	 "${ENJON_BULLET_DIR}/LinearMath/*.cpp"
	 "${ENJON_BULLET_DIR}/LinearMath/*.h"
	 "${ENJON_BULLET_DIR}/LinearMath/TaskScheduler/*.cpp"
	 "${ENJON_BULLET_DIR}/LinearMath/TaskScheduler/*.h"
) 

file(GLOB_RECURSE SHADERS "${ENJON_DIR}/Assets/Shaders/*.glsl" )

file(GLOB_RECURSE REFLECTION_INCLUDES
	"${ENJON_INC}/*.h"
)

# Add all engine source files
set(
	GENERAL_SOURCES
	${ENJON_INC}/Application.h
	${ENJON_INC}/CVarsSystem.h
	${ENJON_INC}/Defines.h
	${ENJON_INC}/Engine.h
	${ENJON_INC}/Enjon.h
	${ENJON_INC}/TypeCatalog.h
	${ENJON_INC}/Subsystem.h
	${ENJON_INC}/SubsystemCatalog.h

	${ENJON_SRC}/Application.cpp
	${ENJON_SRC}/CVarsSystem.cpp
	${ENJON_SRC}/Engine.cpp
	${ENJON_SRC}/TypeCatalog.cpp
	${ENJON_SRC}/SubsystemCatalog.cpp
	${ENJON_GENERATED_DIR}/Enjon_Generated.cpp
)

set(
	ALL_SRC 
	${SRC_BASE}
	${SRC_ENTITY} 
	${SRC_UTILS} 
	${SRC_GRAPHICS} 
	${SRC_RESOURCE} 
	${SRC_SYSTEM} 
	${SRC_INPUT} 
	${SRC_UI} 
	${SRC_PHYSICS} 
	${SRC_MATH} 
	${SRC_ASSET} 
	${SRC_SCENE}
	${SRC_SHADER_GRAPH}
	${SRC_SERIALIZE}
	${GENERAL_SOURCES}
	${SRC_BULLET}
	${SHADERS}
) 

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

add_library( Enjon ${ALL_SRC})

# Link libraries
if (MSVC)
	target_link_libraries( Enjon debug SDL2_d.lib optimized SDL2.lib )
	target_link_libraries( Enjon debug SDL2main_d.lib optimized SDL2main.lib )
	target_link_libraries( Enjon debug glew32_d.lib optimized glew32.lib )
	target_link_libraries( Enjon debug glew32s_d.lib optimized glew32s.lib )
	target_link_libraries( Enjon debug freetype_d.lib optimized freetype.lib )
	target_link_libraries( Enjon debug assimp-vc140-mt.lib optimized assimp-vc140-mt.lib ) 
	target_link_libraries( Enjon debug nfd_d.lib optimized nfd.lib ) 
else()
	target_link_libraries( Enjon libSDL2.a )
	target_link_libraries( Enjon libSDL2main.a )
	target_link_libraries( Enjon libassimp.a )
	target_link_libraries( Enjon libIrrXML.a )
endif()

if (MSVC)
	add_dependencies( Enjon Reflection )
	add_dependencies( Editor Reflection )
	add_dependencies( Sandbox Reflection )
	add_dependencies( Editor Enjon )
	add_dependencies( Sandbox Enjon )
	add_dependencies( Editor Sandbox )
endif()

source_group("Base"			FILES ${SRC_BASE})
source_group("Entity"		FILES ${SRC_ENTITY})
source_group("Math"			FILES ${SRC_MATH})
source_group("Physics"		FILES ${SRC_PHYSICS})
source_group("Resource"		FILES ${SRC_RESOURCE})
source_group("Input"		FILES ${SRC_INPUT})
source_group("Graphics"		FILES ${SRC_GRAPHICS})
source_group("UI"			FILES ${SRC_UI})
source_group("System"		FILES ${SRC_SYSTEM})
source_group("Utils"		FILES ${SRC_UTILS})
source_group("Asset"		FILES ${SRC_ASSET})
source_group("ShaderGraph"	FILES ${SRC_SHADER_GRAPH}) 
source_group("Serialize"	FILES ${SRC_SERIALIZE}) 
source_group("Scene"		FILES ${SRC_SCENE}) 
source_group("Shaders"		FILES ${SHADERS}) 

# Write to .user file for command arguments
if (MSVC)
	file(WRITE ${ENJON_DIR}/Build/Enjon.vcxproj.user
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>
		<Project ToolsVersion=\"14.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
		  <PropertyGroup>
		    <LocalDebuggerCommandArguments>--enjon-path ${ENJON_DIR} --project-path ${PROJECT_DIR}</LocalDebuggerCommandArguments>
		     <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
		  </PropertyGroup>
		</Project>" 
	)
endif()

# List of necessary DLLs 
set(
		DLL_LIST

		SDL2.dll
		freetype.dll
		glew32.dll
		assimp-vc142-mt.dll
		assimp-vc142-mtd.dll
		libzmq-v142-mt-4_3_3.dll
		libzmq-v142-mt-gd-4_3_3.dll
	)

# List of build directories
set(
		BUILD_DIRECTORIES

		${ENJON_RELDEBUGINFO_DIR}
		${ENJON_RELEASE_DIR}
		${ENJON_DEBUG_DIR}
	)

# Install directories
foreach(dir ${BUILD_DIRECTORIES})
	if (NOT IS_DIRECTORY ${dir})
		file(MAKE_DIRECTORY ${dir})
		execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${dir})
	endif()

endforeach(dir)

# Add DLLs to builds
if (MSVC)
	foreach(dll ${DLL_LIST})
		add_custom_command(TARGET Enjon POST_BUILD        		
		    COMMAND ${CMAKE_COMMAND} -E copy_if_different  		
		        "${ENJON_DIR}/ThirdParty/Build/DLLs/${dll}"   
		        ${ENJON_BUILD_DIR}/RelWithDebInfo/)

		add_custom_command(TARGET Enjon POST_BUILD        		
		    COMMAND ${CMAKE_COMMAND} -E copy_if_different  		
		        "${ENJON_DIR}/ThirdParty/Build/DLLs/${dll}"   
		        ${ENJON_BUILD_DIR}/Release/)

		add_custom_command(TARGET Enjon POST_BUILD        		
		    COMMAND ${CMAKE_COMMAND} -E copy_if_different  		
		        "${ENJON_DIR}/ThirdParty/Build/DLLs/${dll}"   
		        ${ENJON_BUILD_DIR}/Debug/)
	endforeach(dll) 
endif()

# Create reflection list
file(WRITE ${ENJON_BUILD_DIR}/Generator/reflection.cfg "#target_dir \"${ENJON_DIR}\"\n") 
file(WRITE ${ENJON_BUILD_DIR}/Generator/reflection.cfg "#engine_dir \"${ENJON_DIR}\"\n") 
file(APPEND ${ENJON_BUILD_DIR}/Generator/reflection.cfg "#project_name \"Enjon\"\n") 
file(APPEND ${ENJON_BUILD_DIR}/Generator/reflection.cfg "#base_type_id 0\n") 
file(APPEND ${ENJON_BUILD_DIR}/Generator/reflection.cfg "#files\n") 

# Add all reflection include files
foreach( file ${REFLECTION_INCLUDES} )
	file(APPEND ${ENJON_BUILD_DIR}/Generator/reflection.cfg "\"${file}\"\n")
endforeach(file) 

# Pre build events are only working for MSVC...annoying
if (MSVC)
	add_custom_command(TARGET Enjon PRE_BUILD
		COMMAND call ${ENJON_RELEASE_DIR}/Reflection.exe ARGS --target-path ${ENJON_DIR}/
		)
endif()

