/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef SURFACE_UTILS_H_

#define SURFACE_UTILS_H_

#include <utils/Errors.h>
#include <utils/StrongPointer.h>

struct ANativeWindow;
class Surface;

namespace android {

struct HDRStaticInfo;

/**
 * Configures |nativeWindow| for given |width|x|height|, pixel |format|, |rotation| and |usage|.
 * If |reconnect| is true, reconnects to the native window before hand.
 * @return first error encountered, or NO_ERROR on success.
 */
status_t setNativeWindowSizeFormatAndUsage(
        ANativeWindow *nativeWindow /* nonnull */,
        int width, int height, int format, int rotation, int usage, bool reconnect);
void setNativeWindowHdrMetadata(
        ANativeWindow *nativeWindow /* nonnull */, HDRStaticInfo *info /* nonnull */);
status_t pushBlankBuffersToNativeWindow(ANativeWindow *nativeWindow /* nonnull */);
status_t nativeWindowConnect(ANativeWindow *surface, const char *reason);
status_t nativeWindowDisconnect(ANativeWindow *surface, const char *reason);

/**
 * Disable buffer dropping behavior of BufferQueue if target sdk of application
 * is Q or later. If the caller is not an app (e.g. MediaPlayer in mediaserver)
 * retain buffer dropping behavior.
 *
 * @return NO_ERROR
 */
status_t disableLegacyBufferDropPostQ(const sp<Surface> &surface);

} // namespace android

#endif  // SURFACE_UTILS_H_
