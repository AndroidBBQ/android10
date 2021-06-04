#include <poll.h>
#include <sys/epoll.h>

#include <pdx/default_transport/client_channel.h>
#include <pdx/default_transport/client_channel_factory.h>
#include <private/dvr/buffer_hub_base.h>

using android::pdx::LocalChannelHandle;
using android::pdx::LocalHandle;
using android::pdx::Status;
using android::pdx::default_transport::ClientChannel;
using android::pdx::default_transport::ClientChannelFactory;

namespace android {
namespace dvr {

BufferHubBase::BufferHubBase(LocalChannelHandle channel_handle)
    : Client{pdx::default_transport::ClientChannel::Create(
          std::move(channel_handle))},
      id_(-1),
      cid_(-1) {}
BufferHubBase::BufferHubBase(const std::string& endpoint_path)
    : Client{pdx::default_transport::ClientChannelFactory::Create(
          endpoint_path)},
      id_(-1),
      cid_(-1) {}

BufferHubBase::~BufferHubBase() {
  // buffer_state and fence_state are not reset here. They will be used to
  // clean up epoll fd if necessary in ProducerChannel::RemoveConsumer method.
  if (metadata_header_ != nullptr) {
    metadata_buffer_.Unlock();
  }
}

Status<LocalChannelHandle> BufferHubBase::CreateConsumer() {
  Status<LocalChannelHandle> status =
      InvokeRemoteMethod<BufferHubRPC::NewConsumer>();
  ALOGE_IF(!status,
           "BufferHub::CreateConsumer: Failed to create consumer channel: %s",
           status.GetErrorMessage().c_str());
  return status;
}

int BufferHubBase::ImportBuffer() {
  ATRACE_NAME("BufferHubBase::ImportBuffer");

  Status<BufferDescription<LocalHandle>> status =
      InvokeRemoteMethod<BufferHubRPC::GetBuffer>();
  if (!status) {
    ALOGE("BufferHubBase::ImportBuffer: Failed to get buffer: %s",
          status.GetErrorMessage().c_str());
    return -status.error();
  } else if (status.get().id() < 0) {
    ALOGE("BufferHubBase::ImportBuffer: Received an invalid id!");
    return -EIO;
  }

  auto buffer_desc = status.take();

  // Stash the buffer id to replace the value in id_.
  const int new_id = buffer_desc.id();

  // Import the buffer.
  IonBuffer ion_buffer;
  ALOGD_IF(TRACE, "BufferHubBase::ImportBuffer: id=%d.", buffer_desc.id());

  if (const int ret = buffer_desc.ImportBuffer(&ion_buffer))
    return ret;

  // Import the metadata.
  IonBuffer metadata_buffer;
  if (const int ret = buffer_desc.ImportMetadata(&metadata_buffer)) {
    ALOGE("Failed to import metadata buffer, error=%d", ret);
    return ret;
  }
  size_t metadata_buf_size = metadata_buffer.width();
  if (metadata_buf_size < BufferHubDefs::kMetadataHeaderSize) {
    ALOGE("BufferHubBase::ImportBuffer: metadata buffer too small: %zu",
          metadata_buf_size);
    return -ENOMEM;
  }

  // If all imports succee, replace the previous buffer and id.
  buffer_ = std::move(ion_buffer);
  metadata_buffer_ = std::move(metadata_buffer);
  metadata_buf_size_ = metadata_buf_size;
  user_metadata_size_ = metadata_buf_size_ - BufferHubDefs::kMetadataHeaderSize;

  void* metadata_ptr = nullptr;
  if (const int ret =
          metadata_buffer_.Lock(BufferHubDefs::kMetadataUsage, /*x=*/0,
                                /*y=*/0, metadata_buf_size_,
                                /*height=*/1, &metadata_ptr)) {
    ALOGE("BufferHubBase::ImportBuffer: Failed to lock metadata.");
    return ret;
  }

  // Set up shared fences.
  shared_acquire_fence_ = buffer_desc.take_acquire_fence();
  shared_release_fence_ = buffer_desc.take_release_fence();
  if (!shared_acquire_fence_ || !shared_release_fence_) {
    ALOGE("BufferHubBase::ImportBuffer: Failed to import shared fences.");
    return -EIO;
  }

  metadata_header_ =
      reinterpret_cast<BufferHubDefs::MetadataHeader*>(metadata_ptr);
  if (user_metadata_size_) {
    user_metadata_ptr_ =
        reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(metadata_ptr) +
                                BufferHubDefs::kMetadataHeaderSize);
  } else {
    user_metadata_ptr_ = nullptr;
  }

