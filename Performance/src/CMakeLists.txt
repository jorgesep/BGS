CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT( Performance )
# The version number.
SET( Performance_MAJOR_VERSION 0 )
SET( Performance_MINOR_VERSION 1 )
SET( Performance_PATCH_LEVEL 0 )


SET( CMAKE_C_FLAGS   "-g -Wall" )

FILE ( GLOB CONFIG ${PROJECT_SOURCE_DIR}/../config/*.jpg ${PROJECT_SOURCE_DIR}/../config/*.PNG ${PROJECT_SOURCE_DIR}/../config/*.png )

#MESSAGE("Project Source Dir set to ${PROJECT_SOURCE_DIR}")
#MESSAGE("Project Binary Dir set to ${PROJECT_BINARY_DIR}")
#MESSAGE("BGS Binary Dir set to ${BGS_BINARY_DIR}")
#MESSAGE("BGS Source Dir set to ${BGS_SOURCE_DIR}")
#MESSAGE("Performance Binary Dir set to ${Performance_BINARY_DIR}")
#MESSAGE("Performance Source Dir set to ${Performance_SOURCE_DIR}")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/include)
# This include was added to support boost/program_options.hpp
#INCLUDE_DIRECTORIES("/opt/local/include")
find_package( Boost REQUIRED COMPONENTS program_options regex )
include_directories( ${Boost_INCLUDE_DIRS} )
#find_library(Logging log4cplus PATHS /opt/local/lib)

link_directories(${BGS_BINARY_DIR}/Utils)

# Set DEBUG option
OPTION(DEBUG "DEBUG option to print out more info" ON)
set(OpenCV_DIR "/usr/local/share/OpenCV"
       CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

ADD_EXECUTABLE( testMatBinary testMatBinary )
TARGET_LINK_LIBRARIES( testMatBinary ${OpenCV_LIBS} )
#set_property(TARGET testMatBinary PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( testPerformance testPerformance )
TARGET_LINK_LIBRARIES( testPerformance Performance ${OpenCV_LIBS} )
set_property(TARGET testPerformance PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( pmbgs Measure )
TARGET_LINK_LIBRARIES( pmbgs Performance ${Boost_PROGRAM_OPTIONS_LIBRARY} Utils )
set_property(TARGET pmbgs PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_LIBRARY(Performance SHARED Performance)
target_link_libraries(Performance ${OpenCV_LIBS})
set_property(TARGET Performance PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)

file(COPY ${CONFIG} DESTINATION ${BGS_BINARY_DIR}/config/)

