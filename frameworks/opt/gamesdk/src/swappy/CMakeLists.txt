cmake_minimum_required(VERSION 3.4.1)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Werror -Wthread-safety" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D _LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS -O3 -fPIC" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections" )
if ( DEFINED GAMESDK_THREAD_CHECKS )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGAMESDK_THREAD_CHECKS=${GAMESDK_THREAD_CHECKS}" )
endif()

set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections" )
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s" )

set ( SOURCE_LOCATION .)

include_directories( ../../include )
include_directories( ../common )

message( STATUS "Building swappy_static to ${CMAKE_CURRENT_BINARY_DIR}/build" )
add_library( swappy_static

             STATIC

             ${SOURCE_LOCATION}/ChoreographerFilter.cpp
             ${SOURCE_LOCATION}/ChoreographerThread.cpp
             ${SOURCE_LOCATION}/EGL.cpp
             ${SOURCE_LOCATION}/FrameStatistics.cpp
             ${SOURCE_LOCATION}/CpuInfo.cpp
             ${SOURCE_LOCATION}/SystemProperties.cpp
             ${SOURCE_LOCATION}/Swappy.cpp
             ${SOURCE_LOCATION}/Settings.cpp
             ${SOURCE_LOCATION}/Thread.cpp
             ${SOURCE_LOCATION}/Swappy.cpp
             ${SOURCE_LOCATION}/Swappy_c.cpp

             # Add new source files here
             )

set_target_properties( swappy_static PROPERTIES
     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build )

add_library( swappy

             SHARED

             ${SOURCE_LOCATION}/Swappy_c.cpp)
                            # Looks like I need to specify at least one file to build,
                            # the result .so size suggests it's including the rest of the
                            # source files from the library
                            # also, removing the target_link to the static library
                            # fails the build


 target_link_libraries( swappy

                        swappy_static
                        android
                        EGL
                        GLESv2
                        log )
