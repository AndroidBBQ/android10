#!/usr/bin/env python
#
# Copyright 2015 The Android Open Source Project
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

# This script is for converting the #defines in the gl2/3.h into the Java
# form used in the GLES2/3Header.java-if stub files that are then used by
# the code generator. Provide input with stdin and receive output on stdout.

import sys

allDefines = []
maxLen = 0

for line in sys.stdin:
    defineValuePair = line.strip().split()[1:]
    maxLen = max(maxLen, len(defineValuePair[0]))
    allDefines.append(defineValuePair)
for define in sorted(allDefines, key=lambda define: define[1]):
    print('    public static final int {0[0]:<{1}} = {0[1]};'.format(define, maxLen))
