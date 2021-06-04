#ifndef ANDROID_DVR_CONFIG_H
#define ANDROID_DVR_CONFIG_H

// This header is shared by VrCore and Android and must be kept in sync.

#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

// This is a shared memory buffer for passing config data from VrCore to
// libvrflinger in SurfaceFlinger.
struct __attribute__((packed, aligned(16))) DvrConfig {
  // Offset before vsync to submit frames to hardware composer.
  int32_t frame_post_offset_ns{4000000};

  // If the number of pending fences goes over this count at the point when we
  // are about to submit a new frame to HWC, we will drop the frame. This
  // should be a signal that the display driver has begun queuing frames. Note
  // that with smart displays (with RAM), the fence is signaled earlier than
  // the next vsync, at the point when the DMA to the display completes.
  // Currently we use a smart display and the EDS timing coincides with zero
  // pending fences, so this is 0.
  int32_t allowed_pending_fence_count{0};

  // New fields should always be added to the end for backwards compat.

  // Reserved padding to 16 bytes.
  uint8_t pad[8];
};

__END_DECLS

#endif  // ANDROID_DVR_CONFIG_H
