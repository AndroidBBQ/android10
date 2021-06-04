#ifndef ANDROID_DVR_DISPLAY_PROTOCOL_H_
#define ANDROID_DVR_DISPLAY_PROTOCOL_H_

#include <sys/types.h>

#include <array>
#include <map>

#include <dvr/dvr_display_types.h>

#include <dvr/dvr_api.h>
#include <pdx/rpc/buffer_wrapper.h>
#include <pdx/rpc/remote_method.h>
#include <pdx/rpc/serializable.h>
#include <pdx/rpc/variant.h>
#include <private/dvr/bufferhub_rpc.h>

// RPC protocol definitions for DVR display services (VrFlinger).

namespace android {
namespace dvr {
namespace display {

// Native display metrics.
struct Metrics {
  // Basic display properties.
  uint32_t display_width;
  uint32_t display_height;
  uint32_t display_x_dpi;
  uint32_t display_y_dpi;
  uint32_t vsync_period_ns;

  // HMD metrics.
  // TODO(eieio): Determine how these fields should be populated. On phones
  // these values are determined at runtime by VrCore based on which headset the
  // phone is in. On dedicated hardware this needs to come from somewhere else.
  // Perhaps these should be moved to a separate structure that is returned by a
  // separate runtime call.
  uint32_t distorted_width;
  uint32_t distorted_height;
  uint32_t hmd_ipd_mm;
  float inter_lens_distance_m;
  std::array<float, 4> left_fov_lrbt;
  std::array<float, 4> right_fov_lrbt;

 private:
  PDX_SERIALIZABLE_MEMBERS(Metrics, display_width, display_height,
                           display_x_dpi, display_y_dpi, vsync_period_ns,
                           distorted_width, distorted_height, hmd_ipd_mm,
                           inter_lens_distance_m, left_fov_lrbt,
                           right_fov_lrbt);
};

// Serializable base type for enum structs. Enum structs are easier to use than
// enum classes, especially for bitmasks. This base type provides common
// utilities for flags types.
template <typename Integer>
class Flags {
 public:
  using Base = Flags<Integer>;
  using Type = Integer;

  // NOLINTNEXTLINE(google-explicit-constructor)
  Flags(const Integer& value) : value_{value} {}
  Flags(const Flags&) = default;
  Flags& operator=(const Flags&) = default;

  Integer value() const { return value_; }
  // NOLINTNEXTLINE(google-explicit-constructor)
  operator Integer() const { return value_; }

  bool IsSet(Integer bits) const { return (value_ & bits) == bits; }
  bool IsClear(Integer bits) const { return (value_ & bits) == 0; }

  void Set(Integer bits) { value_ |= bits; }
  void Clear(Integer bits) { value_ &= ~bits; }

  Integer operator|(Integer bits) const { return value_ | bits; }
  Integer operator&(Integer bits) const { return value_ & bits; }

  Flags& operator|=(Integer bits) {
    value_ |= bits;
    return *this;
  }
  Flags& operator&=(Integer bits) {
    value_ &= bits;
    return *this;
  }

 private:
  Integer value_;

  PDX_SERIALIZABLE_MEMBERS(Flags<Integer>, value_);
};

// Flags indicating what changed since last update.
struct SurfaceUpdateFlags : public Flags<uint32_t> {
  enum : Type {
    None = DVR_SURFACE_UPDATE_FLAGS_NONE,
    NewSurface = DVR_SURFACE_UPDATE_FLAGS_NEW_SURFACE,
    BuffersChanged = DVR_SURFACE_UPDATE_FLAGS_BUFFERS_CHANGED,
    VisibilityChanged = DVR_SURFACE_UPDATE_FLAGS_VISIBILITY_CHANGED,
    AttributesChanged = DVR_SURFACE_UPDATE_FLAGS_ATTRIBUTES_CHANGED,
  };

  SurfaceUpdateFlags() : Base{None} {}
  using Base::Base;
};

// Surface attribute key/value types.
using SurfaceAttributeKey = int32_t;
using SurfaceAttributeValue =
    pdx::rpc::Variant<int32_t, int64_t, bool, float, std::array<float, 2>,
                      std::array<float, 3>, std::array<float, 4>,
                      std::array<float, 8>, std::array<float, 16>>;

// Defined surface attribute keys.
struct SurfaceAttribute : public Flags<SurfaceAttributeKey> {
  enum : Type {
    // Keys in the negative integer space are interpreted by VrFlinger for
    // direct surfaces.
    Direct = DVR_SURFACE_ATTRIBUTE_DIRECT,
    ZOrder = DVR_SURFACE_ATTRIBUTE_Z_ORDER,
    Visible = DVR_SURFACE_ATTRIBUTE_VISIBLE,

    // Invalid key. May be used to terminate C style lists in public API code.
    Invalid = DVR_SURFACE_ATTRIBUTE_INVALID,

    // Positive keys are interpreted by the compositor only.
    FirstUserKey = DVR_SURFACE_ATTRIBUTE_FIRST_USER_KEY,
  };

  SurfaceAttribute() : Base{Invalid} {}
  using Base::Base;
};

// Collection of surface attribute key/value pairs.
using SurfaceAttributes = std::map<SurfaceAttributeKey, SurfaceAttributeValue>;

struct SurfaceState {
  int32_t surface_id;
  int32_t process_id;
  int32_t user_id;

  SurfaceAttributes surface_attributes;
  SurfaceUpdateFlags update_flags;
  std::vector<int32_t> queue_ids;

