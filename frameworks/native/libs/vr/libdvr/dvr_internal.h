#ifndef ANDROID_DVR_INTERNAL_H_
#define ANDROID_DVR_INTERNAL_H_

#include <sys/cdefs.h>

#include <memory>

extern "C" {

typedef struct DvrBuffer DvrBuffer;
typedef struct DvrReadBuffer DvrReadBuffer;
typedef struct DvrWriteBuffer DvrWriteBuffer;

}  // extern "C"

namespace android {
namespace dvr {

class IonBuffer;

DvrBuffer* CreateDvrBufferFromIonBuffer(
    const std::shared_ptr<IonBuffer>& ion_buffer);

}  // namespace dvr
}  // namespace android

extern "C" {

struct DvrWriteBuffer {
  // The slot nubmer of the buffer, a valid slot number must be in the range of
  // [0, android::BufferQueueDefs::NUM_BUFFER_SLOTS). This is only valid for
  // DvrWriteBuffer acquired from a DvrWriteBufferQueue.
  int32_t slot = -1;

  std::shared_ptr<android::dvr::ProducerBuffer> write_buffer;
};

struct DvrReadBuffer {
  // The slot nubmer of the buffer, a valid slot number must be in the range of
  // [0, android::BufferQueueDefs::NUM_BUFFER_SLOTS). This is only valid for
  // DvrReadBuffer acquired from a DvrReadBufferQueue.
  int32_t slot = -1;

  std::shared_ptr<android::dvr::ConsumerBuffer> read_buffer;
};

struct DvrBuffer {
  std::shared_ptr<android::dvr::IonBuffer> buffer;
};

}  // extern "C"

#endif  // ANDROID_DVR_INTERNAL_H_
