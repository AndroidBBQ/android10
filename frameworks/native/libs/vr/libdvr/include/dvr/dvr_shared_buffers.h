#ifndef ANDROID_DVR_SHARED_BUFFERS_H_
#define ANDROID_DVR_SHARED_BUFFERS_H_

#include <dvr/dvr_config.h>
#include <dvr/dvr_pose.h>
#include <dvr/dvr_vsync.h>
#include <libbroadcastring/broadcast_ring.h>

// This header is shared by VrCore and Android and must be kept in sync.
namespace android {
namespace dvr {

// Increment when the layout for the buffers change.
enum : uint32_t { kSharedBufferLayoutVersion = 2 };

// Note: These buffers will be mapped from various system processes as well
// as VrCore and the application processes in a r/w manner.
//
// Therefore it is possible for the application to mess with the contents of
// these buffers.
//
// While using them, assume garbage memory: Your logic must not crash or lead
// to execution of unsafe code as a function of the contents of these buffers.

// Sanity check for basic type sizes.
static_assert(sizeof(DvrPoseAsync) == 128, "Unexpected size for DvrPoseAsync");
static_assert(sizeof(DvrPose) == 112, "Unexpected size for DvrPose");
static_assert(sizeof(DvrVsync) == 32, "Unexpected size for DvrVsync");
static_assert(sizeof(DvrConfig) == 16, "Unexpected size for DvrConfig");

// A helper class that provides compile time sized traits for the BroadcastRing.
template <class DvrType, size_t StaticCount>
class DvrRingBufferTraits {
 public:
  using Record = DvrType;
  static constexpr bool kUseStaticRecordSize = false;
  static constexpr uint32_t kStaticRecordCount = StaticCount;
  static constexpr int kMaxReservedRecords = 1;
  static constexpr int kMinAvailableRecords = 1;
};

// Traits classes.
using DvrPoseTraits = DvrRingBufferTraits<DvrPose, 0>;
using DvrVsyncTraits = DvrRingBufferTraits<DvrVsync, 2>;
using DvrConfigTraits = DvrRingBufferTraits<DvrConfig, 2>;

// The broadcast ring classes that will expose the data.
using DvrPoseRing = BroadcastRing<DvrPose, DvrPoseTraits>;
using DvrVsyncRing = BroadcastRing<DvrVsync, DvrVsyncTraits>;
using DvrConfigRing = BroadcastRing<DvrConfig, DvrConfigTraits>;

// This is a shared memory buffer for passing pose data estimated at vsyncs.
//
// This will be primarily used for late latching and EDS where we bind this
// buffer in a shader and extract the right vsync-predicted pose.
struct __attribute__((packed, aligned(16))) DvrVsyncPoseBuffer {
  enum : int {
    // The number vsync predicted poses to keep in the ring buffer.
    // Must be a power of 2.
    kSize = 8,
    kIndexMask = kSize - 1,

    // The number of vsyncs (from the current vsync) we predict in vsync buffer.
    // The other poses are left alone.
    kMinFutureCount = 4
  };

  // The vsync predicted poses.
  // The pose for the vsync n is:
  // vsync_poses[n % kSize]
  //
  // This buffer is unsynchronized: It is possible to get torn reads as the
  // sensor service updates the predictions as new sensor measurements come
  // in. In particular, it is possible to get the position and an updated
  // orientation while reading.
  DvrPoseAsync vsync_poses[kSize];

  // The latest sensor pose for GPU usage.
  DvrPose current_pose;

  // Current vsync_count (where sensord is writing poses from).
  uint32_t vsync_count;

  // For 16 byte alignment.
  uint8_t padding[12];
};

static_assert(sizeof(DvrVsyncPoseBuffer) == 1152,
              "Unexpected size for DvrVsyncPoseBuffer");

// The keys for the dvr global buffers.
enum DvrGlobalBuffers : int32_t {
  kVsyncPoseBuffer = 1,
  kVsyncBuffer = 2,
  kSensorPoseBuffer = 3,
  kVrFlingerConfigBufferKey = 4
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_SHARED_BUFFERS_H_
