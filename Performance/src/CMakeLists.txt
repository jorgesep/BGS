CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT( Performance )
# The version number.
SET( Performance_MAJOR_VERSION 0 )
SET( Performance_MINOR_VERSION 1 )
SET( Performance_PATCH_LEVEL 0 )


SET( CMAKE_C_FLAGS   "-g -Wall" )

#MESSAGE("Project Source Dir set to ${PROJECT_SOURCE_DIR}")
#MESSAGE("Project Binary Dir set to ${PROJECT_BINARY_DIR}")
#MESSAGE("BGS Binary Dir set to ${BGS_BINARY_DIR}")
#MESSAGE("BGS Source Dir set to ${BGS_SOURCE_DIR}")
#MESSAGE("Performance Binary Dir set to ${Performance_BINARY_DIR}")
#MESSAGE("Performance Source Dir set to ${Performance_SOURCE_DIR}")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../include)

# Set DEBUG option
OPTION(DEBUG "DEBUG option to print out more info" ON)
set(OpenCV_DIR "/usr/local/share/OpenCV"
       CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

ADD_EXECUTABLE( testPerformance testPerformance )
TARGET_LINK_LIBRARIES( testPerformance Performance ${OpenCV_LIBS} )
set_property(TARGET testPerformance PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_LIBRARY(Performance SHARED Performance)
target_link_libraries(Performance ${OpenCV_LIBS})
set_property(TARGET Performance PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)


