#ifndef ANDROID_DVR_BROADCAST_RING_H_
#define ANDROID_DVR_BROADCAST_RING_H_

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <atomic>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "android-base/logging.h"

#if ATOMIC_LONG_LOCK_FREE != 2 || ATOMIC_INT_LOCK_FREE != 2
#error "This file requires lock free atomic uint32_t and long"
#endif

namespace android {
namespace dvr {

struct DefaultRingTraits {
  // Set this to false to allow compatibly expanding the record size.
  static constexpr bool kUseStaticRecordSize = false;

  // Set this to a nonzero value to fix the number of records in the ring.
  static constexpr uint32_t kStaticRecordCount = 0;

  // Set this to the max number of records that can be written simultaneously.
  static constexpr uint32_t kMaxReservedRecords = 1;

  // Set this to the min number of records that must be readable.
  static constexpr uint32_t kMinAvailableRecords = 1;
};

// Nonblocking ring suitable for concurrent single-writer, multi-reader access.
//
// Readers never block the writer and thus this is a nondeterministically lossy
// transport in the absence of external synchronization. Don't use this as a
// transport when deterministic behavior is required.
//
// Readers may have a read-only mapping; each reader's state is a single local
// sequence number.
//
// The implementation takes care to avoid data races on record access.
// Inconsistent data can only be returned if at least 2^32 records are written
// during the read-side critical section.
//
// In addition, both readers and the writer are careful to avoid accesses
// outside the bounds of the mmap area passed in during initialization even if
// there is a misbehaving or malicious task with write access to the mmap area.
//
// When dynamic record size is enabled, readers use the record size in the ring
// header when indexing the ring, so that it is possible to extend the record
// type without breaking the read-side ABI.
//
// Avoid calling Put() in a tight loop; there should be significantly more time
// between successive puts than it takes to read one record from memory to
// ensure Get() completes quickly. This requirement should not be difficult to
// achieve for most practical uses; 4kB puts at 10,000Hz is well below the
// scaling limit on current mobile chips.
//
// Example Writer Usage:
//
//   using Record = MyRecordType;
//   using Ring = BroadcastRing<Record>;
//
//   uint32_t record_count = kMyDesiredCount;
//   uint32_t ring_size = Ring::MemorySize(record_count);
//
//   size_t page_size = sysconf(_SC_PAGESIZE);
//   uint32_t mmap_size = (ring_size + (page_size - 1)) & ~(page_size - 1);
//
//   // Allocate & map via your preferred mechanism, e.g.
//   int fd = open("/dev/shm/ring_test", O_CREAT|O_RDWR|O_CLOEXEC, 0600);
//   CHECK(fd >= 0);
//   CHECK(!ftruncate(fd, ring_size));
//   void *mmap_base = mmap(nullptr, mmap_size, PROT_READ|PROT_WRITE,
//                          MAP_SHARED, fd, 0);
//   CHECK(mmap_base != MAP_FAILED);
//   close(fd);
//
//   Ring ring = Ring::Create(mmap_base, mmap_size, record_count);
//
//   while (!done)
//     ring.Put(BuildNextRecordBlocking());
//
//   CHECK(!munmap(mmap_base, mmap_size));
//
// Example Reader Usage:
//
//   using Record = MyRecordType;
//   using Ring = BroadcastRing<Record>;
//
//   // Map via your preferred mechanism, e.g.
//   int fd = open("/dev/shm/ring_test", O_RDONLY|O_CLOEXEC);
//   CHECK(fd >= 0);
//   struct stat st;
//   CHECK(!fstat(fd, &st));
//   size_t mmap_size = st.st_size;
//   void *mmap_base = mmap(nullptr, mmap_size, PROT_READ,
//                          MAP_SHARED, fd, 0);
//   CHECK(mmap_base != MAP_FAILED);
//   close(fd);
//
//   Ring ring;
//   bool import_ok;
//   std::tie(ring, import_ok) = Ring::Import(mmap_base, mmap_size);
//   CHECK(import_ok);
//
//   uint32_t sequence;
//
//   // Choose starting point (using "0" is unpredictable but not dangerous)
//   sequence = ring.GetOldestSequence();  // The oldest available
//   sequence = ring.GetNewestSequence();  // The newest available
//   sequence = ring.GetNextSequence();    // The next one produced
//
//   while (!done) {
//     Record record;
//
//     if (you_want_to_process_all_available_records) {
//       while (ring.Get(&sequence, &record)) {
//         ProcessRecord(sequence, record);
//         sequence++;
//       }
//     } else if (you_want_to_skip_to_the_newest_record) {
//       if (ring.GetNewest(&sequence, &record)) {
//         ProcessRecord(sequence, record);
//         sequence++;
//       }
//     }
//
//     DoSomethingExpensiveOrBlocking();
//   }
//
//   CHECK(!munmap(mmap_base, mmap_size));
//
template <typename RecordType, typename BaseTraits = DefaultRingTraits>
class BroadcastRing {
 public:
  using Record = RecordType;
  struct Traits : public BaseTraits {
    // Must have enough space for writers, plus enough space for readers.
    static constexpr int kMinRecordCount =
        BaseTraits::kMaxReservedRecords + BaseTraits::kMinAvailableRecords;

