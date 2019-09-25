cmake_minimum_required(VERSION 2.6) 

set (TARGET_NAME ThirdParty)

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

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()	
	
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")	

# Set compiler flags and build options
if (MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /W1")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD /W1")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /W1")
else()
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 -O3")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")
endif()
set(CMAKE_CXX_STANDARD 11)

message("cxx Flags:" ${CMAKE_CXX_FLAGS})

# Add include directories
include_directories(${TARGET_DIR}/Include
					${ENJON_DIR}/ThirdParty/Build/Include
					${ENJON_DIR}/ThirdParty/Build/Include/Bullet
					${ENJON_DIR}/ThirdParty/Build/Include/zmq/include
					${ENJON_INC}
					)

# Add lib directories
link_directories(${ENJON_DIR}/ThirdParty/Build/Libraries/Release)
link_directories(${ENJON_DIR}/ThirdParty/Build/Libraries/Debug)
link_directories(${ENJON_BUILD_DIR}/Debug)
link_directories(${ENJON_BUILD_DIR}/Release)
link_directories(${ENJON_BUILD_DIR}/RelWithDebInfo)
link_directories(${ENJON_DIR}/ThirdParty/Build/Libraries/OSX/Release)
link_directories(${ENJON_BUILD_DIR}/)

set(TARGET_INC_DIR "${TARGET_DIR}/Include/")
set(TARGET_DIR_DIR "${TARGET_DIR}/Source/")
set(TARGET_GENERATED_DIR ${TARGET_BUILD_DIR}/Generator/Linked/) 

# Output generation file
if (NOT IS_DIRECTORY ${TARGET_GENERATED_DIR})
	file(MAKE_DIRECTORY ${TARGET_GENERATED_DIR})
endif()

if ( NOT EXISTS ${TARGET_GENERATED_DIR}/${TARGET_NAME}_Generated.cpp )
	file(WRITE ${TARGET_GENERATED_DIR}/${TARGET_NAME}_Generated.cpp "// Target_Generated.cpp\n") 
endif()

# Pull in the resource file for the application
if (MSVC)
	set( RESOURCE_FILE ${ENJON_DIR}/Editor/editor_resource.rc )
endif()

file(
		GLOB_RECURSE TARGET_SOURCES

		"${TARGET_INC_DIR}/*.h"
		"${TARGET_INC_DIR}/*.c"
		"${TARGET_DIR_DIR}/*.cpp" 
		${TARGET_GENERATED_DIR}/${TARGET_NAME}_Generated.cpp
)

set(
		ALL_SRC 
		${TARGET_SOURCES}
		${RESOURCE_FILE}
	) 

source_group("Source"			FILES ${ALL_SRC})
source_group("Resources"		FILES ${RESOURCE_FILE})

file(
		GLOB_RECURSE REFLECTION_INCLUDES

		"${TARGET_INC_DIR}/*.h"
	)

if (NOT MSVC)
	find_package(SDL2 REQUIRED)
	include_directories(${SDL_INCLUDE_DIRS})
endif()

# Compile final executable
add_executable( ${TARGET_NAME} ${ALL_SRC} ) 

# Link libraries to target
if (MSVC)
	target_link_libraries( ${TARGET_NAME} debug Enjond.lib optimized Enjon.lib )
	target_link_libraries( ${TARGET_NAME} debug Opengl32.lib optimized Opengl32.lib ) 
	target_link_libraries( ${TARGET_NAME} debug SDL2_d.lib optimized SDL2.lib )
	target_link_libraries( ${TARGET_NAME} debug SDL2main_d.lib optimized SDL2main.lib )
	target_link_libraries( ${TARGET_NAME} debug glew32_d.lib optimized glew32.lib )
	target_link_libraries( ${TARGET_NAME} debug glew32s_d.lib optimized glew32s.lib )
	target_link_libraries( ${TARGET_NAME} debug freetype_d.lib optimized freetype.lib )
	target_link_libraries( ${TARGET_NAME} debug fmt_d.lib optimized fmt.lib )
	target_link_libraries( ${TARGET_NAME} debug liblz4_d.lib optimized liblz4.lib ) 
	target_link_libraries( ${TARGET_NAME} debug assimp-vc140-mt.lib optimized assimp-vc140-mt.lib ) 
	target_link_libraries( ${TARGET_NAME} debug nfd_d.lib optimized nfd.lib ) 
	target_link_libraries( ${TARGET_NAME} debug libzmq_d.lib optimized libzmq.lib ) 
	target_link_libraries( ${TARGET_NAME} debug libzmq2_d.lib optimized libzmq2.lib ) 
