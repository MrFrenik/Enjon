cmake_minimum_required(VERSION 2.6) 

# Declare project file
project(Enjon)

# set top level directory
set (ENJON_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (REFLECTION_DIR "${ENJON_DIR}/GENERATOR")
set (ENJON_BUILD_DIR "${ENJON_DIR}/Build")
set (ENJON_RELEASE_DIR "${ENJON_BUILD_DIR}/Release")
set (ENJON_RELDEBUGINFO_DIR "${ENJON_BUILD_DIR}/RelWithDebInfo")
set (ENJON_DEBUG_DIR "${ENJON_BUILD_DIR}/Debug")
set (ENJON_GENERATED_DIR "${ENJON_BUILD_DIR}/Generator/Linked")
set (PROJECT_DIR "${ENJON_DIR}/IsoARPG/")

# Set build directory for solution
set (EXECUTABLE_OUTPUT_PATH ${ENJON_BUILD_DIR})

# Turn on ability to create folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set compiler flags and build options
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /W1")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /W1")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT /W1")

message("cxx FLgs:" ${CMAKE_CXX_FLAGS})

# Add additional sub-directories
add_subdirectory( Generator )

# Add include directories
include_directories(${ENJON_DIR}/Include
					${ENJON_DIR}/ThirdParty/Build/Include
					${ENJON_DIR}/IsoARPG/include)

# Output generation file
if (NOT IS_DIRECTORY ${ENJON_GENERATED_DIR})
	file(MAKE_DIRECTORY ${dir})
endif()

if ( EXISTS ${ENJON_GENERATED_DIR}/Enjon_Generated.cpp )
else()
	file(WRITE ${ENJON_GENERATED_DIR}/Enjon_Generated.cpp "// Enjon_Generated.cpp\n") 
endif()

# Add libraries to be linked against
set(
	PROJECT_RELEASE_LIBRARIES
	Opengl32.lib 
	SDL2.lib 
	SDL2main.lib 
	glew32.lib 
	glew32s.lib 
	freetype.lib 
	Bullet3Collision.lib 
	Bullet3Dynamics.lib
	Bullet3Common.lib 
	LinearMath.lib 
	BulletDynamics.lib 
	BulletCollision.lib
	fmt.lib
	CrossGuid.lib
	liblz4.lib
)

set(
	PROJECT_DEBUG_LIBRARIES
	Opengl32.lib 
	SDL2_d.lib 
	SDL2main_d.lib 
	glew32_d.lib 
	glew32s_d.lib 
	freetype_d.lib 
	Bullet3Collision_d.lib 
	Bullet3Dynamics_d.lib
	Bullet3Common_d.lib 
	LinearMath_d.lib 
	BulletDynamics_d.lib 
	BulletCollision_d.lib
	fmt_d.lib
	CrossGuid_d.lib
	liblz4_d.lib
)

# Add lib directories
link_directories(${ENJON_DIR}/ThirdParty/Build/Libraries/Release)
link_directories(${ENJON_DIR}/ThirdParty/Build/Libraries/Debug)

set(ENJON_SRC "${ENJON_DIR}/Source")
set(ENJON_INC "${ENJON_DIR}/Include")
set(GAME_INC "${ENJON_DIR}/IsoARPG/include}")
set(GAME_SRC "${ENJON_DIR}/IsoARPG/src}")

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
	"${ENJON_INC}/ImGui/*.inl")

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

file(GLOB_RECURSE GAME_FILES
	"${ENJON_DIR}/IsoARPG/include/Game.h"
	"${ENJON_DIR}/IsoARPG/src/Game.cpp"
	"${ENJON_DIR}/IsoARPG/src/main.cpp") 

file(GLOB_RECURSE SHADERS "${ENJON_DIR}/Assets/Shaders/*.glsl" ) 

file(GLOB_RECURSE REFLECTION_INCLUDES
	"${ENJON_INC}/*.h"
	"${GAME_INC}/*.h"
	)

# Add all engine source files
set(
	GENERAL_SOURCES
	${ENJON_INC}/Application.h
	${ENJON_INC}/Console.h
	${ENJON_INC}/CVarsSystem.h
	${ENJON_INC}/Defines.h
	${ENJON_INC}/Engine.h
	${ENJON_INC}/Enjon.h
	${ENJON_INC}/TypeCatalog.h
	${ENJON_INC}/Subsystem.h
	${ENJON_INC}/SubsystemCatalog.h

	${ENJON_SRC}/Console.cpp
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
	${SRC_SHADER_GRAPH}
	${SRC_SERIALIZE}
	${GAME_FILES}
	${GENERAL_SOURCES}
	${SHADERS}
	)
 
SET( LINK_LIBRARY debug ${PROJECT_DEBUG_LIBRARIES} optimized ${PROJECT_RELEASE_LIBRARIES} )

add_executable(Enjon ${ALL_SRC})
#target_link_libraries(Enjon debug ${PROJECT_DEBUG_LIBRARIES} optimized ${PROJECT_RELEASE_LIBRARIES})
target_link_libraries(Enjon ${PROJECT_DEBUG_LIBRARIES})
#target_link_libraries(Enjon ${PROJECT_RELEASE_LIBRARIES})

add_dependencies( Enjon Reflection )

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
source_group("Game"			FILES ${GAME_FILES}) 
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
		freetype6.dll
		glew32.dll
		glut32.dll
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

add_custom_command(TARGET Enjon PRE_BUILD
	COMMAND call ${ENJON_RELDEBUGINFO_DIR}/Reflection.exe --enjon-path ${ENJON_DIR}
	)

# Add DLLs to builds
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

# Create reflection list
file(WRITE ${ENJON_DIR}/Generator/config.cfg "#enjon_dir \"${ENJON_DIR}\"\n") 
file(APPEND ${ENJON_DIR}/Generator/config.cfg "#files\n") 

# Add all reflection include files
foreach( file ${REFLECTION_INCLUDES} )

	file(APPEND ${ENJON_DIR}/Generator/config.cfg "\"${file}\"\n") 

endforeach( file ) 

# Generate Files - Not sure the best way to handle this just yet...
if(WIN32) 
	execute_process( COMMAND ${ENJON_RELEASE_DIR}/Reflection.exe --enjon-path ${ENJON_DIR} )

	add_custom_command(
		OUTPUT generated.cpp
		COMMAND  ${ENJON_RELEASE_DIR}/Reflection.exe --enjon-path ${ENJON_DIR}
		DEPENDS generated_files
	)
	
endif()










