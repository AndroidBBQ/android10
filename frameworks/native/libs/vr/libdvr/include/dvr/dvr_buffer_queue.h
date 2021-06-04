#ifndef ANDROID_DVR_BUFFER_QUEUE_H_
#define ANDROID_DVR_BUFFER_QUEUE_H_

#include <sys/cdefs.h>

#include <dvr/dvr_buffer.h>

__BEGIN_DECLS

typedef struct ANativeWindow ANativeWindow;

typedef struct DvrWriteBufferQueue DvrWriteBufferQueue;
typedef struct DvrReadBufferQueue DvrReadBufferQueue;

// Creates a write buffer queue to be used locally.
//
// Note that this API is mostly for testing purpose. For now there is no
// mechanism to send a DvrWriteBufferQueue cross process. Use
// dvrSurfaceCreateWriteBufferQueue if cross-process buffer transport is
// intended.
//
// @param width The width of the buffers that this queue will produce.
// @param height The height of buffers that this queue will produce.
// @param format The format of the buffers that this queue will produce. This
//     must be one of the AHARDWAREBUFFER_FORMAT_XXX enums.
// @param layer_count The number of layers of the buffers that this queue will
//     produce.
// @param usage The usage of the buffers that this queue will produce. This
//     must a combination of the AHARDWAREBUFFER_USAGE_XXX flags.
// @param capacity The number of buffer that this queue will allocate. Note that
//     all buffers will be allocated on create. Currently, the number of buffers
//     is the queue cannot be changed after creation though DVR API. However,
//     ANativeWindow can choose to reallocate, attach, or detach buffers from
//     a DvrWriteBufferQueue through Android platform logic.
// @param metadata_size The size of metadata in bytes.
// @param out_write_queue The pointer of a DvrWriteBufferQueue will be filled
//      here if the method call succeeds. The metadata size must match
//      the metadata size in dvrWriteBufferPost/dvrReadBufferAcquire.
// @return Zero on success, or negative error code.
int dvrWriteBufferQueueCreate(uint32_t width, uint32_t height, uint32_t format,
                              uint32_t layer_count, uint64_t usage,
                              size_t capacity, size_t metadata_size,
                              DvrWriteBufferQueue** out_write_queue);

// Destroy a write buffer queue.
//
// @param write_queue The DvrWriteBufferQueue of interest.
void dvrWriteBufferQueueDestroy(DvrWriteBufferQueue* write_queue);

// Get the total number of buffers in a write buffer queue.
//
// @param write_queue The DvrWriteBufferQueue of interest.
// @return The capacity on success; or negative error code.
ssize_t dvrWriteBufferQueueGetCapacity(DvrWriteBufferQueue* write_queue);

// Get the system unique queue id of a write buffer queue.
//
// @param write_queue The DvrWriteBufferQueue of interest.
// @return Queue id on success; or negative error code.
int dvrWriteBufferQueueGetId(DvrWriteBufferQueue* write_queue);

// Gets an ANativeWindow backed by the DvrWriteBufferQueue
//
// Can be casted to a Java Surface using ANativeWindow_toSurface NDK API. Note
// that the native window is lazily created at the first time |GetNativeWindow|
// is called, and the created ANativeWindow will be cached so that multiple
// calls to this method will return the same object. Also note that this method
// does not acquire an additional reference to the ANativeWindow returned, don't
// call ANativeWindow_release on it.
//
// @param write_queue The DvrWriteBufferQueue of interest.
// @param out_window The pointer of an ANativeWindow will be filled here if
//     the method call succeeds.
// @return Zero on success; or -EINVAL if this DvrWriteBufferQueue does not
//     support being used as an android Surface.
int dvrWriteBufferQueueGetANativeWindow(DvrWriteBufferQueue* write_queue,
                                        ANativeWindow** out_window);

// Create a read buffer queue from an existing write buffer queue.
//
// @param write_queue The DvrWriteBufferQueue of interest.
// @param out_read_queue The pointer of a DvrReadBufferQueue will be filled here
//     if the method call succeeds.
// @return Zero on success, or negative error code.
int dvrWriteBufferQueueCreateReadQueue(DvrWriteBufferQueue* write_queue,
                                       DvrReadBufferQueue** out_read_queue);

