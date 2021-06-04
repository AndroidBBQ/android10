# Copyright 2016, The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!/bin/bash

ME=$0

function usage {
  echo >&2 "$ME: $*: Expected [-d|--dump] [-t|--trace] <SPIRV_TOOLS_PATH> <SCRIPT_NAME> <OUTPUT_DIR>)"
  exit 2
}

function dump {
  if [[ -z "${DUMP:-}" ]] ; then
    return 0
  fi
  eval rs2spirv "$output_folder/$script.spv" -print-as-words
  return $?
}

DUMP=
TRACE=

while [[ "${1:-}" = -* ]] ; do
  case "$1" in
    -d|--dump)
      DUMP=t
      ;;
    -t|--trace)
      TRACE=t
      ;;
    *)
      usage "Unexpected option \"$1\""
      ;;
  esac
  shift
done

if [[ $# -ne 3 ]] ; then
  usage "Bad argument count (got $#)"
fi

if [[ -n "${TRACE:-}" ]] ; then
  set -x
fi

AND_HOME=$ANDROID_BUILD_TOP
SPIRV_TOOLS_PATH=$1

script_name="$2"
script=`basename ${2%.*}` # Remove enclosing directories and extension.

output_folder="$3"
mkdir -p $output_folder

eval llvm-rs-cc -o "$output_folder" -S -emit-llvm -Wall -Werror -target-api 24 \
  -I "$AND_HOME/external/clang/lib/Headers" -I "$AND_HOME/frameworks/rs/script_api/include" \
  "$script_name" &&
eval llvm-as "$output_folder/bc32/$script.ll" -o "$output_folder/$script.bc" &&
eval rs2spirv "$output_folder/$script.bc" -o "$output_folder/$script.spv" &&
dump &&
eval "$SPIRV_TOOLS_PATH/spirv-val" "$output_folder/$script.spv" &&

exit $?
