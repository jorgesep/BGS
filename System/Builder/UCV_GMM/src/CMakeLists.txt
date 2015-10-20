cmake_minimum_required(VERSION 2.8)
project( ucv_gmm )

# The version number.
SET( ucv_gmm_MAJOR_VERSION 0 )
SET( ucv_gmm_MINOR_VERSION 1 )
SET( ucv_gmm_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS "-Wall -g -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3 -Wno-uninitialized")        ## Optimize
endif()

FILE ( GLOB SRCS *.cpp *.h )
FILE ( GLOB BUILDER UCVBuilder.cpp UCVBuilder.h )
FILE ( GLOB SCRIPTS ${PROJECT_SOURCE_DIR}/scripts/*.py ${PROJECT_SOURCE_DIR}/scripts/*.sh )

add_definitions(-D__EDAQ__)

INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/FileUtils/include) 
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/ThirdParty/uCV/inc ) 
INCLUDE_DIRECTORIES(${FrameReader_SOURCE_DIR})
INCLUDE_DIRECTORIES(${BGSInterface_SOURCE_DIR})

#delete bin files in make clean
SET_DIRECTORY_PROPERTIES (
    PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_SOURCE_DIR}/../bin/*"
)

# openCV library
set(OpenCV_DIR "/opt/local/share/OpenCV"
           CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

find_library(ucv_gmm uCV PATHS ${ThirdParty_SOURCE_DIR}/uCV/lib )
find_library(ucv_gmm_methods uCVMethods PATHS ${ThirdParty_SOURCE_DIR}/uCV/lib )

find_package( Boost REQUIRED COMPONENTS filesystem system )
include_directories( ${Boost_INCLUDE_DIRS} )

ADD_LIBRARY( UCVBuilder SHARED ${BUILDER} )
target_link_libraries( UCVBuilder ${OpenCV_LIBS} ${Boost_LIBRARIES} ${ucv_gmm_methods} ${ucv_gmm} Timer )
set_property(TARGET UCVBuilder PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)

ADD_EXECUTABLE( testUCV ${SRCS} )
target_link_libraries( testUCV ${OpenCV_LIBS} ${Boost_LIBRARIES} ${ucv_gmm_methods} ${ucv_gmm} FrameReader Utils Timer)
set_property(TARGET testUCV PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

file(COPY ${PROJECT_SOURCE_DIR}/../config DESTINATION ${BGS_BINARY_DIR}/)
file(COPY ${SCRIPTS} DESTINATION ${BGS_BINARY_DIR}/bin/)
