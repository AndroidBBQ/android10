#ifndef ANDROID_VR_HARDWARE_COMPOSER_DEFS_H
#define ANDROID_VR_HARDWARE_COMPOSER_DEFS_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// NOTE: These definitions must match the ones in
// //hardware/libhardware/include/hardware/hwcomposer2.h. They are used by the
// client side which does not have access to hwc2 headers.
enum DvrHwcBlendMode {
  DVR_HWC_BLEND_MODE_INVALID = 0,
  DVR_HWC_BLEND_MODE_NONE = 1,
  DVR_HWC_BLEND_MODE_PREMULTIPLIED = 2,
  DVR_HWC_BLEND_MODE_COVERAGE = 3,
};

enum DvrHwcComposition {
  DVR_HWC_COMPOSITION_INVALID = 0,
  DVR_HWC_COMPOSITION_CLIENT = 1,
  DVR_HWC_COMPOSITION_DEVICE = 2,
  DVR_HWC_COMPOSITION_SOLID_COLOR = 3,
  DVR_HWC_COMPOSITION_CURSOR = 4,
  DVR_HWC_COMPOSITION_SIDEBAND = 5,
};

enum DvrHwcTransform {
  DVR_HWC_TRANSFORM_NONE = 0,
  DVR_HWC_TRANSFORM_FLIP_H = 1,
  DVR_HWC_TRANSFORM_FLIP_V = 2,
  DVR_HWC_TRANSFORM_ROT_90 = 4,
  DVR_HWC_TRANSFORM_ROT_180 = 3,
  DVR_HWC_TRANSFORM_ROT_270 = 7,
};

typedef uint64_t DvrHwcDisplay;
typedef uint64_t DvrHwcLayer;

struct DvrHwcRecti {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
};

struct DvrHwcRectf {
  float left;
  float top;
  float right;
  float bottom;
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANDROID_DVR_HARDWARE_COMPOSER_DEFS_H
