#ifndef ANDROID_DVR_NATIVE_HANDLE_WRAPPER_H_
#define ANDROID_DVR_NATIVE_HANDLE_WRAPPER_H_

#include <cutils/native_handle.h>
#include <log/log.h>
#include <pdx/rpc/serializable.h>

#include <vector>

namespace android {
namespace dvr {

// A PDX-friendly wrapper to maintain the life cycle of a native_handle_t
// object.
//
// See https://source.android.com/devices/architecture/hidl/types#handle_t for
// more information about native_handle_t.
template <typename FileHandleType>
class NativeHandleWrapper {
 public:
  NativeHandleWrapper() = default;
  NativeHandleWrapper(NativeHandleWrapper&& other) = default;
  NativeHandleWrapper& operator=(NativeHandleWrapper&& other) = default;

  // Create a new NativeHandleWrapper by duplicating the handle.
  explicit NativeHandleWrapper(const native_handle_t* handle) {
    const int fd_count = handle->numFds;
    const int int_count = handle->numInts;

    // Populate the fd and int vectors: native_handle->data[] is an array of fds
    // followed by an array of opaque ints.
    for (int i = 0; i < fd_count; i++) {
      fds_.emplace_back(FileHandleType::AsDuplicate(handle->data[i]));
    }
    for (int i = 0; i < int_count; i++) {
      ints_.push_back(handle->data[fd_count + i]);
    }
  }

  size_t int_count() const { return ints_.size(); }
  size_t fd_count() const { return fds_.size(); }
  bool IsValid() const { return ints_.size() != 0 || fds_.size() != 0; }

  // Duplicate a native handle from the wrapper.
  native_handle_t* DuplicateHandle() const {
    if (!IsValid()) {
      return nullptr;
    }

    // numFds + numInts ints.
    std::vector<FileHandleType> fds;
    for (const auto& fd : fds_) {
      if (!fd.IsValid()) {
        return nullptr;
      }
      fds.emplace_back(fd.Duplicate());
    }

    return FromFdsAndInts(std::move(fds), ints_);
  }

  // Takes the native handle out of the wrapper.
  native_handle_t* TakeHandle() {
    if (!IsValid()) {
      return nullptr;
    }

    return FromFdsAndInts(std::move(fds_), std::move(ints_));
  }

 private:
  NativeHandleWrapper(const NativeHandleWrapper&) = delete;
  void operator=(const NativeHandleWrapper&) = delete;

  static native_handle_t* FromFdsAndInts(std::vector<FileHandleType> fds,
                                         std::vector<int> ints) {
    native_handle_t* handle = native_handle_create(fds.size(), ints.size());
    if (!handle) {
      ALOGE("NativeHandleWrapper::TakeHandle: Failed to create new handle.");
      return nullptr;
    }

    // numFds + numInts ints.
    for (int i = 0; i < handle->numFds; i++) {
      handle->data[i] = fds[i].Release();
    }
    memcpy(&handle->data[handle->numFds], ints.data(),
           sizeof(int) * handle->numInts);

    return handle;
  }

  std::vector<int> ints_;
  std::vector<FileHandleType> fds_;

  PDX_SERIALIZABLE_MEMBERS(NativeHandleWrapper<FileHandleType>, ints_, fds_);
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_NATIVE_HANDLE_WRAPPER_H_
