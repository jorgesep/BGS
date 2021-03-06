CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT( Performance )
# The version number.
SET( Performance_MAJOR_VERSION 0 )
SET( Performance_MINOR_VERSION 1 )
SET( Performance_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS   "-g -Wall -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3 -Wno-uninitialized")        ## Optimize
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using regular Clang or AppleClang
    SET( CMAKE_CXX_FLAGS "-Wall -Wno-unused-local-typedef -Wno-overloaded-virtual -g -O3" )
endif()

FILE ( GLOB CONFIG ${PROJECT_SOURCE_DIR}/../config/*.jpg ${PROJECT_SOURCE_DIR}/../config/*.PNG ${PROJECT_SOURCE_DIR}/../config/*.png )
FILE ( GLOB SCRIPTS ${PROJECT_SOURCE_DIR}/scripts/*.py ${PROJECT_SOURCE_DIR}/scripts/*.sh )

#MESSAGE("Project Source Dir set to ${PROJECT_SOURCE_DIR}")
#MESSAGE("Project Binary Dir set to ${PROJECT_BINARY_DIR}")
#MESSAGE("BGS Binary Dir set to ${BGS_BINARY_DIR}")
#MESSAGE("BGS Source Dir set to ${BGS_SOURCE_DIR}")
#MESSAGE("Performance Binary Dir set to ${Performance_BINARY_DIR}")
#MESSAGE("Performance Source Dir set to ${Performance_SOURCE_DIR}")
#MESSAGE("CMAKE_CURRENT_SOURCE_DIR Source Dir set to ${CMAKE_CURRENT_SOURCE_DIR}")
#MESSAGE("CMAKE_CURRENT_BINARY_DIR Source Dir set to ${CMAKE_CURRENT_BINARY_DIR}")
# — OpenMP —
if(NOT HAVE_TBB AND NOT HAVE_CSTRIPES)
    include (FindOpenMP) # — since cmake version 2.6.3
    if (OPENMP_FOUND)
        set (HAVE_OPENMP TRUE)
        set (CMAKE_CXX_FLAGS “${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}”)
        set (CMAKE_C_FLAGS “${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}”)
    else()
        set ( HAVE_OPENMP FALSE)
    endif()
else()
    set(HAVE_OPENMP 0)
endif()



INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/FileUtils/include)

# This include was added to support boost/program_options.hpp
SET(Boost_USE_MULTITHREAD ON)
find_package( Boost REQUIRED COMPONENTS system program_options regex filesystem thread )
include_directories( ${Boost_INCLUDE_DIRS} )

link_directories(${BGS_BINARY_DIR}/Utils)

# Set DEBUG option
OPTION(DEBUG "DEBUG option to print out more info" ON)
set(OpenCV_DIR "/opt/local/share/OpenCV"
       CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

ADD_EXECUTABLE( testMatBinary testMatBinary )
TARGET_LINK_LIBRARIES( testMatBinary ${OpenCV_LIBS} )
set_property(TARGET testMatBinary PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( testPerformance testPerformance )
TARGET_LINK_LIBRARIES( testPerformance Performance ${OpenCV_LIBS} )
set_property(TARGET testPerformance PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( testDScore testDScore )
TARGET_LINK_LIBRARIES( testDScore Performance ${OpenCV_LIBS} )
set_property(TARGET testDScore PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_EXECUTABLE( createMapDScore CreateDScoreMap )
TARGET_LINK_LIBRARIES( createMapDScore Performance ${OpenCV_LIBS} ${Boost_PROGRAM_OPTIONS_LIBRARY} Utils )
set_property(TARGET createMapDScore PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)


ADD_EXECUTABLE( pmbgs Measure )
TARGET_LINK_LIBRARIES( pmbgs Performance ${Boost_PROGRAM_OPTIONS_LIBRARY} Utils Timer)
set_property(TARGET pmbgs PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_LIBRARY(Performance SHARED Performance)
target_link_libraries(Performance ${OpenCV_LIBS} ${Boost_LIBRARIES} )
set_property(TARGET Performance PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)

file(COPY ${CONFIG} DESTINATION ${BGS_BINARY_DIR}/config/)
file(COPY ${SCRIPTS} DESTINATION ${BGS_BINARY_DIR}/bin/)

INSTALL(DIRECTORY ../include/ DESTINATION include/bgs)

INSTALL(TARGETS testMatBinary testPerformance testDScore createMapDScore pmbgs
                Performance
        RUNTIME DESTINATION bin COMPONENT app
        LIBRARY DESTINATION lib COMPONENT runtime
        ARCHIVE DESTINATION lib COMPONENT runtime
        PUBLIC_HEADER DESTINATION include/bgs COMPONENT dev
        FRAMEWORK DESTINATION "/Library/Frameworks"
)
