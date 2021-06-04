#!/bin/bash

echo "Revert typedefs"

echo "Top is ${ANDROID_BUILD_TOP}"
LIBAAUDIO_DIR=${ANDROID_BUILD_TOP}/frameworks/av/media/libaaudio
echo "LIBAAUDIO_DIR is ${LIBAAUDIO_DIR}"
OBOESERVICE_DIR=${ANDROID_BUILD_TOP}/frameworks/av/services/oboeservice
echo "OBOESERVICE_DIR is ${OBOESERVICE_DIR}"
OBOETEST_DIR=${ANDROID_BUILD_TOP}/cts/tests/tests/nativemedia/aaudio/src/
echo "OBOETEST_DIR is ${OBOETEST_DIR}"

git checkout -- ${LIBAAUDIO_DIR}/examples
git checkout -- ${LIBAAUDIO_DIR}/include
git checkout -- ${LIBAAUDIO_DIR}/src
git checkout -- ${LIBAAUDIO_DIR}/tests
git checkout -- ${OBOESERVICE_DIR}

