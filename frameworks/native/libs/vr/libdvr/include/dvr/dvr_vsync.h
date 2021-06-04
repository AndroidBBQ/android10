#ifndef ANDROID_DVR_VSYNC_H_
#define ANDROID_DVR_VSYNC_H_

#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

// Represents a vsync sample. The size of this struct is 32 bytes.
typedef struct __attribute__((packed, aligned(16))) DvrVsync {
  // The timestamp for the last vsync in nanoseconds.
  uint64_t vsync_timestamp_ns;

  // The index of the last vsync.
  uint32_t vsync_count;

  // Scan out for the left eye = vsync_timestamp_ns + vsync_left_eye_offset_ns.
  int32_t vsync_left_eye_offset_ns;

  // Scan out for the right eye = vsync_timestamp_ns + vsync_right_eye_offset_ns
  int32_t vsync_right_eye_offset_ns;

  // The period of a vsync in nanoseconds.
  uint32_t vsync_period_ns;

  // Padding to 32 bytes so the size is a multiple of 16.
  uint8_t padding[8];
} DvrVsync;

__END_DECLS

#endif  // ANDROID_DVR_VSYNC_H_
