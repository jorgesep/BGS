cmake_minimum_required(VERSION 2.8)
project( Background )

# The version number.
SET( Background_MAJOR_VERSION 0 )
SET( Background_MINOR_VERSION 1 )
SET( Background_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS "-Wall -g -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3 -Wno-uninitialized")        ## Optimize
endif()

FILE ( GLOB SRCS background_subtraction.cpp  mdgkt_filter.cpp icdm_model.cpp sagmm_client.cpp *.h )
FILE ( GLOB MODULE background_subtraction.cpp  mdgkt_filter.cpp icdm_model.cpp *.h )


FILE ( GLOB SCRIPTS ${PROJECT_SOURCE_DIR}/scripts/*.py ${PROJECT_SOURCE_DIR}/scripts/*.sh )


INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/../include")
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/include")
INCLUDE_DIRECTORIES("/opt/local/include")
INCLUDE_DIRECTORIES(${Performance_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/FileUtils/include) 
INCLUDE_DIRECTORIES(${FrameReader_SOURCE_DIR})


#delete bin files in make clean
SET_DIRECTORY_PROPERTIES (
    PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "${Background_SOURCE_DIR}/*~"
        ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_SOURCE_DIR}/../bin/*"
        ADDITIONAL_MAKE_CLEAN_FILES "${Background_BINARY_DIR}/*"
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

ADD_LIBRARY( SAGMM SHARED ${MODULE} )
target_link_libraries(SAGMM ${OpenCV_LIBS})
set_property(TARGET SAGMM PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)

ADD_EXECUTABLE( testVideoMdgkt testVideoMdgkt )
TARGET_LINK_LIBRARIES( testVideoMdgkt SAGMM ${OpenCV_LIBS} )
set_property(TARGET testVideoMdgkt PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( testIcdm testIcdm )
TARGET_LINK_LIBRARIES( testIcdm SAGMM ${OpenCV_LIBS} )
set_property(TARGET testIcdm PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( testMdgkt testMdgkt )
TARGET_LINK_LIBRARIES( testMdgkt SAGMM ${OpenCV_LIBS} )
set_property(TARGET testMdgkt PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( plot_point bg_points )
TARGET_LINK_LIBRARIES( plot_point ${OpenCV_LIBS} )
set_property(TARGET plot_point PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( play_sequence play_sequence )
TARGET_LINK_LIBRARIES( play_sequence FrameReader Utils ${OpenCV_LIBS} )
set_property(TARGET play_sequence PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( play_mask play_sequence_masks )
TARGET_LINK_LIBRARIES( play_mask Utils ${OpenCV_LIBS} )
set_property(TARGET play_mask PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( sagmm sagmm_client )
TARGET_LINK_LIBRARIES( sagmm ${OpenCV_LIBS} ${Boost_LIBRARIES} SAGMM Performance Utils Timer ImageUtils )
set_property(TARGET sagmm PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

file(COPY ${PROJECT_SOURCE_DIR}/../config DESTINATION ${BGS_BINARY_DIR}/)
file(COPY ${SCRIPTS} DESTINATION ${BGS_BINARY_DIR}/bin/)


