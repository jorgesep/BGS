cmake_minimum_required(VERSION 2.8)
project( MOG2 )

# The version number.
SET( MOG2_MAJOR_VERSION 0 )
SET( MOG2_MINOR_VERSION 1 )
SET( MOG2_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS "-Wall -g -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3")        ## Optimize
endif()

FILE ( GLOB SRCS *.cpp *.h )

#INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/")
#INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/../include")
#INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/include")
#INCLUDE_DIRECTORIES("/opt/local/include")
#INCLUDE_DIRECTORIES(${Performance_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/FileUtils/include) 
INCLUDE_DIRECTORIES(${BGSInterface_SOURCE_DIR})

#delete bin files in make clean
SET_DIRECTORY_PROPERTIES (
    PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "${MOG2_SOURCE_DIR}/*~"
        ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_SOURCE_DIR}/../bin/*"
        ADDITIONAL_MAKE_CLEAN_FILES "${MOG2_BINARY_DIR}/*"
)

# openCV library
set(OpenCV_DIR "/usr/local/share/OpenCV"
           CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

find_package( Boost REQUIRED COMPONENTS filesystem system )
include_directories( ${Boost_INCLUDE_DIRS} )

ADD_LIBRARY( MOG2Builder SHARED ${SRCS} )
target_link_libraries( MOG2Builder ${OpenCV_LIBS} ${Boost_LIBRARIES} Timer )
set_property(TARGET MOG2Builder PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)

file(COPY ${PROJECT_SOURCE_DIR}/../config DESTINATION ${BGS_BINARY_DIR}/)
