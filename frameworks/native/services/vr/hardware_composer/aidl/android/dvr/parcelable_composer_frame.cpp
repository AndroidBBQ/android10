#include "aidl/android/dvr/parcelable_composer_frame.h"

#include <binder/Parcel.h>

#include "aidl/android/dvr/parcelable_composer_layer.h"

namespace android {
namespace dvr {

ParcelableComposerFrame::ParcelableComposerFrame() {}

ParcelableComposerFrame::ParcelableComposerFrame(
    const ComposerView::Frame& frame)
    : frame_(frame) {}

ParcelableComposerFrame::~ParcelableComposerFrame() {}

status_t ParcelableComposerFrame::writeToParcel(Parcel* parcel) const {
  status_t ret = parcel->writeUint64(frame_.display_id);
  if (ret != OK) return ret;

  ret = parcel->writeInt32(frame_.display_width);
  if (ret != OK) return ret;

  ret = parcel->writeInt32(frame_.display_height);
  if (ret != OK) return ret;

  ret = parcel->writeBool(frame_.removed);
  if (ret != OK) return ret;

  ret = parcel->writeUint32(static_cast<uint32_t>(frame_.active_config));
  if (ret != OK) return ret;

  ret = parcel->writeUint32(static_cast<uint32_t>(frame_.color_mode));
  if (ret != OK) return ret;

  ret = parcel->writeUint32(static_cast<uint32_t>(frame_.power_mode));
  if (ret != OK) return ret;

  ret = parcel->writeUint32(static_cast<uint32_t>(frame_.vsync_enabled));
  if (ret != OK) return ret;

  ret = parcel->writeInt32(frame_.color_transform_hint);
  if (ret != OK) return ret;

  for(size_t i = 0; i < 16; i++) {
    ret = parcel->writeFloat(frame_.color_transform[i]);
    if (ret != OK) return ret;
  }

  std::vector<ParcelableComposerLayer> layers;
  for (size_t i = 0; i < frame_.layers.size(); ++i)
    layers.push_back(ParcelableComposerLayer(frame_.layers[i]));

  ret = parcel->writeParcelableVector(layers);

  return ret;
}

status_t ParcelableComposerFrame::readFromParcel(const Parcel* parcel) {
  status_t ret = parcel->readUint64(&frame_.display_id);
  if (ret != OK) return ret;

  ret = parcel->readInt32(&frame_.display_width);
  if (ret != OK) return ret;

  ret = parcel->readInt32(&frame_.display_height);
  if (ret != OK) return ret;

  ret = parcel->readBool(&frame_.removed);
  if (ret != OK) return ret;

  uint32_t value;
  ret = parcel->readUint32(&value);
  if (ret != OK) return ret;
  frame_.active_config = static_cast<Config>(value);

  ret = parcel->readUint32(&value);
  if (ret != OK) return ret;
  frame_.color_mode = static_cast<ColorMode>(value);

  ret = parcel->readUint32(&value);
  if (ret != OK) return ret;
  frame_.power_mode = static_cast<IComposerClient::PowerMode>(value);

  ret = parcel->readUint32(&value);
  if (ret != OK) return ret;
  frame_.vsync_enabled = static_cast<IComposerClient::Vsync>(value);

  ret = parcel->readInt32(&frame_.color_transform_hint);
  if (ret != OK) return ret;

  for(size_t i = 0; i < 16; i++) {
    ret = parcel->readFloat(&frame_.color_transform[i]);
    if (ret != OK) return ret;
  }

  std::vector<ParcelableComposerLayer> layers;
  ret = parcel->readParcelableVector(&layers);
  if (ret != OK) return ret;

  frame_.layers.clear();
  for (size_t i = 0; i < layers.size(); ++i)
    frame_.layers.push_back(layers[i].layer());

  return ret;
}

}  // namespace dvr
}  // namespace android
