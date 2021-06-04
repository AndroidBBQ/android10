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

package android.hardware;

/** @hide */
interface ICameraServiceListener
{

    /**
     * Initial status will be transmitted with onStatusChange immediately
     * after this listener is added to the service listener list.
     *
     * Allowed transitions:
     *
     *     (Any)               -> NOT_PRESENT
     *     NOT_PRESENT         -> PRESENT
     *     NOT_PRESENT         -> ENUMERATING
     *     ENUMERATING         -> PRESENT
     *     PRESENT             -> NOT_AVAILABLE
     *     NOT_AVAILABLE       -> PRESENT
     *
     * A state will never immediately transition back to itself.
     *
     * The enums must match the values in
     * include/hardware/camera_common.h when applicable
     */
    // Device physically unplugged
    const int STATUS_NOT_PRESENT      = 0;
    // Device physically has been plugged in and the camera can be used exclusively
    const int STATUS_PRESENT          = 1;
    // Device physically has been plugged in but it will not be connect-able until enumeration is
    // complete
    const int STATUS_ENUMERATING      = 2;
    // Camera is in use by another app and cannot be used exclusively
    const int STATUS_NOT_AVAILABLE    = -2;

    // Use to initialize variables only
    const int STATUS_UNKNOWN          = -1;

    oneway void onStatusChanged(int status, String cameraId);

    /**
     * The torch mode status of a camera.
     *
     * Initial status will be transmitted with onTorchStatusChanged immediately
     * after this listener is added to the service listener list.
     *
     * The enums must match the values in
     * include/hardware/camera_common.h
     */
    // The camera's torch mode has become not available to use via
    // setTorchMode().
    const int TORCH_STATUS_NOT_AVAILABLE = 0;
    // The camera's torch mode is off and available to be turned on via
    // setTorchMode().
    const int TORCH_STATUS_AVAILABLE_OFF = 1;
    // The camera's torch mode is on and available to be turned off via
    // setTorchMode().
    const int TORCH_STATUS_AVAILABLE_ON  = 2;

    // Use to initialize variables only
    const int TORCH_STATUS_UNKNOWN = -1;

    oneway void onTorchStatusChanged(int status, String cameraId);

    /**
     * Notify registered clients about camera access priority changes.
     * Clients which were previously unable to open a certain camera device
     * can retry after receiving this callback.
     */
    oneway void onCameraAccessPrioritiesChanged();

    /**
     * Notify registered clients about cameras being opened/closed.
     * Only clients with android.permission.CAMERA_OPEN_CLOSE_LISTENER permission
     * will receive such callbacks.
     */
    oneway void onCameraOpened(String cameraId, String clientPackageId);
    oneway void onCameraClosed(String cameraId);
}
