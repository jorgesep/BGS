cmake_minimum_required(VERSION 2.8)
project( utils )

# The version number.
SET( utils_MAJOR_VERSION 0 )
SET( utils_MINOR_VERSION 1 )
SET( utils_PATCH_LEVEL 0 )


SET( CMAKE_C_FLAGS   "-g -Wall -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3")        ## Optimize
endif()

FILE ( GLOB SRCS *.cpp *.h )

INCLUDE_DIRECTORIES("${utils_SOURCE_DIR}/../include")

#delete bin files in make clean
SET_DIRECTORY_PROPERTIES (
    PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "${utils}_SOURCE_DIR}/*~"
        ADDITIONAL_MAKE_CLEAN_FILES "${utils_SOURCE_DIR}/../bin/*"
        ADDITIONAL_MAKE_CLEAN_FILES "${utils_BINARY_DIR}/../bin/*"
        ADDITIONAL_MAKE_CLEAN_FILES "${utils_BINARY_DIR}/../lib/*"
)

# Set DEBUG option
OPTION(DEBUG "DEBUG option to print out more info" ON)

# openCV library
set(OpenCV_DIR "/usr/local/share/OpenCV"
           CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

find_package( Boost REQUIRED COMPONENTS filesystem system regex )
include_directories( ${Boost_INCLUDE_DIRS} )

ADD_LIBRARY(Utils SHARED ${SRCS}) 
TARGET_LINK_LIBRARIES( Utils ${Boost_LIBRARIES} )
set_property(TARGET Utils PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)

