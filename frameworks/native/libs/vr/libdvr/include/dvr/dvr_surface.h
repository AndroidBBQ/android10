#ifndef ANDROID_DVR_SURFACE_H_
#define ANDROID_DVR_SURFACE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

#include <dvr/dvr_api.h>
#include <dvr/dvr_buffer.h>
#include <dvr/dvr_buffer_queue.h>
#include <dvr/dvr_display_types.h>

__BEGIN_DECLS

// Attribute types. The values are one-hot encoded to support singluar types or
// masks of supported types.
enum {
  DVR_SURFACE_ATTRIBUTE_TYPE_NONE = 0,
  DVR_SURFACE_ATTRIBUTE_TYPE_INT32 = (1 << 0),
  DVR_SURFACE_ATTRIBUTE_TYPE_INT64 = (1 << 1),
  DVR_SURFACE_ATTRIBUTE_TYPE_BOOL = (1 << 2),
  DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT = (1 << 3),
  DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT2 = (1 << 4),
  DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT3 = (1 << 5),
  DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT4 = (1 << 6),
  DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT8 = (1 << 7),
  DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT16 = (1 << 8),
};

typedef uint64_t DvrSurfaceAttributeType;
typedef int32_t DvrSurfaceAttributeKey;

typedef struct DvrSurfaceAttributeValue {
  DvrSurfaceAttributeType type;
  union {
    int32_t int32_value;
    int64_t int64_value;
    bool bool_value;
    float float_value;
    float float2_value[2];
    float float3_value[3];
    float float4_value[4];
    float float8_value[8];
    float float16_value[16];
  };
} DvrSurfaceAttributeValue;

typedef struct DvrSurfaceAttribute {
  DvrSurfaceAttributeKey key;
  DvrSurfaceAttributeValue value;
} DvrSurfaceAttribute;

// Creates a new display surface with the given attributes.
// @return 0 on success. Otherwise returns a negative error value.
int dvrSurfaceCreate(const DvrSurfaceAttribute* attributes,
                     size_t attribute_count, DvrSurface** surface_out);

// Destroys the display surface.
void dvrSurfaceDestroy(DvrSurface* surface);

// Gets the DisplayService global id for this surface.
int dvrSurfaceGetId(DvrSurface* surface);

// Sets attributes on the given display surface.
// @return 0 on success. Otherwise returns a negative error value.
int dvrSurfaceSetAttributes(DvrSurface* surface,
                            const DvrSurfaceAttribute* attributes,
                            size_t attribute_count);

// Creates a new write-side buffer queue on the given surface. Direct surfaces
// may only have one queue, the latest call replacing any prior queue. Replaced
// queues are still referenced and should be destryoed using the queue destroy
// API.
// @return 0 on success. Otherwise returns a negative error value.
int dvrSurfaceCreateWriteBufferQueue(DvrSurface* surface, uint32_t width,
                                     uint32_t height, uint32_t format,
                                     uint32_t layer_count, uint64_t usage,
                                     size_t capacity, size_t metadata_size,
                                     DvrWriteBufferQueue** queue_out);

// Sets up a named buffer for shared memory data transfer between display
// clients and the system. Protected API that may only be called with sufficient
// privilege.
// @return 0 on success. Otherwise returns a negative error value.
int dvrSetupGlobalBuffer(DvrGlobalBufferKey key, size_t size, uint64_t usage,
                         DvrBuffer** buffer_out);

// Deletes a named buffer. WARNING: This is dangerous because any existing
// clients of this buffer will not be notified and will remain attached to
// the old buffer. This is useful for tests, but probably not for production
// code.
// @return 0 on success. Otherwise returns a negative error value.
int dvrDeleteGlobalBuffer(DvrGlobalBufferKey key);

// Get a global buffer from the display service.
// @return 0 on success. Otherwise returns a negative error value.
int dvrGetGlobalBuffer(DvrGlobalBufferKey key, DvrBuffer** out_buffer);

// Read the native device display metrics as reported by the hardware composer.
// This is useful as otherwise the device metrics are only reported as
// relative to the current device orientation.
// @param sizeof_metrics the size of the passed in metrics struct. This is used
//   to ensure we don't break each other during active development.
// @param metrics on success holds the retrieved device metrics.
// @return 0 on success. Otherwise returns a negative error value (typically
//   this means the display service is not available).
int dvrGetNativeDisplayMetrics(size_t metrics_struct_size,
                               DvrNativeDisplayMetrics* metrics);

__END_DECLS

#endif  // ANDROID_DVR_SURFACE_H_
