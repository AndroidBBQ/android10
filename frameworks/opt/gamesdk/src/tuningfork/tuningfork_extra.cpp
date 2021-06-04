
#include "tuningfork/tuningfork_extra.h"
#include "tuningfork/protobuf_util.h"
#include "tuningfork_internal.h"
#include "tuningfork_utils.h"

#include <cinttypes>
#include <dlfcn.h>
#include <memory>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <thread>
#include <fstream>
#include <mutex>

#define LOG_TAG "TuningFork"
#include "Log.h"
#include "swappy/swappy_extra.h"

#include <android/asset_manager_jni.h>
#include <jni.h>

using namespace tuningfork;

namespace {

using PFN_Swappy_initTracer = void (*)(const SwappyTracer* tracer);

constexpr TFInstrumentKey TFTICK_WAIT_TIME = 2;
constexpr TFInstrumentKey TFTICK_SWAP_TIME = 3;

class DynamicSwappy {
    typedef void* Handle;
    Handle lib_;
    PFN_Swappy_initTracer inject_tracer_;
public:
    DynamicSwappy(const char* libraryName) {
        static char defaultLibNames[][20] = {"libgamesdk.so", "libswappy.so", "libunity.so"};
        std::vector<const char*> libNames = {
            libraryName, NULL, defaultLibNames[0], defaultLibNames[1], defaultLibNames[2]};
        for(auto libName: libNames) {
            lib_ = dlopen(libName, RTLD_NOW);
            if( lib_ ) {
                inject_tracer_ = (PFN_Swappy_initTracer)dlsym(lib_, "Swappy_injectTracer");
                if(inject_tracer_) {
                    return;
                } else {
                    dlclose(lib_);
                }
            }
        }
        ALOGW("Couldn't find Swappy_injectTracer");
        lib_ = nullptr;
    }
    ~DynamicSwappy() {
        if(lib_) dlclose(lib_);
    }
    void injectTracer(const SwappyTracer* tracer) const {
        if(inject_tracer_)
            inject_tracer_(tracer);
    }
    bool valid() const { return lib_ != nullptr; }
};

class SwappyTuningFork {
    DynamicSwappy swappy_;
    SwappyTracer trace_;
    VoidCallback frame_callback_;
    TFTraceHandle waitTraceHandle_ = 0;
    TFTraceHandle swapTraceHandle_ = 0;
public:
    SwappyTuningFork(const CProtobufSerialization& settings_ser, JNIEnv* env, jobject activity,
                     VoidCallback cbk, const char* libName)
        : swappy_(libName), trace_({}), frame_callback_(cbk) {
        trace_.startFrame = swappyStartFrameCallback;
        trace_.preWait =  swappyPreWaitCallback;
        trace_.postWait = swappyPostWaitCallback;
        trace_.preSwapBuffers = swappyPreSwapBuffersCallback;
        trace_.postSwapBuffers = swappyPostSwapBuffersCallback;
        trace_.userData = this;
        if(swappy_.valid()) {
            TuningFork_init(&settings_ser, env, activity);
            swappy_.injectTracer(&trace_);
        }
    }
    bool valid() const { return swappy_.valid(); }

    // Swappy trace callbacks
    static void swappyStartFrameCallback(void* userPtr, int /*currentFrame*/,
                                         long /*currentFrameTimeStampMs*/) {
        SwappyTuningFork* _this = (SwappyTuningFork*)userPtr;
        _this->frame_callback_();
        TuningFork_frameTick(TFTICK_SYSCPU);
    }
    static void swappyPreWaitCallback(void* userPtr) {
        SwappyTuningFork* _this = (SwappyTuningFork*)userPtr;
        _this->waitTraceHandle_ = TuningFork_startTrace(TFTICK_WAIT_TIME);
    }
    static void swappyPostWaitCallback(void* userPtr) {
        SwappyTuningFork *_this = (SwappyTuningFork *) userPtr;
        if (_this->waitTraceHandle_) {
            TuningFork_endTrace(_this->waitTraceHandle_);
            _this->waitTraceHandle_ = 0;
        }
        TuningFork_frameTick(TFTICK_SYSGPU);
    }
    static void swappyPreSwapBuffersCallback(void* userPtr) {
        SwappyTuningFork* _this = (SwappyTuningFork*)userPtr;
        _this->swapTraceHandle_ = TuningFork_startTrace(TFTICK_SWAP_TIME);
    }
    static void swappyPostSwapBuffersCallback(void* userPtr, long /*desiredPresentationTimeMs*/) {
        SwappyTuningFork *_this = (SwappyTuningFork *) userPtr;
        if (_this->swapTraceHandle_) {
            TuningFork_endTrace(_this->swapTraceHandle_);
            _this->swapTraceHandle_ = 0;
        }
    }
    // Static methods
    static std::unique_ptr<SwappyTuningFork> s_instance_;

