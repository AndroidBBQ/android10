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

import android.hardware.ICamera;
import android.hardware.ICameraClient;
import android.hardware.camera2.ICameraDeviceUser;
import android.hardware.camera2.ICameraDeviceCallbacks;
import android.hardware.camera2.params.VendorTagDescriptor;
import android.hardware.camera2.params.VendorTagDescriptorCache;
import android.hardware.camera2.impl.CameraMetadataNative;
import android.hardware.ICameraServiceListener;
import android.hardware.CameraInfo;
import android.hardware.CameraStatus;

/**
 * Binder interface for the native camera service running in mediaserver.
 *
 * @hide
 */
interface ICameraService
{
    /**
     * All camera service and device Binder calls may return a
     * ServiceSpecificException with the following error codes
     */
    const int ERROR_PERMISSION_DENIED = 1;
    const int ERROR_ALREADY_EXISTS = 2;
    const int ERROR_ILLEGAL_ARGUMENT = 3;
    const int ERROR_DISCONNECTED = 4;
    const int ERROR_TIMED_OUT = 5;
    const int ERROR_DISABLED = 6;
    const int ERROR_CAMERA_IN_USE = 7;
    const int ERROR_MAX_CAMERAS_IN_USE = 8;
    const int ERROR_DEPRECATED_HAL = 9;
    const int ERROR_INVALID_OPERATION = 10;

    /**
     * Types for getNumberOfCameras
     */
    const int CAMERA_TYPE_BACKWARD_COMPATIBLE = 0;
    const int CAMERA_TYPE_ALL = 1;

    /**
     * Return the number of camera devices available in the system
     */
    int getNumberOfCameras(int type);

    /**
     * Fetch basic camera information for a camera device
     */
    CameraInfo getCameraInfo(int cameraId);

    /**
     * Default UID/PID values for non-privileged callers of
     * connect(), connectDevice(), and connectLegacy()
     */
    const int USE_CALLING_UID = -1;
    const int USE_CALLING_PID = -1;

    /**
     * Open a camera device through the old camera API
     */
    ICamera connect(ICameraClient client,
            int cameraId,
            String opPackageName,
            int clientUid, int clientPid);

    /**
     * Open a camera device through the new camera API
     * Only supported for device HAL versions >= 3.2
     */
    ICameraDeviceUser connectDevice(ICameraDeviceCallbacks callbacks,
            String cameraId,
            String opPackageName,
            int clientUid);

    /**
     * halVersion constant for connectLegacy
     */
    const int CAMERA_HAL_API_VERSION_UNSPECIFIED = -1;

    /**
     * Open a camera device in legacy mode, if supported by the camera module HAL.
     */
    ICamera connectLegacy(ICameraClient client,
            int cameraId,
            int halVersion,
            String opPackageName,
            int clientUid);

    /**
     * Add listener for changes to camera device and flashlight state.
     *
     * Also returns the set of currently-known camera IDs and state of each device.
     * Adding a listener will trigger the torch status listener to fire for all
     * devices that have a flash unit.
     */
    CameraStatus[] addListener(ICameraServiceListener listener);

    /**
     * Remove listener for changes to camera device and flashlight state.
     */
    void removeListener(ICameraServiceListener listener);

    /**
     * Read the static camera metadata for a camera device.
     * Only supported for device HAL versions >= 3.2
     */
    CameraMetadataNative getCameraCharacteristics(String cameraId);

    /**
     * Read in the vendor tag descriptors from the camera module HAL.
     * Intended to be used by the native code of CameraMetadataNative to correctly
     * interpret camera metadata with vendor tags.
     */
    VendorTagDescriptor getCameraVendorTagDescriptor();

    /**
     * Retrieve the vendor tag descriptor cache which can have multiple vendor
     * providers.
     * Intended to be used by the native code of CameraMetadataNative to correctly
     * interpret camera metadata with vendor tags.
     */
    VendorTagDescriptorCache getCameraVendorTagCache();

    /**
     * Read the legacy camera1 parameters into a String
     */
    String getLegacyParameters(int cameraId);

    /**
     * apiVersion constants for supportsCameraApi
     */
    const int API_VERSION_1 = 1;
    const int API_VERSION_2 = 2;

    // Determines if a particular API version is supported directly for a cameraId.
    boolean supportsCameraApi(String cameraId, int apiVersion);
    // Determines if a cameraId is a hidden physical camera of a logical multi-camera.
    boolean isHiddenPhysicalCamera(String cameraId);

    void setTorchMode(String cameraId, boolean enabled, IBinder clientBinder);

    /**
     * Notify the camera service of a system event.  Should only be called from system_server.
     *
     * Callers require the android.permission.CAMERA_SEND_SYSTEM_EVENTS permission.
     */
    const int EVENT_NONE = 0;
    const int EVENT_USER_SWITCHED = 1; // The argument is the set of new foreground user IDs.
    oneway void notifySystemEvent(int eventId, in int[] args);

    /**
     * Notify the camera service of a device physical status change. May only be called from
     * a privileged process.
     *
     * newState is a bitfield consisting of DEVICE_STATE_* values combined together. Valid state
     * combinations are device-specific. At device startup, the camera service will assume the device
     * state is NORMAL until otherwise notified.
     *
     * Callers require the android.permission.CAMERA_SEND_SYSTEM_EVENTS permission.
     */
    oneway void notifyDeviceStateChange(long newState);

    // Bitfield constants for notifyDeviceStateChange
    // All bits >= 32 are for custom vendor states
    // Written as ints since AIDL does not support long constants.
    const int DEVICE_STATE_NORMAL = 0;
    const int DEVICE_STATE_BACK_COVERED = 1;
    const int DEVICE_STATE_FRONT_COVERED = 2;
    const int DEVICE_STATE_FOLDED = 4;
    const int DEVICE_STATE_LAST_FRAMEWORK_BIT = 0x80000000; // 1 << 31;

}
