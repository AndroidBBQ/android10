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

#include "tuningfork.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*VoidCallback)();
typedef void (*ProtoCallback)(const CProtobufSerialization*);

// Load settings from assets/tuningfork/tuningfork_settings.bin.
// Returns true and fills 'settings' if the file could be loaded.
// Ownership of settings is passed to the caller: call
//  CProtobufSerialization_Free to deallocate any memory.
// Returns false if the file was not found or there was an error.
bool TuningFork_findSettingsInAPK(JNIEnv* env, jobject activity,
                                  CProtobufSerialization* settings);

// Load fidelity params from assets/tuningfork/dev_tuningfork_fidelityparams_#.bin.
// Call once with fps=NULL to get the number of files in fp_count.
// The call a second time with a pre-allocated array of size fp_count in fps.
// Ownership of serializations is passed to the caller: call
//  CProtobufSerialization_Free to deallocate any memory.
void TuningFork_findFidelityParamsInAPK(JNIEnv* env, jobject activity,
                                        CProtobufSerialization* fps,
                                        int* fp_count);

// Initialize tuning fork and automatically inject tracers into Swappy.
// If Swappy is not available or could not be initialized, the function returns
//  false.
// When using Swappy, there will be 2 default tick points added and the
//  histogram settings need to be coordinated with your swap rate.
// If you know the shared library in which Swappy is, pass it in libraryName.
// If libraryName is NULL or TuningFork cannot find Swappy in the library, the
//  function  will look in the current module and then try in order:
//  [libgamesdk.so, libswappy.so, libunity.so]
// frame_callback is called once per frame: you can set any Annotations
//  during this callback if you wish.
bool TuningFork_initWithSwappy(const CProtobufSerialization* settings,
                               JNIEnv* env, jobject activity,
                               const char* libraryName,
                               VoidCallback frame_callback);

// Set a callback to be called on a separate thread every time TuningFork
//  performs an upload.
void TuningFork_setUploadCallback(ProtoCallback cbk);

// This function calls initWithSwappy and also performs the following:
// 1) Settings and default fidelity params are retrieved from the APK.
// 2) A download thread is activated to retrieve fideloty params and retries are
//    performed until a download is successful or a timeout occurs.
// 3) Downloaded params are stored locally and used in preference of default
//    params when the app is started in future.
// fpDefaultFileNum is the index of the dev_tuningfork_fidelityparams_#.bin file you
//  wish to use when there is no download connection and no saved params. It has a
//  special meaning if it is negative: in this case, saved params are reset to
//  dev_tuningfork_fidelity_params_(-$fpDefaultFileNum).bin
// fidelity_params_callback is called with any downloaded params or with default /
//  saved params.
// initialTimeoutMs is the time to wait for an initial download. The fidelity_params_callback
//  will be called after this time with the default / saved params if no params
//  could be downloaded..
// ultimateTimeoutMs is the time after which to stop retrying the download.
// The following error codes may be returned:
enum TFErrorCode {
    TFERROR_OK = 0, // No error
    TFERROR_NO_SETTINGS = 1, // No tuningfork_settings.bin found in assets/tuningfork.
    TFERROR_NO_SWAPPY = 2, // Not able to find Swappy.
    TFERROR_INVALID_DEFAULT_FIDELITY_PARAMS = 3, // fpDefaultFileNum is out of range.
    TFERROR_NO_FIDELITY_PARAMS = 4 // No dev_tuningfork_fidelityparams_#.bin found
                                 //  in assets/tuningfork.
};
TFErrorCode TuningFork_initFromAssetsWithSwappy(JNIEnv* env, jobject activity,
                             const char* libraryName,
                             VoidCallback frame_callback,
                             int fpDefaultFileNum,
                             ProtoCallback fidelity_params_callback,
                             int initialTimeoutMs, int ultimateTimeoutMs);

#ifdef __cplusplus
}
#endif
