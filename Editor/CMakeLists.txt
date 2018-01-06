cmake_minimum_required(VERSION 2.6) 

set (TARGET_NAME EnjonEditor)

# Declare project file
project(${TARGET_NAME} C CXX)

# set top level directory
set (TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (ENJON_DIR ${TARGET_DIR}/..)
set (ENJON_INC ${ENJON_DIR}/Include)
set (ENJON_BUILD_DIR ${ENJON_DIR}/Build)
set (TARGET_BUILD_DIR "${TARGET_DIR}/Build")
set (ENJON_RELDEBUGINFO_DIR ${ENJON_BUILD_DIR}/RelWithDebInfo)
set (ENJON_RELEASE_DIR ${ENJON_BUILD_DIR}/Release)
set (ENJON_DEBUG_DIR ${ENJON_BUILD_DIR}/Debug)

set (TARGET_RELEASE_DIR "${ENJON_RELEASE_DIR}")
set (TARGET_RELEASEDEBUGINFO_DIR "${ENJON_RELDEBUGINFO_DIR}")
set (TARGET_DEBUG_DIR "${ENJON_DEBUG_DIR}")

# Set build directory for solution
set (EXECUTABLE_OUTPUT_PATH ${ENJON_BUILD_DIR})

# Turn on ability to create folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set compiler flags and build options
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /W1")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD /W1")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /W1")

message("cxx FLgs:" ${CMAKE_CXX_FLAGS})

# Add include directories
include_directories(${TARGET_DIR}/Include
					${ENJON_DIR}/ThirdParty/Build/Include
					${ENJON_INC}
					)

# Add lib directories
link_directories(${ENJON_DIR}/ThirdParty/Build/Libraries/Release)
link_directories(${ENJON_DIR}/ThirdParty/Build/Libraries/Debug)
link_directories(${ENJON_BUILD_DIR}/Debug)
link_directories(${ENJON_BUILD_DIR}/Release)
link_directories(${ENJON_BUILD_DIR}/RelWithDebInfo)

set(TARGET_INC_DIR "${TARGET_DIR}/Include/")
set(TARGET_DIR_DIR "${TARGET_DIR}/Source/")

file(GLOB_RECURSE TARGET_SOURCES
	"${TARGET_INC_DIR}/*.h"
	"${TARGET_INC_DIR}/*.c"
	"${TARGET_DIR_DIR}/*.cpp"
	)

set(
	ALL_SRC 
	${TARGET_SOURCES}
) 

add_executable(${TARGET_NAME} ${TARGET_SOURCES}) 

# Link libraries to target
target_link_libraries( ${TARGET_NAME} debug Enjond.lib optimized Enjon.lib )
target_link_libraries( ${TARGET_NAME} debug Opengl32.lib optimized Opengl32.lib ) 
target_link_libraries( ${TARGET_NAME} debug SDL2_d.lib optimized SDL2.lib )
target_link_libraries( ${TARGET_NAME} debug SDL2main_d.lib optimized SDL2main.lib )
target_link_libraries( ${TARGET_NAME} debug glew32_d.lib optimized glew32.lib )
target_link_libraries( ${TARGET_NAME} debug glew32s_d.lib optimized glew32s.lib )
target_link_libraries( ${TARGET_NAME} debug freetype_d.lib optimized freetype.lib )
target_link_libraries( ${TARGET_NAME} debug Bullet3Collision_d.lib optimized Bullet3Collision.lib )
target_link_libraries( ${TARGET_NAME} debug Bullet3Dynamics_d.lib optimized Bullet3Dynamics.lib )
target_link_libraries( ${TARGET_NAME} debug Bullet3Common_d.lib optimized Bullet3Common.lib )
target_link_libraries( ${TARGET_NAME} debug LinearMath_d.lib optimized LinearMath.lib )
target_link_libraries( ${TARGET_NAME} debug BulletDynamics_d.lib optimized BulletDynamics.lib )
target_link_libraries( ${TARGET_NAME} debug BulletCollision_d.lib optimized BulletCollision.lib )
target_link_libraries( ${TARGET_NAME} debug fmt_d.lib optimized fmt.lib )
target_link_libraries( ${TARGET_NAME} debug liblz4_d.lib optimized liblz4.lib ) 

set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

source_group("Source"		FILES ${TARGET_SOURCES}) 

# Write to .user file for command arguments
if (MSVC)
	file(WRITE ${ENJON_BUILD_DIR}/${TARGET}/${TARGET}.vcxproj.user
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>
		<Project ToolsVersion=\"14.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
		  <PropertyGroup>
		    <LocalDebuggerCommandArguments>--enjon-path ${ENJON_DIR}</LocalDebuggerCommandArguments>
		     <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
		  </PropertyGroup>
		</Project>" 
	)
endif()
