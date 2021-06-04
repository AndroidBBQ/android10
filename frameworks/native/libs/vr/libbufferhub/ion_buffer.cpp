#include <private/dvr/ion_buffer.h>

#include <log/log.h>
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

#include <mutex>

namespace {

constexpr uint32_t kDefaultGraphicBufferLayerCount = 1;

}  // anonymous namespace

namespace android {
namespace dvr {

IonBuffer::IonBuffer() : IonBuffer(nullptr, 0, 0, 0, 0, 0, 0) {}

IonBuffer::IonBuffer(uint32_t width, uint32_t height, uint32_t format,
                     uint64_t usage)
    : IonBuffer() {
  Alloc(width, height, kDefaultGraphicBufferLayerCount, format, usage);
}

IonBuffer::IonBuffer(buffer_handle_t handle, uint32_t width, uint32_t height,
                     uint32_t stride, uint32_t format, uint64_t usage)
    : IonBuffer(handle, width, height, kDefaultGraphicBufferLayerCount, stride,
                format, usage) {}

IonBuffer::IonBuffer(buffer_handle_t handle, uint32_t width, uint32_t height,
                     uint32_t layer_count, uint32_t stride, uint32_t format,
                     uint64_t usage)
    : buffer_(nullptr) {
  ALOGD_IF(TRACE,
           "IonBuffer::IonBuffer: handle=%p width=%u height=%u layer_count=%u "
           "stride=%u format=%u usage=%" PRIx64,
           handle, width, height, layer_count, stride, format, usage);
  if (handle != 0) {
    Import(handle, width, height, layer_count, stride, format, usage);
  }
}

IonBuffer::~IonBuffer() {
  ALOGD_IF(TRACE,
           "IonBuffer::~IonBuffer: handle=%p width=%u height=%u stride=%u "
           "format=%u usage=%" PRIx64,
           handle(), width(), height(), stride(), format(), usage());
  FreeHandle();
}

IonBuffer::IonBuffer(IonBuffer&& other) noexcept : IonBuffer() {
  *this = std::move(other);
}

IonBuffer& IonBuffer::operator=(IonBuffer&& other) noexcept {
  ALOGD_IF(TRACE, "IonBuffer::operator=: handle_=%p other.handle_=%p", handle(),
           other.handle());

  if (this != &other) {
    buffer_ = other.buffer_;
    other.FreeHandle();
  }
  return *this;
}

void IonBuffer::FreeHandle() {
  if (buffer_.get()) {
    // GraphicBuffer unregisters and cleans up the handle if needed
    buffer_ = nullptr;
  }
}

int IonBuffer::Alloc(uint32_t width, uint32_t height, uint32_t layer_count,
                     uint32_t format, uint64_t usage) {
  ALOGD_IF(TRACE,
           "IonBuffer::Alloc: width=%u height=%u layer_count=%u format=%u "
           "usage=%" PRIx64, width, height, layer_count, format, usage);

  sp<GraphicBuffer> buffer =
      new GraphicBuffer(width, height, format, layer_count, usage);
  if (buffer->initCheck() != OK) {
    ALOGE("IonBuffer::Aloc: Failed to allocate buffer");
    return -EINVAL;
  } else {
    buffer_ = buffer;
    return 0;
  }
}

void IonBuffer::Reset(buffer_handle_t handle, uint32_t width, uint32_t height,
                      uint32_t layer_count, uint32_t stride, uint32_t format,
                      uint64_t usage) {
  ALOGD_IF(TRACE,
           "IonBuffer::Reset: handle=%p width=%u height=%u layer_count=%u "
           "stride=%u format=%u usage=%" PRIx64,
           handle, width, height, layer_count, stride, format, usage);
  Import(handle, width, height, layer_count, stride, format, usage);
}

int IonBuffer::Import(buffer_handle_t handle, uint32_t width, uint32_t height,
                      uint32_t layer_count, uint32_t stride, uint32_t format,
                      uint64_t usage) {
  ATRACE_NAME("IonBuffer::Import1");
  ALOGD_IF(TRACE,
           "IonBuffer::Import: handle=%p width=%u height=%u layer_count=%u "
           "stride=%u format=%u usage=%" PRIx64,
           handle, width, height, layer_count, stride, format, usage);
  FreeHandle();
  sp<GraphicBuffer> buffer =
      new GraphicBuffer(handle, GraphicBuffer::TAKE_UNREGISTERED_HANDLE, width,
                        height, format, layer_count, usage, stride);
  if (buffer->initCheck() != OK) {
    ALOGE("IonBuffer::Import: Failed to import buffer");
    return -EINVAL;
  } else {
    buffer_ = buffer;
    return 0;
  }
}

int IonBuffer::Import(const int* fd_array, int fd_count, const int* int_array,
                      int int_count, uint32_t width, uint32_t height,
                      uint32_t layer_count, uint32_t stride, uint32_t format,
                      uint64_t usage) {
  ATRACE_NAME("IonBuffer::Import2");
  ALOGD_IF(TRACE,
           "IonBuffer::Import: fd_count=%d int_count=%d width=%u height=%u "
           "layer_count=%u stride=%u format=%u usage=%" PRIx64,
           fd_count, int_count, width, height, layer_count, stride, format,
           usage);

  if (fd_count < 0 || int_count < 0) {
    ALOGE("IonBuffer::Import: invalid arguments.");
    return -EINVAL;
  }

  native_handle_t* handle = native_handle_create(fd_count, int_count);
  if (!handle) {
    ALOGE("IonBuffer::Import: failed to create new native handle.");
    return -ENOMEM;
  }

  // Copy fd_array into the first part of handle->data and int_array right
  // after it.
  memcpy(handle->data, fd_array, sizeof(int) * fd_count);
  memcpy(handle->data + fd_count, int_array, sizeof(int) * int_count);

  const int ret =
      Import(handle, width, height, layer_count, stride, format, usage);
  if (ret < 0) {
    ALOGE("IonBuffer::Import: failed to import raw native handle: %s",
          strerror(-ret));
    native_handle_close(handle);
    native_handle_delete(handle);
  }

  return ret;
}

int IonBuffer::Duplicate(const IonBuffer* other) {
  if (!other->handle())
    return -EINVAL;

  const int fd_count = other->handle()->numFds;
  const int int_count = other->handle()->numInts;

  if (fd_count < 0 || int_count < 0)
    return -EINVAL;

  native_handle_t* handle = native_handle_create(fd_count, int_count);
  if (!handle) {
    ALOGE("IonBuffer::Duplicate: Failed to create new native handle.");
    return -ENOMEM;
  }

  // Duplicate the file descriptors from the other native handle.
  for (int i = 0; i < fd_count; i++)
    handle->data[i] = dup(other->handle()->data[i]);

  // Copy the ints after the file descriptors.
  memcpy(handle->data + fd_count, other->handle()->data + fd_count,
         sizeof(int) * int_count);

  const int ret =
      Import(handle, other->width(), other->height(), other->layer_count(),
             other->stride(), other->format(), other->usage());
  if (ret < 0) {
    ALOGE("IonBuffer::Duplicate: Failed to import duplicate native handle: %s",
          strerror(-ret));
    native_handle_close(handle);
    native_handle_delete(handle);
  }

  return ret;
}

int IonBuffer::Lock(uint32_t usage, int x, int y, int width, int height,
                    void** address) {
  ATRACE_NAME("IonBuffer::Lock");
  ALOGD_IF(TRACE,
           "IonBuffer::Lock: handle=%p usage=%d x=%d y=%d width=%d height=%d "
           "address=%p",
           handle(), usage, x, y, width, height, address);

  status_t err =
      buffer_->lock(usage, Rect(x, y, x + width, y + height), address);
  if (err != OK)
    return -EINVAL;
  else
    return 0;
}

int IonBuffer::LockYUV(uint32_t usage, int x, int y, int width, int height,
                       struct android_ycbcr* yuv) {
  ATRACE_NAME("IonBuffer::LockYUV");
  ALOGD_IF(TRACE,
           "IonBuffer::Lock: handle=%p usage=%d x=%d y=%d width=%d height=%d",
           handle(), usage, x, y, width, height);

  status_t err =
      buffer_->lockYCbCr(usage, Rect(x, y, x + width, y + height), yuv);
  if (err != OK)
    return -EINVAL;
  else
    return 0;
}

int IonBuffer::Unlock() {
  ATRACE_NAME("IonBuffer::Unlock");
  ALOGD_IF(TRACE, "IonBuffer::Unlock: handle=%p", handle());

  status_t err = buffer_->unlock();
  if (err != OK)
    return -EINVAL;
  else
    return 0;
}
}  // namespace dvr
}  // namespace android
