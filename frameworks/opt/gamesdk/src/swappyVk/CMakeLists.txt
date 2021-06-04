cmake_minimum_required(VERSION 3.4.1)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Werror -Wthread-safety" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D _LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS -O3 -fPIC" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections" )

set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections" )
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s" )

include_directories( ../../include )
include_directories( ../common )
include_directories( ${ANDROID_NDK}/sources/third_party/vulkan/src/common )

message( STATUS "Building swappyVk_static to ${CMAKE_CURRENT_BINARY_DIR}/build" )
add_library( swappyVk_static

             STATIC

             SwappyVk.cpp
             ${ANDROID_NDK}/sources/third_party/vulkan/src/common/vulkan_wrapper.cpp

             # Add new source files here
             )

set_target_properties( swappyVk_static PROPERTIES
     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build )

add_library( swappyVk

             SHARED

             SwappyVk.cpp
           )


 target_link_libraries( swappyVk

                        swappyVk_static
                        android
                        log )
