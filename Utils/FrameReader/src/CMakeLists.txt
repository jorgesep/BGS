cmake_minimum_required(VERSION 2.8)
project( FrameReader )

# The version number.
SET( FrameReader_MAJOR_VERSION 0 )
SET( FrameReader_MINOR_VERSION 1 )
SET( FrameReader_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS "-Wall -g -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3")        ## Optimize
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using regular Clang or AppleClang
    SET( CMAKE_CXX_FLAGS "-Wall -Wno-unused-local-typedef -g -O3" )
endif()

FILE ( GLOB SRCS *.cpp *.h )

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/../include")
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/include")
INCLUDE_DIRECTORIES("/opt/local/include")
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/include) 


# Set DEBUG option
OPTION(DEBUG "DEBUG option to print out more info" ON)

# openCV library
set(OpenCV_DIR " /opt/local/share/OpenCV"
           CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

find_package( Boost REQUIRED COMPONENTS filesystem system )
include_directories( ${Boost_INCLUDE_DIRS} )

#link_directories(${BGS_BINARY_DIR}/Utils) 


ADD_LIBRARY( FrameReader SHARED ${SRCS} )
target_link_libraries( FrameReader ${OpenCV_LIBS} ${Boost_LIBRARIES} )
set_property(TARGET FrameReader PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)




