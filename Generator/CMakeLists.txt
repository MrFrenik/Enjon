cmake_minimum_required(VERSION 2.6)

project( Reflection )

set( REFL_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
set( ENJON_DIR "${REFL_DIR}/.." )
set( ENJON_INC ${ENJON_DIR}/include )
set( BUILD_DIR ${ENJON_DIR}/Build)
set( INTERMEDIATE_DIR ${BUILD_DIR}/Generator/Intermediate )

if (NOT IS_DIRECTORY ${INTERMEDIATE_DIR})
		file(MAKE_DIRECTORY ${INTERMEDIATE_DIR})
		execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${INTERMEDIATE_DIR})
endif() 

# Add include directory
include_directories(${REFL_DIR}/Include)

# Set source files
file( GLOB_RECURSE SRC_REFL
	  ${REFL_DIR}/Source/*.cpp
	  ${REFL_DIR}/Include/*.h
	  ${REFL_DIR}/Include/*.inl
	) 

# Files to run over for reflection
file( GLOB_RECURSE FILES "${ENJON_INC}/*.h" ) 

# Add project executable
add_executable( Reflection ${SRC_REFL} ) 

# Iterate through files to run reflection over
foreach(inc ${FILES}) 

	message ( "Generating Reflection Data for ${inc}" )
	
	# Need to iterate through all include directories
	add_custom_command( TARGET Reflection POST_BUILD
		COMMAND ${ENJON_BUILD_DIR}/RelWithDebInfo/Reflection.exe --output-directory ${INTERMEDIATE_DIR} --input-file-path ${inc}
		)

endforeach(inc) 

