#!/usr/bin/env bash

# Copyright (C) 2018 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if [ -z $ANDROID_BUILD_TOP ]; then
  echo "You need to source and lunch before you can use this script"
  exit 1
fi

set -ex

function run_libbinder_ndk_test() {
    adb shell /data/nativetest64/libbinder_ndk_test_server/libbinder_ndk_test_server &

    # avoid getService 1s delay for most runs, non-critical
    sleep 0.1

    adb shell /data/nativetest64/libbinder_ndk_test_client/libbinder_ndk_test_client; \
        adb shell killall libbinder_ndk_test_server
}

[ "$1" != "--skip-build" ] && $ANDROID_BUILD_TOP/build/soong/soong_ui.bash --make-mode \
    MODULES-IN-frameworks-native-libs-binder-ndk

adb root
adb wait-for-device
adb sync data

# very simple unit tests, tests things outside of the NDK as well
run_libbinder_ndk_test

# CTS tests (much more comprehensive, new tests should ideally go here)
atest android.binder.cts
