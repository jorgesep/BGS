cmake_minimum_required(VERSION 2.8)
project( sagmm )

# The version number.
SET( sagmm_MAJOR_VERSION 0 )
SET( sagmm_MINOR_VERSION 1 )
SET( sagmm_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS "-Wall -g" )

#FILE ( GLOB SRCS *.cpp *.h )
FILE ( GLOB SRCS background_subtraction.cpp  mdgkt_filter.cpp bgs.cpp *.h )


FILE ( GLOB SCRIPTS ${PROJECT_SOURCE_DIR}/scripts/*.py ${PROJECT_SOURCE_DIR}/scripts/*.sh )


INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/../include")
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/include")
INCLUDE_DIRECTORIES("/opt/local/include")
INCLUDE_DIRECTORIES(${Performance_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/include) 

#set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/local/lib)

#delete bin files in make clean
SET_DIRECTORY_PROPERTIES (
    PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "${sagmm}_SOURCE_DIR}/*~"
        ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_SOURCE_DIR}/../bin/*"
        ADDITIONAL_MAKE_CLEAN_FILES "${sagmm}_BINARY_DIR}/*"
)

# Set DEBUG option
OPTION(DEBUG "DEBUG option to print out more info" ON)

# openCV library
set(OpenCV_DIR "/usr/local/share/OpenCV"
           CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

link_directories(${BGS_SOURCE_DIR}/Performance)
link_directories(${BGS_BINARY_DIR}/Utils) 


#find_library(Logging log4cplus PATHS /opt/local/lib)


#ADD_LIBRARY(Background SHARED  mdgkt_filter background_subtraction ${OpenCV_LIBS})
#set_property(TARGET Background PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)

ADD_EXECUTABLE( testVideoMdgkt mdgkt_filter.cpp testVideoMdgkt.cpp )
TARGET_LINK_LIBRARIES( testVideoMdgkt ${OpenCV_LIBS} )
set_property(TARGET testVideoMdgkt PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( testMdgkt mdgkt_filter.cpp testMdgkt.cpp )
TARGET_LINK_LIBRARIES( testMdgkt ${OpenCV_LIBS} )
set_property(TARGET testMdgkt PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( plot_point bg_points.cpp )
TARGET_LINK_LIBRARIES( plot_point ${OpenCV_LIBS} )
set_property(TARGET plot_point PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( bgs ${SRCS} )
TARGET_LINK_LIBRARIES( bgs ${OpenCV_LIBS} Performance Utils)
set_property(TARGET bgs PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

file(COPY ${PROJECT_SOURCE_DIR}/../config DESTINATION ${BGS_BINARY_DIR}/)
file(COPY ${SCRIPTS} DESTINATION ${BGS_BINARY_DIR}/bin/)


