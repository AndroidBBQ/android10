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

#include "tuningfork/tuningfork.h"
#include "tuningfork_internal.h"
#include <jni.h>

#include <cstdlib>

namespace {

tuningfork::ProtobufSerialization ToProtobufSerialization(const CProtobufSerialization& cpbs) {
    return tuningfork::ProtobufSerialization(cpbs.bytes, cpbs.bytes + cpbs.size);
}
void ToCProtobufSerialization(const tuningfork::ProtobufSerialization& pbs,
                              CProtobufSerialization* cpbs) {
    cpbs->bytes = (uint8_t*)::malloc(pbs.size());
    memcpy(cpbs->bytes, pbs.data(), pbs.size());
    cpbs->size = pbs.size();
    cpbs->dealloc = ::free;
}

} // anonymous namespace

extern "C" {

// init must be called before any other functions
//  If no backend is passed, a debug version is used which returns empty fidelity params
//   and outputs histograms in protobuf text format to logcat.
//  If no timeProvider is passed, std::chrono::steady_clock is used.
void TuningFork_init(const CProtobufSerialization *settings, JNIEnv* env, jobject activity) {
    if(settings)
        tuningfork::Init(ToProtobufSerialization(*settings), env, activity);
}

// Blocking call to get fidelity parameters from the server.
// Returns true if parameters could be downloaded within the timeout, false otherwise.
// Note that once fidelity parameters are downloaded, any timing information is recorded
//  as being associated with those parameters.
bool TuningFork_getFidelityParameters(const CProtobufSerialization *defaultParams,
                                      CProtobufSerialization *params, size_t timeout_ms) {
    tuningfork::ProtobufSerialization defaults;
    if(defaultParams)
        defaults = ToProtobufSerialization(*defaultParams);
    tuningfork::ProtobufSerialization s;
    bool result = tuningfork::GetFidelityParameters(defaults, s, timeout_ms);
    if(result && params)
        ToCProtobufSerialization(s, params);
    return result;
}

// Protobuf serialization of the current annotation
// Returns 0 if the annotation could be set, -1 if not
int TuningFork_setCurrentAnnotation(const CProtobufSerialization *annotation) {
    if(annotation)
        // Note that SetCurrentAnnotation returns the internal annotation id if it could be set
        //  or -1 if it could not.
        if(tuningfork::SetCurrentAnnotation(ToProtobufSerialization(*annotation))==-1)
            return -1;
        else
            return 0;
    else
        return -1;
}

// Record a frame tick that will be associated with the instrumentation key and the current
//   annotation
void TuningFork_frameTick(TFInstrumentKey id) {
    tuningfork::FrameTick(id);
}

// Record a frame tick using an external time, rather than system time
void TuningFork_frameDeltaTimeNanos(TFInstrumentKey id, TFDuration dt) {
    tuningfork::FrameDeltaTimeNanos(id, std::chrono::nanoseconds(dt));
}

// Start a trace segment
TFTraceHandle TuningFork_startTrace(TFInstrumentKey key) {
    return tuningfork::StartTrace(key);
}

// Record a trace with the key and annotation set using startTrace
void TuningFork_endTrace(TFTraceHandle h) {
    tuningfork::EndTrace(h);
}

} // extern "C" {
