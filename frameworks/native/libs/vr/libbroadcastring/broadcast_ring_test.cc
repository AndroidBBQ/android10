#include "libbroadcastring/broadcast_ring.h"

#include <stdlib.h>
#include <memory>
#include <thread>  // NOLINT
#include <sys/mman.h>

#include <gtest/gtest.h>

namespace android {
namespace dvr {
namespace {

template <uint32_t N>
struct alignas(8) Aligned {
  char v[N];
};

template <uint32_t N>
struct alignas(8) Sized {
  Sized() { Clear(); }
  explicit Sized(char c) { Fill(c); }
  char v[sizeof(Aligned<N>)];
  void Clear() { memset(v, 0, sizeof(v)); }
  void Fill(char c) { memset(v, c, sizeof(v)); }
  static Sized Pattern(uint8_t c) {
    Sized sized;
    for (size_t i = 0; i < sizeof(v); ++i) {
      sized.v[i] = static_cast<char>(c + i);
    }
    return sized;
  }
  bool operator==(const Sized& right) const {
    static_assert(sizeof(*this) == sizeof(v), "Size mismatch");
    return !memcmp(v, right.v, sizeof(v));
  }
  template <typename SmallerSized>
  SmallerSized Truncate() const {
    SmallerSized val;
    static_assert(sizeof(val.v) <= sizeof(v), "Cannot truncate to larger size");
    memcpy(val.v, v, sizeof(val.v));
    return val;
  }
};

char FillChar(int val) { return static_cast<char>(val); }

struct FakeMmap {
  explicit FakeMmap(size_t size) : size(size), data(new char[size]) {}
  size_t size;
  std::unique_ptr<char[]> data;
  void* mmap() { return static_cast<void*>(data.get()); }
};

template <typename Ring>
FakeMmap CreateRing(Ring* ring, uint32_t count) {
  FakeMmap mmap(Ring::MemorySize(count));
  *ring = Ring::Create(mmap.mmap(), mmap.size, count);
  return mmap;
}

template <typename RecordType, bool StaticSize = false,
          uint32_t StaticCount = 0, uint32_t MaxReserved = 1,
          uint32_t MinAvailable = 0>
struct Traits {
  using Record = RecordType;
  static constexpr bool kUseStaticRecordSize = StaticSize;
  static constexpr uint32_t kStaticRecordCount = StaticCount;
  static constexpr uint32_t kMaxReservedRecords = MaxReserved;
  static constexpr uint32_t kMinAvailableRecords = MinAvailable;
  static constexpr uint32_t kMinRecordCount = MaxReserved + MinAvailable;
};

template <typename Record, bool StaticSize = false, uint32_t MaxReserved = 1,
          uint32_t MinAvailable = 7>
struct TraitsDynamic
    : public Traits<Record, StaticSize, 0, MaxReserved, MinAvailable> {
  using Ring = BroadcastRing<Record, TraitsDynamic>;
  static uint32_t MinCount() { return MaxReserved + MinAvailable; }
};

template <typename Record, uint32_t StaticCount = 1, bool StaticSize = true,
          uint32_t MaxReserved = 1, uint32_t MinAvailable = 0>
struct TraitsStatic
    : public Traits<Record, true, StaticCount, MaxReserved, MinAvailable> {
  using Ring = BroadcastRing<Record, TraitsStatic>;
  static uint32_t MinCount() { return StaticCount; }
};

using Dynamic_8_NxM = TraitsDynamic<Sized<8>>;
using Dynamic_16_NxM = TraitsDynamic<Sized<16>>;
using Dynamic_32_NxM = TraitsDynamic<Sized<32>>;
using Dynamic_32_32xM = TraitsDynamic<Sized<32>, true>;
using Dynamic_16_NxM_1plus0 = TraitsDynamic<Sized<16>, false, 1, 0>;
using Dynamic_16_NxM_1plus1 = TraitsDynamic<Sized<16>, false, 1, 1>;
using Dynamic_16_NxM_5plus11 = TraitsDynamic<Sized<16>, false, 5, 11>;
using Dynamic_256_NxM_1plus0 = TraitsDynamic<Sized<256>, false, 1, 0>;

using Static_8_8x1 = TraitsStatic<Sized<8>, 1>;
using Static_8_8x16 = TraitsStatic<Sized<8>, 16>;
using Static_16_16x8 = TraitsStatic<Sized<16>, 8>;
using Static_16_16x16 = TraitsStatic<Sized<16>, 16>;
using Static_16_16x32 = TraitsStatic<Sized<16>, 32>;
using Static_32_Nx8 = TraitsStatic<Sized<32>, 8, false>;

using TraitsList = ::testing::Types<Dynamic_8_NxM,           //
                                    Dynamic_16_NxM,          //
                                    Dynamic_32_NxM,          //
                                    Dynamic_32_32xM,         //
                                    Dynamic_16_NxM_1plus0,   //
                                    Dynamic_16_NxM_1plus1,   //
                                    Dynamic_16_NxM_5plus11,  //
                                    Dynamic_256_NxM_1plus0,  //
                                    Static_8_8x1,            //
                                    Static_8_8x16,           //
                                    Static_16_16x8,          //
                                    Static_16_16x16,         //
                                    Static_16_16x32,         //
                                    Static_32_Nx8>;

}  // namespace

template <typename T>
class BroadcastRingTest : public ::testing::Test {};

TYPED_TEST_CASE(BroadcastRingTest, TraitsList);

TYPED_TEST(BroadcastRingTest, Geometry) {
  using Record = typename TypeParam::Record;
  using Ring = typename TypeParam::Ring;
  Ring ring;
  auto mmap = CreateRing(&ring, Ring::Traits::MinCount());
  EXPECT_EQ(Ring::Traits::MinCount(), ring.record_count());
  EXPECT_EQ(sizeof(Record), ring.record_size());
}

TYPED_TEST(BroadcastRingTest, PutGet) {
  using Record = typename TypeParam::Record;
  using Ring = typename TypeParam::Ring;
  Ring ring;
  auto mmap = CreateRing(&ring, Ring::Traits::MinCount());
  const uint32_t oldest_sequence_at_start = ring.GetOldestSequence();
  const uint32_t next_sequence_at_start = ring.GetNextSequence();
  {
    uint32_t sequence = oldest_sequence_at_start;
    Record record;
    EXPECT_FALSE(ring.Get(&sequence, &record));
    EXPECT_EQ(oldest_sequence_at_start, sequence);
    EXPECT_EQ(Record(), record);
  }
  const Record original_record(0x1a);
  ring.Put(original_record);
  {
    uint32_t sequence = next_sequence_at_start;
    Record record;
    EXPECT_TRUE(ring.Get(&sequence, &record));
    EXPECT_EQ(next_sequence_at_start, sequence);
    EXPECT_EQ(original_record, record);
  }
  {
    uint32_t sequence = next_sequence_at_start + 1;
    Record record;
    EXPECT_FALSE(ring.Get(&sequence, &record));
    EXPECT_EQ(next_sequence_at_start + 1, sequence);
    EXPECT_EQ(Record(), record);
  }
}

TYPED_TEST(BroadcastRingTest, FillOnce) {
  using Record = typename TypeParam::Record;
  using Ring = typename TypeParam::Ring;
  Ring ring;
  auto mmap = CreateRing(&ring, Ring::Traits::MinCount());
  const uint32_t next_sequence_at_start = ring.GetNextSequence();
  for (uint32_t i = 0; i < ring.record_count(); ++i)
    ring.Put(Record(FillChar(i)));
  for (uint32_t i = 0; i < ring.record_count(); ++i) {
    const uint32_t expected_sequence = next_sequence_at_start + i;
    const Record expected_record(FillChar(i));
    {
      uint32_t sequence = ring.GetOldestSequence() + i;
      Record record;
      EXPECT_TRUE(ring.Get(&sequence, &record));
      EXPECT_EQ(expected_sequence, sequence);
      EXPECT_EQ(expected_record, record);
    }
  }
  {
    uint32_t sequence = ring.GetOldestSequence() + ring.record_count();
    Record record;
    EXPECT_FALSE(ring.Get(&sequence, &record));
  }
}

TYPED_TEST(BroadcastRingTest, FillTwice) {
  using Record = typename TypeParam::Record;
  using Ring = typename TypeParam::Ring;
  Ring ring;
  auto mmap = CreateRing(&ring, Ring::Traits::MinCount());
  const uint32_t next_sequence_at_start = ring.GetNextSequence();
  for (uint32_t i = 0; i < 2 * ring.record_count(); ++i) {
    const Record newest_record(FillChar(i));
    ring.Put(newest_record);

    const uint32_t newest_sequence = next_sequence_at_start + i;
    const uint32_t records_available = std::min(i + 1, ring.record_count());
    const uint32_t oldest_sequence = newest_sequence - records_available + 1;
    EXPECT_EQ(newest_sequence, ring.GetNewestSequence());
    EXPECT_EQ(oldest_sequence, ring.GetOldestSequence());
    EXPECT_EQ(newest_sequence + 1, ring.GetNextSequence());

    for (uint32_t j = 0; j < records_available; ++j) {
      const uint32_t sequence_jth_newest = newest_sequence - j;
      const Record record_jth_newest(FillChar(i - j));

      {
        uint32_t sequence = sequence_jth_newest;
        Record record;
        EXPECT_TRUE(ring.Get(&sequence, &record));
        EXPECT_EQ(sequence_jth_newest, sequence);
        EXPECT_EQ(record_jth_newest, record);
      }

      {
        uint32_t sequence = sequence_jth_newest;
        Record record;
        EXPECT_TRUE(ring.GetNewest(&sequence, &record));
        EXPECT_EQ(newest_sequence, sequence);
        EXPECT_EQ(newest_record, record);
      }
    }

    const Record oldest_record(
        FillChar(i + (oldest_sequence - newest_sequence)));
    const uint32_t sequence_0th_overwritten = oldest_sequence - 1;
    const uint32_t sequence_0th_future = newest_sequence + 1;
    const uint32_t sequence_1st_future = newest_sequence + 2;

    {
      uint32_t sequence = sequence_0th_overwritten;
      Record record;
      EXPECT_TRUE(ring.Get(&sequence, &record));
      EXPECT_EQ(oldest_sequence, sequence);
      EXPECT_EQ(oldest_record, record);
    }

    {
      uint32_t sequence = sequence_0th_overwritten;
      Record record;
      EXPECT_TRUE(ring.GetNewest(&sequence, &record));
      EXPECT_EQ(newest_sequence, sequence);
      EXPECT_EQ(newest_record, record);
    }

    {
      uint32_t sequence = sequence_0th_future;
      Record record;
      EXPECT_FALSE(ring.Get(&sequence, &record));
      EXPECT_EQ(sequence_0th_future, sequence);
      EXPECT_EQ(Record(), record);
    }

    {
      uint32_t sequence = sequence_0th_future;
      Record record;
      EXPECT_FALSE(ring.GetNewest(&sequence, &record));
      EXPECT_EQ(sequence_0th_future, sequence);
      EXPECT_EQ(Record(), record);
    }

    {
      uint32_t sequence = sequence_1st_future;
      Record record;
      EXPECT_TRUE(ring.Get(&sequence, &record));
      EXPECT_EQ(oldest_sequence, sequence);
      EXPECT_EQ(oldest_record, record);
    }

    {
      uint32_t sequence = sequence_1st_future;
      Record record;
      EXPECT_TRUE(ring.GetNewest(&sequence, &record));
      EXPECT_EQ(newest_sequence, sequence);
      EXPECT_EQ(newest_record, record);
    }
  }
}

TYPED_TEST(BroadcastRingTest, Import) {
  using Record = typename TypeParam::Record;
  using Ring = typename TypeParam::Ring;
  Ring ring;
  auto mmap = CreateRing(&ring, Ring::Traits::MinCount());

  const uint32_t sequence_0 = ring.GetNextSequence();
  const uint32_t sequence_1 = ring.GetNextSequence() + 1;
  const Record record_0 = Record::Pattern(0x00);
  const Record record_1 = Record::Pattern(0x80);
  ring.Put(record_0);
  ring.Put(record_1);

  {
    Ring imported_ring;
    bool import_ok;
    std::tie(imported_ring, import_ok) = Ring::Import(mmap.mmap(), mmap.size);
    EXPECT_TRUE(import_ok);
    EXPECT_EQ(ring.record_size(), imported_ring.record_size());
    EXPECT_EQ(ring.record_count(), imported_ring.record_count());

    if (ring.record_count() != 1) {
      uint32_t sequence = sequence_0;
      Record imported_record;
      EXPECT_TRUE(imported_ring.Get(&sequence, &imported_record));
      EXPECT_EQ(sequence_0, sequence);
      EXPECT_EQ(record_0, imported_record);
    }

    {
      uint32_t sequence = sequence_1;
      Record imported_record;
      EXPECT_TRUE(imported_ring.Get(&sequence, &imported_record));
      EXPECT_EQ(sequence_1, sequence);
      EXPECT_EQ(record_1, imported_record);
    }
  }
}

TEST(BroadcastRingTest, ShouldFailImportIfStaticSizeMismatch) {
  using OriginalRing = typename Static_16_16x16::Ring;
  using RecordSizeMismatchRing = typename Static_8_8x16::Ring;
  using RecordCountMismatchRing = typename Static_16_16x8::Ring;

  OriginalRing original_ring;
  auto mmap = CreateRing(&original_ring, OriginalRing::Traits::MinCount());

  {
    using ImportedRing = RecordSizeMismatchRing;
    ImportedRing imported_ring;
    bool import_ok;
    std::tie(imported_ring, import_ok) =
        ImportedRing::Import(mmap.mmap(), mmap.size);
    EXPECT_FALSE(import_ok);
    auto mmap_imported =
        CreateRing(&imported_ring, ImportedRing::Traits::MinCount());
    EXPECT_NE(original_ring.record_size(), imported_ring.record_size());
    EXPECT_EQ(original_ring.record_count(), imported_ring.record_count());
  }

  {
    using ImportedRing = RecordCountMismatchRing;
    ImportedRing imported_ring;
    bool import_ok;
    std::tie(imported_ring, import_ok) =
        ImportedRing::Import(mmap.mmap(), mmap.size);
    EXPECT_FALSE(import_ok);
    auto mmap_imported =
        CreateRing(&imported_ring, ImportedRing::Traits::MinCount());
    EXPECT_EQ(original_ring.record_size(), imported_ring.record_size());
    EXPECT_NE(original_ring.record_count(), imported_ring.record_count());
  }
}

TEST(BroadcastRingTest, ShouldFailImportIfDynamicSizeGrows) {
  using OriginalRing = typename Dynamic_8_NxM::Ring;
  using RecordSizeGrowsRing = typename Dynamic_16_NxM::Ring;

  OriginalRing original_ring;
  auto mmap = CreateRing(&original_ring, OriginalRing::Traits::MinCount());

  {
    using ImportedRing = RecordSizeGrowsRing;
    ImportedRing imported_ring;
    bool import_ok;
    std::tie(imported_ring, import_ok) =
        ImportedRing::Import(mmap.mmap(), mmap.size);
    EXPECT_FALSE(import_ok);
    auto mmap_imported =
        CreateRing(&imported_ring, ImportedRing::Traits::MinCount());
    EXPECT_LT(original_ring.record_size(), imported_ring.record_size());
    EXPECT_EQ(original_ring.record_count(), imported_ring.record_count());
  }
}

TEST(BroadcastRingTest, ShouldFailImportIfCountTooSmall) {
  using OriginalRing = typename Dynamic_16_NxM_1plus0::Ring;
  using MinCountRing = typename Dynamic_16_NxM_1plus1::Ring;

  OriginalRing original_ring;
  auto mmap = CreateRing(&original_ring, OriginalRing::Traits::MinCount());

  {
    using ImportedRing = MinCountRing;
    ImportedRing imported_ring;
    bool import_ok;
    std::tie(imported_ring, import_ok) =
        ImportedRing::Import(mmap.mmap(), mmap.size);
    EXPECT_FALSE(import_ok);
    auto mmap_imported =
        CreateRing(&imported_ring, ImportedRing::Traits::MinCount());
    EXPECT_EQ(original_ring.record_size(), imported_ring.record_size());
    EXPECT_LT(original_ring.record_count(), imported_ring.record_count());
  }
}

TEST(BroadcastRingTest, ShouldFailImportIfMmapTooSmall) {
  using OriginalRing = typename Dynamic_16_NxM::Ring;

  OriginalRing original_ring;
  auto mmap = CreateRing(&original_ring, OriginalRing::Traits::MinCount());

  {
    using ImportedRing = OriginalRing;
    ImportedRing imported_ring;
    bool import_ok;
    const size_t kMinSize =
        ImportedRing::MemorySize(original_ring.record_count());
    std::tie(imported_ring, import_ok) = ImportedRing::Import(mmap.mmap(), 0);
    EXPECT_FALSE(import_ok);
    std::tie(imported_ring, import_ok) =
        ImportedRing::Import(mmap.mmap(), kMinSize - 1);
    EXPECT_FALSE(import_ok);
    std::tie(imported_ring, import_ok) =
        ImportedRing::Import(mmap.mmap(), kMinSize);
    EXPECT_TRUE(import_ok);
    EXPECT_EQ(original_ring.record_size(), imported_ring.record_size());
    EXPECT_EQ(original_ring.record_count(), imported_ring.record_count());
  }
}

TEST(BroadcastRingTest, ShouldImportIfDynamicSizeShrinks) {
  using OriginalRing = typename Dynamic_16_NxM::Ring;
  using RecordSizeShrinksRing = typename Dynamic_8_NxM::Ring;

  OriginalRing original_ring;
  auto mmap = CreateRing(&original_ring, OriginalRing::Traits::MinCount());

  using OriginalRecord = typename OriginalRing::Record;
  const uint32_t original_sequence_0 = original_ring.GetNextSequence();
  const uint32_t original_sequence_1 = original_ring.GetNextSequence() + 1;
  const OriginalRecord original_record_0 = OriginalRecord::Pattern(0x00);
  const OriginalRecord original_record_1 = OriginalRecord::Pattern(0x80);
  original_ring.Put(original_record_0);
  original_ring.Put(original_record_1);

  {
    using ImportedRing = RecordSizeShrinksRing;
    using ImportedRecord = typename ImportedRing::Record;
    ImportedRing imported_ring;
    bool import_ok;
    std::tie(imported_ring, import_ok) =
        ImportedRing::Import(mmap.mmap(), mmap.size);
    EXPECT_TRUE(import_ok);
    EXPECT_EQ(original_ring.record_size(), imported_ring.record_size());
    EXPECT_EQ(original_ring.record_count(), imported_ring.record_count());
    EXPECT_GT(sizeof(OriginalRecord), sizeof(ImportedRecord));

    {
      uint32_t sequence = original_sequence_0;
      ImportedRecord shrunk_record;
      EXPECT_TRUE(imported_ring.Get(&sequence, &shrunk_record));
      EXPECT_EQ(original_sequence_0, sequence);
      EXPECT_EQ(original_record_0.Truncate<ImportedRecord>(), shrunk_record);
    }

    {
      uint32_t sequence = original_sequence_1;
      ImportedRecord shrunk_record;
      EXPECT_TRUE(imported_ring.Get(&sequence, &shrunk_record));
      EXPECT_EQ(original_sequence_1, sequence);
      EXPECT_EQ(original_record_1.Truncate<ImportedRecord>(), shrunk_record);
    }
  }
}

TEST(BroadcastRingTest, ShouldImportIfCompatibleDynamicToStatic) {
  using OriginalRing = typename Dynamic_16_NxM::Ring;
  using ImportedRing = typename Static_16_16x16::Ring;
  using OriginalRecord = typename OriginalRing::Record;
  using ImportedRecord = typename ImportedRing::Record;
  using StaticRing = ImportedRing;

  OriginalRing original_ring;
  auto mmap = CreateRing(&original_ring, StaticRing::Traits::MinCount());

  const uint32_t original_sequence_0 = original_ring.GetNextSequence();
  const uint32_t original_sequence_1 = original_ring.GetNextSequence() + 1;
  const OriginalRecord original_record_0 = OriginalRecord::Pattern(0x00);
  const OriginalRecord original_record_1 = OriginalRecord::Pattern(0x80);
  original_ring.Put(original_record_0);
  original_ring.Put(original_record_1);

  {
    ImportedRing imported_ring;
    bool import_ok;
    std::tie(imported_ring, import_ok) =
        ImportedRing::Import(mmap.mmap(), mmap.size);
    EXPECT_TRUE(import_ok);
    EXPECT_EQ(original_ring.record_size(), imported_ring.record_size());
    EXPECT_EQ(original_ring.record_count(), imported_ring.record_count());

    {
      uint32_t sequence = original_sequence_0;
      ImportedRecord imported_record;
      EXPECT_TRUE(imported_ring.Get(&sequence, &imported_record));
      EXPECT_EQ(original_sequence_0, sequence);
      EXPECT_EQ(original_record_0, imported_record);
    }

    {
      uint32_t sequence = original_sequence_1;
      ImportedRecord imported_record;
      EXPECT_TRUE(imported_ring.Get(&sequence, &imported_record));
      EXPECT_EQ(original_sequence_1, sequence);
      EXPECT_EQ(original_record_1, imported_record);
    }
  }
}

TEST(BroadcastRingTest, ShouldImportIfCompatibleStaticToDynamic) {
  using OriginalRing = typename Static_16_16x16::Ring;
  using ImportedRing = typename Dynamic_16_NxM::Ring;
  using OriginalRecord = typename OriginalRing::Record;
  using ImportedRecord = typename ImportedRing::Record;
  using StaticRing = OriginalRing;

  OriginalRing original_ring;
  auto mmap = CreateRing(&original_ring, StaticRing::Traits::MinCount());

  const uint32_t original_sequence_0 = original_ring.GetNextSequence();
  const uint32_t original_sequence_1 = original_ring.GetNextSequence() + 1;
  const OriginalRecord original_record_0 = OriginalRecord::Pattern(0x00);
  const OriginalRecord original_record_1 = OriginalRecord::Pattern(0x80);
  original_ring.Put(original_record_0);
  original_ring.Put(original_record_1);

  {
    ImportedRing imported_ring;
    bool import_ok;
    std::tie(imported_ring, import_ok) =
        ImportedRing::Import(mmap.mmap(), mmap.size);
    EXPECT_TRUE(import_ok);
    EXPECT_EQ(original_ring.record_size(), imported_ring.record_size());
    EXPECT_EQ(original_ring.record_count(), imported_ring.record_count());

    {
      uint32_t sequence = original_sequence_0;
      ImportedRecord imported_record;
      EXPECT_TRUE(imported_ring.Get(&sequence, &imported_record));
      EXPECT_EQ(original_sequence_0, sequence);
      EXPECT_EQ(original_record_0, imported_record);
    }

    {
      uint32_t sequence = original_sequence_1;
      ImportedRecord imported_record;
      EXPECT_TRUE(imported_ring.Get(&sequence, &imported_record));
      EXPECT_EQ(original_sequence_1, sequence);
      EXPECT_EQ(original_record_1, imported_record);
    }
  }
}

TEST(BroadcastRingTest, ShouldImportIfReadonlyMmap) {
  using Ring = Dynamic_32_NxM::Ring;
  using Record = Ring::Record;

  uint32_t record_count = Ring::Traits::MinCount();
  size_t ring_size = Ring::MemorySize(record_count);

  size_t page_size = sysconf(_SC_PAGESIZE);
  size_t mmap_size = (ring_size + (page_size - 1)) & ~(page_size - 1);
  ASSERT_GE(mmap_size, ring_size);

  void* mmap_base = mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  ASSERT_NE(MAP_FAILED, mmap_base);

  Ring ring = Ring::Create(mmap_base, mmap_size, record_count);
  for (uint32_t i = 0; i < record_count; ++i) ring.Put(Record(FillChar(i)));

  ASSERT_EQ(0, mprotect(mmap_base, mmap_size, PROT_READ));

  {
    Ring imported_ring;
    bool import_ok;
    std::tie(imported_ring, import_ok) = Ring::Import(mmap_base, mmap_size);
    EXPECT_TRUE(import_ok);
    EXPECT_EQ(ring.record_size(), imported_ring.record_size());
    EXPECT_EQ(ring.record_count(), imported_ring.record_count());

    uint32_t oldest_sequence = imported_ring.GetOldestSequence();
    for (uint32_t i = 0; i < record_count; ++i) {
      uint32_t sequence = oldest_sequence + i;
      Record record;
      EXPECT_TRUE(imported_ring.Get(&sequence, &record));
      EXPECT_EQ(Record(FillChar(i)), record);
    }
  }

  ASSERT_EQ(0, munmap(mmap_base, mmap_size));
}

TEST(BroadcastRingTest, ShouldDieIfPutReadonlyMmap) {
  using Ring = Dynamic_32_NxM::Ring;
  using Record = Ring::Record;

  uint32_t record_count = Ring::Traits::MinCount();
  size_t ring_size = Ring::MemorySize(record_count);

  size_t page_size = sysconf(_SC_PAGESIZE);
  size_t mmap_size = (ring_size + (page_size - 1)) & ~(page_size - 1);
  ASSERT_GE(mmap_size, ring_size);

  void* mmap_base = mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  ASSERT_NE(MAP_FAILED, mmap_base);

  Ring ring = Ring::Create(mmap_base, mmap_size, record_count);
  for (uint32_t i = 0; i < record_count; ++i) ring.Put(Record(FillChar(i)));

  ASSERT_EQ(0, mprotect(mmap_base, mmap_size, PROT_READ));

  EXPECT_DEATH_IF_SUPPORTED({ ring.Put(Record(7)); }, "");

  ASSERT_EQ(0, munmap(mmap_base, mmap_size));
}

TEST(BroadcastRingTest, ShouldDieIfCreationMmapTooSmall) {
  using Ring = Dynamic_32_NxM::Ring;
  using Record = Ring::Record;

  uint32_t record_count = Ring::Traits::MinCount();
  size_t ring_size = Ring::MemorySize(record_count);
  FakeMmap mmap(ring_size);

  EXPECT_DEATH_IF_SUPPORTED({
    Ring ring = Ring::Create(mmap.mmap(), ring_size - 1, record_count);
  }, "");

  Ring ring = Ring::Create(mmap.mmap(), ring_size, record_count);

  ring.Put(Record(3));

  {
    uint32_t sequence = ring.GetNewestSequence();
    Record record;
    EXPECT_TRUE(ring.Get(&sequence, &record));
    EXPECT_EQ(Record(3), record);
  }
}

TEST(BroadcastRingTest, ShouldDieIfCreationMmapMisaligned) {
  using Ring = Static_8_8x1::Ring;
  using Record = Ring::Record;

  constexpr int kAlign = Ring::mmap_alignment();
  constexpr int kMisalign = kAlign / 2;
  size_t ring_size = Ring::MemorySize();
  std::unique_ptr<char[]> buf(new char[ring_size + kMisalign]);

  EXPECT_DEATH_IF_SUPPORTED(
      { Ring ring = Ring::Create(buf.get() + kMisalign, ring_size); }, "");

  Ring ring = Ring::Create(buf.get(), ring_size);

  ring.Put(Record(3));

  {
    uint32_t sequence = ring.GetNewestSequence();
    Record record;
    EXPECT_TRUE(ring.Get(&sequence, &record));
    EXPECT_EQ(Record(3), record);
  }
}

template <typename Ring>
std::unique_ptr<std::thread> CopyTask(std::atomic<bool>* quit, void* in_base,
                                      size_t in_size, void* out_base,
                                      size_t out_size) {
  return std::unique_ptr<std::thread>(
      new std::thread([quit, in_base, in_size, out_base, out_size]() {
        using Record = typename Ring::Record;

        bool import_ok;
        Ring in_ring;
        Ring out_ring;
        std::tie(in_ring, import_ok) = Ring::Import(in_base, in_size);
        ASSERT_TRUE(import_ok);
        std::tie(out_ring, import_ok) = Ring::Import(out_base, out_size);
        ASSERT_TRUE(import_ok);

        uint32_t sequence = in_ring.GetOldestSequence();
        while (!std::atomic_load_explicit(quit, std::memory_order_relaxed)) {
          Record record;
          if (in_ring.Get(&sequence, &record)) {
            out_ring.Put(record);
            sequence++;
          }
        }
      }));
}

TEST(BroadcastRingTest, ThreadedCopySingle) {
  using Ring = Dynamic_32_NxM::Ring;
  using Record = Ring::Record;
  Ring in_ring;
  auto in_mmap = CreateRing(&in_ring, Ring::Traits::MinCount());

  Ring out_ring;
  auto out_mmap = CreateRing(&out_ring, Ring::Traits::MinCount());

  std::atomic<bool> quit(false);
  std::unique_ptr<std::thread> copy_task = CopyTask<Ring>(
      &quit, out_mmap.mmap(), out_mmap.size, in_mmap.mmap(), in_mmap.size);

  const Record out_record(0x1c);
  out_ring.Put(out_record);

  uint32_t in_sequence = in_ring.GetOldestSequence();
  Record in_record;
  while (!in_ring.Get(&in_sequence, &in_record)) {
    // Do nothing.
  }

  EXPECT_EQ(out_record, in_record);
  std::atomic_store_explicit(&quit, true, std::memory_order_relaxed);
  copy_task->join();
}

TEST(BroadcastRingTest, ThreadedCopyLossless) {
  using Ring = Dynamic_32_NxM::Ring;
  using Record = Ring::Record;
  Ring in_ring;
  auto in_mmap = CreateRing(&in_ring, Ring::Traits::MinCount());

  Ring out_ring;
  auto out_mmap = CreateRing(&out_ring, Ring::Traits::MinCount());

  std::atomic<bool> quit(false);
  std::unique_ptr<std::thread> copy_task = CopyTask<Ring>(
      &quit, out_mmap.mmap(), out_mmap.size, in_mmap.mmap(), in_mmap.size);

  constexpr uint32_t kRecordsToProcess = 10000;
  uint32_t out_records = 0;
  uint32_t in_records = 0;
  uint32_t in_sequence = in_ring.GetNextSequence();
  while (out_records < kRecordsToProcess || in_records < kRecordsToProcess) {
    if (out_records < kRecordsToProcess &&
        out_records - in_records < out_ring.record_count()) {
      const Record out_record(FillChar(out_records));
      out_ring.Put(out_record);
      out_records++;
    }

    Record in_record;
    while (in_ring.Get(&in_sequence, &in_record)) {
      EXPECT_EQ(Record(FillChar(in_records)), in_record);
      in_records++;
      in_sequence++;
    }
  }

  EXPECT_EQ(kRecordsToProcess, out_records);
  EXPECT_EQ(kRecordsToProcess, in_records);

  std::atomic_store_explicit(&quit, true, std::memory_order_relaxed);
  copy_task->join();
}

TEST(BroadcastRingTest, ThreadedCopyLossy) {
  using Ring = Dynamic_32_NxM::Ring;
  using Record = Ring::Record;
  Ring in_ring;
  auto in_mmap = CreateRing(&in_ring, Ring::Traits::MinCount());

  Ring out_ring;
  auto out_mmap = CreateRing(&out_ring, Ring::Traits::MinCount());

  std::atomic<bool> quit(false);
  std::unique_ptr<std::thread> copy_task = CopyTask<Ring>(
      &quit, out_mmap.mmap(), out_mmap.size, in_mmap.mmap(), in_mmap.size);

  constexpr uint32_t kRecordsToProcess = 100000;
  uint32_t out_records = 0;
  uint32_t in_records = 0;
  uint32_t in_sequence = in_ring.GetNextSequence();
  while (out_records < kRecordsToProcess) {
    const Record out_record(FillChar(out_records));
    out_ring.Put(out_record);
    out_records++;

    Record in_record;
    if (in_ring.GetNewest(&in_sequence, &in_record)) {
      EXPECT_EQ(Record(in_record.v[0]), in_record);
      in_records++;
      in_sequence++;
    }
  }

  EXPECT_EQ(kRecordsToProcess, out_records);
  EXPECT_GE(kRecordsToProcess, in_records);

  std::atomic_store_explicit(&quit, true, std::memory_order_relaxed);
  copy_task->join();
}

template <typename Ring>
std::unique_ptr<std::thread> CheckFillTask(std::atomic<bool>* quit,
                                           void* in_base, size_t in_size) {
  return std::unique_ptr<std::thread>(
      new std::thread([quit, in_base, in_size]() {
        using Record = typename Ring::Record;

        bool import_ok;
        Ring in_ring;
        std::tie(in_ring, import_ok) = Ring::Import(in_base, in_size);
        ASSERT_TRUE(import_ok);

        uint32_t sequence = in_ring.GetOldestSequence();
        while (!std::atomic_load_explicit(quit, std::memory_order_relaxed)) {
          Record record;
          if (in_ring.Get(&sequence, &record)) {
            ASSERT_EQ(Record(record.v[0]), record);
            sequence++;
          }
        }
      }));
}

template <typename Ring>
void ThreadedOverwriteTorture() {
  using Record = typename Ring::Record;

  // Maximize overwrites by having few records.
  const int kMinRecordCount = 1;
  const int kMaxRecordCount = 4;

  for (int count = kMinRecordCount; count <= kMaxRecordCount; count *= 2) {
    Ring out_ring;
    auto out_mmap = CreateRing(&out_ring, count);

    std::atomic<bool> quit(false);
    std::unique_ptr<std::thread> check_task =
        CheckFillTask<Ring>(&quit, out_mmap.mmap(), out_mmap.size);

    constexpr int kIterations = 10000;
    for (int i = 0; i < kIterations; ++i) {
      const Record record(FillChar(i));
      out_ring.Put(record);
    }

    std::atomic_store_explicit(&quit, true, std::memory_order_relaxed);
    check_task->join();
  }
}

TEST(BroadcastRingTest, ThreadedOverwriteTortureSmall) {
  ThreadedOverwriteTorture<Dynamic_16_NxM_1plus0::Ring>();
}

TEST(BroadcastRingTest, ThreadedOverwriteTortureLarge) {
  ThreadedOverwriteTorture<Dynamic_256_NxM_1plus0::Ring>();
}

} // namespace dvr
} // namespace android
