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

#include "tuningfork_internal.h"

#include <cinttypes>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <sstream>

#define LOG_TAG "TuningFork"
#include "Log.h"
#include "pb_decode.h"
#include "Trace.h"

#include "histogram.h"
#include "prong.h"
#include "uploadthread.h"
#include "clearcutserializer.h"
#include "tuningfork/protobuf_nano_util.h"
#include "clearcut_backend.h"
#include "annotation_util.h"

using PBSettings = com_google_tuningfork_Settings;

/* Annotations come into tuning fork as a serialized protobuf. The protobuf can only have
 * enums in it. We form an integer annotation id from the annotation interpreted as a mixed-radix
 * number. E.g. say we have the following in the proto:
 * enum A { A_1 = 1, A_2 = 2, A_3 = 3};
 * enum B { B_1 = 1, B_2 = 2};
 * enum C { C_1 = 1};
 * extend Annotation { optional A a = 1; optional B b = 2; optional C c = 3};
 * Then a serialization might be:
 * 0x16 0x01
 * Here, 'a' and 'c' are missing and 'b' has the value B_1. Note the shift of 3 bits for the key.
 * https://developers.google.com/protocol-buffers/docs/encoding
 *
 * Assume we have 2 possible instrumentation keys: NUM_IKEY = 2
 *
 * The annotation id then is (0 + 1*4 + 0)*NUM_IKEY = 8, where the factor of 4 comes from the radix
 * associated with 'a', i.e. 3 values for enum A + option missing
 *
 * A compound id is formed from the annotation id and the instrument key:
 * compound_id = annotation_id + instrument_key;
 *
 * So for instrument key 1, the compound_id with the above annotation is 9
 *
 * This compound_id is used to look up a histogram in the ProngCache.
 *
 * annotation_radix_mult_ stores the multiplied radixes, so for the above, it is:
 * [4 4*3 4*3*2] = [4 12 24]
 * and the maximum number of annotations is 24
 *
 * */

namespace tuningfork {

typedef uint64_t AnnotationId;

class MonoTimeProvider : public ITimeProvider {
public:
    virtual TimePoint NowNs() {
        return std::chrono::steady_clock::now();
    }
};

std::unique_ptr<MonoTimeProvider> s_mono_time_provider = std::make_unique<MonoTimeProvider>();

class TuningForkImpl {
private:
    Settings settings_;
    std::unique_ptr<ProngCache> prong_caches_[2];
    ProngCache *current_prong_cache_;
    TimePoint last_submit_time_ns_;
    std::unique_ptr<gamesdk::Trace> trace_;
    std::vector<TimePoint> live_traces_;
    Backend *backend_;
    ParamsLoader *loader_;
    UploadThread upload_thread_;
    SerializedAnnotation current_annotation_;
    std::vector<int> annotation_radix_mult_;
    AnnotationId current_annotation_id_;
    ITimeProvider *time_provider_;
public:
    TuningForkImpl(const Settings& settings,
                   const ExtraUploadInfo& extra_upload_info,
                   Backend *backend,
                   ParamsLoader *loader,
                   ITimeProvider *time_provider) : settings_(settings),
                                                    trace_(gamesdk::Trace::create()),
                                                    backend_(backend),
                                                    loader_(loader),
                                                    upload_thread_(backend, extra_upload_info),
                                                    current_annotation_id_(0),
                                                    time_provider_(time_provider) {
        if (time_provider_ == nullptr) {
            time_provider_ = s_mono_time_provider.get();
        }
        last_submit_time_ns_ = time_provider_->NowNs();

        InitHistogramSettings();
        InitAnnotationRadixes();

        size_t max_num_prongs_ = 0;
        int max_ikeys = settings.aggregation_strategy.max_instrumentation_keys;

        if (annotation_radix_mult_.size() == 0 || max_ikeys == 0)
            ALOGE("Neither max_annotations nor max_instrumentation_keys can be zero");
        else
            max_num_prongs_ = max_ikeys * annotation_radix_mult_.back();
        auto serializeId = [this](uint64_t id) { return SerializeAnnotationId(id); };
        prong_caches_[0] = std::make_unique<ProngCache>(max_num_prongs_, max_ikeys,
                                                        settings_.histograms, serializeId);
        prong_caches_[1] = std::make_unique<ProngCache>(max_num_prongs_, max_ikeys,
                                                        settings_.histograms, serializeId);
        current_prong_cache_ = prong_caches_[0].get();
        live_traces_.resize(max_num_prongs_);
        for (auto &t: live_traces_) t = TimePoint::min();

        ALOGI("TuningFork initialized");
    }

