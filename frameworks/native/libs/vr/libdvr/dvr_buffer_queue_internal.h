#ifndef ANDROID_DVR_BUFFER_QUEUE_INTERNAL_H_
#define ANDROID_DVR_BUFFER_QUEUE_INTERNAL_H_

#include <gui/Surface.h>
#include <private/dvr/buffer_hub_queue_client.h>
#include <sys/cdefs.h>

#include <array>
#include <memory>

#include "dvr_internal.h"

struct ANativeWindow;

typedef struct DvrNativeBufferMetadata DvrNativeBufferMetadata;
typedef struct DvrReadBuffer DvrReadBuffer;
typedef struct DvrReadBufferQueue DvrReadBufferQueue;
typedef struct DvrWriteBuffer DvrWriteBuffer;
typedef void (*DvrReadBufferQueueBufferAvailableCallback)(void* context);
typedef void (*DvrReadBufferQueueBufferRemovedCallback)(DvrReadBuffer* buffer,
                                                        void* context);

struct DvrWriteBufferQueue {
  using BufferHubQueue = android::dvr::BufferHubQueue;
  using ProducerQueue = android::dvr::ProducerQueue;

  // Create a concrete object for DvrWriteBufferQueue.
  //
  // @param producer_queue The BufferHub's ProducerQueue that is used to back
  //     this DvrWriteBufferQueue, must not be NULL.
  explicit DvrWriteBufferQueue(
      const std::shared_ptr<ProducerQueue>& producer_queue);

  int id() const { return producer_queue_->id(); }
  uint32_t width() const { return width_; };
  uint32_t height() const { return height_; };
  uint32_t format() const { return format_; };
  size_t capacity() const { return producer_queue_->capacity(); }
  const std::shared_ptr<ProducerQueue>& producer_queue() const {
    return producer_queue_;
  }

  int GetNativeWindow(ANativeWindow** out_window);
  int CreateReadQueue(DvrReadBufferQueue** out_read_queue);
  int Dequeue(int timeout, DvrWriteBuffer* write_buffer, int* out_fence_fd);
  int GainBuffer(int timeout, DvrWriteBuffer** out_write_buffer,
                 DvrNativeBufferMetadata* out_meta, int* out_fence_fd);
  int PostBuffer(DvrWriteBuffer* write_buffer,
                 const DvrNativeBufferMetadata* meta, int ready_fence_fd);
  int ResizeBuffer(uint32_t width, uint32_t height);

 private:
  std::shared_ptr<ProducerQueue> producer_queue_;
  std::array<std::unique_ptr<DvrWriteBuffer>, BufferHubQueue::kMaxQueueCapacity>
      write_buffers_;

  int64_t next_post_index_ = 0;
  uint32_t width_;
  uint32_t height_;
  uint32_t format_;

  android::sp<android::Surface> native_window_;
};

struct DvrReadBufferQueue {
  using BufferHubQueue = android::dvr::BufferHubQueue;
  using ConsumerQueue = android::dvr::ConsumerQueue;

  explicit DvrReadBufferQueue(
      const std::shared_ptr<ConsumerQueue>& consumer_queue);

  int id() const { return consumer_queue_->id(); }
  int event_fd() const { return consumer_queue_->queue_fd(); }
  size_t capacity() const { return consumer_queue_->capacity(); }

  int CreateReadQueue(DvrReadBufferQueue** out_read_queue);
  int Dequeue(int timeout, DvrReadBuffer* read_buffer, int* out_fence_fd,
              void* out_meta, size_t user_metadata_size);
  int AcquireBuffer(int timeout, DvrReadBuffer** out_read_buffer,
                    DvrNativeBufferMetadata* out_meta, int* out_fence_fd);
  int ReleaseBuffer(DvrReadBuffer* read_buffer,
                    const DvrNativeBufferMetadata* meta, int release_fence_fd);
  void SetBufferAvailableCallback(
      DvrReadBufferQueueBufferAvailableCallback callback, void* context);
  void SetBufferRemovedCallback(
      DvrReadBufferQueueBufferRemovedCallback callback, void* context);
  int HandleEvents();

 private:
  std::shared_ptr<ConsumerQueue> consumer_queue_;
  std::array<std::unique_ptr<DvrReadBuffer>, BufferHubQueue::kMaxQueueCapacity>
      read_buffers_;
};

#endif  // ANDROID_DVR_BUFFER_QUEUE_INTERNAL_H_
