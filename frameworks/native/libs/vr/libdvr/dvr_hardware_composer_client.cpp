#include "include/dvr/dvr_hardware_composer_client.h"

#include <android/dvr/IVrComposer.h>
#include <android/dvr/BnVrComposerCallback.h>
#include <android/hardware_buffer.h>
#include <binder/IServiceManager.h>
#include <private/android/AHardwareBufferHelpers.h>

#include <functional>
#include <memory>
#include <mutex>

struct DvrHwcFrame {
  android::dvr::ComposerView::Frame frame;
};

namespace {

class HwcCallback : public android::dvr::BnVrComposerCallback {
 public:
  using CallbackFunction = std::function<int(DvrHwcFrame*)>;

  explicit HwcCallback(const CallbackFunction& callback);
  ~HwcCallback() override;

  // Reset the callback. This needs to be done early to avoid use after free
  // accesses from binder thread callbacks.
  void Shutdown();

  std::unique_ptr<DvrHwcFrame> DequeueFrame();

 private:
  // android::dvr::BnVrComposerCallback:
  android::binder::Status onNewFrame(
      const android::dvr::ParcelableComposerFrame& frame,
      android::dvr::ParcelableUniqueFd* fence) override;

  // Protects the |callback_| from uses from multiple threads. During shutdown
  // there may be in-flight frame update events. In those cases the callback
  // access needs to be protected otherwise binder threads may access an invalid
  // callback.
  std::mutex mutex_;
  CallbackFunction callback_;

  HwcCallback(const HwcCallback&) = delete;
  void operator=(const HwcCallback&) = delete;
};

HwcCallback::HwcCallback(const CallbackFunction& callback)
    : callback_(callback) {}

HwcCallback::~HwcCallback() {}

void HwcCallback::Shutdown() {
  std::lock_guard<std::mutex> guard(mutex_);
  callback_ = nullptr;
}

android::binder::Status HwcCallback::onNewFrame(
    const android::dvr::ParcelableComposerFrame& frame,
    android::dvr::ParcelableUniqueFd* fence) {
  std::lock_guard<std::mutex> guard(mutex_);

  if (!callback_) {
    fence->set_fence(android::base::unique_fd());
    return android::binder::Status::ok();
  }

  std::unique_ptr<DvrHwcFrame> dvr_frame(new DvrHwcFrame());
  dvr_frame->frame = frame.frame();

  fence->set_fence(android::base::unique_fd(callback_(dvr_frame.release())));
  return android::binder::Status::ok();
}

}  // namespace

struct DvrHwcClient {
  android::sp<android::dvr::IVrComposer> composer;
  android::sp<HwcCallback> callback;
};

DvrHwcClient* dvrHwcClientCreate(DvrHwcOnFrameCallback callback, void* data) {
  std::unique_ptr<DvrHwcClient> client(new DvrHwcClient());

  android::sp<android::IServiceManager> sm(android::defaultServiceManager());
  client->composer = android::interface_cast<android::dvr::IVrComposer>(
      sm->getService(android::dvr::IVrComposer::SERVICE_NAME()));
  if (!client->composer.get())
    return nullptr;

  client->callback = new HwcCallback(std::bind(callback, data,
                                               std::placeholders::_1));
  android::binder::Status status = client->composer->registerObserver(
      client->callback);
  if (!status.isOk())
    return nullptr;

  return client.release();
}

void dvrHwcClientDestroy(DvrHwcClient* client) {
  client->composer->clearObserver();

  // NOTE: Deleting DvrHwcClient* isn't enough since DvrHwcClient::callback is a
  // shared pointer that could be referenced from a binder thread. But the
  // client callback isn't valid past this calls so that needs to be reset.
  client->callback->Shutdown();

  delete client;
}

void dvrHwcFrameDestroy(DvrHwcFrame* frame) {
  delete frame;
}

DvrHwcDisplay dvrHwcFrameGetDisplayId(DvrHwcFrame* frame) {
  return frame->frame.display_id;
}

int32_t dvrHwcFrameGetDisplayWidth(DvrHwcFrame* frame) {
  return frame->frame.display_width;
}

int32_t dvrHwcFrameGetDisplayHeight(DvrHwcFrame* frame) {
  return frame->frame.display_height;
}

bool dvrHwcFrameGetDisplayRemoved(DvrHwcFrame* frame) {
  return frame->frame.removed;
}

size_t dvrHwcFrameGetLayerCount(DvrHwcFrame* frame) {
  return frame->frame.layers.size();
}

uint32_t dvrHwcFrameGetActiveConfig(DvrHwcFrame* frame) {
  return static_cast<uint32_t>(frame->frame.active_config);
}

uint32_t dvrHwcFrameGetColorMode(DvrHwcFrame* frame) {
  return static_cast<uint32_t>(frame->frame.color_mode);
}

void dvrHwcFrameGetColorTransform(DvrHwcFrame* frame, float* out_matrix,
                                  int32_t* out_hint) {
  *out_hint = frame->frame.color_transform_hint;
  memcpy(out_matrix, frame->frame.color_transform,
         sizeof(frame->frame.color_transform));
}