    ~TuningForkImpl() {
    }

    void InitHistogramSettings();

    void InitAnnotationRadixes();

    // Returns true if the fidelity params were retrieved
    bool GetFidelityParameters(const ProtobufSerialization& defaultParams,
                               ProtobufSerialization &fidelityParams, size_t timeout_ms);

    // Returns the set annotation id or -1 if it could not be set
    uint64_t SetCurrentAnnotation(const ProtobufSerialization &annotation);

    void FrameTick(InstrumentationKey id);

    void FrameDeltaTimeNanos(InstrumentationKey id, Duration dt);

    // Returns the handle to be used by EndTrace
    TraceHandle StartTrace(InstrumentationKey key);

    void EndTrace(TraceHandle);

    void SetUploadCallback(void(*cbk)(const CProtobufSerialization*));

private:
    Prong *TickNanos(uint64_t compound_id, TimePoint t);

    Prong *TraceNanos(uint64_t compound_id, Duration dt);

    void CheckForSubmit(TimePoint t_ns, Prong *prong);

    bool ShouldSubmit(TimePoint t_ns, Prong *prong);

    AnnotationId DecodeAnnotationSerialization(const SerializedAnnotation &ser);

    int GetInstrumentationKey(uint64_t compoundId) {
        return compoundId % settings_.aggregation_strategy.max_instrumentation_keys;
    }

    uint64_t MakeCompoundId(InstrumentationKey k, AnnotationId a) {
        return k + a;
    }