    // Count of zero means dynamic, non-zero means static.
    static constexpr bool kUseStaticRecordCount =
        (BaseTraits::kStaticRecordCount != 0);

    // If both record size and count are static then the overall size is too.
    static constexpr bool kIsStaticSize =
        BaseTraits::kUseStaticRecordSize && kUseStaticRecordCount;
  };

  static constexpr bool IsPowerOfTwo(uint32_t size) {
    return (size & (size - 1)) == 0;
  }

  // Sanity check the options provided in Traits.
  static_assert(Traits::kMinRecordCount >= 1, "Min record count too small");
  static_assert(!Traits::kUseStaticRecordCount ||
                    Traits::kStaticRecordCount >= Traits::kMinRecordCount,
                "Static record count is too small");
  static_assert(!Traits::kStaticRecordCount ||
                    IsPowerOfTwo(Traits::kStaticRecordCount),
                "Static record count is not a power of two");
  static_assert(std::is_standard_layout<Record>::value,
                "Record type must be standard layout");

  BroadcastRing() {}

  // Creates a new ring at |mmap| with |record_count| records.
  //
  // There must be at least |MemorySize(record_count)| bytes of space already
  // allocated at |mmap|. The ring does not take ownership.
  static BroadcastRing Create(void* mmap, size_t mmap_size,
                              uint32_t record_count) {
    BroadcastRing ring(mmap);
    CHECK(ring.ValidateGeometry(mmap_size, sizeof(Record), record_count));
    ring.InitializeHeader(sizeof(Record), record_count);
    return ring;
  }

  // Creates a new ring at |mmap|.
  //
  // There must be at least |MemorySize()| bytes of space already allocated at
  // |mmap|. The ring does not take ownership.
  static BroadcastRing Create(void* mmap, size_t mmap_size) {
    return Create(mmap, mmap_size,
                  Traits::kUseStaticRecordCount
                      ? Traits::kStaticRecordCount
                      : BroadcastRing::GetRecordCount(mmap_size));
  }

  // Imports an existing ring at |mmap|.
  //
  // Import may fail if the ring parameters in the mmap header are not sensible.
  // In this case the returned boolean is false; make sure to check this value.
  static std::tuple<BroadcastRing, bool> Import(void* mmap, size_t mmap_size) {
    BroadcastRing ring(mmap);
    uint32_t record_size = 0;
    uint32_t record_count = 0;
    if (mmap_size >= sizeof(Header)) {
      record_size = std::atomic_load_explicit(&ring.header_mmap()->record_size,
                                              std::memory_order_relaxed);
      record_count = std::atomic_load_explicit(
          &ring.header_mmap()->record_count, std::memory_order_relaxed);
    }
    bool ok = ring.ValidateGeometry(mmap_size, record_size, record_count);
    return std::make_tuple(ring, ok);
  }

  ~BroadcastRing() {}

  // Calculates the space necessary for a ring of size |record_count|.
  //
  // Use this function for dynamically sized rings.
  static constexpr size_t MemorySize(uint32_t record_count) {
    return sizeof(Header) + sizeof(Record) * record_count;
  }

