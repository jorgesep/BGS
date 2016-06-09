cmake_minimum_required(VERSION 2.8)
project( BaseSystem )

# The version number.
SET( BaseSystem_MAJOR_VERSION 0 )
SET( BaseSystem_MINOR_VERSION 1 )
SET( BaseSystem_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS "-Wall -g -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3")        ## Optimize
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using regular Clang or AppleClang
    SET( CMAKE_CXX_FLAGS "-Wall -Wno-unused-local-typedef -Wno-overloaded-virtual -g -O3" )
endif()

#FILE ( GLOB SRCS FrameReaderFactory.cpp BGSSystem*.cpp *.h )
FILE ( GLOB SRCS BGSSystem*.cpp *.h )
FILE ( GLOB SCRIPTS ${PROJECT_SOURCE_DIR}/scripts/*.py ${PROJECT_SOURCE_DIR}/scripts/*.sh )

INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/include")
INCLUDE_DIRECTORIES("/opt/local/include")
INCLUDE_DIRECTORIES(${MOG_SOURCE_DIR})
INCLUDE_DIRECTORIES(${MOG2_SOURCE_DIR})
INCLUDE_DIRECTORIES(${NPBGS_SOURCE_DIR})
INCLUDE_DIRECTORIES(${SAGMM_SOURCE_DIR})
INCLUDE_DIRECTORIES(${ucv_gmm_SOURCE_DIR})
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/FileUtils/include) 
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/ThirdParty/bg_subt_code)
INCLUDE_DIRECTORIES(${Background_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(${FrameReader_SOURCE_DIR})
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/ThirdParty/uCV/inc )
INCLUDE_DIRECTORIES(${BGSInterface_SOURCE_DIR})
# this new definition is for UCV
add_definitions(-D__EDAQ__)

#delete bin files in make clean
SET_DIRECTORY_PROPERTIES (
    PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "${BaseSystem_SOURCE_DIR}/*~"
        ADDITIONAL_MAKE_CLEAN_FILES "${BaseSystem_BINARY_DIR}/*"
        ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_SOURCE_DIR}/../bin/*"
)

# Set DEBUG option
OPTION(DEBUG "DEBUG option to print out more info" ON)

# openCV library
set(OpenCV_DIR "/opt/local/share/OpenCV"
           CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

find_package( Boost REQUIRED COMPONENTS filesystem system )
include_directories( ${Boost_INCLUDE_DIRS} )

link_directories(${BGS_SOURCE_DIR}/Performance)
link_directories(${BGS_BINARY_DIR}/Utils) 


ADD_LIBRARY( BaseSystem SHARED ${SRCS} )
target_link_libraries( BaseSystem FrameReader ${OpenCV_LIBS} ${Boost_LIBRARIES} )
set_property(TARGET BaseSystem PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)


ADD_EXECUTABLE( bgs_system SystemClient )
#TARGET_LINK_LIBRARIES( bgs_system BaseSystem MOGBuilder NPBuilder MOG2Builder SAGMMBuilder UCVBuilder Utils ${OpenCV_LIBS} )
TARGET_LINK_LIBRARIES( bgs_system BaseSystem MOGBuilder NPBuilder MOG2Builder SAGMMBuilder Utils ${OpenCV_LIBS} )
set_property(TARGET bgs_system PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)


file(COPY ${PROJECT_SOURCE_DIR}/../config DESTINATION ${BGS_BINARY_DIR}/)
file(COPY ${SCRIPTS} DESTINATION ${BGS_BINARY_DIR}/bin/)

FILE( GLOB files *.h )
INSTALL(FILES ${files} DESTINATION include/bgs )

INSTALL(TARGETS bgs_system
        RUNTIME DESTINATION bin COMPONENT app
        LIBRARY DESTINATION lib COMPONENT runtime
        ARCHIVE DESTINATION lib COMPONENT runtime
        PUBLIC_HEADER DESTINATION include/bgs COMPONENT dev
        FRAMEWORK DESTINATION "/Library/Frameworks"
        )
