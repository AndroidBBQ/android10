#ifndef ANDROID_DVR_SHARED_BUFFER_HELPERS_H_
#define ANDROID_DVR_SHARED_BUFFER_HELPERS_H_

#include <assert.h>
#include <tuple>

#include <libbroadcastring/broadcast_ring.h>
#include <private/dvr/display_client.h>

namespace android {
namespace dvr {

// The buffer usage type for mapped shared buffers.
enum class CPUUsageMode { READ_OFTEN, READ_RARELY, WRITE_OFTEN, WRITE_RARELY };

// Holds the memory for the mapped shared buffer. Unlocks and releases the
// underlying IonBuffer in destructor.
class CPUMappedBuffer {
 public:
  // This constructor will create a display client and get the buffer from it.
  CPUMappedBuffer(DvrGlobalBufferKey key, CPUUsageMode mode);

  // If you already have the IonBuffer, use this. It will take ownership.
  CPUMappedBuffer(std::unique_ptr<IonBuffer> buffer, CPUUsageMode mode);

  // Use this if you do not want to take ownership.
  CPUMappedBuffer(IonBuffer* buffer, CPUUsageMode mode);

  ~CPUMappedBuffer();

  // Getters.
  size_t Size() const { return size_; }
  void* Address() const { return address_; }
  bool IsMapped() const { return Address() != nullptr; }

  // Attempt mapping this buffer to the CPU addressable space.
  // This will create a display client and see if the buffer exists.
  // If the buffer has not been setup yet, you will need to try again later.
  void TryMapping();

 protected:
  // The memory area if we managed to map it.
  size_t size_ = 0;
  void* address_ = nullptr;

  // If we are polling the display client, the buffer key here.
  DvrGlobalBufferKey buffer_key_;

  // If we just own the IonBuffer outright, it's here.
  std::unique_ptr<IonBuffer> owned_buffer_ = nullptr;

  // The last time we connected to the display service.
  int64_t last_display_service_connection_ns_ = 0;

  // If we do not own the IonBuffer, it's here
  IonBuffer* buffer_ = nullptr;

  // The usage mode.
  CPUUsageMode usage_mode_ = CPUUsageMode::READ_OFTEN;
};

// Represents a broadcast ring inside a mapped shared memory buffer.
// If has the same set of constructors as CPUMappedBuffer.
// The template argument is the concrete BroadcastRing class that this buffer
// holds.
template <class RingType>
class CPUMappedBroadcastRing : public CPUMappedBuffer {
 public:
  CPUMappedBroadcastRing(DvrGlobalBufferKey key, CPUUsageMode mode)
      : CPUMappedBuffer(key, mode) {}

  CPUMappedBroadcastRing(std::unique_ptr<IonBuffer> buffer, CPUUsageMode mode)
      : CPUMappedBuffer(std::move(buffer), mode) {}

  CPUMappedBroadcastRing(IonBuffer* buffer, CPUUsageMode mode)
      : CPUMappedBuffer(buffer, mode) {}

  // Helper function for publishing records in the ring.
  void Publish(const typename RingType::Record& record) {
    assert((usage_mode_ == CPUUsageMode::WRITE_OFTEN) ||
           (usage_mode_ == CPUUsageMode::WRITE_RARELY));

    auto ring = Ring();
    if (ring) {
      ring->Put(record);
    }
  }

  // Helper function for getting records from the ring.
  // Returns true if we were able to retrieve the latest.
  bool GetNewest(typename RingType::Record* record) {
    assert((usage_mode_ == CPUUsageMode::READ_OFTEN) ||
           (usage_mode_ == CPUUsageMode::READ_RARELY));

    auto ring = Ring();
    if (ring) {
      return ring->GetNewest(&sequence_, record);
    }

    return false;
  }

  // Try obtaining the ring. If the named buffer has not been created yet, it
  // will return nullptr.
  RingType* Ring() {
    // No ring created yet?
    if (ring_ == nullptr) {
      // Not mapped the memory yet?
      if (IsMapped() == false) {
        TryMapping();
      }

      // If have the memory mapped, allocate the ring.
      if (IsMapped()) {
        switch (usage_mode_) {
          case CPUUsageMode::READ_OFTEN:
          case CPUUsageMode::READ_RARELY: {
            RingType ring;
            bool import_ok;
            std::tie(ring, import_ok) = RingType::Import(address_, size_);
            if (import_ok) {
              ring_ = std::make_unique<RingType>(ring);
            }
          } break;
          case CPUUsageMode::WRITE_OFTEN:
          case CPUUsageMode::WRITE_RARELY:
            ring_ =
                std::make_unique<RingType>(RingType::Create(address_, size_));
            break;
        }
      }
    }

    return ring_.get();
  }

 protected:
  std::unique_ptr<RingType> ring_ = nullptr;

  uint32_t sequence_ = 0;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_SHARED_BUFFER_HELPERS_H_
