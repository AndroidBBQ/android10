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

package android.hardware.camera2;

import android.hardware.camera2.impl.CameraMetadataNative;
import android.hardware.camera2.impl.CaptureResultExtras;
import android.hardware.camera2.impl.PhysicalCaptureResultInfo;

/** @hide */
interface ICameraDeviceCallbacks
{
    // Error codes for onDeviceError
    const int ERROR_CAMERA_INVALID_ERROR = -1; // To indicate all invalid error codes
    const int ERROR_CAMERA_DISCONNECTED = 0;
    const int ERROR_CAMERA_DEVICE = 1;
    const int ERROR_CAMERA_SERVICE = 2;
    const int ERROR_CAMERA_REQUEST = 3;
    const int ERROR_CAMERA_RESULT = 4;
    const int ERROR_CAMERA_BUFFER = 5;
    const int ERROR_CAMERA_DISABLED = 6;

    oneway void onDeviceError(int errorCode, in CaptureResultExtras resultExtras);
    oneway void onDeviceIdle();
    oneway void onCaptureStarted(in CaptureResultExtras resultExtras, long timestamp);
    oneway void onResultReceived(in CameraMetadataNative result,
                                 in CaptureResultExtras resultExtras,
                                 in PhysicalCaptureResultInfo[] physicalCaptureResultInfos);
    oneway void onPrepared(int streamId);

    /**
     * Repeating request encountered an error and was stopped.
     *
     * @param lastFrameNumber Frame number of the last frame of the streaming request.
     * @param repeatingRequestId the ID of the repeating request being stopped
     */
    oneway void onRepeatingRequestError(in long lastFrameNumber,
                                        in int repeatingRequestId);
    oneway void onRequestQueueEmpty();
}