  // Calculates the space necessary for a statically sized ring.
  //
  // Use this function for statically sized rings.
  static constexpr size_t MemorySize() {
    static_assert(
        Traits::kUseStaticRecordCount,
        "Wrong MemorySize() function called for dynamic record count");
    return MemorySize(Traits::kStaticRecordCount);
  }

  static uint32_t NextPowerOf2(uint32_t n) {
    if (n == 0)
      return 0;
    n -= 1;
    n |= n >> 16;
    n |= n >> 8;
    n |= n >> 4;
    n |= n >> 2;
    n |= n >> 1;
    return n + 1;
  }

  // Gets the biggest power of 2 record count that can fit into this mmap.
  //
  // The header size has been taken into account.
  static uint32_t GetRecordCount(size_t mmap_size) {
    if (mmap_size <= sizeof(Header)) {
      return 0;
    }
    uint32_t count =
        static_cast<uint32_t>((mmap_size - sizeof(Header)) / sizeof(Record));
    return IsPowerOfTwo(count) ? count : (NextPowerOf2(count) / 2);
  }

  // Writes a record to the ring.
  //
  // The oldest record is overwritten unless the ring is not already full.
  void Put(const Record& record) {
    const int kRecordCount = 1;
    Reserve(kRecordCount);
    Geometry geometry = GetGeometry();
    PutRecordInternal(&record, record_mmap_writer(geometry.tail_index));
    Publish(kRecordCount);
  }

  // Gets sequence number of the oldest currently available record.
  uint32_t GetOldestSequence() const {
    return std::atomic_load_explicit(&header_mmap()->head,
                                     std::memory_order_relaxed);
  }

  // Gets sequence number of the first future record.
  //
  // If the returned value is passed to Get() and there is no concurrent Put(),
  // Get() will return false.
  uint32_t GetNextSequence() const {
    return std::atomic_load_explicit(&header_mmap()->tail,
                                     std::memory_order_relaxed);
  }

  // Gets sequence number of the newest currently available record.
  uint32_t GetNewestSequence() const { return GetNextSequence() - 1; }

  // Copies the oldest available record with sequence at least |*sequence| to
  // |record|.
  //
  // Returns false if there is no recent enough record available.
  //
  // Updates |*sequence| with the sequence number of the record returned. To get
  // the following record, increment this number by one.
  //
  // This function synchronizes with two other operations:
  //
  //    (1) Load-Acquire of |tail|
  //
  //        Together with the store-release in Publish(), this load-acquire
  //        ensures each store to a record in PutRecordInternal() happens-before
  //        any corresponding load in GetRecordInternal().
  //
  //        i.e. the stores for the records with sequence numbers < |tail| have
  //        completed from our perspective
  //
  //    (2) Acquire Fence between record access & final load of |head|
  //
  //        Together with the release fence in Reserve(), this ensures that if
  //        GetRecordInternal() loads a value stored in some execution of
  //        PutRecordInternal(), then the store of |head| in the Reserve() that
  //        preceeded it happens-before our final load of |head|.
  //
  //        i.e. if we read a record with sequence number >= |final_head| then
  //        no later store to that record has completed from our perspective
  bool Get(uint32_t* sequence /*inout*/, Record* record /*out*/) const {
    for (;;) {
      uint32_t tail = std::atomic_load_explicit(&header_mmap()->tail,
                                                std::memory_order_acquire);
      uint32_t head = std::atomic_load_explicit(&header_mmap()->head,
                                                std::memory_order_relaxed);

      if (tail - head > record_count())
        continue;  // Concurrent modification; re-try.

      if (*sequence - head > tail - head)
        *sequence = head;  // Out of window, skip forward to first available.

      if (*sequence == tail) return false;  // No new records available.

      Geometry geometry =
          CalculateGeometry(record_count(), record_size(), *sequence, tail);

      // Compute address explicitly in case record_size > sizeof(Record).
      RecordStorage* record_storage = record_mmap_reader(geometry.head_index);

      GetRecordInternal(record_storage, record);

      // NB: It is not sufficient to change this to a load-acquire of |head|.
      std::atomic_thread_fence(std::memory_order_acquire);

      uint32_t final_head = std::atomic_load_explicit(
          &header_mmap()->head, std::memory_order_relaxed);

      if (final_head - head > *sequence - head)
        continue;  // Concurrent modification; re-try.

      // Note: Combining the above 4 comparisons gives:
      // 0 <= final_head - head <= sequence - head < tail - head <= record_count
      //
      // We can also write this as:
      // head <=* final_head <=* sequence <* tail <=* head + record_count
      //
      // where <* orders by difference from head: x <* y if x - head < y - head.
      // This agrees with the order of sequence updates during "put" operations.
      return true;
    }
  }

