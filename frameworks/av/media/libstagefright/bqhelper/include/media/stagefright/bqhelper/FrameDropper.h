/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef FRAME_DROPPER_H_

#define FRAME_DROPPER_H_

#include <utils/Errors.h>
#include <utils/RefBase.h>

#include <media/stagefright/foundation/ABase.h>

namespace android {

struct FrameDropper : public RefBase {
    // No frames will be dropped until a valid max frame rate is set.
    FrameDropper();

    // maxFrameRate required to be positive.
    status_t setMaxFrameRate(float maxFrameRate);

    // Returns false if max frame rate has not been set via setMaxFrameRate.
    bool shouldDrop(int64_t timeUs);

    // Returns true if all frame drop logic should be disabled.
    bool disabled() { return (mMinIntervalUs == -1ll); }

protected:
    virtual ~FrameDropper();

private:
    int64_t mDesiredMinTimeUs;
    int64_t mMinIntervalUs;

    DISALLOW_EVIL_CONSTRUCTORS(FrameDropper);
};

}  // namespace android

#endif  // FRAME_DROPPER_H_
