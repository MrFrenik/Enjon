cmake_minimum_required(VERSION 2.6)

# Declare project file
project(Enjon)

# set top level directory
set (cur_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Set build directory for solution
set (build_dir ${cur_dir}/Build)
set (EXECUTABLE_OUTPUT_PATH ${build_dir})

# Turn on ability to create folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set compiler flags and build options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W1")

message("cxx FLgs:" ${CMAKE_CXX_FLAGS})


# Add include directories
include_directories(${cur_dir}/include
					${cur_dir}/ThirdParty/Build/Include
					${cur_dir}/IsoARPG/include)

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
link_directories(${cur_dir}/ThirdParty/Build/Libraries/Release)

set(ENJON_SRC "${cur_dir}/src")
set(ENJON_INC "${cur_dir}/include")
set(GAME_INC "${cur_dir}/IsoARPG/include}")
set(GAME_SRC "${cur_dir}/IsoARPG/src}")

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

file(GLOB_RECURSE GAME_FILES
	"${cur_dir}/IsoARPG/include/Game.h"
	"${cur_dir}/IsoARPG/src/Game.cpp"
	"${cur_dir}/IsoARPG/src/main.cpp") 

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

	src/Console.cpp
	src/CVarsSystem.cpp
	src/Engine.cpp
	src/TypeCatalog.cpp
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
source_group("Game"			FILES ${GAME_FILES})

