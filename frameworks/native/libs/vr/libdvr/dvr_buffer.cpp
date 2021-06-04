#include "include/dvr/dvr_buffer.h"

#include <android/hardware_buffer.h>
#include <dvr/dvr_shared_buffers.h>
#include <private/dvr/consumer_buffer.h>
#include <private/dvr/producer_buffer.h>
#include <ui/GraphicBuffer.h>

#include "dvr_internal.h"

using namespace android;

namespace android {
namespace dvr {

DvrBuffer* CreateDvrBufferFromIonBuffer(
    const std::shared_ptr<IonBuffer>& ion_buffer) {
  if (!ion_buffer)
    return nullptr;
  return new DvrBuffer{std::move(ion_buffer)};
}

}  // namespace dvr
}  // namespace android

namespace {

int ConvertToAHardwareBuffer(GraphicBuffer* graphic_buffer,
                             AHardwareBuffer** hardware_buffer) {
  if (!hardware_buffer || !graphic_buffer) {
    return -EINVAL;
  }
  *hardware_buffer = reinterpret_cast<AHardwareBuffer*>(graphic_buffer);
  AHardwareBuffer_acquire(*hardware_buffer);
  return 0;
}

}  // anonymous namespace

extern "C" {

void dvrWriteBufferDestroy(DvrWriteBuffer* write_buffer) {
  if (write_buffer != nullptr) {
    ALOGW_IF(
        write_buffer->slot != -1,
        "dvrWriteBufferDestroy: Destroying a buffer associated with a valid "
        "buffer queue slot. This may indicate possible leaks, buffer_id=%d.",
        dvrWriteBufferGetId(write_buffer));
    delete write_buffer;
  }
}

int dvrWriteBufferIsValid(DvrWriteBuffer* write_buffer) {
  return write_buffer && write_buffer->write_buffer;
}

int dvrWriteBufferGetId(DvrWriteBuffer* write_buffer) {
  if (!write_buffer || !write_buffer->write_buffer)
    return -EINVAL;

  return write_buffer->write_buffer->id();
}

int dvrWriteBufferGetAHardwareBuffer(DvrWriteBuffer* write_buffer,
                                     AHardwareBuffer** hardware_buffer) {
  if (!write_buffer || !write_buffer->write_buffer)
    return -EINVAL;

  return ConvertToAHardwareBuffer(
      write_buffer->write_buffer->buffer()->buffer().get(), hardware_buffer);
}

void dvrReadBufferDestroy(DvrReadBuffer* read_buffer) {
  if (read_buffer != nullptr) {
    ALOGW_IF(
        read_buffer->slot != -1,
        "dvrReadBufferDestroy: Destroying a buffer associated with a valid "
        "buffer queue slot. This may indicate possible leaks, buffer_id=%d.",
        dvrReadBufferGetId(read_buffer));
    delete read_buffer;
  }
}

int dvrReadBufferIsValid(DvrReadBuffer* read_buffer) {
  return read_buffer && read_buffer->read_buffer;
}

int dvrReadBufferGetId(DvrReadBuffer* read_buffer) {
  if (!read_buffer || !read_buffer->read_buffer)
    return -EINVAL;

  return read_buffer->read_buffer->id();
}

int dvrReadBufferGetAHardwareBuffer(DvrReadBuffer* read_buffer,
                                    AHardwareBuffer** hardware_buffer) {
  if (!read_buffer || !read_buffer->read_buffer)
    return -EINVAL;

  return ConvertToAHardwareBuffer(
      read_buffer->read_buffer->buffer()->buffer().get(), hardware_buffer);
}

void dvrBufferDestroy(DvrBuffer* buffer) { delete buffer; }

int dvrBufferGetAHardwareBuffer(DvrBuffer* buffer,
                                AHardwareBuffer** hardware_buffer) {
  if (!buffer || !buffer->buffer || !hardware_buffer) {
    return -EINVAL;
  }

  return ConvertToAHardwareBuffer(buffer->buffer->buffer().get(),
                                  hardware_buffer);
}

// Retrieve the shared buffer layout version defined in dvr_shared_buffers.h.
int dvrBufferGlobalLayoutVersionGet() {
  return android::dvr::kSharedBufferLayoutVersion;
}

}  // extern "C"
