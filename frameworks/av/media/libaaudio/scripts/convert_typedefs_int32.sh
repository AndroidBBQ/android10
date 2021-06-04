#!/bin/bash

echo "Use SED to convert typedefs in AAudio API"

echo "Top is ${ANDROID_BUILD_TOP}"
LIBAAUDIO_DIR=${ANDROID_BUILD_TOP}/frameworks/av/media/libaaudio
echo "LIBAAUDIO_DIR is ${LIBAAUDIO_DIR}"
OBOESERVICE_DIR=${ANDROID_BUILD_TOP}/frameworks/av/services/oboeservice
echo "OBOESERVICE_DIR is ${OBOESERVICE_DIR}"
OBOETEST_DIR=${ANDROID_BUILD_TOP}/cts/tests/tests/nativemedia/aaudio/src/
echo "OBOETEST_DIR is ${OBOETEST_DIR}"

function convertPathPattern {
    path=$1
    pattern=$2
    find $path -type f  -name $pattern -exec sed -i -f ${LIBAAUDIO_DIR}/scripts/typedefs_to_int32.sed {} \;
}

function convertPath {
    path=$1
    convertPathPattern $1 '*.cpp'
    convertPathPattern $1 '*.h'
}

convertPath ${LIBAAUDIO_DIR}
convertPath ${OBOESERVICE_DIR}
convertPathPattern ${OBOETEST_DIR} test_aaudio.cpp