  // Copies the newest available record with sequence at least |*sequence| to
  // |record|.
  //
  // Returns false if there is no recent enough record available.
  //
  // Updates |*sequence| with the sequence number of the record returned. To get
  // the following record, increment this number by one.
  bool GetNewest(uint32_t* sequence, Record* record) const {
    uint32_t newest_sequence = GetNewestSequence();
    if (*sequence == newest_sequence + 1) return false;
    *sequence = newest_sequence;
    return Get(sequence, record);
  }

  // Returns true if this instance has been created or imported.
  bool is_valid() const { return !!data_.mmap; }

  uint32_t record_count() const { return record_count_internal(); }
  uint32_t record_size() const { return record_size_internal(); }
  static constexpr uint32_t mmap_alignment() { return alignof(Mmap); }

 private:
  struct Header {
    // Record size for reading out of the ring. Writers always write the full
    // length; readers may need to read a prefix of each record.
    std::atomic<uint32_t> record_size;

    // Number of records in the ring.
    std::atomic<uint32_t> record_count;

    // Readable region is [head % record_count, tail % record_count).
    //
    // The region in [tail % record_count, head % record_count) was either never
    // populated or is being updated.
    //
    // These are sequences numbers, not indexes - indexes should be computed
    // with a modulus.
    //
    // To ensure consistency:
    //
    // (1) Writes advance |head| past any updated records before writing to
    //     them, and advance |tail| after they are written.
    // (2) Readers check |tail| before reading data and |head| after,
    //     making sure to discard any data that was written to concurrently.
    std::atomic<uint32_t> head;
    std::atomic<uint32_t> tail;
  };

  // Store using the standard word size.
  using StorageType = long;  // NOLINT

  // Always require 8 byte alignment so that the same record sizes are legal on
  // 32 and 64 bit builds.
  static constexpr size_t kRecordAlignment = 8;
  static_assert(kRecordAlignment % sizeof(StorageType) == 0,
                "Bad record alignment");

  struct RecordStorage {
    // This is accessed with relaxed atomics to prevent data races on the
    // contained data, which would be undefined behavior.
    std::atomic<StorageType> data[sizeof(Record) / sizeof(StorageType)];
  };

  static_assert(sizeof(StorageType) *
                        std::extent<decltype(RecordStorage::data)>() ==
                    sizeof(Record),
                "Record length must be a multiple of sizeof(StorageType)");

  struct Geometry {
    // Static geometry.
    uint32_t record_count;
    uint32_t record_size;

    // Copy of atomic sequence counts.
    uint32_t head;
    uint32_t tail;

    // First index of readable region.
    uint32_t head_index;

    // First index of writable region.
    uint32_t tail_index;

    // Number of records in readable region.
    uint32_t count;

    // Number of records in writable region.
    uint32_t space;
  };

  // Mmap area layout.
  //
  // Readers should not index directly into |records| as this is not valid when
  // dynamic record sizes are used; use record_mmap_reader() instead.
  struct Mmap {
    Header header;
    RecordStorage records[];
  };

  static_assert(std::is_standard_layout<Mmap>::value,
                "Mmap must be standard layout");
  static_assert(sizeof(std::atomic<uint32_t>) == sizeof(uint32_t),
                "Lockless atomics contain extra state");
  static_assert(sizeof(std::atomic<StorageType>) == sizeof(StorageType),
                "Lockless atomics contain extra state");

  explicit BroadcastRing(void* mmap) {
    CHECK_EQ(0U, reinterpret_cast<uintptr_t>(mmap) % alignof(Mmap));
    data_.mmap = reinterpret_cast<Mmap*>(mmap);
  }

