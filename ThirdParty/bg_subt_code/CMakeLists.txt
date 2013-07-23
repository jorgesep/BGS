CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT( NPBG )
# The version number.
SET( Performance_MAJOR_VERSION 0 )
SET( Performance_MINOR_VERSION 1 )
SET( Performance_PATCH_LEVEL 0 )


SET( CMAKE_C_FLAGS   "-g -Wall" )
SET( CMAKE_C_FLAGS   "-g -Wall" )
#SET( CMAKE_C_FLAGS   "-g -Wall O3" )
#SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
#if(CMAKE_COMPILER_IS_GNUCXX)
#    set(CMAKE_CXX_FLAGS "-Wall -O3")        ## Optimize
#endif()

FILE ( GLOB SRCS KernelTable.cpp NPBGmodel.cpp NPBGSubtractor.cpp *.h )

ADD_LIBRARY( NonParametricModel SHARED ${SRCS} )
set_property(TARGET NonParametricModel PROPERTY LIBRARY_OUTPUT_DIRECTORY ${NPBG_BINARY_DIR}/lib)
