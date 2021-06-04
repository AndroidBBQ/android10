cmake_minimum_required(VERSION 3.4.1)

include("../protobuf/protobuf.cmake")

protobuf_generate_lite_cpp( ${CMAKE_CURRENT_SOURCE_DIR}/../../include/device_info
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/device_info/device_info.proto)

set ( SOURCE_LOCATION .)

include_directories( ../../include )
include_directories( ../common )

include_directories( ${PROTO_GENS_DIR} )
include_directories( ${PROTOBUF_INCLUDE_DIR} )

message( STATUS "Building device_info_static to ${CMAKE_CURRENT_BINARY_DIR}/build" )
add_library( device_info_static

             STATIC

             ${SOURCE_LOCATION}/device_info.cpp
             ${PROTO_GENS_DIR}/lite/device_info.pb.cc

             # Add new source files here
             )

set_target_properties( device_info_static PROPERTIES
     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build )

