/*
 * Copyright 2016 The Android Open Source Project
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

#include <android/frameworks/vr/composer/1.0/IVrComposerClient.h>
#include <hardware/gralloc.h>
#include <hardware/gralloc1.h>
#include <log/log.h>

#include <memory>

#include "impl/vr_hwc.h"
#include "impl/vr_composer_client.h"

namespace android {
namespace dvr {

using android::hardware::graphics::common::V1_0::PixelFormat;
using android::frameworks::vr::composer::V1_0::IVrComposerClient;

VrComposerClient::VrComposerClient(dvr::VrHwc& hal)
    : ComposerClient(&hal), mVrHal(hal) {
  if (!init()) {
      LOG_ALWAYS_FATAL("failed to initialize VrComposerClient");
  }
}

VrComposerClient::~VrComposerClient() {}

std::unique_ptr<ComposerCommandEngine>
VrComposerClient::createCommandEngine() {
  return std::make_unique<VrCommandEngine>(*this);
}

VrComposerClient::VrCommandEngine::VrCommandEngine(VrComposerClient& client)
    : ComposerCommandEngine(client.mHal, client.mResources.get()),
      mVrHal(client.mVrHal) {}

VrComposerClient::VrCommandEngine::~VrCommandEngine() {}

bool VrComposerClient::VrCommandEngine::executeCommand(
    IComposerClient::Command command, uint16_t length) {
  IVrComposerClient::VrCommand vrCommand =
      static_cast<IVrComposerClient::VrCommand>(command);
  switch (vrCommand) {
    case IVrComposerClient::VrCommand::SET_LAYER_INFO:
      return executeSetLayerInfo(length);
    case IVrComposerClient::VrCommand::SET_CLIENT_TARGET_METADATA:
      return executeSetClientTargetMetadata(length);
    case IVrComposerClient::VrCommand::SET_LAYER_BUFFER_METADATA:
      return executeSetLayerBufferMetadata(length);
    default:
      return ComposerCommandEngine::executeCommand(command, length);
  }
}

bool VrComposerClient::VrCommandEngine::executeSetLayerInfo(uint16_t length) {
  if (length != 2) {
    return false;
  }

  auto err = mVrHal.setLayerInfo(mCurrentDisplay, mCurrentLayer, read(), read());
  if (err != Error::NONE) {
    mWriter.setError(getCommandLoc(), err);
  }

  return true;
}

bool VrComposerClient::VrCommandEngine::executeSetClientTargetMetadata(
    uint16_t length) {
  if (length != 7)
    return false;

  auto err = mVrHal.setClientTargetMetadata(mCurrentDisplay, readBufferMetadata());
  if (err != Error::NONE)
    mWriter.setError(getCommandLoc(), err);

  return true;
}

bool VrComposerClient::VrCommandEngine::executeSetLayerBufferMetadata(
    uint16_t length) {
  if (length != 7)
    return false;

  auto err = mVrHal.setLayerBufferMetadata(mCurrentDisplay, mCurrentLayer,
                                           readBufferMetadata());
  if (err != Error::NONE)
    mWriter.setError(getCommandLoc(), err);

  return true;
}

IVrComposerClient::BufferMetadata
VrComposerClient::VrCommandEngine::readBufferMetadata() {
  IVrComposerClient::BufferMetadata metadata = {
    .width = read(),
    .height = read(),
    .stride = read(),
    .layerCount = read(),
    .format = static_cast<PixelFormat>(readSigned()),
    .usage = read64(),
  };
  return metadata;
}

}  // namespace dvr
}  // namespace android
