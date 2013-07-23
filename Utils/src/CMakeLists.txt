cmake_minimum_required(VERSION 2.8)
project( utils )

# The version number.
SET( utils_MAJOR_VERSION 0 )
SET( utils_MINOR_VERSION 1 )
SET( utils_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS "-Wall -g" )

FILE ( GLOB SRCS *.cpp *.h )

INCLUDE_DIRECTORIES("${utils_SOURCE_DIR}/../include")

#MESSAGE("PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")
#MESSAGE("PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR}")
#
#MESSAGE("utils_SOURCE_DIR ${utils_SOURCE_DIR}")
#MESSAGE("utils_BINARY_DIR ${utils_BINARY_DIR}")
#
#MESSAGE("BGS_SOURCE_DIR ${BGS_SOURCE_DIR}")
#MESSAGE("BGS_BINARY_DIR ${BGS_BINARY_DIR}")


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

ADD_LIBRARY(Utils SHARED ${SRCS}) 
set_property(TARGET Utils PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)