else()
	target_link_libraries( ${TARGET_NAME} debug Enjond optimized Enjon )
	target_link_libraries( ${TARGET_NAME} assimp.a )
	target_link_libraries( ${TARGET_NAME} IrrXML.a )
	target_link_libraries( ${TARGET_NAME} SDL2main.a )
	target_link_libraries( ${TARGET_NAME} fmt.a )
	target_link_libraries( ${TARGET_NAME} GLEW )
	target_link_libraries( ${TARGET_NAME} nfd )
	target_link_libraries( ${TARGET_NAME} freetype.a )
	target_link_libraries( ${TARGET_NAME} m )
	target_link_libraries( ${TARGET_NAME} z )
	target_link_libraries( ${TARGET_NAME} iconv )
	target_link_libraries( ${TARGET_NAME} png )
	target_link_libraries( ${TARGET_NAME} bz2 )
	target_link_libraries( ${TARGET_NAME} "-framework CoreAudio" )
	target_link_libraries( ${TARGET_NAME} "-framework CoreMedia" )
	target_link_libraries( ${TARGET_NAME} "-framework Cocoa" )
	target_link_libraries( ${TARGET_NAME} "-framework CoreServices" )
	target_link_libraries( ${TARGET_NAME} "-framework CoreAudioKit" )
	target_link_libraries( ${TARGET_NAME} "-framework OpenGL" )
	target_link_libraries( ${TARGET_NAME} "-framework CoreFoundation" )
	target_link_libraries( ${TARGET_NAME} "-framework ForceFeedback" )
	target_link_libraries( ${TARGET_NAME} "-framework Carbon" )
	target_link_libraries( ${TARGET_NAME} "-framework IOKit" )
	target_link_libraries( ${TARGET_NAME} "-framework AudioToolbox" )
	target_link_libraries( ${TARGET_NAME} "-framework AudioUnit" )
	target_link_libraries( ${TARGET_NAME} "-framework CoreVideo" )
	target_link_libraries( ${TARGET_NAME} "-framework Metal" )
	target_link_libraries( ${TARGET_NAME} ${SDL2_LIBRARIES} )
endif()

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

# Reflection information
set( TARGET_REFLECTION_DIR ${TARGET_BUILD_DIR}/Generator/ )

# Create directory if doesn't exist 
message("Editor Reflection Dir:" ${TARGET_REFLECTION_DIR})
if (NOT IS_DIRECTORY ${TARGET_REFLECTION_DIR})
	file(MAKE_DIRECTORY ${TARGET_REFLECTION_DIR})
	file(MAKE_DIRECTORY ${TARGET_REFLECTION_DIR}/Intermediate/)
	file(MAKE_DIRECTORY ${TARGET_REFLECTION_DIR}/Linked/)
	file(MAKE_DIRECTORY ${TARGET_REFLECTION_DIR}/Test)
endif()

if (NOT IS_DIRECTORY ${TARGET_REFLECTION_DIR}/Intermediate/)
	file(MAKE_DIRECTORY ${TARGET_REFLECTION_DIR}/Intermediate/)
endif()

if (NOT IS_DIRECTORY ${TARGET_REFLECTION_DIR}/Linked/)
	file(MAKE_DIRECTORY ${TARGET_REFLECTION_DIR}/Linked/)
endif()

# Create reflection list for project
file(WRITE ${TARGET_REFLECTION_DIR}/reflection.cfg "#target_dir \"${TARGET_DIR}/\"\n") 
file(APPEND ${TARGET_REFLECTION_DIR}/reflection.cfg "#engine_dir \"${ENJON_DIR}\"\n") 
file(APPEND ${TARGET_REFLECTION_DIR}/reflection.cfg "#project_name \"${TARGET_NAME}\"\n") 
file(APPEND ${TARGET_REFLECTION_DIR}/reflection.cfg "#generate_app_bindings true \n") 
file(APPEND ${TARGET_REFLECTION_DIR}/reflection.cfg "#base_type_id 20000 \n") 
file(APPEND ${TARGET_REFLECTION_DIR}/reflection.cfg "#files\n") 

# Add all reflection include files
foreach( file ${REFLECTION_INCLUDES} )

	file(APPEND ${TARGET_REFLECTION_DIR}/reflection.cfg "\"${file}\"\n") 

endforeach(file)

# Need to get this working with projects
if (MSVC)
	add_custom_command(
		TARGET ${TARGET_NAME} PRE_BUILD
		COMMAND call ${ENJON_RELEASE_DIR}/Reflection.exe ARGS --target-path ${TARGET_DIR}/
	)
endif()