    static bool Init(const CProtobufSerialization* settings, JNIEnv* env,
                     jobject activity, const char* libName, void (*frame_callback)()) {
        s_instance_ = std::unique_ptr<SwappyTuningFork>(
            new SwappyTuningFork(*settings, env, activity, frame_callback, libName));
        return s_instance_->valid();
    }
};

std::unique_ptr<SwappyTuningFork> SwappyTuningFork::s_instance_;

// Gets the serialized settings from the APK.
// Returns false if there was an error.
bool GetSettingsSerialization(JNIEnv* env, jobject activity,
                                        CProtobufSerialization& settings_ser) {
    auto asset = apk_utils::GetAsset(env, activity, "tuningfork/tuningfork_settings.bin");
    if (asset == nullptr )
        return false;
    ALOGI("Got settings from tuningfork/tuningfork_settings.bin");
    // Get serialized settings from assets
    uint64_t size = AAsset_getLength64(asset);
    settings_ser.bytes = (uint8_t*)::malloc(size);
    memcpy(settings_ser.bytes, AAsset_getBuffer(asset), size);
    settings_ser.size = size;
    settings_ser.dealloc = ::free;
    AAsset_close(asset);
    return true;
}

// Gets the serialized fidelity params from the APK.
// Call this function once with fps_ser=NULL to get the count of files present,
// then allocate an array of CProtobufSerializations and pass this as fps_ser
// to a second call.
void GetFidelityParamsSerialization(JNIEnv* env, jobject activity,
                                            CProtobufSerialization* fps_ser,
                                            int* fp_count) {
    std::vector<AAsset*> fps;
    for( int i=1; i<16; ++i ) {
        std::stringstream name;
        name << "tuningfork/dev_tuningfork_fidelityparams_" << i << ".bin";
        auto fp = apk_utils::GetAsset(env, activity, name.str().c_str());
        if ( fp == nullptr ) break;
        fps.push_back(fp);
    }
    *fp_count = fps.size();
    if( fps_ser==nullptr )
        return;
    for(int i=0; i<*fp_count; ++i) {
        // Get serialized FidelityParams from assets
        AAsset* asset = fps[i];
        CProtobufSerialization& fp_ser = fps_ser[i];
        uint64_t size = AAsset_getLength64(asset);
        fp_ser.bytes = (uint8_t*)::malloc(size);
        memcpy(fp_ser.bytes, AAsset_getBuffer(asset), size);
        fp_ser.size = size;
        fp_ser.dealloc = ::free;
        AAsset_close(asset);
    }
}

// Get the name of the tuning fork save file. Returns true if the directory
//  for the file exists and false on error.
bool GetSavedFileName(JNIEnv* env, jobject activity, std::string& name) {

    // Create tuningfork/version folder if it doesn't exist
    std::stringstream tf_path_str;
    tf_path_str << file_utils::GetAppCacheDir(env, activity) << "/tuningfork";
    if (!file_utils::CheckAndCreateDir(tf_path_str.str())) {
        return false;
    }
    tf_path_str << "/V" << apk_utils::GetVersionCode(env, activity);
    if (!file_utils::CheckAndCreateDir(tf_path_str.str())) {
        return false;
    }
    tf_path_str << "/saved_fp.bin";
    name = tf_path_str.str();
    return true;
}

// Get a previously save fidelity param serialization.
bool GetSavedFidelityParams(JNIEnv* env, jobject activity, CProtobufSerialization* params) {
    std::string save_filename;
    if (GetSavedFileName(env, activity, save_filename)) {
        std::ifstream save_file(save_filename, std::ios::binary);
        if (save_file.good()) {
            save_file.seekg(0, std::ios::end);
            params->size = save_file.tellg();
            params->bytes = (uint8_t*)::malloc(params->size);
            params->dealloc = ::free;
            save_file.seekg(0, std::ios::beg);
            save_file.read((char*)params->bytes, params->size);
            ALOGI("Loaded fps from %s (%zu bytes)", save_filename.c_str(), params->size);
            return true;
        }
        ALOGI("Couldn't load fps from %s", save_filename.c_str());
    }
    return false;
}

// Save fidelity params to the save file.
bool SaveFidelityParams(JNIEnv* env, jobject activity, const CProtobufSerialization* params) {
    std::string save_filename;
    if (GetSavedFileName(env, activity, save_filename)) {
        std::ofstream save_file(save_filename, std::ios::binary);
        if (save_file.good()) {
            save_file.write((const char*)params->bytes, params->size);
            ALOGI("Saved fps to %s (%zu bytes)", save_filename.c_str(), params->size);
            return true;
        }
        ALOGI("Couldn't save fps to %s", save_filename.c_str());
    }
    return false;
}

// Check if we have saved fidelity params.
bool SavedFidelityParamsFileExists(JNIEnv* env, jobject activity) {
    std::string save_filename;
    if (GetSavedFileName(env, activity, save_filename)) {
        return file_utils::FileExists(save_filename);
    }
    return false;
}

// Download FPs on a separate thread
void StartFidelityParamDownloadThread(JNIEnv* env, jobject activity,
                                      const CProtobufSerialization& defaultParams,
                                      ProtoCallback fidelity_params_callback,
                                      int initialTimeoutMs, int ultimateTimeoutMs) {
    static std::mutex threadMutex;
    std::lock_guard<std::mutex> lock(threadMutex);
    static std::thread fpThread;
    if (fpThread.joinable()) {
        ALOGW("Fidelity param download thread already started");
        return;
    }
    JavaVM *vm;
    env->GetJavaVM(&vm);
    auto newActivity = env->NewGlobalRef(activity);
    fpThread = std::thread([=](CProtobufSerialization defaultParams) {
        CProtobufSerialization params = {};
        int waitTimeMs = initialTimeoutMs;
        bool first_time = true;
        JNIEnv *newEnv;
        if (vm->AttachCurrentThread(&newEnv, NULL) == 0) {
            while (true) {
                if (TuningFork_getFidelityParameters(&defaultParams,
                                                     &params, waitTimeMs)) {
                    ALOGI("Got fidelity params from server");
                    SaveFidelityParams(newEnv, newActivity, &params);
                    CProtobufSerialization_Free(&defaultParams);
                    fidelity_params_callback(&params);
                    CProtobufSerialization_Free(&params);
                    break;
                } else {
                    ALOGI("Could not get fidelity params from server");
                    if (first_time) {
                        fidelity_params_callback(&defaultParams);
                        first_time = false;
                    }
                    if (waitTimeMs > ultimateTimeoutMs) {
                        ALOGW("Not waiting any longer for fidelity params");
                        CProtobufSerialization_Free(&defaultParams);
                        break;
                    }
                    waitTimeMs *= 2; // back off
                }
            }
            newEnv->DeleteGlobalRef(newActivity);
            vm->DetachCurrentThread();
        }
    }, defaultParams);
}

} // anonymous namespace