  id_ = new_id;
  cid_ = buffer_desc.buffer_cid();
  client_state_mask_ = buffer_desc.client_state_mask();

  // Note that here the buffer_state, fence_state and active_clients_bit_mask
  // are mapped from shared memory as an atomic object. The std::atomic's
  // constructor will not be called so that the original value stored in the
  // memory region will be preserved.
  buffer_state_ = &metadata_header_->bufferState;
  ALOGD_IF(TRACE,
           "BufferHubBase::ImportBuffer: id=%d, buffer_state=%" PRIx32 ".",
           id(), buffer_state_->load(std::memory_order_acquire));
  fence_state_ = &metadata_header_->fenceState;
  ALOGD_IF(TRACE,
           "BufferHubBase::ImportBuffer: id=%d, fence_state=%" PRIx32 ".", id(),
           fence_state_->load(std::memory_order_acquire));
  active_clients_bit_mask_ = &metadata_header_->activeClientsBitMask;
  ALOGD_IF(
      TRACE,
      "BufferHubBase::ImportBuffer: id=%d, active_clients_bit_mask=%" PRIx32
      ".",
      id(), active_clients_bit_mask_->load(std::memory_order_acquire));

  return 0;
}

int BufferHubBase::CheckMetadata(size_t user_metadata_size) const {
  if (user_metadata_size && !user_metadata_ptr_) {
    ALOGE("BufferHubBase::CheckMetadata: doesn't support custom metadata.");
    return -EINVAL;
  }
  if (user_metadata_size > user_metadata_size_) {
    ALOGE("BufferHubBase::CheckMetadata: too big: %zu, maximum: %zu.",
          user_metadata_size, user_metadata_size_);
    return -E2BIG;
  }
  return 0;
}

int BufferHubBase::UpdateSharedFence(const LocalHandle& new_fence,
                                     const LocalHandle& shared_fence) {
  if (pending_fence_fd_.Get() != new_fence.Get()) {
    // First, replace the old fd if there was already one. Skipping if the new
    // one is the same as the old.
    if (pending_fence_fd_.IsValid()) {
      const int ret = epoll_ctl(shared_fence.Get(), EPOLL_CTL_DEL,
                                pending_fence_fd_.Get(), nullptr);
      ALOGW_IF(ret,
               "BufferHubBase::UpdateSharedFence: failed to remove old fence "
               "fd from epoll set, error: %s.",
               strerror(errno));
    }

    if (new_fence.IsValid()) {
      // If ready fence is valid, we put that into the epoll set.
      epoll_event event;
      event.events = EPOLLIN;
      event.data.u32 = client_state_mask();
      pending_fence_fd_ = new_fence.Duplicate();
      if (epoll_ctl(shared_fence.Get(), EPOLL_CTL_ADD, pending_fence_fd_.Get(),
                    &event) < 0) {
        const int error = errno;
        ALOGE(
            "BufferHubBase::UpdateSharedFence: failed to add new fence fd "
            "into epoll set, error: %s.",
            strerror(error));
        return -error;
      }
      // Set bit in fence state to indicate that there is a fence from this
      // producer or consumer.
      fence_state_->fetch_or(client_state_mask());
    } else {
      // Unset bit in fence state to indicate that there is no fence, so that
      // when consumer to acquire or producer to acquire, it knows no need to
      // check fence for this buffer.
      fence_state_->fetch_and(~client_state_mask());
    }
  }

  return 0;
}

int BufferHubBase::Lock(int usage, int x, int y, int width, int height,
                        void** address) {
  return buffer_.Lock(usage, x, y, width, height, address);
}

int BufferHubBase::Unlock() { return buffer_.Unlock(); }

int BufferHubBase::GetBlobReadWritePointer(size_t size, void** addr) {
  int width = static_cast<int>(size);
  int height = 1;
  int ret = Lock(usage(), 0, 0, width, height, addr);
  if (ret == 0)
    Unlock();
  return ret;
}

}  // namespace dvr
}  // namespace android
