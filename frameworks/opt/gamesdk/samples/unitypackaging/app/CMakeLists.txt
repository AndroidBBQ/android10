cmake_minimum_required(VERSION 3.4.1)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Werror -Wthread-safety" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D _LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS -O3 -fPIC" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections" )

set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections" )
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${CMAKE_SOURCE_DIR}/version.script" )
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s" )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../include)

add_library( androidgamesdk

             SHARED

             src/main/cpp/swappy/ChoreographerFilter.cpp
             src/main/cpp/swappy/EGL.cpp
             src/main/cpp/swappy/Swappy.cpp
             src/main/cpp/swappy/Settings.cpp
             src/main/cpp/swappy/Thread.cpp
             src/main/cpp/swappy/Swappy_c.cpp)

include_directories( src/main/cpp )

target_link_libraries( androidgamesdk
                       EGL
                       log )
