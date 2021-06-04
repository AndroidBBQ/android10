#
# Copyright (C) 2015 The Android Open Source Project
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

header:
summary: Overview
description:
 RenderScript is a high-performance runtime that provides compute operations at the native level.
 RenderScript code is compiled on devices at runtime to allow platform-independence as well.

 This reference documentation describes the RenderScript runtime APIs, which you can utilize
 to write RenderScript code in C99. The RenderScript compute header files are automatically
 included for you.

 To use RenderScript, you need to utilize the RenderScript runtime APIs documented here as well
 as the Android framework APIs for RenderScript.  For documentation on the Android framework
 APIs, see the <a target="_parent" href="http://developer.android.com/reference/android/renderscript/package-summary.html">android.renderscript</a> package reference.

 For more information on how to develop with RenderScript and how the runtime and Android
 framework APIs interact, see the <a target="_parent" href="http://developer.android.com/guide/topics/renderscript/index.html">RenderScript developer guide</a> and the <a target="_parent" href="http://developer.android.com/resources/samples/RenderScript/index.html">RenderScript samples</a>.
include:
 #define RS_KERNEL __attribute__((kernel))

 #include "stdbool.h"

 #include "rs_value_types.rsh"
 #include "rs_object_types.rsh"

 #include "rs_allocation_create.rsh"
 #include "rs_allocation_data.rsh"
 #include "rs_atomic.rsh"
 #include "rs_convert.rsh"
 #include "rs_debug.rsh"
 #include "rs_for_each.rsh"
 #include "rs_io.rsh"
 #include "rs_math.rsh"
 #include "rs_matrix.rsh"
 #include "rs_object_info.rsh"
 #include "rs_quaternion.rsh"
 #include "rs_time.rsh"
 #include "rs_vector_math.rsh"
end:
