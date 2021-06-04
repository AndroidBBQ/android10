#ifndef ANDROID_DVR_ION_BUFFER_H_
#define ANDROID_DVR_ION_BUFFER_H_

#include <hardware/gralloc.h>
#include <log/log.h>
#include <ui/GraphicBuffer.h>

namespace android {
namespace dvr {

// IonBuffer is an abstraction of Ion/Gralloc buffers.
class IonBuffer {
 public:
  IonBuffer();
  IonBuffer(uint32_t width, uint32_t height, uint32_t format, uint64_t usage);
  IonBuffer(buffer_handle_t handle, uint32_t width, uint32_t height,
            uint32_t stride, uint32_t format, uint64_t usage);
  IonBuffer(buffer_handle_t handle, uint32_t width, uint32_t height,
            uint32_t layer_count, uint32_t stride, uint32_t format,
            uint64_t usage);
  ~IonBuffer();

  IonBuffer(IonBuffer&& other) noexcept;
  IonBuffer& operator=(IonBuffer&& other) noexcept;

  // Returns check this IonBuffer holds a valid Gralloc buffer.
  bool IsValid() const { return buffer_ && buffer_->initCheck() == OK; }

  // Frees the underlying native handle and leaves the instance initialized to
  // empty.
  void FreeHandle();

  // Allocates a new native handle with the given parameters, freeing the
  // previous native handle if necessary. Returns 0 on success or a negative
  // errno code otherwise. If allocation fails the previous native handle is
  // left intact.
  int Alloc(uint32_t width, uint32_t height, uint32_t layer_count,
            uint32_t format, uint64_t usage);

  // Resets the underlying native handle and parameters, freeing the previous
  // native handle if necessary.
  void Reset(buffer_handle_t handle, uint32_t width, uint32_t height,
             uint32_t layer_count, uint32_t stride, uint32_t format,
             uint64_t usage);

  // Like Reset but also registers the native handle, which is necessary for
  // native handles received over IPC. Returns 0 on success or a negative errno
  // code otherwise. If import fails the previous native handle is left intact.
  int Import(buffer_handle_t handle, uint32_t width, uint32_t height,
             uint32_t layer_count, uint32_t stride, uint32_t format,
             uint64_t usage);

  // Like Reset but imports a native handle from raw fd and int arrays. Returns
  // 0 on success or a negative errno code otherwise. If import fails the
  // previous native handle is left intact.
  int Import(const int* fd_array, int fd_count, const int* int_array,
             int int_count, uint32_t width, uint32_t height,
             uint32_t layer_count, uint32_t stride, uint32_t format,
             uint64_t usage);

  // Duplicates the native handle underlying |other| and then imports it. This
  // is useful for creating multiple, independent views of the same Ion/Gralloc
  // buffer. Returns 0 on success or a negative errno code otherwise. If
  // duplication or import fail the previous native handle is left intact.
  int Duplicate(const IonBuffer* other);

  int Lock(uint32_t usage, int x, int y, int width, int height, void** address);
  int LockYUV(uint32_t usage, int x, int y, int width, int height,
              struct android_ycbcr* yuv);
  int Unlock();

  sp<GraphicBuffer>& buffer() { return buffer_; }
  const sp<GraphicBuffer>& buffer() const { return buffer_; }
  buffer_handle_t handle() const {
    return buffer_.get() ? buffer_->handle : nullptr;
  }
  uint32_t width() const { return buffer_.get() ? buffer_->getWidth() : 0; }
  uint32_t height() const { return buffer_.get() ? buffer_->getHeight() : 0; }
  uint32_t layer_count() const {
    return buffer_.get() ? buffer_->getLayerCount() : 0;
  }
  uint32_t stride() const { return buffer_.get() ? buffer_->getStride() : 0; }
  uint32_t format() const {
    return buffer_.get() ? buffer_->getPixelFormat() : 0;
  }
  uint64_t usage() const {
    return buffer_.get() ? static_cast<uint64_t>(buffer_->getUsage()) : 0;
  }

 private:
  sp<GraphicBuffer> buffer_;

  IonBuffer(const IonBuffer&) = delete;
  void operator=(const IonBuffer&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_ION_BUFFER_H_
