#ifndef ANDROID_DVR_TRACKING_TYPES_H_
#define ANDROID_DVR_TRACKING_TYPES_H_

#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

typedef struct DvrTrackingBufferMetadata {
  // Specifies the source of this image.
  uint32_t camera_mask;
  // Specifies the memory format of this image.
  uint32_t format;
  /// The width of the image data.
  uint32_t width;
  /// The height of the image data.
  uint32_t height;
  /// The number of bytes per scanline of image data.
  uint32_t stride;
  /// The frame number of this image.
  int32_t frame_number;
  /// The timestamp of this image in nanoseconds. Taken in the middle of the
  /// exposure interval.
  int64_t timestamp_ns;
  // This is the timestamp for recording when the system using the HAL
  // received the callback.  It will not be populated by the HAL.
  int64_t callback_timestamp_ns;
  /// The exposure duration of this image in nanoseconds.
  int64_t exposure_duration_ns;
} DvrTrackingBufferMetadata;

// Represents a set of features extracted from a camera frame. Note that this
// should be in sync with TangoHalCallbacks defined in tango-hal.h.
typedef struct DvrTrackingFeatures {
  // Specifies the source of the features.
  uint32_t camera_mask;

  // This is unused.
  uint32_t unused;

  // The timestamp in nanoseconds from the image that generated the features.
  // Taken in the middle of the exposure interval.
  int64_t timestamp_ns;

  // This is the timestamp for recording when the system using the HAL
  // received the callback.  It will not be populated by the HAL.
  int64_t callback_timestamp_ns;

  // The frame number from the image that generated the features.
  int64_t frame_number;

  // The number of features.
  int count;

  // An array of 2D image points for each feature in the current image.
  // This is sub-pixel refined extremum location at the fine resolution.
  float (*positions)[2];

  // The id of these measurements.
  int32_t* ids;

  // The feature descriptors.
  uint64_t (*descriptors)[8];

  // Laplacian scores for each feature.
  float* scores;

  // Is this feature a minimum or maximum in the Laplacian image.
  // 0 if the feature is a maximum, 1 if it is a minimum.
  int32_t* is_minimum;

  // This corresponds to the sub-pixel index of the laplacian image
  // that the extremum was found.
  float* scales;

  // Computed orientation of keypoint as part of FREAK extraction, except
  // it's represented in radians and measured anti-clockwise.
  float* angles;

  // Edge scores for each feature.
  float* edge_scores;
} DvrTrackingFeatures;

// Represents a sensor event.
typedef struct DvrTrackingSensorEvent {
  // The sensor type.
  int32_t sensor;

  // Event type.
  int32_t type;

  // This is the timestamp recorded from the device. Taken in the middle
  // of the integration interval and adjusted for any low pass filtering.
  int64_t timestamp_ns;

  // The event data.
  float x;
  float y;
  float z;
} DvrTrackingSensorEvent;

__END_DECLS

#endif  // ANDROID_DVR_TRACKING_TYPES_H_