uint32_t dvrHwcFrameGetPowerMode(DvrHwcFrame* frame) {
  return static_cast<uint32_t>(frame->frame.power_mode);
}

uint32_t dvrHwcFrameGetVsyncEnabled(DvrHwcFrame* frame) {
  return static_cast<uint32_t>(frame->frame.vsync_enabled);
}

DvrHwcLayer dvrHwcFrameGetLayerId(DvrHwcFrame* frame, size_t layer_index) {
  return frame->frame.layers[layer_index].id;
}

AHardwareBuffer* dvrHwcFrameGetLayerBuffer(DvrHwcFrame* frame,
                                           size_t layer_index) {
  AHardwareBuffer* buffer = android::AHardwareBuffer_from_GraphicBuffer(
      frame->frame.layers[layer_index].buffer.get());
  AHardwareBuffer_acquire(buffer);
  return buffer;
}

int dvrHwcFrameGetLayerFence(DvrHwcFrame* frame, size_t layer_index) {
  return frame->frame.layers[layer_index].fence->dup();
}

DvrHwcRecti dvrHwcFrameGetLayerDisplayFrame(DvrHwcFrame* frame,
                                            size_t layer_index) {
  return DvrHwcRecti{
    frame->frame.layers[layer_index].display_frame.left,
    frame->frame.layers[layer_index].display_frame.top,
    frame->frame.layers[layer_index].display_frame.right,
    frame->frame.layers[layer_index].display_frame.bottom,
  };
}

DvrHwcRectf dvrHwcFrameGetLayerCrop(DvrHwcFrame* frame, size_t layer_index) {
  return DvrHwcRectf{
    frame->frame.layers[layer_index].crop.left,
    frame->frame.layers[layer_index].crop.top,
    frame->frame.layers[layer_index].crop.right,
    frame->frame.layers[layer_index].crop.bottom,
  };
}

DvrHwcBlendMode dvrHwcFrameGetLayerBlendMode(DvrHwcFrame* frame,
                                             size_t layer_index) {
  return static_cast<DvrHwcBlendMode>(
      frame->frame.layers[layer_index].blend_mode);
}

float dvrHwcFrameGetLayerAlpha(DvrHwcFrame* frame, size_t layer_index) {
  return frame->frame.layers[layer_index].alpha;
}

uint32_t dvrHwcFrameGetLayerType(DvrHwcFrame* frame, size_t layer_index) {
  return frame->frame.layers[layer_index].type;
}

uint32_t dvrHwcFrameGetLayerApplicationId(DvrHwcFrame* frame,
                                          size_t layer_index) {
  return frame->frame.layers[layer_index].app_id;
}

uint32_t dvrHwcFrameGetLayerZOrder(DvrHwcFrame* frame, size_t layer_index) {
  return frame->frame.layers[layer_index].z_order;
}

void dvrHwcFrameGetLayerCursor(DvrHwcFrame* frame, size_t layer_index,
                               int32_t* out_x, int32_t* out_y) {
  *out_x = frame->frame.layers[layer_index].cursor_x;
  *out_y = frame->frame.layers[layer_index].cursor_y;
}

uint32_t dvrHwcFrameGetLayerTransform(DvrHwcFrame* frame, size_t layer_index) {
  return frame->frame.layers[layer_index].transform;
}

uint32_t dvrHwcFrameGetLayerDataspace(DvrHwcFrame* frame, size_t layer_index) {
  return frame->frame.layers[layer_index].dataspace;
}

uint32_t dvrHwcFrameGetLayerColor(DvrHwcFrame* frame, size_t layer_index) {
  const auto& color = frame->frame.layers[layer_index].color;
  return color.r | (static_cast<uint32_t>(color.g) << 8) |
         (static_cast<uint32_t>(color.b) << 16) |
         (static_cast<uint32_t>(color.a) << 24);
}

uint32_t dvrHwcFrameGetLayerNumVisibleRegions(DvrHwcFrame* frame,
                                              size_t layer_index) {
  return frame->frame.layers[layer_index].visible_regions.size();
}

DvrHwcRecti dvrHwcFrameGetLayerVisibleRegion(DvrHwcFrame* frame,
                                             size_t layer_index, size_t index) {
  return DvrHwcRecti{
      frame->frame.layers[layer_index].visible_regions[index].left,
      frame->frame.layers[layer_index].visible_regions[index].top,
      frame->frame.layers[layer_index].visible_regions[index].right,
      frame->frame.layers[layer_index].visible_regions[index].bottom,
  };
}

uint32_t dvrHwcFrameGetLayerNumDamagedRegions(DvrHwcFrame* frame,
                                              size_t layer_index) {
  return frame->frame.layers[layer_index].damaged_regions.size();
}

DvrHwcRecti dvrHwcFrameGetLayerDamagedRegion(DvrHwcFrame* frame,
                                             size_t layer_index, size_t index) {
  return DvrHwcRecti{
      frame->frame.layers[layer_index].damaged_regions[index].left,
      frame->frame.layers[layer_index].damaged_regions[index].top,
      frame->frame.layers[layer_index].damaged_regions[index].right,
      frame->frame.layers[layer_index].damaged_regions[index].bottom,
  };
}
