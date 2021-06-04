#ifndef ANDROID_PDX_CHANNEL_HANDLE_H_
#define ANDROID_PDX_CHANNEL_HANDLE_H_

#include <cstdint>
#include <type_traits>

namespace android {
namespace pdx {

enum class ChannelHandleMode {
  Local,
  Borrowed,
  Remote,
};

class ChannelManagerInterface {
 public:
  virtual void CloseHandle(std::int32_t handle) = 0;

 protected:
  // Nobody should be allowed to delete the instance of channel manager
  // through this interface.
  virtual ~ChannelManagerInterface() = default;
};

class ChannelHandleBase {
 public:
  ChannelHandleBase() = default;
  explicit ChannelHandleBase(const int32_t& value) : value_{value} {}

  ChannelHandleBase(const ChannelHandleBase&) = delete;
  ChannelHandleBase& operator=(const ChannelHandleBase&) = delete;

  std::int32_t value() const { return value_; }
  bool valid() const { return value_ >= 0; }
  explicit operator bool() const { return valid(); }

  void Close() { value_ = kEmptyHandle; }

 protected:
  // Must not be used by itself. Must be derived from.
  ~ChannelHandleBase() = default;
  enum : std::int32_t { kEmptyHandle = -1 };

  std::int32_t value_{kEmptyHandle};
};

template <ChannelHandleMode Mode>
class ChannelHandle : public ChannelHandleBase {
 public:
  ChannelHandle() = default;
  using ChannelHandleBase::ChannelHandleBase;
  ChannelHandle(ChannelHandle&& other) noexcept : ChannelHandleBase{other.value_} {
    other.value_ = kEmptyHandle;
  }
  ~ChannelHandle() = default;

  ChannelHandle Duplicate() const { return ChannelHandle{value_}; }

  ChannelHandle& operator=(ChannelHandle&& other) noexcept {
    value_ = other.value_;
    other.value_ = kEmptyHandle;
    return *this;
  }
};

template <>
class ChannelHandle<ChannelHandleMode::Local> : public ChannelHandleBase {
 public:
  ChannelHandle() = default;
  ChannelHandle(ChannelManagerInterface* manager, int32_t value)
      : ChannelHandleBase{value}, manager_{manager} {}

  ChannelHandle(const ChannelHandle&) = delete;
  ChannelHandle& operator=(const ChannelHandle&) = delete;

  ChannelHandle(ChannelHandle&& other) noexcept
      : ChannelHandleBase{other.value_}, manager_{other.manager_} {
    other.manager_ = nullptr;
    other.value_ = kEmptyHandle;
  }

  ChannelHandle& operator=(ChannelHandle&& other) noexcept {
    value_ = other.value_;
    manager_ = other.manager_;
    other.value_ = kEmptyHandle;
    other.manager_ = nullptr;
    return *this;
  }

  ~ChannelHandle() {
    if (manager_)
      manager_->CloseHandle(value_);
  }

  ChannelHandle<ChannelHandleMode::Borrowed> Borrow() const {
    return ChannelHandle<ChannelHandleMode::Borrowed>{value_};
  }

  void Close() {
    if (manager_)
      manager_->CloseHandle(value_);
    manager_ = nullptr;
    value_ = kEmptyHandle;
  }

 private:
  ChannelManagerInterface* manager_{nullptr};
};

using LocalChannelHandle = ChannelHandle<ChannelHandleMode::Local>;
using BorrowedChannelHandle = ChannelHandle<ChannelHandleMode::Borrowed>;
using RemoteChannelHandle = ChannelHandle<ChannelHandleMode::Remote>;

// ChannelReference is a 32 bit integer used in IPC serialization to be
// transferred across processes. You can convert this value to a local channel
// handle by calling Transaction.GetChannelHandle().
using ChannelReference = int32_t;

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_CHANNEL_HANDLE_H_
