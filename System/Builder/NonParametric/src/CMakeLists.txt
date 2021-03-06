CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT( NPBGS )

# The version number.
SET( NPBGS_MAJOR_VERSION 0 )
SET( NPBGS_MINOR_VERSION 1 )
SET( NPBGS_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS   "-g -Wall -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3")        ## Optimize
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using regular Clang or AppleClang
    SET( CMAKE_CXX_FLAGS "-Wall -Wno-unused-local-typedef -Wno-overloaded-virtual -Wno-unused-const-variable -Wno-unused-private-field -g -O3" )
endif()

FILE ( GLOB TEST test*.cpp *.h )
FILE ( GLOB SRCS NPBG*.cpp *.h )
FILE ( GLOB BUILDER  NPBuilder.cpp *.h )
FILE ( GLOB SCRIPTS ${PROJECT_SOURCE_DIR}/scripts/*.py ${PROJECT_SOURCE_DIR}/scripts/*.sh )

INCLUDE_DIRECTORIES(${BGSInterface_SOURCE_DIR})
INCLUDE_DIRECTORIES(${FrameReader_SOURCE_DIR})
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/Utils/FileUtils/include)
INCLUDE_DIRECTORIES(${BGS_SOURCE_DIR}/ThirdParty/bg_subt_code)

#link_directories(${BGS_BINARY_DIR}/Utils)
#link_directories(${BGS_BINARY_DIR}/Performance)
#link_directories(${BGS_BINARY_DIR}/Background)
#link_directories(${BGS_BINARY_DIR}/lib)

# openCV library
set(OpenCV_DIR "/opt/local/share/OpenCV"
               CACHE PATH "The path where OpenCVConfig.cmake is placed")
FIND_PACKAGE( OpenCV REQUIRED )

find_package( Boost REQUIRED COMPONENTS filesystem system )
include_directories( ${Boost_INCLUDE_DIRS} )


ADD_EXECUTABLE( testNP ${TEST} )
TARGET_LINK_LIBRARIES( testNP  ${OpenCV_LIBS} NonParametricModel )
set_property(TARGET testNP PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

ADD_LIBRARY( NPBuilder SHARED ${BUILDER} )
target_link_libraries( NPBuilder NonParametricModel ${OpenCV_LIBS} ${Boost_LIBRARIES})
set_property(TARGET NPBuilder PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)


ADD_EXECUTABLE( npbgs ${SRCS} )
TARGET_LINK_LIBRARIES( npbgs NonParametricModel  Utils Performance  FrameReader ${OpenCV_LIBS} ${Boost_LIBRARIES} Timer)
set_property(TARGET npbgs PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/bin)

file(COPY ${PROJECT_SOURCE_DIR}/../config DESTINATION ${BGS_BINARY_DIR}/)
file(COPY ${SCRIPTS} DESTINATION ${BGS_BINARY_DIR}/bin/)

FILE( GLOB files *.h )
INSTALL(FILES ${files} DESTINATION include/bgs )

INSTALL(TARGETS npbgs testNP NPBuilder
        RUNTIME DESTINATION bin COMPONENT app
        LIBRARY DESTINATION lib COMPONENT runtime
        ARCHIVE DESTINATION lib COMPONENT runtime
        PUBLIC_HEADER DESTINATION include/bgs COMPONENT dev
        FRAMEWORK DESTINATION "/Library/Frameworks"
                                                   )
