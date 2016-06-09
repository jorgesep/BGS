CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT( NPBG )
# The version number.
SET( Performance_MAJOR_VERSION 0 )
SET( Performance_MINOR_VERSION 1 )
SET( Performance_PATCH_LEVEL 0 )


#SET( CMAKE_C_FLAGS   "-g -Wall" )
#SET( CMAKE_C_FLAGS   "-g -Wall" )
SET( CMAKE_C_FLAGS   "-g -Wall O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3 -Wno-uninitialized -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare")        ## Optimize
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using regular Clang or AppleClang
    SET( CMAKE_CXX_FLAGS "-Wall -Wno-unused-local-typedef -Wno-unused-variable -Wno-overloaded-virtual -g -O3" )
endif()

FILE ( GLOB SRCS KernelTable.cpp NPBGmodel.cpp NPBGSubtractor.cpp *.h )

ADD_LIBRARY( NonParametricModel SHARED ${SRCS} )
set_property(TARGET NonParametricModel PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BGS_BINARY_DIR}/lib)
#set_property(TARGET NonParametricModel PROPERTY LIBRARY_OUTPUT_DIRECTORY ${NPBG_BINARY_DIR}/lib)

FILE( GLOB files *.h )
INSTALL(FILES ${files} DESTINATION include/bgs )

INSTALL(TARGETS
        RUNTIME DESTINATION bin COMPONENT app
        LIBRARY DESTINATION lib COMPONENT runtime
        ARCHIVE DESTINATION lib COMPONENT runtime
        PUBLIC_HEADER DESTINATION include/bgs COMPONENT dev
        FRAMEWORK DESTINATION "/Library/Frameworks"
        )