  // Initializes the mmap area header for a new ring.
  void InitializeHeader(uint32_t record_size, uint32_t record_count) {
    constexpr uint32_t kInitialSequence = -256;  // Force an early wrap.
    std::atomic_store_explicit(&header_mmap()->record_size, record_size,
                               std::memory_order_relaxed);
    std::atomic_store_explicit(&header_mmap()->record_count, record_count,
                               std::memory_order_relaxed);
    std::atomic_store_explicit(&header_mmap()->head, kInitialSequence,
                               std::memory_order_relaxed);
    std::atomic_store_explicit(&header_mmap()->tail, kInitialSequence,
                               std::memory_order_relaxed);
  }

  // Validates ring geometry.
  //
  // Ring geometry is validated carefully on import and then cached. This allows
  // us to avoid out-of-range accesses even if the parameters in the header are
  // later changed.
  bool ValidateGeometry(size_t mmap_size, uint32_t header_record_size,
                        uint32_t header_record_count) {
    set_record_size(header_record_size);
    set_record_count(header_record_count);

    if (record_size() != header_record_size) return false;
    if (record_count() != header_record_count) return false;
    if (record_count() < Traits::kMinRecordCount) return false;
    if (record_size() < sizeof(Record)) return false;
    if (record_size() % kRecordAlignment != 0) return false;
    if (!IsPowerOfTwo(record_count())) return false;

    size_t memory_size = record_count() * record_size();
    if (memory_size / record_size() != record_count()) return false;
    if (memory_size + sizeof(Header) < memory_size) return false;
    if (memory_size + sizeof(Header) > mmap_size) return false;

    return true;
  }

  // Copies a record into the ring.
  //
  // This is done with relaxed atomics because otherwise it is racy according to
  // the C++ memory model. This is very low overhead once optimized.
  static inline void PutRecordInternal(const Record* in, RecordStorage* out) {
    StorageType data[sizeof(Record) / sizeof(StorageType)];
    memcpy(data, in, sizeof(*in));
    for (size_t i = 0; i < std::extent<decltype(data)>(); ++i) {
      std::atomic_store_explicit(&out->data[i], data[i],
                                 std::memory_order_relaxed);
    }
  }

  // Copies a record out of the ring.
  //
  // This is done with relaxed atomics because otherwise it is racy according to
  // the C++ memory model. This is very low overhead once optimized.
  static inline void GetRecordInternal(RecordStorage* in, Record* out) {
    StorageType data[sizeof(Record) / sizeof(StorageType)];
    for (size_t i = 0; i < std::extent<decltype(data)>(); ++i) {
      data[i] =
          std::atomic_load_explicit(&in->data[i], std::memory_order_relaxed);
    }
    memcpy(out, &data, sizeof(*out));
  }

  // Converts a record's sequence number into a storage index.
  static uint32_t SequenceToIndex(uint32_t sequence, uint32_t record_count) {
    return sequence & (record_count - 1);
  }

  // Computes readable & writable ranges from ring parameters.
  static Geometry CalculateGeometry(uint32_t record_count, uint32_t record_size,
                                    uint32_t head, uint32_t tail) {
    Geometry geometry;
    geometry.record_count = record_count;
    geometry.record_size = record_size;
    DCHECK_EQ(0U, geometry.record_size % kRecordAlignment);
    geometry.head = head;
    geometry.tail = tail;
    geometry.head_index = SequenceToIndex(head, record_count);
    geometry.tail_index = SequenceToIndex(tail, record_count);
    geometry.count = geometry.tail - geometry.head;
    DCHECK_LE(geometry.count, record_count);
    geometry.space = geometry.record_count - geometry.count;
    return geometry;
  }

  // Gets the current ring readable & writable regions.
  //
  // This this is always safe from the writing thread since it is the only
  // thread allowed to update the header.
  Geometry GetGeometry() const {
    return CalculateGeometry(
        record_count(), record_size(),
        std::atomic_load_explicit(&header_mmap()->head,
                                  std::memory_order_relaxed),
        std::atomic_load_explicit(&header_mmap()->tail,
                                  std::memory_order_relaxed));
  }