extern "C" {

bool TuningFork_findSettingsInAPK(JNIEnv* env, jobject activity,
                                  CProtobufSerialization* settings_ser) {
    if(settings_ser) {
        return GetSettingsSerialization(env, activity, *settings_ser);
    } else {
        return false;
    }
}
void TuningFork_findFidelityParamsInAPK(JNIEnv* env, jobject activity,
                                        CProtobufSerialization* fps, int* fp_count) {
    GetFidelityParamsSerialization(env, activity, fps, fp_count);
}

bool TuningFork_initWithSwappy(const CProtobufSerialization* settings, JNIEnv* env,
                               jobject activity, const char* libraryName,
                               VoidCallback frame_callback) {
    return SwappyTuningFork::Init(settings, env, activity, libraryName, frame_callback);
}

void TuningFork_setUploadCallback(void(*cbk)(const CProtobufSerialization*)) {
    tuningfork::SetUploadCallback(cbk);
}

TFErrorCode TuningFork_initFromAssetsWithSwappy(JNIEnv* env, jobject activity,
                                                const char* libraryName,
                                                VoidCallback frame_callback,
                                                int fpFileNum,
                                                ProtoCallback fidelity_params_callback,
                                                int initialTimeoutMs, int ultimateTimeoutMs) {
    CProtobufSerialization ser;
    if (!TuningFork_findSettingsInAPK(env, activity, &ser))
        return TFERROR_NO_SETTINGS;
    if (!TuningFork_initWithSwappy(&ser, env, activity, libraryName, frame_callback))
        return TFERROR_NO_SWAPPY;
    CProtobufSerialization defaultParams = {};
    // Special meaning for negative fpFileNum: don't load saved params, overwrite them instead
    bool resetSavedFPs = fpFileNum<0;
    fpFileNum = abs(fpFileNum);
    // Use the saved params as default, if they exist
    if (!resetSavedFPs && SavedFidelityParamsFileExists(env, activity)) {
        GetSavedFidelityParams(env, activity, &defaultParams);
    } else {
        int nfps=0;
        TuningFork_findFidelityParamsInAPK(env, activity, NULL, &nfps);
        if (nfps>0) {
            std::vector<CProtobufSerialization> fps(nfps);
            TuningFork_findFidelityParamsInAPK(env, activity, fps.data(), &nfps);
            int chosen = fpFileNum - 1; // File indices start at 1
            for (int i=0;i<nfps;++i) {
                if (i==chosen) {
                    defaultParams = fps[i];
                } else {
                    CProtobufSerialization_Free(&fps[i]);
                }
            }
            if (chosen>=0 && chosen<nfps) {
                ALOGI("Using params from dev_tuningfork_fidelityparams_%d.bin as default",
                    fpFileNum);
            } else {
                return TFERROR_INVALID_DEFAULT_FIDELITY_PARAMS;
            }
        } else {
            return TFERROR_NO_FIDELITY_PARAMS;
        }
        // Save the default params
        SaveFidelityParams(env, activity, &defaultParams);
    }
    StartFidelityParamDownloadThread(env, activity, defaultParams, fidelity_params_callback,
        initialTimeoutMs, ultimateTimeoutMs);
    return TFERROR_OK;
}

} // extern "C"
