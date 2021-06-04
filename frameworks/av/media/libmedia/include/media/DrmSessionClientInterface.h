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

#ifndef DRM_PROXY_INTERFACE_H_
#define DRM_PROXY_INTERFACE_H_

#include <utils/RefBase.h>
#include <utils/Vector.h>

namespace android {

struct DrmSessionClientInterface : public RefBase {
    virtual bool reclaimSession(const Vector<uint8_t>& sessionId) = 0;

protected:
    virtual ~DrmSessionClientInterface() {}
};

}  // namespace android

#endif  // DRM_PROXY_INTERFACE_H_
