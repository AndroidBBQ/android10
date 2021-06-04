/*
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

#include "tuningfork/protobuf_util.h"
#include "tuningfork/tuningfork_extra.h"
#include "swappy/swappy.h"
#include "full/tuningfork.pb.h"
#include "full/tuningfork_clearcut_log.pb.h"
#include "full/dev_tuningfork.pb.h"
#include <sstream>
#include <jni.h>
#include <android/native_window_jni.h>

#define LOG_TAG "tftestapp"
#include "Log.h"
#include "Renderer.h"

using ::com::google::tuningfork::FidelityParams;
using ::com::google::tuningfork::Settings;
using ::com::google::tuningfork::Annotation;
using ::logs::proto::tuningfork::TuningForkLogEvent;
using ::logs::proto::tuningfork::TuningForkHistogram;

namespace proto_tf = com::google::tuningfork;
namespace tf = tuningfork;
using namespace samples;

bool swappy_enabled = false;

namespace {

constexpr TFInstrumentKey TFTICK_CHOREOGRAPHER = 4;

struct HistogramSettings {
    float start, end;
    int nBuckets;
};
Settings TestSettings(Settings::AggregationStrategy::Submission method, int n_ticks, int n_keys,
                      std::vector<int> annotation_size,
                      const std::vector<HistogramSettings>& hists = {}) {
    // Make sure we set all required fields
    Settings s;
    s.mutable_aggregation_strategy()->set_method(method);
    s.mutable_aggregation_strategy()->set_intervalms_or_count(n_ticks);
    s.mutable_aggregation_strategy()->set_max_instrumentation_keys(n_keys);
    for(int i=0;i<annotation_size.size();++i)
        s.mutable_aggregation_strategy()->add_annotation_enum_size(annotation_size[i]);
    int i=0;
    for(auto& h: hists) {
        auto sh = s.add_histograms();
        sh->set_bucket_min(h.start);
        sh->set_bucket_max(h.end);
        sh->set_n_buckets(h.nBuckets);
        sh->set_instrument_key(i++);
    }
    return s;
}

std::string ReplaceReturns(const std::string& s) {
    std::string r = s;
    for (int i=0; i<r.length(); ++i) {
        if (r[i]=='\n') r[i] = ',';
        if (r[i]=='\r') r[i] = ' ';
    }
    return r;
}

std::string PrettyPrintTuningForkLogEvent(const TuningForkLogEvent& evt) {
    std::stringstream eventStr;
    eventStr << "TuningForkLogEvent {\n";
    if (evt.has_fidelityparams()) {
        FidelityParams p;
        p.ParseFromArray(evt.fidelityparams().c_str(), evt.fidelityparams().length());
        eventStr << "  fidelityparams : " << ReplaceReturns(p.DebugString()) << "\n";
    }
    for (int i=0; i<evt.histograms_size(); ++i) {
        auto &h = evt.histograms(i);
        Annotation ann;
        ann.ParseFromArray(h.annotation().c_str(), h.annotation().length());
        bool first = true;
        eventStr << "  histogram {\n";
        eventStr << "    instrument_id : " << h.instrument_id() << "\n";
        eventStr << "    annotation : " << ReplaceReturns(ann.DebugString()) << "\n    counts : ";
        eventStr << "[";
        for (int j=0; j<h.counts_size(); ++j) {
            if (first) {
                first = false;
            } else {
                eventStr << ",";
            }
            eventStr << h.counts(j);
        }
        eventStr << "]\n  }\n";
    }
    if (evt.has_experiment_id()) {
        eventStr << "  experiment_id : " << evt.experiment_id() << "\n";
    }
    if (evt.has_session_id()) {
        eventStr << "  session_id : " << evt.session_id() << "\n";
    }
    if (evt.has_device_info()) {
        eventStr << "  device_info : {" << ReplaceReturns(evt.device_info().DebugString()) << "}\n";
    }
    if (evt.has_apk_package_name()) {
        eventStr << "  apk_package_name : " << evt.apk_package_name() << "\n";
    }
    if (evt.has_apk_version_code()) {
        eventStr << "  apk_version_code : " << evt.apk_version_code() << "\n";
    }
    if (evt.has_tuningfork_version()) {
        eventStr << "  tuningfork_version : " << evt.tuningfork_version() << "\n";
    }
    eventStr << "}";
    return eventStr.str();
}
void SplitAndLog(const std::string& s) {
    std::istringstream str(s);
    std::string line;
    std::stringstream to_log;
    const int nlines_per_log = 16;
    int l = nlines_per_log;
    while (std::getline(str, line, '\n')) {
        to_log << line << '\n';
        if(--l<=0) {
            ALOGI("%s", to_log.str().c_str());
            l = nlines_per_log;
            to_log.str("");
        }
    }
    if (to_log.str().length()>0)
        ALOGI("%s", to_log.str().c_str());
}

void UploadCallback(const CProtobufSerialization *tuningfork_log_event) {
    if(tuningfork_log_event) {
        TuningForkLogEvent evt;
        evt.ParseFromArray(tuningfork_log_event->bytes, tuningfork_log_event->size);
        auto pp = PrettyPrintTuningForkLogEvent(evt);
        SplitAndLog(pp);
    }
}

static int sLevel = proto_tf::LEVEL_1;
extern "C"
void SetAnnotations() {
    if(proto_tf::Level_IsValid(sLevel)) {
        Annotation a;
        a.set_level((proto_tf::Level)sLevel);
        auto next_level = sLevel + 1;
        a.set_next_level((proto_tf::Level)(next_level>proto_tf::Level_MAX?1:next_level));
        auto ser = tf::CProtobufSerialization_Alloc(a);
        TuningFork_setCurrentAnnotation(&ser);
        tf::CProtobufSerialization_Free(&ser);
    }
}

void SetFidelityParams(const CProtobufSerialization* params) {
    FidelityParams p;
    // Set default values
    p.set_num_spheres(20);
    p.set_tesselation_percent(50);
    std::vector<uint8_t> params_ser(params->bytes, params->bytes + params->size);
    tf::Deserialize(params_ser, p);
    std::string s = p.DebugString();
    ALOGI("Using FidelityParams: %s", ReplaceReturns(s).c_str());
    int nSpheres = p.num_spheres();
    int tesselation = p.tesselation_percent();
    Renderer::getInstance()->setQuality(nSpheres, tesselation);
}

} // anonymous namespace

extern "C" {

JNIEXPORT void JNICALL
Java_com_google_tuningfork_TFTestActivity_initTuningFork(JNIEnv *env, jobject activity) {
    Swappy_init(env, activity);
    swappy_enabled = Swappy_isEnabled();
    if (swappy_enabled) {
        int defaultFPIndex = 3; // i.e. dev_tuningfork_fidelityparams_3.bin
        int initialTimeoutMs = 1000;
        int ultimateTimeoutMs = 100000;
        TFErrorCode c = TuningFork_initFromAssetsWithSwappy(env, activity, "libnative-lib.so",
                                                    SetAnnotations, defaultFPIndex,
                                                    SetFidelityParams,
                                                    initialTimeoutMs, ultimateTimeoutMs);
        if (c==TFERROR_OK) {
            TuningFork_setUploadCallback(UploadCallback);
            SetAnnotations();
        } else {
            ALOGW("Error initializing TuningFork: %d", c);
        }
    } else {
        ALOGW("Couldn't enable Swappy.");
        CProtobufSerialization settings = {};
        TuningFork_findSettingsInAPK(env, activity, &settings);
        TuningFork_init(&settings, env, activity);
        tuningfork::CProtobufSerialization_Free(&settings);
        int fp_count;
        TuningFork_findFidelityParamsInAPK(env, activity, NULL, &fp_count);
        CProtobufSerialization fps = {};
        std::vector<CProtobufSerialization> defaultFPs(fp_count);
        TuningFork_findFidelityParamsInAPK(env, activity, defaultFPs.data(), &fp_count);
        CProtobufSerialization* defaultFP = &defaultFPs[fp_count/2-1]; // Middle settings level
        if (TuningFork_getFidelityParameters(defaultFP, &fps, 1000)) {
            SetFidelityParams(&fps);
            tuningfork::CProtobufSerialization_Free(&fps);
        }
        else {
            SetFidelityParams(defaultFP);
        }
        for(auto& a: defaultFPs) {
            tuningfork::CProtobufSerialization_Free(&a);
        }
        TuningFork_setUploadCallback(UploadCallback);
        SetAnnotations();
    }
}

JNIEXPORT void JNICALL
Java_com_google_tuningfork_TFTestActivity_onChoreographer(JNIEnv */*env*/, jclass clz, jlong /*frameTimeNanos*/) {
    TuningFork_frameTick(TFTICK_CHOREOGRAPHER);
    // After 600 ticks, switch to the next level
    static int tick_count = 0;
    ++tick_count;
    if(tick_count>=600) {
        ++sLevel;
        if(sLevel>proto_tf::Level_MAX) sLevel = proto_tf::LEVEL_1;
        SetAnnotations();
        tick_count = 0;
    }
}
JNIEXPORT void JNICALL
Java_com_google_tuningfork_TFTestActivity_resize(JNIEnv *env, jclass /*clz*/, jobject surface, jint width, jint height) {
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    Renderer::getInstance()->setWindow(window,
                                       static_cast<int32_t>(width),
                                       static_cast<int32_t>(height));
}
JNIEXPORT void JNICALL
Java_com_google_tuningfork_TFTestActivity_clearSurface(JNIEnv */*env*/, jclass /*clz*/ ) {
    Renderer::getInstance()->setWindow(nullptr, 0, 0);
}
JNIEXPORT void JNICALL
Java_com_google_tuningfork_TFTestActivity_start(JNIEnv */*env*/, jclass /*clz*/ ) {
    Renderer::getInstance()->start();
}
JNIEXPORT void JNICALL
Java_com_google_tuningfork_TFTestActivity_stop(JNIEnv */*env*/, jclass /*clz*/ ) {
    Renderer::getInstance()->stop();
}

}
