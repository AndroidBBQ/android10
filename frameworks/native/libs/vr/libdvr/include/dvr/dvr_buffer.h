#ifndef ANDROID_DVR_BUFFER_H_
#define ANDROID_DVR_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <memory>

__BEGIN_DECLS

typedef struct DvrWriteBuffer DvrWriteBuffer;
typedef struct DvrReadBuffer DvrReadBuffer;
typedef struct DvrBuffer DvrBuffer;
typedef struct AHardwareBuffer AHardwareBuffer;
struct native_handle;

// Destroys the write buffer.
void dvrWriteBufferDestroy(DvrWriteBuffer* write_buffer);

// Returns 1 if the given write buffer object contains a buffer, 0 otherwise.
int dvrWriteBufferIsValid(DvrWriteBuffer* write_buffer);

// Returns the global BufferHub id of this buffer.
int dvrWriteBufferGetId(DvrWriteBuffer* write_buffer);

// Returns an AHardwareBuffer for the underlying buffer.
// Caller must call AHardwareBuffer_release on hardware_buffer.
int dvrWriteBufferGetAHardwareBuffer(DvrWriteBuffer* write_buffer,
                                     AHardwareBuffer** hardware_buffer);

// Destroys the read buffer.
void dvrReadBufferDestroy(DvrReadBuffer* read_buffer);

// Returns 1 if the given write buffer object contains a buffer, 0 otherwise.
int dvrReadBufferIsValid(DvrReadBuffer* read_buffer);

// Returns the global BufferHub id of this buffer.
int dvrReadBufferGetId(DvrReadBuffer* read_buffer);

// Returns an AHardwareBuffer for the underlying buffer.
// Caller must call AHardwareBuffer_release on hardware_buffer.
int dvrReadBufferGetAHardwareBuffer(DvrReadBuffer* read_buffer,
                                    AHardwareBuffer** hardware_buffer);

// Destroys the buffer.
void dvrBufferDestroy(DvrBuffer* buffer);

// Gets an AHardwareBuffer from the buffer.
// Caller must call AHardwareBuffer_release on hardware_buffer.
int dvrBufferGetAHardwareBuffer(DvrBuffer* buffer,
                                AHardwareBuffer** hardware_buffer);

// Retrieve the shared buffer layout version defined in dvr_shared_buffers.h.
int dvrBufferGlobalLayoutVersionGet();

__END_DECLS

#endif  // ANDROID_DVR_BUFFER_H_
