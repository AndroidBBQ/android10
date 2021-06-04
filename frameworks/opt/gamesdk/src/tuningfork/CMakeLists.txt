cmake_minimum_required(VERSION 3.4.1)

include("../protobuf/protobuf.cmake")
set( MODPB64_DIR "${EXTERNAL_ROOT}/modp_b64")

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections,-s")
endif (${CMAKE_BUILD_TYPE} STREQUAL "Release")

function(extra_tf_link_options libname)
  set_link_options(${libname} "tuningfork/version.script")
endfunction()

protobuf_generate_nano_c( ${CMAKE_CURRENT_SOURCE_DIR}/proto
  proto/tuningfork.proto
  proto/tuningfork_clearcut_log.proto
  proto/example_tuningfork.proto)

include_directories(${PROTOBUF_NANO_SRC_DIR})
include_directories( ../../include )
include_directories( ../../src/swappy )
include_directories( ../common )
include_directories( ${MODPB64_DIR}/modp_b64)

include_directories(${PROTO_GENS_DIR})

set( TUNINGFORK_SRCS
  clearcut_backend.cpp
  histogram.cpp
  prong.cpp
  uploadthread.cpp
  tuningfork.cpp
  tuningfork_c.cpp
  clearcutserializer.cpp
  protobuf_util.cpp
  annotation_util.cpp
  tuningfork_extra.cpp
  tuningfork_utils.cpp
  ${MODPB64_DIR}/modp_b64.cc
  ${PROTO_GENS_DIR}/nano/tuningfork.pb.c
  ${PROTO_GENS_DIR}/nano/tuningfork_clearcut_log.pb.c
  ${PROTO_GENS_DIR}/nano/example_tuningfork.pb.c)

add_library( tuningfork_static
  STATIC ${TUNINGFORK_SRCS} ${PROTOBUF_NANO_SRCS})
set_target_properties( tuningfork_static PROPERTIES
  COMPILE_OPTIONS "-DPROTOBUF_NANO" )

add_library( tuningfork
  SHARED ${TUNINGFORK_SRCS} ${PROTOBUF_NANO_SRCS})
set_target_properties( tuningfork PROPERTIES COMPILE_OPTIONS "-DPROTOBUF_NANO" )
target_link_libraries( tuningfork
  android
  GLESv2
  log)
extra_tf_link_options( tuningfork )
