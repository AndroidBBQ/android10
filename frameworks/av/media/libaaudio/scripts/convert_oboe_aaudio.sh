#!/bin/bash
# Use SED to convert the Oboe API to the AAudio API

echo "Convert Oboe names to AAudio names"

echo "Top is ${ANDROID_BUILD_TOP}"
LIBOBOE_DIR=${ANDROID_BUILD_TOP}/frameworks/av/media/liboboe
echo "LIBOBOE_DIR is ${LIBOBOE_DIR}"
OBOESERVICE_DIR=${ANDROID_BUILD_TOP}/frameworks/av/services/oboeservice
echo "OBOESERVICE_DIR is ${OBOESERVICE_DIR}"
OBOETEST_DIR=${ANDROID_BUILD_TOP}/cts/tests/tests/nativemedia/aaudio/src/
echo "OBOETEST_DIR is ${OBOETEST_DIR}"

function convertPathPattern {
    path=$1
    pattern=$2
    find $path -type f  -name $pattern -exec sed -i -f ${LIBOBOE_DIR}/scripts/oboe_to_aaudio.sed {} \;
}

function convertPath {
    path=$1
    convertPathPattern $1 '*.cpp'
    convertPathPattern $1 '*.h'
    # the mk match does not work!
    convertPathPattern $1 '*.mk'
    convertPathPattern $1 '*.md'
    convertPathPattern $1 '*.bp'
}

#convertPath ${LIBOBOE_DIR}/examples
#convertPath ${LIBOBOE_DIR}/include
#convertPath ${LIBOBOE_DIR}/src
#convertPath ${LIBOBOE_DIR}/tests
convertPath ${LIBOBOE_DIR}
convertPathPattern ${LIBOBOE_DIR} Android.mk
convertPathPattern ${LIBOBOE_DIR} liboboe.map.txt

convertPath ${OBOESERVICE_DIR}
convertPathPattern ${OBOESERVICE_DIR} Android.mk

convertPathPattern ${OBOETEST_DIR} test_aaudio.cpp

mv ${LIBOBOE_DIR}/include/oboe ${LIBOBOE_DIR}/include/aaudio
mv ${LIBOBOE_DIR}/include/aaudio/OboeAudio.h ${LIBOBOE_DIR}/include/aaudio/AAudio.h
mv ${OBOESERVICE_DIR}/OboeService.h ${OBOESERVICE_DIR}/AAudioServiceDefinitions.h
mv ${LIBOBOE_DIR}/tests/test_oboe_api.cpp ${LIBOBOE_DIR}/tests/test_aaudio_api.cpp

# Rename files with Oboe in the name.
find -name "*OboeAudioService*.cpp"      | rename -v "s/OboeAudioService/AAudioService/g"
find -name "*OboeAudioService*.h"      | rename -v "s/OboeAudioService/AAudioService/g"
find -name "*Oboe*.cpp"      | rename -v "s/Oboe/AAudio/g"
find -name "*Oboe*.h"        | rename -v "s/Oboe/AAudio/g"
