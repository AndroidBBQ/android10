/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef _UI_INPUT_CLASSIFIER_CONVERTER_H
#define _UI_INPUT_CLASSIFIER_CONVERTER_H

#include "InputListener.h"
#include <android/hardware/input/common/1.0/types.h>


namespace android {

/**
 * Convert from framework's NotifyMotionArgs to hidl's common::V1_0::MotionEvent
 */
::android::hardware::input::common::V1_0::MotionEvent notifyMotionArgsToHalMotionEvent(
        const NotifyMotionArgs& args);

} // namespace android

#endif // _UI_INPUT_CLASSIFIER_CONVERTER_H
