#ifndef ANDROID_DVR_DELETER_H_
#define ANDROID_DVR_DELETER_H_

#include <sys/cdefs.h>

#include <memory>

// Header-only C++ helper to delete opaque DVR objects.

__BEGIN_DECLS

// Use forward declarations to avoid dependency on other headers.
typedef struct DvrBuffer DvrBuffer;
typedef struct DvrReadBuffer DvrReadBuffer;
typedef struct DvrWriteBuffer DvrWriteBuffer;
typedef struct DvrReadBufferQueue DvrReadBufferQueue;
typedef struct DvrWriteBufferQueue DvrWriteBufferQueue;
typedef struct DvrDisplayManager DvrDisplayManager;
typedef struct DvrSurfaceState DvrSurfaceState;
typedef struct DvrSurface DvrSurface;
typedef struct DvrHwcClient DvrHwcClient;
typedef struct DvrHwcFrame DvrHwcFrame;

void dvrBufferDestroy(DvrBuffer* buffer);
void dvrReadBufferDestroy(DvrReadBuffer* read_buffer);
void dvrWriteBufferDestroy(DvrWriteBuffer* write_buffer);
void dvrReadBufferQueueDestroy(DvrReadBufferQueue* read_queue);
void dvrWriteBufferQueueDestroy(DvrWriteBufferQueue* write_queue);
void dvrDisplayManagerDestroy(DvrDisplayManager* client);
void dvrSurfaceStateDestroy(DvrSurfaceState* surface_state);
void dvrSurfaceDestroy(DvrSurface* surface);
void dvrHwcClientDestroy(DvrHwcClient* client);
void dvrHwcFrameDestroy(DvrHwcFrame* frame);

__END_DECLS

// Avoid errors if this header is included in C code.
#if defined(__cplusplus)

namespace android {
namespace dvr {

// Universal DVR object deleter. May be passed to smart pointer types to handle
// deletion of DVR API objects.
struct DvrObjectDeleter {
  void operator()(DvrBuffer* p) { dvrBufferDestroy(p); }
  void operator()(DvrReadBuffer* p) { dvrReadBufferDestroy(p); }
  void operator()(DvrWriteBuffer* p) { dvrWriteBufferDestroy(p); }
  void operator()(DvrReadBufferQueue* p) { dvrReadBufferQueueDestroy(p); }
  void operator()(DvrWriteBufferQueue* p) { dvrWriteBufferQueueDestroy(p); }
  void operator()(DvrDisplayManager* p) { dvrDisplayManagerDestroy(p); }
  void operator()(DvrSurfaceState* p) { dvrSurfaceStateDestroy(p); }
  void operator()(DvrSurface* p) { dvrSurfaceDestroy(p); }
  void operator()(DvrHwcClient* p) { dvrHwcClientDestroy(p); }
  void operator()(DvrHwcFrame* p) { dvrHwcFrameDestroy(p); }
};

// Helper to define unique pointers for DVR object types.
template <typename T>
using MakeUniqueDvrPointer = std::unique_ptr<T, DvrObjectDeleter>;

// Unique pointer types for DVR objects.
using UniqueDvrBuffer = MakeUniqueDvrPointer<DvrBuffer>;
using UniqueDvrReadBuffer = MakeUniqueDvrPointer<DvrReadBuffer>;
using UniqueDvrWriteBuffer = MakeUniqueDvrPointer<DvrWriteBuffer>;
using UniqueDvrReadBufferQueue = MakeUniqueDvrPointer<DvrReadBufferQueue>;
using UniqueDvrWriteBufferQueue = MakeUniqueDvrPointer<DvrWriteBufferQueue>;
using UniqueDvrDisplayManager = MakeUniqueDvrPointer<DvrDisplayManager>;
using UniqueDvrSurfaceState = MakeUniqueDvrPointer<DvrSurfaceState>;
using UniqueDvrSurface = MakeUniqueDvrPointer<DvrSurface>;
using UniqueDvrHwcClient = MakeUniqueDvrPointer<DvrHwcClient>;
using UniqueDvrHwcFrame = MakeUniqueDvrPointer<DvrHwcFrame>;

// TODO(eieio): Add an adapter for std::shared_ptr that injects the deleter into
// the relevant constructors.

}  // namespace dvr
}  // namespace android

#endif // defined(__cplusplus)

#endif  // ANDROID_DVR_DELETER_H_
