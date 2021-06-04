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

#include "tuningfork/tuningfork.h"
#include "tuningfork/tuningfork_extra.h"

#ifdef PROTOBUF_NANO
#include "pb_encode.h"
#include "pb_decode.h"
#endif

#include <stdint.h>
#include <string>
#include <chrono>
#include <vector>
#include <jni.h>

class AAsset;

namespace tuningfork {

typedef std::vector<uint8_t> ProtobufSerialization;

// The instrumentation key identifies a tick point within a frame or a trace segment
typedef uint16_t InstrumentationKey;
typedef uint64_t TraceHandle;
typedef std::chrono::steady_clock::time_point TimePoint;
typedef std::chrono::steady_clock::duration Duration;

struct Settings {
    struct AggregationStrategy {
        enum Submission {
            TICK_BASED,
            TIME_BASED
        };
        Submission method;
        int32_t intervalms_or_count;
        int32_t max_instrumentation_keys;
        std::vector<int> annotation_enum_size;
    };
    struct Histogram {
        int32_t instrument_key;
        float bucket_min;
        float bucket_max;
        int32_t n_buckets;
    };
    AggregationStrategy aggregation_strategy;
    std::vector<Histogram> histograms;
};

// Extra information that is uploaded with the ClearCut proto.
struct ExtraUploadInfo {
    std::string experiment_id;
    std::string session_id;
    uint64_t total_memory_bytes;
    uint32_t gl_es_version;
    std::string build_fingerprint;
    std::string build_version_sdk;
    std::vector<uint64_t> cpu_max_freq_hz;
    std::string apk_package_name;
    int apk_version_code;
    int tuningfork_version;
};

class Backend {
public:
    virtual ~Backend() {};
    virtual bool Process(const ProtobufSerialization &tuningfork_log_event) = 0;
};

class ParamsLoader {
public:
    virtual ~ParamsLoader() {};
    virtual bool GetFidelityParams(ProtobufSerialization &fidelity_params, size_t timeout_ms) {
        return false;
    }
};

class ProtoPrint {
public:
    virtual ~ProtoPrint() {};
    virtual void Print(const ProtobufSerialization &tuningfork_log_event);
};

class DebugBackend : public Backend {
public:
    ~DebugBackend() override;
    bool Process(const ProtobufSerialization &tuningfork_log_event) override;
};

// You can provide your own time source rather than steady_clock by inheriting this and passing
//   it to init.
class ITimeProvider {
public:
    virtual std::chrono::steady_clock::time_point NowNs() = 0;
};

// init must be called before any other functions
// If no backend is passed, a debug version is used which returns empty fidelity params
// and outputs histograms in protobuf text format to logcat.
// If no timeProvider is passed, std::chrono::steady_clock is used.
void Init(const ProtobufSerialization &settings, const ExtraUploadInfo& extra_info,
          Backend *backend = 0, ParamsLoader *loader = 0, ITimeProvider *time_provider = 0);

// Init must be called before any other functions
void Init(const ProtobufSerialization &settings, JNIEnv* env, jobject activity);

// Blocking call to get fidelity parameters from the server.
// Returns true if parameters could be downloaded within the timeout, false otherwise.
// Note that once fidelity parameters are downloaded, any timing information is recorded
//  as being associated with those parameters.
// If you subsequently call GetFidelityParameters, any data that is already collected will be
// submitted to the backend.
bool GetFidelityParameters(const ProtobufSerialization& defaultParams,
                           ProtobufSerialization &params, size_t timeout_ms);

// Protobuf serialization of the current annotation
// Returns the internal annotation id if it was set or -1 if not
uint64_t SetCurrentAnnotation(const ProtobufSerialization &annotation);

// Record a frame tick that will be associated with the instrumentation key and the current
//   annotation
void FrameTick(InstrumentationKey id);

// Record a frame tick using an external time, rather than system time
void FrameDeltaTimeNanos(InstrumentationKey id, Duration dt);

// Start a trace segment
TraceHandle StartTrace(InstrumentationKey key);

// Record a trace with the key and annotation set using startTrace
void EndTrace(TraceHandle h);

void SetUploadCallback(ProtoCallback cbk);

} // namespace tuningfork