  // Makes space for at least |reserve_count| records.
  //
  // There is nothing to prevent overwriting records that have concurrent
  // readers. We do however ensure that this situation can be detected: the
  // fence ensures the |head| update will be the first update seen by readers,
  // and readers check this value after reading and discard data that may have
  // been concurrently modified.
  void Reserve(uint32_t reserve_count) {
    Geometry geometry = GetGeometry();
    DCHECK_LE(reserve_count, Traits::kMaxReservedRecords);
    uint32_t needed =
        (geometry.space >= reserve_count ? 0 : reserve_count - geometry.space);

    std::atomic_store_explicit(&header_mmap()->head, geometry.head + needed,
                               std::memory_order_relaxed);

    // NB: It is not sufficient to change this to a store-release of |head|.
    std::atomic_thread_fence(std::memory_order_release);
  }

  // Makes |publish_count| records visible to readers.
  //
  // Space must have been reserved by a previous call to Reserve().
  void Publish(uint32_t publish_count) {
    Geometry geometry = GetGeometry();
    DCHECK_LE(publish_count, geometry.space);
    std::atomic_store_explicit(&header_mmap()->tail,
                               geometry.tail + publish_count,
                               std::memory_order_release);
  }

  // Helpers to compute addresses in mmap area.
  Mmap* mmap() const { return data_.mmap; }
  Header* header_mmap() const { return &data_.mmap->header; }
  RecordStorage* record_mmap_writer(uint32_t index) const {
    DCHECK_EQ(sizeof(Record), record_size());
    return &data_.mmap->records[index];
  }
  RecordStorage* record_mmap_reader(uint32_t index) const {
    if (Traits::kUseStaticRecordSize) {
      return &data_.mmap->records[index];
    } else {
      // Calculate the location of a record in the ring without assuming that
      // sizeof(Record) == record_size.
      return reinterpret_cast<RecordStorage*>(
          reinterpret_cast<char*>(data_.mmap->records) + index * record_size());
    }
  }

  // The following horrifying template gunk enables us to store just the mmap
  // base pointer for compile-time statically sized rings. Dynamically sized
  // rings also store the validated copy of the record size & count.
  //
  // This boils down to: use a compile time constant if available, and otherwise
  // load the value that was validated on import from a member variable.
  template <typename T = Traits>
  typename std::enable_if<T::kUseStaticRecordSize, uint32_t>::type
  record_size_internal() const {
    return sizeof(Record);
  }

  template <typename T = Traits>
  typename std::enable_if<!T::kUseStaticRecordSize, uint32_t>::type
  record_size_internal() const {
    return data_.record_size;
  }

  template <typename T = Traits>
  typename std::enable_if<T::kUseStaticRecordSize, void>::type set_record_size(
      uint32_t /*record_size*/) {}

  template <typename T = Traits>
  typename std::enable_if<!T::kUseStaticRecordSize, void>::type set_record_size(
      uint32_t record_size) {
    data_.record_size = record_size;
  }

  template <typename T = Traits>
  typename std::enable_if<T::kUseStaticRecordCount, uint32_t>::type
  record_count_internal() const {
    return Traits::kStaticRecordCount;
  }

  template <typename T = Traits>
  typename std::enable_if<!T::kUseStaticRecordCount, uint32_t>::type
  record_count_internal() const {
    return data_.record_count;
  }

  template <typename T = Traits>
  typename std::enable_if<T::kUseStaticRecordCount, void>::type
  set_record_count(uint32_t /*record_count*/) const {}

  template <typename T = Traits>
  typename std::enable_if<!T::kUseStaticRecordCount, void>::type
  set_record_count(uint32_t record_count) {
    data_.record_count = record_count;
  }

  // Data we need to store for statically sized rings.
  struct DataStaticSize {
    Mmap* mmap = nullptr;
  };

  // Data we need to store for dynamically sized rings.
  struct DataDynamicSize {
    Mmap* mmap = nullptr;

    // These are cached to make sure misbehaving writers cannot cause
    // out-of-bounds memory accesses by updating the values in the mmap header.
    uint32_t record_size = 0;
    uint32_t record_count = 0;
  };

  using DataStaticOrDynamic =
      typename std::conditional<Traits::kIsStaticSize, DataStaticSize,
                                DataDynamicSize>::type;

  DataStaticOrDynamic data_;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_BROADCAST_RING_H_
