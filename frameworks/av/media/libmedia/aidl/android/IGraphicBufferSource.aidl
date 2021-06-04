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

package android;

import android.IOMXNode;

/**
 * Binder interface for controlling a graphic buffer source.
 *
 * @hide
 */
interface IGraphicBufferSource {
    void configure(IOMXNode omxNode, int dataSpace);
    void setSuspend(boolean suspend, long suspendTimeUs);
    void setRepeatPreviousFrameDelayUs(long repeatAfterUs);
    void setMaxFps(float maxFps);
    void setTimeLapseConfig(double fps, double captureFps);
    void setStartTimeUs(long startTimeUs);
    void setStopTimeUs(long stopTimeUs);
    long getStopTimeOffsetUs();
    void setColorAspects(int aspects);
    void setTimeOffsetUs(long timeOffsetsUs);
    void signalEndOfInputStream();
}
