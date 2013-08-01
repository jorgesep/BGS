CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT( NPBGS )
# The version number.
SET( Performance_MAJOR_VERSION 0 )
SET( Performance_MINOR_VERSION 1 )
SET( Performance_PATCH_LEVEL 0 )


#SET( CMAKE_C_FLAGS   "-g -Wall" )
#SET( CMAKE_CXX_FLAGS "-Wall -g" )
SET( CMAKE_C_FLAGS   "-g -Wall O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3")        ## Optimize
endif()

FILE ( GLOB TEST test*.cpp *.h )
FILE ( GLOB SRCS NPBG*.cpp *.h )
FILE ( GLOB BUILDER  NPBuilder.cpp *.h )
#find_package( Boost REQUIRED COMPONENTS program_options )

# openCV library
set(OpenCV_DIR "/usr/local/share/OpenCV"
               CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

find_package( Boost REQUIRED COMPONENTS filesystem system )
include_directories( ${Boost_INCLUDE_DIRS} )

INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/include)
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Performance/include)
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Background/include)
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/ThirdParty/bg_subt_code)
INCLUDE_DIRECTORIES(${Framework_SOURCE_DIR})
link_directories(${BGS_BINARY_DIR}/Utils)
link_directories(${BGS_BINARY_DIR}/Performance)
link_directories(${BGS_BINARY_DIR}/Background)
link_directories(${BGS_BINARY_DIR}/lib)

ADD_EXECUTABLE( testNP ${TEST} )
#TARGET_LINK_LIBRARIES( testNP  ${OpenCV_LIBS} NonParametricModel ${Boost_LIBRARIES} )
TARGET_LINK_LIBRARIES( testNP  ${OpenCV_LIBS} NonParametricModel )
set_property(TARGET testNP PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_LIBRARY( NPBuilder SHARED ${BUILDER} )
target_link_libraries( NPBuilder NonParametricModel ${OpenCV_LIBS} ${Boost_LIBRARIES})
set_property(TARGET NPBuilder PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)


ADD_EXECUTABLE( npbgs ${SRCS} )
TARGET_LINK_LIBRARIES( npbgs NonParametricModel  Utils Performance SAGMM ${OpenCV_LIBS} ${Boost_LIBRARIES} )
set_property(TARGET npbgs PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

file(COPY ${PROJECT_SOURCE_DIR}/../config DESTINATION ${BGS_BINARY_DIR}/)
