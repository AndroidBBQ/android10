/*
 * Copyright 2018, The Android Open Source Project
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

#ifndef MEDIA_C2_VIDEO_HIDL_TEST_COMMON_H
#define MEDIA_C2_VIDEO_HIDL_TEST_COMMON_H

#define ENCODER_TIMESTAMP_INCREMENT 40000
#define ENC_NUM_FRAMES 32
#define ENC_DEFAULT_FRAME_WIDTH 352
#define ENC_DEFAULT_FRAME_HEIGHT 288
#define MAX_ITERATIONS 128

#define ALIGN(_sz, _align) ((_sz + (_align - 1)) & ~(_align - 1))

/*
 * Common video utils
 */


#endif  // MEDIA_C2_VIDEO_HIDL_TEST_COMMON_H
