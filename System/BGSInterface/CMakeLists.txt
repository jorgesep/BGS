cmake_minimum_required(VERSION 2.8)
project( BGSInterface )

# The version number.
SET( BGSInterface_MAJOR_VERSION 0 )
SET( BGSInterface_MINOR_VERSION 1 )
SET( BGSInterface_PATCH_LEVEL 0 )

SET( CMAKE_C_FLAGS "-Wall -g -O3" )
SET( CMAKE_CXX_FLAGS "-Wall -g -O3" )
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -O3")        ## Optimize
endif()

