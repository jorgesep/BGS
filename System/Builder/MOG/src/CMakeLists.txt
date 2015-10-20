cmake_minimum_required(VERSION 2.8)
project( MOG )

# The version number.
SET( MOG_MAJOR_VERSION 0 )
SET( MOG_MINOR_VERSION 1 )
SET( MOG_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS "-Wall -g -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3")        ## Optimize
endif()

FILE ( GLOB SRCS *.cpp *.h )

INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/include) 
INCLUDE_DIRECTORIES(${BGSInterface_SOURCE_DIR})

#delete bin files in make clean
SET_DIRECTORY_PROPERTIES (
    PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "${MOG_SOURCE_DIR}/*~"
        ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_SOURCE_DIR}/../bin/*"
        ADDITIONAL_MAKE_CLEAN_FILES "${MOG_BINARY_DIR}/*"
)

# openCV library
set(OpenCV_DIR "/opt/local/share/OpenCV"
           CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

find_package( Boost REQUIRED COMPONENTS filesystem system )
include_directories( ${Boost_INCLUDE_DIRS} )

ADD_LIBRARY( MOGBuilder SHARED ${SRCS} )
target_link_libraries( MOGBuilder ${OpenCV_LIBS} ${Boost_LIBRARIES} Timer )
set_property(TARGET MOGBuilder PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)
