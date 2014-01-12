cmake_minimum_required(VERSION 2.8)
project( Framework )

# The version number.
SET( Framework_MAJOR_VERSION 0 )
SET( Framework_MINOR_VERSION 1 )
SET( Framework_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS "-Wall -g -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3")        ## Optimize
endif()

#FILE ( GLOB SRCS FrameReaderFactory.cpp BGSFramework*.cpp *.h )
FILE ( GLOB SRCS BGSFramework*.cpp *.h )
FILE ( GLOB SCRIPTS ${PROJECT_SOURCE_DIR}/scripts/*.py ${PROJECT_SOURCE_DIR}/scripts/*.sh )

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/../include")
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/include")
INCLUDE_DIRECTORIES("/opt/local/include")
INCLUDE_DIRECTORIES(${MOG_SOURCE_DIR})
INCLUDE_DIRECTORIES(${MOG2_SOURCE_DIR})
INCLUDE_DIRECTORIES(${NPBGS_SOURCE_DIR})
INCLUDE_DIRECTORIES(${SAGMM_SOURCE_DIR})
INCLUDE_DIRECTORIES(${ucv_gmm_SOURCE_DIR})
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/include) 
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/ThirdParty/bg_subt_code)
INCLUDE_DIRECTORIES(${Background_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(${FrameReader_SOURCE_DIR})
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/ThirdParty/uCV/inc )
# this new definition is for UCV
add_definitions(-D__EDAQ__)

#delete bin files in make clean
SET_DIRECTORY_PROPERTIES (
    PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "${Framework_SOURCE_DIR}/*~"
        ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_SOURCE_DIR}/../bin/*"
        ADDITIONAL_MAKE_CLEAN_FILES "${Framework_BINARY_DIR}/*"
)

# Set DEBUG option
OPTION(DEBUG "DEBUG option to print out more info" ON)

# openCV library
set(OpenCV_DIR "/usr/local/share/OpenCV"
           CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

find_package( Boost REQUIRED COMPONENTS filesystem system )
include_directories( ${Boost_INCLUDE_DIRS} )

link_directories(${BGS_SOURCE_DIR}/Performance)
link_directories(${BGS_BINARY_DIR}/Utils) 


ADD_LIBRARY( Framework SHARED ${SRCS} )
target_link_libraries( Framework FrameReader ${OpenCV_LIBS} ${Boost_LIBRARIES} )
set_property(TARGET Framework PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)


ADD_EXECUTABLE( bgs_framework FrameworkImpl )
TARGET_LINK_LIBRARIES( bgs_framework Framework MOGBuilder NPBuilder MOG2Builder SAGMMBuilder UCVBuilder Utils ${OpenCV_LIBS} )
set_property(TARGET bgs_framework PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)


file(COPY ${PROJECT_SOURCE_DIR}/../config DESTINATION ${BGS_BINARY_DIR}/)
file(COPY ${SCRIPTS} DESTINATION ${BGS_BINARY_DIR}/bin/)