// Gains a buffer to write into.
//
// @param write_queue The DvrWriteBufferQueue to gain buffer from.
// @param timeout Specifies the number of milliseconds that the method will
//     block. Specifying a timeout of -1 causes it to block indefinitely,
//     while specifying a timeout equal to zero cause it to return immediately,
//     even if no buffers are available.
// @param out_buffer A targeting DvrWriteBuffer object to hold the output of the
//     dequeue operation.
// @param out_meta A DvrNativeBufferMetadata object populated by the
//     corresponding dvrReadBufferQueueReleaseBuffer API.
// @param out_fence_fd A sync fence fd defined in NDK's sync.h API, which
//     signals the release of underlying buffer. The producer should wait until
//     this fence clears before writing data into it.
// @return Zero on success, or negative error code.
int dvrWriteBufferQueueGainBuffer(DvrWriteBufferQueue* write_queue, int timeout,
                                  DvrWriteBuffer** out_write_buffer,
                                  DvrNativeBufferMetadata* out_meta,
                                  int* out_fence_fd);

// Posts a buffer and signals its readiness to be read from.
//
// @param write_queue The DvrWriteBufferQueue to post buffer into.
// @param write_buffer The buffer to be posted.
// @param meta The buffer metadata describing the buffer.
// @param ready_fence_fd  A sync fence fd defined in NDK's sync.h API, which
//     signals the readdiness of underlying buffer. When a valid fence gets
//     passed in, the consumer will wait the fence to be ready before it starts
//     to ready from the buffer.
// @return Zero on success, or negative error code.
int dvrWriteBufferQueuePostBuffer(DvrWriteBufferQueue* write_queue,
                                  DvrWriteBuffer* write_buffer,
                                  const DvrNativeBufferMetadata* meta,
                                  int ready_fence_fd);

// Overrides buffer dimension with new width and height.
//
// After the call successfully returns, each |dvrWriteBufferQueueDequeue| call
// will return buffer with newly assigned |width| and |height|. When necessary,
// old buffer will be removed from the buffer queue and replaced with new buffer
// matching the new buffer size.
//
// @param write_queue The DvrWriteBufferQueue of interest.
// @param width Desired width, cannot be Zero.
// @param height Desired height, cannot be Zero.
// @return Zero on success, or negative error code.
int dvrWriteBufferQueueResizeBuffer(DvrWriteBufferQueue* write_queue,
                                    uint32_t width, uint32_t height);

// Destroy a read buffer queue.
//
// @param read_queue The DvrReadBufferQueue of interest.
void dvrReadBufferQueueDestroy(DvrReadBufferQueue* read_queue);

// Get the total number of buffers in a read buffer queue.
//
// @param read_queue The DvrReadBufferQueue of interest.
// @return The capacity on success; or negative error code.
ssize_t dvrReadBufferQueueGetCapacity(DvrReadBufferQueue* read_queue);

// Get the system unique queue id of a read buffer queue.
//
// @param read_queue The DvrReadBufferQueue of interest.
// @return Queue id on success; or negative error code.
int dvrReadBufferQueueGetId(DvrReadBufferQueue* read_queue);

// Get the event fd that signals when queue updates occur.
//
// Use ReadBufferQueueHandleEvents to trigger registered event callbacks.
//
// @param read_queue The DvrReadBufferQueue of interest.
// @return Fd on success; or negative error code.
int dvrReadBufferQueueGetEventFd(DvrReadBufferQueue* read_queue);

// Create a read buffer queue from an existing read buffer queue.
//
// @param read_queue The DvrReadBufferQueue of interest.
// @param out_read_queue The pointer of a DvrReadBufferQueue will be filled here
//     if the method call succeeds.
// @return Zero on success, or negative error code.
int dvrReadBufferQueueCreateReadQueue(DvrReadBufferQueue* read_queue,
                                      DvrReadBufferQueue** out_read_queue);

