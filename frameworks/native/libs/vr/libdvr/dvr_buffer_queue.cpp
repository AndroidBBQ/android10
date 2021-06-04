#include "include/dvr/dvr_api.h"
#include "include/dvr/dvr_buffer_queue.h"

#include <android/native_window.h>
#include <gui/BufferHubProducer.h>

#include "dvr_internal.h"
#include "dvr_buffer_queue_internal.h"

using namespace android;
using android::dvr::BufferHubBase;
using android::dvr::ConsumerBuffer;
using android::dvr::ConsumerQueue;
using android::dvr::ProducerBuffer;
using android::dvr::ProducerQueue;
using android::dvr::ProducerQueueConfigBuilder;
using android::dvr::UsagePolicy;

extern "C" {

DvrWriteBufferQueue::DvrWriteBufferQueue(
    const std::shared_ptr<ProducerQueue>& producer_queue)
    : producer_queue_(producer_queue),
      width_(producer_queue->default_width()),
      height_(producer_queue->default_height()),
      format_(producer_queue->default_format()) {}

int DvrWriteBufferQueue::GetNativeWindow(ANativeWindow** out_window) {
  if (native_window_ == nullptr) {
    // Lazy creation of |native_window|, as not everyone is using
    // DvrWriteBufferQueue as an external surface.
    sp<IGraphicBufferProducer> gbp = BufferHubProducer::Create(producer_queue_);
    native_window_ = new Surface(gbp, true);
  }

  *out_window = static_cast<ANativeWindow*>(native_window_.get());
  return 0;
}

int DvrWriteBufferQueue::CreateReadQueue(DvrReadBufferQueue** out_read_queue) {
  std::unique_ptr<ConsumerQueue> consumer_queue =
      producer_queue_->CreateConsumerQueue();
  if (consumer_queue == nullptr) {
    ALOGE(
        "DvrWriteBufferQueue::CreateReadQueue: Failed to create consumer queue "
        "from producer queue: queue_id=%d.", producer_queue_->id());
    return -ENOMEM;
  }

  *out_read_queue = new DvrReadBufferQueue(std::move(consumer_queue));
  return 0;
}

int DvrWriteBufferQueue::Dequeue(int timeout, DvrWriteBuffer* write_buffer,
                                 int* out_fence_fd) {
  DvrNativeBufferMetadata meta;
  DvrWriteBuffer* buffer = nullptr;
  int fence_fd = -1;
  if (const int ret = GainBuffer(timeout, &buffer, &meta, &fence_fd))
    return ret;
  if (!buffer)
    return -ENOMEM;

  write_buffers_[buffer->slot].reset(buffer);
  write_buffer->write_buffer = std::move(buffer->write_buffer);
  *out_fence_fd = fence_fd;
  return 0;
}

int DvrWriteBufferQueue::GainBuffer(int timeout,
                                    DvrWriteBuffer** out_write_buffer,
                                    DvrNativeBufferMetadata* out_meta,
                                    int* out_fence_fd) {
  size_t slot;
  pdx::LocalHandle release_fence;

  // Need to retry N+1 times, where N is total number of buffers in the queue.
  // As in the worst case, we will dequeue all N buffers and reallocate them, on
  // the {N+1}th dequeue, we are guaranteed to get a buffer with new dimension.
  size_t max_retries = 1 + producer_queue_->capacity();
  size_t retry = 0;

  for (; retry < max_retries; retry++) {
    auto buffer_status =
        producer_queue_->Dequeue(timeout, &slot, out_meta, &release_fence);
    if (!buffer_status) {
      ALOGE_IF(buffer_status.error() != ETIMEDOUT,
               "DvrWriteBufferQueue::GainBuffer: Failed to dequeue buffer: %s",
               buffer_status.GetErrorMessage().c_str());
      return -buffer_status.error();
    }

    if (write_buffers_[slot] == nullptr) {
      // Lazy initialization of a write_buffers_ slot. Note that a slot will
      // only be dynamically allocated once during the entire cycle life of a
      // queue.
      write_buffers_[slot] = std::make_unique<DvrWriteBuffer>();
      write_buffers_[slot]->slot = slot;
    }

    LOG_ALWAYS_FATAL_IF(
        write_buffers_[slot]->write_buffer,
        "DvrWriteBufferQueue::GainBuffer: Buffer slot is not empty: %zu", slot);
    write_buffers_[slot]->write_buffer = std::move(buffer_status.take());

    const auto& producer_buffer = write_buffers_[slot]->write_buffer;
    if (!producer_buffer)
      return -ENOMEM;

    if (width_ == producer_buffer->width() &&
        height_ == producer_buffer->height() &&
        format_ == producer_buffer->format()) {
      // Producer queue returns a buffer matches the current request.
      break;
    }

    // Needs reallocation. Note that if there are already multiple available
    // buffers in the queue, the next one returned from |queue_->Dequeue| may
    // still have the old buffer dimension or format. Retry up to N+1 times or
    // until we dequeued a buffer with new configuration.
    ALOGD_IF(TRACE,
             "DvrWriteBufferQueue::Dequeue: requested buffer at slot: %zu "
             "(w=%u, h=%u, fmt=%u) is different from the buffer returned "
             "(w=%u, h=%u, fmt=%u). Need re-allocation.",
             slot, width_, height_, format_, producer_buffer->width(),
             producer_buffer->height(), producer_buffer->format());

    // Currently, we are not storing |layer_count| and |usage| in queue
    // configuration. Copy those setup from the last buffer dequeued before we
    // remove it.
    uint32_t old_layer_count = producer_buffer->layer_count();
    uint64_t old_usage = producer_buffer->usage();

    // Allocate a new producer buffer with new buffer configs. Note that if
    // there are already multiple available buffers in the queue, the next one
    // returned from |queue_->Dequeue| may still have the old buffer dimension
    // or format. Retry up to BufferHubQueue::kMaxQueueCapacity times or until
    // we dequeued a buffer with new configuration.
    auto remove_status = producer_queue_->RemoveBuffer(slot);
    if (!remove_status) {
      ALOGE("DvrWriteBufferQueue::Dequeue: Failed to remove buffer: %s",
            remove_status.GetErrorMessage().c_str());
      return -remove_status.error();
    }
    // Make sure that the previously allocated buffer is dereferenced from
    // write_buffers_ array.
    write_buffers_[slot]->write_buffer = nullptr;

    auto allocate_status = producer_queue_->AllocateBuffer(
        width_, height_, old_layer_count, format_, old_usage);
    if (!allocate_status) {
      ALOGE("DvrWriteBufferQueue::Dequeue: Failed to allocate buffer: %s",
            allocate_status.GetErrorMessage().c_str());
      return -allocate_status.error();
    }
  }

  if (retry >= max_retries) {
    ALOGE(
        "DvrWriteBufferQueue::Dequeue: Failed to re-allocate buffer after "
        "resizing.");
    return -ENOMEM;
  }

  *out_write_buffer = write_buffers_[slot].release();
  *out_fence_fd = release_fence.Release();

  return 0;
}

int DvrWriteBufferQueue::PostBuffer(DvrWriteBuffer* write_buffer,
                                    const DvrNativeBufferMetadata* meta,
                                    int ready_fence_fd) {
  // Some basic sanity checks before we put the buffer back into a slot.
  size_t slot = static_cast<size_t>(write_buffer->slot);
  LOG_FATAL_IF(
      (write_buffers->slot < 0 || write_buffers->slot >= write_buffers_.size()),
      "DvrWriteBufferQueue::ReleaseBuffer: Invalid slot: %zu", slot);

  if (write_buffers_[slot] != nullptr) {
    ALOGE("DvrWriteBufferQueue::PostBuffer: Slot is not empty: %zu", slot);
    return -EINVAL;
  }
  if (write_buffer->write_buffer == nullptr) {
    ALOGE("DvrWriteBufferQueue::PostBuffer: Invalid write buffer.");
    return -EINVAL;
  }
  if (write_buffer->write_buffer->id() != producer_queue_->GetBufferId(slot)) {
    ALOGE(
        "DvrWriteBufferQueue::PostBuffer: Buffer to be posted does not "
        "belong to this buffer queue. Posting buffer: id=%d, buffer in "
        "queue: id=%d",
        write_buffer->write_buffer->id(), producer_queue_->GetBufferId(slot));
    return -EINVAL;
  }

  write_buffer->write_buffer->SetQueueIndex(next_post_index_++);
  pdx::LocalHandle fence(ready_fence_fd);
  const int ret = write_buffer->write_buffer->PostAsync(meta, fence);
  if (ret < 0) {
    ALOGE("DvrWriteBufferQueue::PostBuffer: Failed to post buffer, ret=%d",
          ret);
    return ret;
  }

  // Put the DvrWriteBuffer pointer back into its slot for reuse.
  write_buffers_[slot].reset(write_buffer);
  // It's import to reset the write buffer client now. It should stay invalid
  // until next GainBuffer on the same slot.
  write_buffers_[slot]->write_buffer = nullptr;
  return 0;
}

int DvrWriteBufferQueue::ResizeBuffer(uint32_t width, uint32_t height) {
  if (width == 0 || height == 0) {
    ALOGE(
        "DvrWriteBufferQueue::ResizeBuffer: invalid buffer dimension: w=%u, "
        "h=%u.",
        width, height);
    return -EINVAL;
  }

  width_ = width;
  height_ = height;
  return 0;
}

int dvrWriteBufferQueueCreate(uint32_t width, uint32_t height, uint32_t format,
                              uint32_t layer_count, uint64_t usage,
                              size_t capacity, size_t metadata_size,
                              DvrWriteBufferQueue** out_write_queue) {
  if (!out_write_queue)
    return -EINVAL;

  auto config_builder = ProducerQueueConfigBuilder()
                            .SetDefaultWidth(width)
                            .SetDefaultHeight(height)
                            .SetDefaultFormat(format)
                            .SetMetadataSize(metadata_size);
  std::unique_ptr<ProducerQueue> producer_queue =
      ProducerQueue::Create(config_builder.Build(), UsagePolicy{});
  if (!producer_queue) {
    ALOGE("dvrWriteBufferQueueCreate: Failed to create producer queue.");
    return -ENOMEM;
  }

  auto status = producer_queue->AllocateBuffers(width, height, layer_count,
                                                format, usage, capacity);
  if (!status.ok()) {
    ALOGE("dvrWriteBufferQueueCreate: Failed to allocate buffers.");
    return -ENOMEM;
  }

  *out_write_queue = new DvrWriteBufferQueue(std::move(producer_queue));
  return 0;
}

void dvrWriteBufferQueueDestroy(DvrWriteBufferQueue* write_queue) {
  delete write_queue;
}

ssize_t dvrWriteBufferQueueGetCapacity(DvrWriteBufferQueue* write_queue) {
  if (!write_queue)
    return -EINVAL;

  return write_queue->capacity();
}

int dvrWriteBufferQueueGetId(DvrWriteBufferQueue* write_queue) {
  if (!write_queue)
    return -EINVAL;

  return write_queue->id();
}

int dvrWriteBufferQueueGetANativeWindow(DvrWriteBufferQueue* write_queue,
                                        ANativeWindow** out_window) {
  if (!write_queue || !out_window)
    return -EINVAL;

  return write_queue->GetNativeWindow(out_window);
}

int dvrWriteBufferQueueCreateReadQueue(DvrWriteBufferQueue* write_queue,
                                       DvrReadBufferQueue** out_read_queue) {
  if (!write_queue || !out_read_queue)
    return -EINVAL;

  return write_queue->CreateReadQueue(out_read_queue);
}

int dvrWriteBufferQueueGainBuffer(DvrWriteBufferQueue* write_queue, int timeout,
                                  DvrWriteBuffer** out_write_buffer,
                                  DvrNativeBufferMetadata* out_meta,
                                  int* out_fence_fd) {
  if (!write_queue || !out_write_buffer || !out_meta || !out_fence_fd)
    return -EINVAL;

  return write_queue->GainBuffer(timeout, out_write_buffer, out_meta,
                                 out_fence_fd);
}

int dvrWriteBufferQueuePostBuffer(DvrWriteBufferQueue* write_queue,
                                  DvrWriteBuffer* write_buffer,
                                  const DvrNativeBufferMetadata* meta,
                                  int ready_fence_fd) {
  if (!write_queue || !write_buffer || !write_buffer->write_buffer || !meta)
    return -EINVAL;

  return write_queue->PostBuffer(write_buffer, meta, ready_fence_fd);
}

int dvrWriteBufferQueueResizeBuffer(DvrWriteBufferQueue* write_queue,
                                    uint32_t width, uint32_t height) {
  if (!write_queue)
    return -EINVAL;

  return write_queue->ResizeBuffer(width, height);
}

// ReadBufferQueue

DvrReadBufferQueue::DvrReadBufferQueue(
    const std::shared_ptr<ConsumerQueue>& consumer_queue)
    : consumer_queue_(consumer_queue) {}

int DvrReadBufferQueue::CreateReadQueue(DvrReadBufferQueue** out_read_queue) {
  std::unique_ptr<ConsumerQueue> consumer_queue =
      consumer_queue_->CreateConsumerQueue();
  if (consumer_queue == nullptr) {
    ALOGE(
        "DvrReadBufferQueue::CreateReadQueue: Failed to create consumer queue "
        "from producer queue: queue_id=%d.", consumer_queue_->id());
    return -ENOMEM;
  }

  *out_read_queue = new DvrReadBufferQueue(std::move(consumer_queue));
  return 0;
}

int DvrReadBufferQueue::AcquireBuffer(int timeout,
                                      DvrReadBuffer** out_read_buffer,
                                      DvrNativeBufferMetadata* out_meta,
                                      int* out_fence_fd) {
  size_t slot;
  pdx::LocalHandle acquire_fence;
  auto buffer_status =
      consumer_queue_->Dequeue(timeout, &slot, out_meta, &acquire_fence);
  if (!buffer_status) {
    ALOGE_IF(buffer_status.error() != ETIMEDOUT,
             "DvrReadBufferQueue::AcquireBuffer: Failed to dequeue buffer: %s",
             buffer_status.GetErrorMessage().c_str());
    return -buffer_status.error();
  }

  if (read_buffers_[slot] == nullptr) {
    // Lazy initialization of a read_buffers_ slot. Note that a slot will only
    // be dynamically allocated once during the entire cycle life of a queue.
    read_buffers_[slot] = std::make_unique<DvrReadBuffer>();
    read_buffers_[slot]->slot = slot;
  }

  LOG_FATAL_IF(
      read_buffers_[slot]->read_buffer,
      "DvrReadBufferQueue::AcquireBuffer: Buffer slot is not empty: %zu", slot);
  read_buffers_[slot]->read_buffer = std::move(buffer_status.take());

  *out_read_buffer = read_buffers_[slot].release();
  *out_fence_fd = acquire_fence.Release();

  return 0;
}

int DvrReadBufferQueue::ReleaseBuffer(DvrReadBuffer* read_buffer,
                                      const DvrNativeBufferMetadata* meta,
                                      int release_fence_fd) {
  // Some basic sanity checks before we put the buffer back into a slot.
  size_t slot = static_cast<size_t>(read_buffer->slot);
  LOG_FATAL_IF(
      (read_buffers->slot < 0 || read_buffers->slot >= read_buffers_size()),
      "DvrReadBufferQueue::ReleaseBuffer: Invalid slot: %zu", slot);

  if (read_buffers_[slot] != nullptr) {
    ALOGE("DvrReadBufferQueue::ReleaseBuffer: Slot is not empty: %zu", slot);
    return -EINVAL;
  }
  if (read_buffer->read_buffer == nullptr) {
    ALOGE("DvrReadBufferQueue::ReleaseBuffer: Invalid read buffer.");
    return -EINVAL;
  }
  if (read_buffer->read_buffer->id() != consumer_queue_->GetBufferId(slot)) {
    if (consumer_queue_->GetBufferId(slot) > 0) {
      ALOGE(
          "DvrReadBufferQueue::ReleaseBuffer: Buffer to be released may not "
          "belong to this queue (queue_id=%d): attempting to release buffer "
          "(buffer_id=%d) at slot %d which holds a different buffer "
          "(buffer_id=%d).",
          consumer_queue_->id(), read_buffer->read_buffer->id(),
          static_cast<int>(slot), consumer_queue_->GetBufferId(slot));
    } else {
      ALOGI(
          "DvrReadBufferQueue::ReleaseBuffer: Buffer to be released may not "
          "belong to this queue (queue_id=%d): attempting to release buffer "
          "(buffer_id=%d) at slot %d which is empty.",
          consumer_queue_->id(), read_buffer->read_buffer->id(),
          static_cast<int>(slot));
    }
  }

  pdx::LocalHandle fence(release_fence_fd);
  int ret = read_buffer->read_buffer->ReleaseAsync(meta, fence);
  if (ret < 0) {
    ALOGE("DvrReadBufferQueue::ReleaseBuffer: Failed to release buffer, ret=%d",
          ret);
    return ret;
  }

  // Put the DvrReadBuffer pointer back into its slot for reuse.
  read_buffers_[slot].reset(read_buffer);
  // It's import to reset the read buffer client now. It should stay invalid
  // until next AcquireBuffer on the same slot.
  read_buffers_[slot]->read_buffer = nullptr;
  return 0;
}

void DvrReadBufferQueue::SetBufferAvailableCallback(
    DvrReadBufferQueueBufferAvailableCallback callback, void* context) {
  if (callback == nullptr) {
    consumer_queue_->SetBufferAvailableCallback(nullptr);
  } else {
    consumer_queue_->SetBufferAvailableCallback(
        [callback, context]() { callback(context); });
  }
}

void DvrReadBufferQueue::SetBufferRemovedCallback(
    DvrReadBufferQueueBufferRemovedCallback callback, void* context) {
  if (callback == nullptr) {
    consumer_queue_->SetBufferRemovedCallback(nullptr);
  } else {
    consumer_queue_->SetBufferRemovedCallback(
        [callback, context](const std::shared_ptr<BufferHubBase>& buffer) {
          // When buffer is removed from the queue, the slot is already invalid.
          auto read_buffer = std::make_unique<DvrReadBuffer>();
          read_buffer->read_buffer =
              std::static_pointer_cast<ConsumerBuffer>(buffer);
          callback(read_buffer.release(), context);
        });
  }
}

int DvrReadBufferQueue::HandleEvents() {
  // TODO(jwcai) Probably should change HandleQueueEvents to return Status.
  consumer_queue_->HandleQueueEvents();
  return 0;
}

void dvrReadBufferQueueDestroy(DvrReadBufferQueue* read_queue) {
  delete read_queue;
}

ssize_t dvrReadBufferQueueGetCapacity(DvrReadBufferQueue* read_queue) {
  if (!read_queue)
    return -EINVAL;

  return read_queue->capacity();
}

int dvrReadBufferQueueGetId(DvrReadBufferQueue* read_queue) {
  if (!read_queue)
    return -EINVAL;

  return read_queue->id();
}

int dvrReadBufferQueueGetEventFd(DvrReadBufferQueue* read_queue) {
  if (!read_queue)
    return -EINVAL;

  return read_queue->event_fd();
}

int dvrReadBufferQueueCreateReadQueue(DvrReadBufferQueue* read_queue,
                                      DvrReadBufferQueue** out_read_queue) {
  if (!read_queue || !out_read_queue)
    return -EINVAL;

  return read_queue->CreateReadQueue(out_read_queue);
}

int dvrReadBufferQueueDequeue(DvrReadBufferQueue* read_queue, int timeout,
                              DvrReadBuffer* read_buffer, int* out_fence_fd,
                              void* out_meta, size_t meta_size_bytes) {
  if (!read_queue || !read_buffer || !out_fence_fd)
    return -EINVAL;

  if (meta_size_bytes != 0 && !out_meta)
    return -EINVAL;

  return read_queue->Dequeue(timeout, read_buffer, out_fence_fd, out_meta,
                             meta_size_bytes);
}

int dvrReadBufferQueueAcquireBuffer(DvrReadBufferQueue* read_queue, int timeout,
                                    DvrReadBuffer** out_read_buffer,
                                    DvrNativeBufferMetadata* out_meta,
                                    int* out_fence_fd) {
  if (!read_queue || !out_read_buffer || !out_meta || !out_fence_fd)
    return -EINVAL;

  return read_queue->AcquireBuffer(timeout, out_read_buffer, out_meta,
                                   out_fence_fd);
}

int dvrReadBufferQueueReleaseBuffer(DvrReadBufferQueue* read_queue,
                                    DvrReadBuffer* read_buffer,
                                    const DvrNativeBufferMetadata* meta,
                                    int release_fence_fd) {
  if (!read_queue || !read_buffer || !read_buffer->read_buffer || !meta)
    return -EINVAL;

  return read_queue->ReleaseBuffer(read_buffer, meta, release_fence_fd);
}

int dvrReadBufferQueueSetBufferAvailableCallback(
    DvrReadBufferQueue* read_queue,
    DvrReadBufferQueueBufferAvailableCallback callback, void* context) {
  if (!read_queue)
    return -EINVAL;

  read_queue->SetBufferAvailableCallback(callback, context);
  return 0;
}

int dvrReadBufferQueueSetBufferRemovedCallback(
    DvrReadBufferQueue* read_queue,
    DvrReadBufferQueueBufferRemovedCallback callback, void* context) {
  if (!read_queue)
    return -EINVAL;

  read_queue->SetBufferRemovedCallback(callback, context);
  return 0;
}

int dvrReadBufferQueueHandleEvents(DvrReadBufferQueue* read_queue) {
  if (!read_queue)
    return -EINVAL;

  return read_queue->HandleEvents();
}

}  // extern "C"
