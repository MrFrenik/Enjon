cmake_minimum_required(VERSION 2.6)

# Declare project file
project(Enjon)

# set top level directory
set (ENJON_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (ENJON_BUILD_DIR "${ENJON_DIR}/Build")
set (ENJON_RELEASE_DIR "${ENJON_BUILD_DIR}/Release")
set (ENJON_RELDEBUGINFO_DIR "${ENJON_BUILD_DIR}/RelWithDebInfo")
set (ENJON_DEBUG_DIR "${ENJON_BUILD_DIR}/Debug")

# Set build directory for solution
set (EXECUTABLE_OUTPUT_PATH ${ENJON_BUILD_DIR})

# Turn on ability to create folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set compiler flags and build options
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /W1")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /W1")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT /W1")

message("cxx FLgs:" ${CMAKE_CXX_FLAGS})


# Add include directories
include_directories(${ENJON_DIR}/include
					${ENJON_DIR}/ThirdParty/Build/Include
					${ENJON_DIR}/IsoARPG/include)

# Add libraries to be linked against
set(
	PROJECT_LINK_LIBRARIES
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
)

# Add lib directories
link_directories(${ENJON_DIR}/ThirdParty/Build/Libraries/Release)

set(ENJON_SRC "${ENJON_DIR}/src")
set(ENJON_INC "${ENJON_DIR}/include")
set(GAME_INC "${ENJON_DIR}/IsoARPG/include}")
set(GAME_SRC "${ENJON_DIR}/IsoARPG/src}")

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

file(GLOB_RECURSE SRC_UTILS
	"${ENJON_INC}/Utils/*.h"
	"${ENJON_SRC}/Utils/*.cpp"
	"${ENJON_INC}/Utils/*.inl")

file(GLOB_RECURSE SRC_ASSET
	"${ENJON_INC}/Asset/*.h"
	"${ENJON_SRC}/Asset/*.cpp"
	"${ENJON_INC}/Asset/*.inl")

file(GLOB_RECURSE GAME_FILES
	"${ENJON_DIR}/IsoARPG/include/Game.h"
	"${ENJON_DIR}/IsoARPG/src/Game.cpp"
	"${ENJON_DIR}/IsoARPG/src/main.cpp") 

# Add all engine source files
set(
	GENERAL_SOURCES
	include/Application.h
	include/Console.h
	include/CVarsSystem.h
	include/Defines.h
	include/Engine.h
	include/Enjon.h
	include/TypeCatalog.h
	include/Subsystem.h
	include/SubsystemCatalog.h

	src/Console.cpp
	src/CVarsSystem.cpp
	src/Engine.cpp
	src/TypeCatalog.cpp
	src/SubsystemCatalog.cpp
	)

set(
	ALL_SRC 
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
	${GAME_FILES}
	${GENERAL_SOURCES}
	)

add_executable(Enjon ${ALL_SRC})
target_link_libraries(Enjon ${PROJECT_LINK_LIBRARIES})

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
source_group("Game"			FILES ${GAME_FILES})

# Write to .user file for command arguments
if (MSVC)
	file(WRITE ${ENJON_DIR}/Build/Enjon.vcxproj.user
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>
		<Project ToolsVersion=\"14.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
		  <PropertyGroup>
		    <LocalDebuggerCommandArguments>--enjon-path ${ENJON_DIR}</LocalDebuggerCommandArguments>
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
		execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory {directory})
	endif()

endforeach(dir)


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

