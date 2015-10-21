cmake_minimum_required(VERSION 2.8)
project( SAGMM )

# The version number.
SET( SAGMM_MAJOR_VERSION 0 )
SET( SAGMM_MINOR_VERSION 1 )
SET( SAGMM_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS "-Wall -g -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3")        ## Optimize
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using regular Clang or AppleClang
    SET( CMAKE_CXX_FLAGS "-Wall -Wno-unused-local-typedef -Wno-overloaded-virtual -Wno-unused-const-variable -Wno-unused-private-field -g -O3" )
endif()

FILE ( GLOB SRCS *.cpp *.h )


INCLUDE_DIRECTORIES(${BGSInterface_SOURCE_DIR})
INCLUDE_DIRECTORIES(${Background_SOURCE_DIR}/../include)

#delete bin files in make clean
SET_DIRECTORY_PROPERTIES (
    PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${SAGMM_SOURCE_DIR}/*~"
    ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_SOURCE_DIR}/../bin/*"
    ADDITIONAL_MAKE_CLEAN_FILES "${SAGMM_BINARY_DIR}/*"
)

# openCV library
set(OpenCV_DIR "/opt/local/share/OpenCV"
           CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

find_package( Boost REQUIRED COMPONENTS filesystem system )
include_directories( ${Boost_INCLUDE_DIRS} )

ADD_LIBRARY( SAGMMBuilder SHARED ${SRCS} )
target_link_libraries( SAGMMBuilder SAGMM ${OpenCV_LIBS} ${Boost_LIBRARIES} )
set_property(TARGET SAGMMBuilder PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)

