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

#include "uploadthread.h"
#include "tuningfork_utils.h"

#include <sys/system_properties.h>
#include <GLES3/gl32.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include "clearcutserializer.h"
#include "modp_b64.h"

#define LOG_TAG "TuningFork"
#include "Log.h"

namespace tuningfork {

DebugBackend::~DebugBackend() {}

bool DebugBackend::Process(const ProtobufSerialization &evt_ser) {
    if (evt_ser.size() == 0) return false;
    auto encode_len = modp_b64_encode_len(evt_ser.size());
    std::vector<char> dest_buf(encode_len);
    // This fills the dest buffer with a null-terminated string. It returns the length of
    //  the string, not including the null char
    auto n_encoded = modp_b64_encode(&dest_buf[0], reinterpret_cast<const char*>(&evt_ser[0]),
        evt_ser.size());
    if (n_encoded == -1 || encode_len != n_encoded+1) {
        ALOGW("Could not b64 encode protobuf");
        return false;
    }
    std::string s(&dest_buf[0], n_encoded);
    // Split the serialization into <128-byte chunks to avoid logcat line
    //  truncation.
    constexpr size_t maxStrLen = 128;
    int n = (s.size() + maxStrLen - 1) / maxStrLen; // Round up
    for (int i = 0, j = 0; i < n; ++i) {
        std::stringstream str;
        str << "(TCL" << (i + 1) << "/" << n << ")";
        int m = std::min(s.size() - j, maxStrLen);
        str << s.substr(j, m);
        j += m;
        ALOGI("%s", str.str().c_str());
    }
    return true;
}

std::unique_ptr<DebugBackend> s_debug_backend = std::make_unique<DebugBackend>();

UploadThread::UploadThread(Backend *backend, const ExtraUploadInfo& extraInfo) : backend_(backend),
                                               current_fidelity_params_(0),
                                               upload_callback_(nullptr),
                                               extra_info_(extraInfo) {
    if (backend_ == nullptr)
        backend_ = s_debug_backend.get();
    Start();
}

UploadThread::~UploadThread() {
    Stop();
}

void UploadThread::Start() {
    if (thread_) {
        ALOGW("Can't start an already running thread");
        return;
    }
    do_quit_ = false;
    ready_ = nullptr;
    thread_ = std::make_unique<std::thread>([&] { return Run(); });
}

void UploadThread::Stop() {
    if (!thread_->joinable()) {
        ALOGW("Can't stop a thread that's not started");
        return;
    }
    do_quit_ = true;
    cv_.notify_one();
    thread_->join();
}

void UploadThread::Run() {
    while (!do_quit_) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (ready_) {
            ProtobufSerialization evt_ser;
            UpdateGLVersion(); // Needs to be done with an active gl context
            ClearcutSerializer::SerializeEvent(*ready_, current_fidelity_params_,
                                               extra_info_,
                                               evt_ser);
            if(upload_callback_) {
                CProtobufSerialization cser = { evt_ser.data(), evt_ser.size(), nullptr};
                upload_callback_(&cser);
            }
            backend_->Process(evt_ser);
            ready_ = nullptr;
        }
        cv_.wait_for(lock, std::chrono::milliseconds(1000));
    }
}

// Returns true if we submitted, false if we are waiting for a previous submit to complete
bool UploadThread::Submit(const ProngCache *prongs) {
    if (ready_ == nullptr) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ready_ = prongs;
        }
        cv_.notify_one();
        return true;
    } else
        return false;
}

namespace {

// TODO: replace these with device_info library calls once they are available

std::string slurpFile(const char* fname) {
    std::ifstream f(fname);
    if (f.good()) {
        std::stringstream str;
        str << f.rdbuf();
        return str.str();
    }
    return "";
}

const char* skipSpace(const char* q) {
    while(*q && (*q==' ' || *q=='\t')) ++q;
    return q;
}
std::string getSystemPropViaGet(const char* key) {
    char buffer[PROP_VALUE_MAX + 1]="";  // +1 for terminator
    int bufferLen = __system_property_get(key, buffer);
    if(bufferLen>0)
        return buffer;
    else
        return "";
}

}

/* static */
ExtraUploadInfo UploadThread::GetExtraUploadInfo(JNIEnv* env, jobject activity) {
    ExtraUploadInfo extra_info;
    // Total memory
    std::string s = slurpFile("/proc/meminfo");
    if (!s.empty()) {
        // Lines like 'MemTotal:        3749460 kB'
        std::string to_find("MemTotal:");
        auto it = s.find(to_find);
        if(it!=std::string::npos) {
            const char* p = s.data() + it + to_find.length();
            p = skipSpace(p);
            std::istringstream str(p);
            uint64_t x;
            str >> x;
            std::string units;
            str >> units;
            static std::string unitPrefix = "bBkKmMgGtTpP";
            auto j = unitPrefix.find(units[0]);
            uint64_t mult = 1;
            if (j!=std::string::npos) {
                mult = ::pow(1024L,j/2);
            }
            extra_info.total_memory_bytes = x*mult;
        }
    }
    extra_info.build_version_sdk = getSystemPropViaGet("ro.build.version.sdk");
    extra_info.build_fingerprint = getSystemPropViaGet("ro.build.fingerprint");

    extra_info.session_id = UniqueId(env);

    extra_info.cpu_max_freq_hz.clear();
    for(int index = 1;;++index) {
        std::stringstream str;
        str << "/sys/devices/system/cpu/cpu" << index << "/cpufreq/cpuinfo_max_freq";
        auto cpu_freq_file = slurpFile(str.str().c_str());
        if (cpu_freq_file.empty())
            break;
        uint64_t freq;
        std::istringstream cstr(cpu_freq_file);
        cstr >> freq;
        extra_info.cpu_max_freq_hz.push_back(freq*1000); // File is in kHz
    }

    extra_info.apk_version_code = apk_utils::GetVersionCode(env, activity,
        &extra_info.apk_package_name);

    extra_info.tuningfork_version = TUNINGFORK_PACKED_VERSION;

    return extra_info;
}

void UploadThread::UpdateGLVersion() {
    // gl_es_version
    GLint glVerMajor = 2;
    GLint glVerMinor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &glVerMajor);
    if (glGetError() != GL_NO_ERROR) {
        glVerMajor = 0;
        glVerMinor = 0;
    } else {
        glGetIntegerv(GL_MINOR_VERSION, &glVerMinor);
    }
    extra_info_.gl_es_version = (glVerMajor<<16) + glVerMinor;
}

} // namespace tuningfork
