#ifndef ANDROID_DVR_CONSUMER_BUFFER_H_
#define ANDROID_DVR_CONSUMER_BUFFER_H_

#include <private/dvr/buffer_hub_base.h>

namespace android {
namespace dvr {

// This is a connection to a producer buffer, which can be located in another
// application. When that buffer is Post()ed, this fd will be signaled and
// Acquire allows read access. The user is responsible for making sure that
// Acquire is called with the correct metadata structure. The only guarantee the
// API currently provides is that an Acquire() with metadata of the wrong size
// will fail.
class ConsumerBuffer : public pdx::ClientBase<ConsumerBuffer, BufferHubBase> {
 public:
  // This call assumes ownership of |fd|.
  static std::unique_ptr<ConsumerBuffer> Import(LocalChannelHandle channel);
  static std::unique_ptr<ConsumerBuffer> Import(
      Status<LocalChannelHandle> status);

  // Attempt to retrieve a post event from buffer hub. If successful,
  // |ready_fence| will be set to a fence to wait on until the buffer is ready.
  // This call will only succeed after the fd is signalled. This call may be
  // performed as an alternative to the Acquire() with metadata. In such cases
  // the metadata is not read.
  //
  // This returns zero or negative unix error code.
  int Acquire(LocalHandle* ready_fence);

  // Attempt to retrieve a post event from buffer hub. If successful,
  // |ready_fence| is set to a fence signaling that the contents of the buffer
  // are available. This call will only succeed if the buffer is in the posted
  // state.
  // Returns zero on success, or a negative errno code otherwise.
  int Acquire(LocalHandle* ready_fence, void* meta, size_t user_metadata_size);

  // Asynchronously acquires a bufer.
  int AcquireAsync(DvrNativeBufferMetadata* out_meta, LocalHandle* out_fence);

  // Releases the buffer from any buffer state. If the fence is valid the fence
  // determines the buffer usage, otherwise the buffer is released immediately.
  // This returns zero or a negative unix error code.
  int Release(const LocalHandle& release_fence);
  int ReleaseAsync();

  // Asynchronously releases a buffer. Similar to the synchronous version above,
  // except that it does not wait for BufferHub to reply with success or error.
  // The fence and metadata are passed to consumer via shared fd and shared
  // memory.
  int ReleaseAsync(const DvrNativeBufferMetadata* meta,
                   const LocalHandle& release_fence);

  // May be called after or instead of Acquire to indicate that the consumer
  // does not need to access the buffer this cycle. This returns zero or a
  // negative unix error code.
  int Discard();

 private:
  friend BASE;

  explicit ConsumerBuffer(LocalChannelHandle channel);

  // Local state transition helpers.
  int LocalAcquire(DvrNativeBufferMetadata* out_meta, LocalHandle* out_fence);
  int LocalRelease(const DvrNativeBufferMetadata* meta,
                   const LocalHandle& release_fence);
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_CONSUMER_BUFFER_H_
