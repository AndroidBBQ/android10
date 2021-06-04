/*
 * Copyright 2017 The Android Open Source Project
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

#ifndef ANDROID_DVR_HARDWARE_COMPOSER_IMPL_VR_COMPOSER_CLIENT_H
#define ANDROID_DVR_HARDWARE_COMPOSER_IMPL_VR_COMPOSER_CLIENT_H

#include <android/frameworks/vr/composer/1.0/IVrComposerClient.h>
#include <composer-command-buffer/2.1/ComposerCommandBuffer.h>
#include <composer-hal/2.1/ComposerClient.h>
#include <composer-hal/2.1/ComposerCommandEngine.h>

namespace android {
namespace dvr {

class VrHwc;

using hardware::graphics::composer::V2_1::hal::ComposerCommandEngine;
using hardware::graphics::composer::V2_1::hal::ComposerHal;
using hardware::graphics::composer::V2_1::hal::detail::ComposerClientImpl;

using ComposerClient = ComposerClientImpl<IVrComposerClient, ComposerHal>;

class VrComposerClient : public ComposerClient {
 public:
  explicit VrComposerClient(android::dvr::VrHwc& hal);
  virtual ~VrComposerClient();

 private:
  class VrCommandEngine : public ComposerCommandEngine {
   public:
    explicit VrCommandEngine(VrComposerClient& client);
    ~VrCommandEngine() override;

    bool executeCommand(IComposerClient::Command command,
                        uint16_t length) override;

   private:
    bool executeSetLayerInfo(uint16_t length);
    bool executeSetClientTargetMetadata(uint16_t length);
    bool executeSetLayerBufferMetadata(uint16_t length);

    IVrComposerClient::BufferMetadata readBufferMetadata();

    android::dvr::VrHwc& mVrHal;

    VrCommandEngine(const VrCommandEngine&) = delete;
    void operator=(const VrCommandEngine&) = delete;
  };

  VrComposerClient(const VrComposerClient&) = delete;
  void operator=(const VrComposerClient&) = delete;

  std::unique_ptr<ComposerCommandEngine> createCommandEngine() override;
  dvr::VrHwc& mVrHal;
};

} // namespace dvr
} // namespace android

#endif  // ANDROID_DVR_HARDWARE_COMPOSER_IMPL_VR_COMPOSER_CLIENT_H
