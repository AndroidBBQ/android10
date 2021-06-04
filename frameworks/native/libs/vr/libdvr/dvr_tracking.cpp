#include "include/dvr/dvr_tracking.h"

#include <utils/Errors.h>
#include <utils/Log.h>

#if !DVR_TRACKING_IMPLEMENTED

extern "C" {

// This file provides the stub implementation of dvrTrackingXXX APIs. On
// platforms that implement these APIs, set -DDVR_TRACKING_IMPLEMENTED=1 in the
// build file.
int dvrTrackingCameraCreate(DvrTrackingCamera**) {
  ALOGE("dvrTrackingCameraCreate is not implemented.");
  return -ENOSYS;
}

void dvrTrackingCameraDestroy(DvrTrackingCamera*) {
  ALOGE("dvrTrackingCameraDestroy is not implemented.");
}

int dvrTrackingCameraStart(DvrTrackingCamera*, DvrWriteBufferQueue*) {
  ALOGE("dvrTrackingCameraCreate is not implemented.");
  return -ENOSYS;
}

int dvrTrackingCameraStop(DvrTrackingCamera*) {
  ALOGE("dvrTrackingCameraCreate is not implemented.");
  return -ENOSYS;
}

int dvrTrackingFeatureExtractorCreate(DvrTrackingFeatureExtractor**) {
  ALOGE("dvrTrackingFeatureExtractorCreate is not implemented.");
  return -ENOSYS;
}

void dvrTrackingFeatureExtractorDestroy(DvrTrackingFeatureExtractor*) {
  ALOGE("dvrTrackingFeatureExtractorDestroy is not implemented.");
}

int dvrTrackingFeatureExtractorStart(DvrTrackingFeatureExtractor*,
                                     DvrTrackingFeatureCallback, void*) {
  ALOGE("dvrTrackingFeatureExtractorCreate is not implemented.");
  return -ENOSYS;
}

int dvrTrackingFeatureExtractorStop(DvrTrackingFeatureExtractor*) {
  ALOGE("dvrTrackingFeatureExtractorCreate is not implemented.");
  return -ENOSYS;
}

int dvrTrackingFeatureExtractorProcessBuffer(DvrTrackingFeatureExtractor*,
                                             DvrReadBuffer*,
                                             const DvrTrackingBufferMetadata*,
                                             bool*) {
  ALOGE("dvrTrackingFeatureExtractorProcessBuffer is not implemented.");
  return -ENOSYS;
}

int dvrTrackingSensorsCreate(DvrTrackingSensors**, const char*) {
  ALOGE("dvrTrackingSensorsCreate is not implemented.");
  return -ENOSYS;
}

void dvrTrackingSensorsDestroy(DvrTrackingSensors*) {
  ALOGE("dvrTrackingSensorsDestroy is not implemented.");
}

int dvrTrackingSensorsStart(DvrTrackingSensors*, DvrTrackingSensorEventCallback,
                            void*) {
  ALOGE("dvrTrackingStart is not implemented.");
  return -ENOSYS;
}

int dvrTrackingSensorsStop(DvrTrackingSensors*) {
  ALOGE("dvrTrackingStop is not implemented.");
  return -ENOSYS;
}

}  // extern "C"

#endif  // DVR_TRACKING_IMPLEMENTED