  // Convenience accessors.
  bool GetVisible() const {
    bool bool_value = false;
    GetAttribute(SurfaceAttribute::Visible, &bool_value,
                 ValidTypes<int32_t, int64_t, bool, float>{});
    return bool_value;
  }

  int GetZOrder() const {
    int int_value = 0;
    GetAttribute(SurfaceAttribute::ZOrder, &int_value,
                 ValidTypes<int32_t, int64_t, float>{});
    return int_value;
  }

 private:
  template <typename... Types>
  struct ValidTypes {};

  template <typename ReturnType, typename... Types>
  bool GetAttribute(SurfaceAttributeKey key, ReturnType* out_value,
                    ValidTypes<Types...>) const {
    auto search = surface_attributes.find(key);
    if (search != surface_attributes.end())
      return pdx::rpc::IfAnyOf<Types...>::Get(&search->second, out_value);
    else
      return false;
  }

  PDX_SERIALIZABLE_MEMBERS(SurfaceState, surface_id, process_id,
                           surface_attributes, update_flags, queue_ids);
};

struct SurfaceInfo {
  int surface_id;
  bool visible;
  int z_order;

 private:
  PDX_SERIALIZABLE_MEMBERS(SurfaceInfo, surface_id, visible, z_order);
};

enum class ConfigFileType : uint32_t {
  kLensMetrics,
  kDeviceMetrics,
  kDeviceConfiguration
};

struct DisplayProtocol {
  // Service path.
  static constexpr char kClientPath[] = "system/vr/display/client";

  // Op codes.
  enum {
    kOpGetMetrics = 0,
    kOpGetConfigurationData,
    kOpSetupGlobalBuffer,
    kOpDeleteGlobalBuffer,
    kOpGetGlobalBuffer,
    kOpIsVrAppRunning,
    kOpCreateSurface,
    kOpGetSurfaceInfo,
    kOpCreateQueue,
    kOpSetAttributes,
  };

  // Aliases.
  using LocalChannelHandle = pdx::LocalChannelHandle;
  using Void = pdx::rpc::Void;

  // Methods.
  PDX_REMOTE_METHOD(GetMetrics, kOpGetMetrics, Metrics(Void));
  PDX_REMOTE_METHOD(GetConfigurationData, kOpGetConfigurationData,
                    std::string(ConfigFileType config_type));
  PDX_REMOTE_METHOD(SetupGlobalBuffer, kOpSetupGlobalBuffer,
                    LocalNativeBufferHandle(DvrGlobalBufferKey key, size_t size,
                                            uint64_t usage));
  PDX_REMOTE_METHOD(DeleteGlobalBuffer, kOpDeleteGlobalBuffer,
                    void(DvrGlobalBufferKey key));
  PDX_REMOTE_METHOD(GetGlobalBuffer, kOpGetGlobalBuffer,
                    LocalNativeBufferHandle(DvrGlobalBufferKey key));
  PDX_REMOTE_METHOD(IsVrAppRunning, kOpIsVrAppRunning, bool(Void));
  PDX_REMOTE_METHOD(CreateSurface, kOpCreateSurface,
                    SurfaceInfo(const SurfaceAttributes& attributes));
  PDX_REMOTE_METHOD(GetSurfaceInfo, kOpGetSurfaceInfo, SurfaceInfo(Void));
  PDX_REMOTE_METHOD(
      CreateQueue, kOpCreateQueue,
      LocalChannelHandle(const ProducerQueueConfig& producer_config));
  PDX_REMOTE_METHOD(SetAttributes, kOpSetAttributes,
                    void(const SurfaceAttributes& attributes));
};

struct DisplayManagerProtocol {
  // Service path.
  static constexpr char kClientPath[] = "system/vr/display/manager";

  // Op codes.
  enum {
    kOpGetSurfaceState = 0,
    kOpGetSurfaceQueue,
  };

  // Aliases.
  using LocalChannelHandle = pdx::LocalChannelHandle;
  using Void = pdx::rpc::Void;

  // Methods.
  PDX_REMOTE_METHOD(GetSurfaceState, kOpGetSurfaceState,
                    std::vector<SurfaceState>(Void));
  PDX_REMOTE_METHOD(GetSurfaceQueue, kOpGetSurfaceQueue,
                    LocalChannelHandle(int surface_id, int queue_id));
};

struct VSyncSchedInfo {
  int64_t vsync_period_ns;
  int64_t timestamp_ns;
  uint32_t next_vsync_count;

 private:
  PDX_SERIALIZABLE_MEMBERS(VSyncSchedInfo, vsync_period_ns, timestamp_ns,
                           next_vsync_count);
};

struct VSyncProtocol {
  // Service path.
  static constexpr char kClientPath[] = "system/vr/display/vsync";

  // Op codes.
  enum {
    kOpWait = 0,
    kOpAck,
    kOpGetLastTimestamp,
    kOpGetSchedInfo,
    kOpAcknowledge,
  };

  // Aliases.
  using Void = pdx::rpc::Void;
  using Timestamp = int64_t;

  // Methods.
  PDX_REMOTE_METHOD(Wait, kOpWait, Timestamp(Void));
  PDX_REMOTE_METHOD(GetLastTimestamp, kOpGetLastTimestamp, Timestamp(Void));
  PDX_REMOTE_METHOD(GetSchedInfo, kOpGetSchedInfo, VSyncSchedInfo(Void));
  PDX_REMOTE_METHOD(Acknowledge, kOpAcknowledge, void(Void));
};

}  // namespace display
}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_DISPLAY_PROTOCOL_H_
