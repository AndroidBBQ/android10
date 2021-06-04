/*
 * Copyright 2018 The Android Open Source Project
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

#include <stdint.h>
#include <jni.h>

#define TUNINGFORK_MAJOR_VERSION 0
#define TUNINGFORK_MINOR_VERSION 1
#define TUNINGFORK_PACKED_VERSION ((TUNINGFORK_MAJOR_VERSION<<16)|(TUNINGFORK_MINOR_VERSION))

// These are reserved instrumentation keys
enum {
    TFTICK_SYSCPU = 0,
    TFTICK_SYSGPU = 1
};

typedef struct {
    uint8_t* bytes;
    size_t size;
    void (*dealloc)(void*);
} CProtobufSerialization;

// The instrumentation key identifies a tick point within a frame or a trace segment
typedef uint16_t TFInstrumentKey;
typedef uint64_t TFTraceHandle;
typedef uint64_t TFTimePoint;
typedef uint64_t TFDuration;

#ifdef __cplusplus
extern "C" {
#endif

// init must be called before any other functions
void TuningFork_init(const CProtobufSerialization *settings, JNIEnv* env, jobject activity);

// Blocking call to get fidelity parameters from the server.
// Returns true if parameters could be downloaded within the timeout, false otherwise.
// Note that once fidelity parameters are downloaded, any timing information is recorded
//  as being associated with those parameters.
// If you subsequently call GetFidelityParameters and a new set of parameters is downloaded,
// any data that is already collected will be submitted to the backend.
// Ownership of 'params' is transferred to the caller, so they must call params->dealloc
// when they are done with it.
bool TuningFork_getFidelityParameters(const CProtobufSerialization *defaultParams,
                             CProtobufSerialization *params, size_t timeout_ms);

// Protobuf serialization of the current annotation
// Returns 0 if the annotation could be set, -1 if not
int TuningFork_setCurrentAnnotation(const CProtobufSerialization *annotation);

// Record a frame tick that will be associated with the instrumentation key and the current
//   annotation
void TuningFork_frameTick(TFInstrumentKey id);

// Record a frame tick using an external time, rather than system time
void TuningFork_frameDeltaTimeNanos(TFInstrumentKey id, TFDuration dt);

// Start a trace segment
TFTraceHandle TuningFork_startTrace(TFInstrumentKey key);

// Record a trace with the key and annotation set using startTrace
void TuningFork_endTrace(TFTraceHandle h);

#ifdef __cplusplus
}
#endif