    SerializedAnnotation SerializeAnnotationId(uint64_t);
};

std::unique_ptr<TuningForkImpl> s_impl;

bool decodeAnnotationEnumSizes(pb_istream_t* stream, const pb_field_t *field, void** arg) {
    Settings* settings = static_cast<Settings*>(*arg);
    uint64_t a;
    pb_decode_varint(stream, &a);
    settings->aggregation_strategy.annotation_enum_size.push_back(a);
    return true;
}
bool decodeHistograms(pb_istream_t* stream, const pb_field_t *field, void** arg) {
    Settings* settings = static_cast<Settings*>(*arg);
    com_google_tuningfork_Settings_Histogram hist;
    pb_decode(stream, com_google_tuningfork_Settings_Histogram_fields, &hist);
    settings->histograms.push_back({hist.instrument_key, hist.bucket_min, hist.bucket_max,
                                    hist.n_buckets});
    return true;
}

void Init(const ProtobufSerialization &settings_ser,
          const ExtraUploadInfo& extra_upload_info,
          Backend *backend,
          ParamsLoader *loader,
          ITimeProvider *time_provider) {
    Settings settings;
    PBSettings pbsettings = com_google_tuningfork_Settings_init_zero;
    pbsettings.aggregation_strategy.annotation_enum_size.funcs.decode = decodeAnnotationEnumSizes;
    pbsettings.aggregation_strategy.annotation_enum_size.arg = &settings;
    pbsettings.histograms.funcs.decode = decodeHistograms;
    pbsettings.histograms.arg = &settings;
    VectorStream str {const_cast<ProtobufSerialization*>(&settings_ser), 0};
    pb_istream_t stream = {VectorStream::Read, &str, settings_ser.size()};
    pb_decode(&stream, com_google_tuningfork_Settings_fields, &pbsettings);
    if(pbsettings.aggregation_strategy.method
          ==com_google_tuningfork_Settings_AggregationStrategy_Submission_TICK_BASED)
        settings.aggregation_strategy.method = Settings::AggregationStrategy::TICK_BASED;
    else
        settings.aggregation_strategy.method = Settings::AggregationStrategy::TIME_BASED;
    settings.aggregation_strategy.intervalms_or_count
      = pbsettings.aggregation_strategy.intervalms_or_count;
    settings.aggregation_strategy.max_instrumentation_keys
      = pbsettings.aggregation_strategy.max_instrumentation_keys;
    s_impl = std::make_unique<TuningForkImpl>(settings, extra_upload_info, backend, loader,
                                              time_provider);
}

ClearcutBackend sBackend;
ProtoPrint sProtoPrint;
ParamsLoader sLoader;

void Init(const ProtobufSerialization &settings_ser, JNIEnv* env, jobject activity) {
    bool backendInited = sBackend.Init(env, activity, &sProtoPrint);

    ExtraUploadInfo extra_upload_info = UploadThread::GetExtraUploadInfo(env, activity);
    if(backendInited) {
        ALOGV("TuningFork.Clearcut: OK");
        Init(settings_ser, extra_upload_info, &sBackend, &sLoader);
    }
    else {
        ALOGV("TuningFork.Clearcut: FAILED");
        Init(settings_ser, extra_upload_info);
    }
}

bool GetFidelityParameters(const ProtobufSerialization &defaultParams,
                           ProtobufSerialization &params, size_t timeout_ms) {
    if (!s_impl) {
        ALOGE("Failed to get TuningFork instance");
        return false;
    } else
        return s_impl->GetFidelityParameters(defaultParams, params, timeout_ms);
}

void FrameTick(InstrumentationKey id) {
    if (!s_impl) {
        ALOGE("Failed to get TuningFork instance");
    } else {
        s_impl->FrameTick(id);
    }
}

void FrameDeltaTimeNanos(InstrumentationKey id, Duration dt) {
    if (!s_impl) {
        ALOGE("Failed to get TuningFork instance");
    } else s_impl->FrameDeltaTimeNanos(id, dt);
}

TraceHandle StartTrace(InstrumentationKey key) {
    if (!s_impl) {
        ALOGE("Failed to get TuningFork instance");
        return 0;
    } else return s_impl->StartTrace(key);
}

void EndTrace(TraceHandle h) {
    if (!s_impl) {
        ALOGE("Failed to get TuningFork instance");
    } else
        s_impl->EndTrace(h);
}

// Return the set annotation id or -1 if it could not be set
uint64_t SetCurrentAnnotation(const ProtobufSerialization &ann) {
    if (!s_impl) {
        ALOGE("Failed to get TuningFork instance");
        return annotation_util::kAnnotationError;
    } else
        return s_impl->SetCurrentAnnotation(ann);
}

void SetUploadCallback(void(*cbk)(const CProtobufSerialization*)) {
    if (!s_impl) {
        ALOGE("Failed to get TuningFork instance");
    } else {
        s_impl->SetUploadCallback(cbk);
    }
}

// Return the set annotation id or -1 if it could not be set
uint64_t TuningForkImpl::SetCurrentAnnotation(const ProtobufSerialization &annotation) {
    current_annotation_ = annotation;
    auto id = DecodeAnnotationSerialization(annotation);
    if (id == annotation_util::kAnnotationError) {
        ALOGW("Error setting annotation of size %zu", annotation.size());
        current_annotation_id_ = 0;
        return annotation_util::kAnnotationError;
    }
    else {
        ALOGV("Set annotation id to %" PRIu64, id);
        current_annotation_id_ = id;
        return current_annotation_id_;
    }
}

AnnotationId TuningForkImpl::DecodeAnnotationSerialization(const SerializedAnnotation &ser) {
    auto id = annotation_util::DecodeAnnotationSerialization(ser, annotation_radix_mult_);
     // Shift over to leave room for the instrument id
    return id * settings_.aggregation_strategy.max_instrumentation_keys;
}

SerializedAnnotation TuningForkImpl::SerializeAnnotationId(AnnotationId id) {
    SerializedAnnotation ann;
    AnnotationId a = id / settings_.aggregation_strategy.max_instrumentation_keys;
    annotation_util::SerializeAnnotationId(a, ann, annotation_radix_mult_);
    return ann;
}

bool TuningForkImpl::GetFidelityParameters(const ProtobufSerialization& defaultParams,
                                           ProtobufSerialization &params_ser, size_t timeout_ms) {
    if(loader_) {
        auto result = loader_->GetFidelityParams(params_ser, timeout_ms);
        if (result) {
            upload_thread_.SetCurrentFidelityParams(params_ser);
        } else {
            upload_thread_.SetCurrentFidelityParams(defaultParams);
        }
        return result;
    }
    else
        return false;
}

TraceHandle TuningForkImpl::StartTrace(InstrumentationKey key) {
    trace_->beginSection("TFTrace");
    uint64_t h = MakeCompoundId(key, current_annotation_id_);
    live_traces_[h] = time_provider_->NowNs();
    return h;
}

void TuningForkImpl::EndTrace(TraceHandle h) {
    trace_->endSection();
    auto i = live_traces_[h];
    if (i != TimePoint::min())
        TraceNanos(h, time_provider_->NowNs() - i);
    live_traces_[h] = TimePoint::min();
}

void TuningForkImpl::FrameTick(InstrumentationKey key) {
    trace_->beginSection("TFTick");
    auto t = time_provider_->NowNs();
    auto compound_id = MakeCompoundId(key, current_annotation_id_);
    auto p = TickNanos(compound_id, t);
    if (p)
        CheckForSubmit(t, p);

    trace_->endSection();
}

void TuningForkImpl::FrameDeltaTimeNanos(InstrumentationKey key, Duration dt) {

    auto compound_d = MakeCompoundId(key, current_annotation_id_);
    auto p = TraceNanos(compound_d, dt);
    if (p)
        CheckForSubmit(time_provider_->NowNs(), p);
}

Prong *TuningForkImpl::TickNanos(uint64_t compound_id, TimePoint t) {
    // Find the appropriate histogram and add this time
    Prong *p = current_prong_cache_->Get(compound_id);
    if (p)
        p->Tick(t);
    else
       ALOGW("Bad id or limit of number of prongs reached");
    return p;
}

Prong *TuningForkImpl::TraceNanos(uint64_t compound_id, Duration dt) {
    // Find the appropriate histogram and add this time
    Prong *h = current_prong_cache_->Get(compound_id);
    if (h)
        h->Trace(dt);
    else
        ALOGW("Bad id or limit of number of prongs reached");
    return h;
}

void TuningForkImpl::SetUploadCallback(void(*cbk)(const CProtobufSerialization*)) {
    upload_thread_.SetUploadCallback(cbk);
}


bool TuningForkImpl::ShouldSubmit(TimePoint t_ns, Prong *prong) {
    auto method = settings_.aggregation_strategy.method;
    auto count = settings_.aggregation_strategy.intervalms_or_count;
    switch (settings_.aggregation_strategy.method) {
        case Settings::AggregationStrategy::TIME_BASED:
            return (t_ns - last_submit_time_ns_) >=
                   std::chrono::milliseconds(count);
        case Settings::AggregationStrategy::TICK_BASED:
            if (prong)
                return prong->Count() >= count;
    }
    return false;
}

void TuningForkImpl::CheckForSubmit(TimePoint t_ns, Prong *prong) {
    if (ShouldSubmit(t_ns, prong)) {
        if (upload_thread_.Submit(current_prong_cache_)) {
            if (current_prong_cache_ == prong_caches_[0].get()) {
                prong_caches_[1]->Clear();
                current_prong_cache_ = prong_caches_[1].get();
            } else {
                prong_caches_[0]->Clear();
                current_prong_cache_ = prong_caches_[0].get();
            }
        }
        last_submit_time_ns_ = t_ns;
    }
}

void TuningForkImpl::InitHistogramSettings() {
    Settings::Histogram default_histogram;
    default_histogram.instrument_key = -1;
    default_histogram.bucket_min = 10;
    default_histogram.bucket_max = 40;
    default_histogram.n_buckets = Histogram::kDefaultNumBuckets;
    for(int i=0; i<settings_.aggregation_strategy.max_instrumentation_keys; ++i) {
        if(settings_.histograms.size()<=i) {
            ALOGW("Couldn't get histogram for key %d. Using default histogram", i);
            settings_.histograms.push_back(default_histogram);
            settings_.histograms.back().instrument_key = i;
        }
        else {
            for(int j=i; j<settings_.aggregation_strategy.max_instrumentation_keys; ++j) {
                auto& h = settings_.histograms[j];
                if(h.instrument_key==i) {
                    if(i!=j) {
                        std::swap(settings_.histograms[j], settings_.histograms[i]);
                    }
                    break;
                }
            }
        }
    }
    ALOGV("Settings::histograms");
    for(int i=0; i< settings_.histograms.size();++i) {
        auto& h = settings_.histograms[i];
        ALOGV("ikey: %d min: %f max: %f nbkts: %d", h.instrument_key, h.bucket_min, h.bucket_max, h.n_buckets);
    }
}

void TuningForkImpl::InitAnnotationRadixes() {
    annotation_util::SetUpAnnotationRadixes(annotation_radix_mult_,
                                            settings_.aggregation_strategy.annotation_enum_size);
}

} // namespace tuningfork {
