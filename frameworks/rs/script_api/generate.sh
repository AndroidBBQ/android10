#!/bin/bash
#
# Copyright (C) 2014 The Android Open Source Project
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
#

# TODO: $ANDROID_BUILD_TOP/prebuilts/clang/host/linux-x86/clang-stable/bin
# no longer contains clang on AOSP master. Need a stable way to reach clang
# binaries here.

if [ x"$1" == "x" ]; then
  echo "Please specify the top-level Piper client directory."
  exit 1
fi

DOC_DIR="$1/googledata/devsite/site-android/en/guide/topics/renderscript/"
if [ ! -d $DOC_DIR ]; then
  echo "Expecting top-level Piper client directory."
  exit 2
fi

cd `dirname $0`

set -e

mkdir -p test
mkdir -p include
mkdir -p docs
mkdir -p slangtest

# The order of the arguments passed to generator matter because:
# 1. The overview is expected to be in the first file.
# 2. The order specified will be the order they will show in the guide_toc.cs snippet.
#    This can be manually changed when cut&pasting the snippet into guide_toc.cs.
# 3. rsIs/Clear/SetObject is documented in rs_object_info but also found in rs_graphics.
#    The latter must appear after the former.
$ANDROID_HOST_OUT/bin/rs-api-doc-generator rs_core.spec rs_value_types.spec rs_object_types.spec rs_convert.spec rs_math.spec rs_vector_math.spec rs_matrix.spec rs_quaternion.spec rs_atomic.spec rs_time.spec rs_allocation_create.spec rs_allocation_data.spec rs_object_info.spec rs_for_each.spec rs_io.spec rs_debug.spec rs_graphics.spec

rm -f ../../../cts/tests/tests/renderscript/src/android/renderscript/cts/generated/*
mv test/* ../../../cts/tests/tests/renderscript/src/android/renderscript/cts/generated/
rmdir test

rm -f "$DOC_DIR"/reference/*.html
mv docs/*.html "$DOC_DIR"/reference

# Current API level : 24
RS_API_LEVEL=24

for ((i=11; i<=RS_API_LEVEL; i++))
  do
    mv slangtest/all$i.rs ../../compile/slang/tests/P_all_api_$i
done
rm -rf slangtest

mv RSStubsWhiteList.cpp ../../compile/libbcc/lib/

