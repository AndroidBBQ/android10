/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RS_RUNTIME_RS_F16_UTIL
#define RS_RUNTIME_RS_F16_UTIL

typedef union {
  half hval;
  short sval;
} fp16_shape_type;

/* half h = unsigned short s; */
#define SET_HALF_WORD(h, s) \
do {                        \
  fp16_shape_type fp16_u;   \
  fp16_u.sval = (s);        \
  (h) = fp16_u.hval;        \
} while (0)

/* unsigned short s = half h; */
#define GET_HALF_WORD(s, h) \
do {                        \
  fp16_shape_type fp16_u;   \
  fp16_u.hval = (h);        \
  (s) = fp16_u.sval;        \
} while (0)

#endif // RS_RUNTIME_RS_F16_UTIL
