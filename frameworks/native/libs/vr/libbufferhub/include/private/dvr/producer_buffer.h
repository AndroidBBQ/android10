#ifndef ANDROID_DVR_PRODUCER_BUFFER_H_
#define ANDROID_DVR_PRODUCER_BUFFER_H_

#include <private/dvr/buffer_hub_base.h>

namespace android {
namespace dvr {

// This represents a writable buffer. Calling Post notifies all clients and
// makes the buffer read-only. Call Gain to acquire write access. A buffer
// may have many consumers.
//
// The user of ProducerBuffer is responsible with making sure that the Post() is
// done with the correct metadata type and size. The user is also responsible
// for making sure that remote ends (ConsumerBuffers) are also using the correct
// metadata when acquiring the buffer. The API guarantees that a Post() with a
// metadata of wrong size will fail. However, it currently does not do any
// type checking.
// The API also assumes that metadata is a serializable type (plain old data).
class ProducerBuffer : public pdx::ClientBase<ProducerBuffer, BufferHubBase> {
 public:
  // Imports a bufferhub producer channel, assuming ownership of its handle.
  static std::unique_ptr<ProducerBuffer> Import(LocalChannelHandle channel);
  static std::unique_ptr<ProducerBuffer> Import(
      Status<LocalChannelHandle> status);

  // Asynchronously posts a buffer. The fence and metadata are passed to
  // consumer via shared fd and shared memory.
  int PostAsync(const DvrNativeBufferMetadata* meta,
                const LocalHandle& ready_fence);

  // Post this buffer, passing |ready_fence| to the consumers. The bytes in
  // |meta| are passed unaltered to the consumers. The producer must not modify
  // the buffer until it is re-gained.
  // This returns zero or a negative unix error code.
  int Post(const LocalHandle& ready_fence, const void* meta,
           size_t user_metadata_size);

  int Post(const LocalHandle& ready_fence) {
    return Post(ready_fence, nullptr, 0);
  }

  // Attempt to re-gain the buffer for writing. If |release_fence| is valid, it
  // must be waited on before using the buffer. If it is not valid then the
  // buffer is free for immediate use. This call will succeed if the buffer
  // is in the released state, or in posted state and gain_posted_buffer is
  // true.
  //
  // @param release_fence output fence.
  // @param gain_posted_buffer whether to gain posted buffer or not.
  // @return This returns zero or a negative unix error code.
  int Gain(LocalHandle* release_fence, bool gain_posted_buffer = false);

  // Asynchronously marks a released buffer as gained. This method is similar to
  // the synchronous version above, except that it does not wait for BufferHub
  // to acknowledge success or failure. Because of the asynchronous nature of
  // the underlying message, no error is returned if this method is called when
  // the buffer is in an incorrect state. Returns zero if sending the message
  // succeeded, or a negative errno code if local error check fails.
  // TODO(b/112007999): gain_posted_buffer true is only used to prevent
  // libdvrtracking from starving when there are non-responding clients. This
  // gain_posted_buffer param can be removed once libdvrtracking start to use
  // the new AHardwareBuffer API.
  int GainAsync(DvrNativeBufferMetadata* out_meta, LocalHandle* out_fence,
                bool gain_posted_buffer = false);
  int GainAsync();

  // Detaches a ProducerBuffer from an existing producer/consumer set. Can only
  // be called when a producer buffer has exclusive access to the buffer (i.e.
  // in the gain'ed state). On the successful return of the IPC call, a new
  // LocalChannelHandle representing a detached buffer will be returned and all
  // existing producer and consumer channels will be closed. Further IPCs
  // towards those channels will return error.
  Status<LocalChannelHandle> Detach();

 private:
  friend BASE;

  // Constructors are automatically exposed through ProducerBuffer::Create(...)
  // static template methods inherited from ClientBase, which take the same
  // arguments as the constructors.

  // Constructs a buffer with the given geometry and parameters.
  ProducerBuffer(uint32_t width, uint32_t height, uint32_t format,
                 uint64_t usage, size_t metadata_size = 0);

  // Constructs a blob (flat) buffer with the given usage flags.
  ProducerBuffer(uint64_t usage, size_t size);

  // Imports the given file handle to a producer channel, taking ownership.
  explicit ProducerBuffer(LocalChannelHandle channel);

  // Local state transition helpers.
  int LocalGain(DvrNativeBufferMetadata* out_meta, LocalHandle* out_fence,
                bool gain_posted_buffer = false);
  int LocalPost(const DvrNativeBufferMetadata* meta,
                const LocalHandle& ready_fence);
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_PRODUCER_BUFFER_H_
