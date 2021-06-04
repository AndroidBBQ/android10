/*
 * Copyright 2019 The Android Open Source Project
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

#pragma once

#include <gmock/gmock.h>
#include <system/window.h>
#include <ui/ANativeObjectBase.h>
#include <ui/GraphicTypes.h>
#include <ui/PixelFormat.h>

namespace android::compositionengine::mock {

/* ------------------------------------------------------------------------
 * Mock NativeWindow
 *
 * An intentionally simplified Mock which implements a minimal subset of the full
 * ANativeWindow interface.
 */
class NativeWindow : public ANativeObjectBase<ANativeWindow, NativeWindow, RefBase> {
public:
    NativeWindow();
    ~NativeWindow();

    MOCK_METHOD1(setSwapInterval, int(int));
    MOCK_METHOD2(dequeueBuffer, int(struct ANativeWindowBuffer**, int*));
    MOCK_METHOD2(cancelBuffer, int(struct ANativeWindowBuffer*, int));
    MOCK_METHOD2(queueBuffer, int(struct ANativeWindowBuffer*, int));
    MOCK_CONST_METHOD2(query, int(int, int*));
    MOCK_METHOD1(connect, int(int));
    MOCK_METHOD1(disconnect, int(int));
    MOCK_METHOD1(lockBuffer_DEPRECATED, int(struct ANativeWindowBuffer*));
    MOCK_METHOD1(setBuffersFormat, int(PixelFormat));
    MOCK_METHOD1(setBuffersDataSpace, int(ui::Dataspace));
    MOCK_METHOD1(setUsage, int(uint64_t));
};

} // namespace android::compositionengine::mock