// Dequeues a buffer to read from.
//
// @param read_queue The DvrReadBufferQueue to acquire buffer from.
// @param timeout Specifies the number of milliseconds that the method will
//     block. Specifying a timeout of -1 causes it to block indefinitely,
//     while specifying a timeout equal to zero cause it to return immediately,
//     even if no buffers are available.
// @param out_buffer A targeting DvrReadBuffer object to hold the output of the
//     dequeue operation. Must be created by |dvrReadBufferCreateEmpty|.
// @param out_meta A DvrNativeBufferMetadata object populated by the
//     corresponding dvrWriteBufferQueuePostBuffer API.
// @param out_fence_fd A sync fence fd defined in NDK's sync.h API, which
//     signals the release of underlying buffer. The consumer should wait until
//     this fence clears before reading data from it.
// @return Zero on success, or negative error code.
int dvrReadBufferQueueAcquireBuffer(DvrReadBufferQueue* read_queue, int timeout,
                                    DvrReadBuffer** out_read_buffer,
                                    DvrNativeBufferMetadata* out_meta,
                                    int* out_fence_fd);

// Releases a buffer and signals its readiness to be written into.
//
// @param read_queue The DvrReadBufferQueue to release buffer into.
// @param read_buffer The buffer to be released.
// @param meta The buffer metadata describing the buffer.
// @param release_fence_fd A sync fence fd defined in NDK's sync.h API, which
//     signals the readdiness of underlying buffer. When a valid fence gets
//     passed in, the producer will wait the fence to be ready before it starts
//     to write into the buffer again.
// @return Zero on success, or negative error code.
int dvrReadBufferQueueReleaseBuffer(DvrReadBufferQueue* read_queue,
                                    DvrReadBuffer* read_buffer,
                                    const DvrNativeBufferMetadata* meta,
                                    int release_fence_fd);

// Callback function which will be called when a buffer is avaiable.
//
// Note that there is no guarantee of thread safety and on which thread the
// callback will be fired.
//
// @param context User provided opaque pointer.
typedef void (*DvrReadBufferQueueBufferAvailableCallback)(void* context);

// Set buffer avaiable callback.
//
// @param read_queue The DvrReadBufferQueue of interest.
// @param callback The callback function. Set this to NULL if caller no longer
//     needs to listen to new buffer available events.
// @param context User provided opaque pointer, will be passed back during
//     callback. The caller is responsible for ensuring the validity of the
//     context through the life cycle of the DvrReadBufferQueue.
// @return Zero on success, or negative error code.
int dvrReadBufferQueueSetBufferAvailableCallback(
    DvrReadBufferQueue* read_queue,
    DvrReadBufferQueueBufferAvailableCallback callback, void* context);

// Callback function which will be called when a buffer is about to be removed.
//
// Note that there is no guarantee of thread safety and on which thread the
// callback will be fired.
//
// @param buffer The buffer being removed. Once the callbacks returns, this
//     buffer will be dereferenced from the buffer queue. If user has ever
//     cached other DvrReadBuffer/AHardwareBuffer/EglImageKHR objects derived
//     from this buffer, it's the user's responsibility to clean them up.
//     Note that the ownership of the read buffer is not passed to this
//     callback, so it should call dvrReadBufferDestroy on the buffer.
// @param context User provided opaque pointer.
typedef void (*DvrReadBufferQueueBufferRemovedCallback)(DvrReadBuffer* buffer,
                                                        void* context);

// Set buffer removed callback.
//
// @param read_queue The DvrReadBufferQueue of interest.
// @param callback The callback function. Set this to NULL if caller no longer
//     needs to listen to buffer removed events.
// @param context User provided opaque pointer, will be passed back during
//     callback. The caller is responsible for ensuring the validity of the
//     context through the life cycle of the DvrReadBufferQueue.
// @return Zero on success, or negative error code.
int dvrReadBufferQueueSetBufferRemovedCallback(
    DvrReadBufferQueue* read_queue,
    DvrReadBufferQueueBufferRemovedCallback callback, void* context);

// Handle all pending events on the read queue.
//
// @param read_queue The DvrReadBufferQueue of interest.
// @return Zero on success, or negative error code.
int dvrReadBufferQueueHandleEvents(DvrReadBufferQueue* read_queue);

__END_DECLS

#endif  // ANDROID_DVR_BUFFER_QUEUE_H_
