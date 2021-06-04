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

#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>
#include "prong.h"

namespace tuningfork {

class UploadThread {
private:
    std::unique_ptr<std::thread> thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool do_quit_;
    const ProngCache *ready_;
    Backend *backend_;
    ProtobufSerialization current_fidelity_params_;
    ProtoCallback upload_callback_;
    ExtraUploadInfo extra_info_;
 public:
    UploadThread(Backend *backend, const ExtraUploadInfo& extraInfo);

    ~UploadThread();

    void Start();

    void Stop();

    void Run();

    // Returns true if we submitted, false if we are waiting for a previous submit to complete
    bool Submit(const ProngCache *prongs);

    void SetCurrentFidelityParams(const ProtobufSerialization &fp) {
        current_fidelity_params_ = fp;
    }

    void SetUploadCallback(ProtoCallback upload_callback) {
        upload_callback_ = upload_callback;
    }

    static ExtraUploadInfo GetExtraUploadInfo(JNIEnv* env, jobject activity);

 private:
    void UpdateGLVersion();

    friend class ClearcutSerializer;
};

} // namespace tuningfork {
