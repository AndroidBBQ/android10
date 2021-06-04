/*
 * Copyright (C) 2013 The Android Open Source Project
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

package android.hardware.camera2;

import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.impl.CameraMetadataNative;
import android.hardware.camera2.params.OutputConfiguration;
import android.hardware.camera2.params.SessionConfiguration;
import android.hardware.camera2.utils.SubmitInfo;
import android.view.Surface;

/** @hide */
interface ICameraDeviceUser
{
    void disconnect();

    const int NO_IN_FLIGHT_REPEATING_FRAMES = -1;

    SubmitInfo submitRequest(in CaptureRequest request, boolean streaming);
    SubmitInfo submitRequestList(in CaptureRequest[] requestList, boolean streaming);

    /**
     * Cancel the repeating request specified by requestId
     * Returns the frame number of the last frame that will be produced from this
     * repeating request, or NO_IN_FLIGHT_REPEATING_FRAMES if no frames were produced
     * by this repeating request.
     *
     * Repeating request may be stopped by camera device due to an error. Canceling a stopped
     * repeating request will trigger ERROR_ILLEGAL_ARGUMENT.
     */
    long cancelRequest(int requestId);

    /**
     * Begin the device configuration.
     *
     * <p>
     * beginConfigure must be called before any call to deleteStream, createStream,
     * or endConfigure.  It is not valid to call this when the device is not idle.
     * <p>
     */
    void beginConfigure();

    /**
     * The standard operating mode for a camera device; all API guarantees are in force
     */
    const int NORMAL_MODE = 0;

    /**
     * High-speed recording mode; only two outputs targeting preview and video recording may be
     * used, and requests must be batched.
     */
    const int CONSTRAINED_HIGH_SPEED_MODE = 1;

    /**
     * Start of custom vendor modes
     */
    const int VENDOR_MODE_START = 0x8000;

    /**
     * End the device configuration.
     *
     * <p>
     * endConfigure must be called after stream configuration is complete (i.e. after
     * a call to beginConfigure and subsequent createStream/deleteStream calls).  This
     * must be called before any requests can be submitted.
     * <p>
     * @param operatingMode The kind of session to create; either NORMAL_MODE or
     *     CONSTRAINED_HIGH_SPEED_MODE. Must be a non-negative value.
     * @param sessionParams Session wide camera parameters
     */
    void endConfigure(int operatingMode, in CameraMetadataNative sessionParams);

    /**
      * Check whether a particular session configuration has camera device
      * support.
      *
      * @param sessionConfiguration Specific session configuration to be verified.
      * @return true  - in case the stream combination is supported.
      *         false - in case there is no device support.
      */
    boolean isSessionConfigurationSupported(in SessionConfiguration sessionConfiguration);

    void deleteStream(int streamId);

    /**
     * Create an output stream
     *
     * <p>Create an output stream based on the given output configuration</p>
     *
     * @param outputConfiguration size, format, and other parameters for the stream
     * @return new stream ID
     */
    int createStream(in OutputConfiguration outputConfiguration);

    /**
     * Create an input stream
     *
     * <p>Create an input stream of width, height, and format</p>
     *
     * @param width Width of the input buffers
     * @param height Height of the input buffers
     * @param format Format of the input buffers. One of HAL_PIXEL_FORMAT_*.
     *
     * @return new stream ID
     */
    int createInputStream(int width, int height, int format);

    /**
     * Get the surface of the input stream.
     *
     * <p>It's valid to call this method only after a stream configuration is completed
     * successfully and the stream configuration includes a input stream.</p>
     *
     * @param surface An output argument for the surface of the input stream buffer queue.
     */
    Surface getInputSurface();

    // Keep in sync with public API in
    // frameworks/base/core/java/android/hardware/camera2/CameraDevice.java
    const int TEMPLATE_PREVIEW = 1;
    const int TEMPLATE_STILL_CAPTURE = 2;
    const int TEMPLATE_RECORD = 3;
    const int TEMPLATE_VIDEO_SNAPSHOT = 4;
    const int TEMPLATE_ZERO_SHUTTER_LAG = 5;
    const int TEMPLATE_MANUAL = 6;

    CameraMetadataNative createDefaultRequest(int templateId);

    CameraMetadataNative getCameraInfo();

    void waitUntilIdle();

    long flush();

    void prepare(int streamId);

    void tearDown(int streamId);

    void prepare2(int maxCount, int streamId);

    void updateOutputConfiguration(int streamId, in OutputConfiguration outputConfiguration);

    void finalizeOutputConfigurations(int streamId, in OutputConfiguration outputConfiguration);
}
