CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(Test)
# The version number.
SET( Test_MAJOR_VERSION 0 )
SET( Test_MINOR_VERSION 1 )
SET( Test_PATCH_LEVEL 0 )


SET( CMAKE_C_FLAGS "-Wall -g" )
FILE ( GLOB SRCS *.cpp *.h )

INCLUDE_DIRECTORIES(${Performance_SOURCE_DIR}/../include)

# Set DEBUG option
OPTION(DEBUG "DEBUG option to print out more info" ON)
set(OpenCV_DIR "/usr/local/share/OpenCV"
       CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

ADD_EXECUTABLE( test ${SRCS} )
TARGET_LINK_LIBRARIES( test ${OpenCV_LIBS} ${Performance_SOURCE_DIR/../lib} )
set_property(TARGET test PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin)

