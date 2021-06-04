#ifndef ANDROID_DVR_DISPLAY_TYPES_H_
#define ANDROID_DVR_DISPLAY_TYPES_H_

#include <sys/cdefs.h>

__BEGIN_DECLS

// Define types used in pose buffer fields. These types have atomicity
// guarantees that are useful in lock-free shared memory ring buffers.
#ifdef __ARM_NEON
#include <arm_neon.h>
#else
#ifndef __FLOAT32X4T_86
#define __FLOAT32X4T_86
typedef float float32x4_t __attribute__((__vector_size__(16)));
typedef struct float32x4x4_t { float32x4_t val[4]; } float32x4x4_t;
#endif
#endif

// VrFlinger display manager surface state snapshots per surface flags
// indicating what changed since the last snapshot.
enum {
  // No changes.
  DVR_SURFACE_UPDATE_FLAGS_NONE = 0,
  // This surface is new.
  DVR_SURFACE_UPDATE_FLAGS_NEW_SURFACE = (1 << 0),
  // Buffer queues added/removed.
  DVR_SURFACE_UPDATE_FLAGS_BUFFERS_CHANGED = (1 << 1),
  // Visibility/z-order changed.
  DVR_SURFACE_UPDATE_FLAGS_VISIBILITY_CHANGED = (1 << 2),
  // Generic attributes changed.
  DVR_SURFACE_UPDATE_FLAGS_ATTRIBUTES_CHANGED = (1 << 3),
};

// Surface attribute keys. VrFlinger defines keys in the negative integer space.
// The compositor is free to use keys in the positive integer space for
// implementation-defined purposes.
enum {
  // DIRECT: bool
  // Determines whether a direct surface is created (compositor output) or an
  // application surface. Defaults to false (application surface). May only be
  // set to true by a process with either UID=root or UID validated with
  // IsTrustedUid() (VrCore).
  DVR_SURFACE_ATTRIBUTE_DIRECT = -3,
  // Z_ORDER: int32_t
  // Interpreted by VrFlinger only on direct surfaces to order the corresponding
  // hardware layers. More positive values render on top of more negative
  // values.
  DVR_SURFACE_ATTRIBUTE_Z_ORDER = -2,
  // VISIBLE: bool
  // Interpreted by VrFlinger only on direct surfaces to determine whether a
  // surface is assigned to a hardware layer or ignored.
  DVR_SURFACE_ATTRIBUTE_VISIBLE = -1,
  // INVALID
  // Invalid key. No attributes should have this key.
  DVR_SURFACE_ATTRIBUTE_INVALID = 0,
  // FIRST_USER_KEY
  // VrFlinger ingores any keys with this value or greater, passing them to the
  // compositor through surface state query results.
  DVR_SURFACE_ATTRIBUTE_FIRST_USER_KEY = 1,
};

__END_DECLS

#endif  // ANDROID_DVR_DISPLAY_TYPES_H_
