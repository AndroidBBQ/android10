#ifndef ANDROID_LIBVRFLINGER_HWCTYPES_H
#define ANDROID_LIBVRFLINGER_HWCTYPES_H

// General HWC type support. Hardware composer type support is a bit of a mess
// between HWC1, HWC2 C/C++11, and HIDL types. Particularly bothersome is the
// use of enum classes, which make analogous types between versions much
// harder to deal with in a uniform way.
//
// These utilities help address some of these pains by providing a type-safe,
// flexible interface to translate between different type spaces.

#define HWC2_INCLUDE_STRINGIFICATION
#define HWC2_USE_CPP11
#include <hardware/hwcomposer2.h>
#undef HWC2_INCLUDE_STRINGIFICATION
#undef HWC2_USE_CPP11

#include <string>
#include <type_traits>

namespace HWC {

// Value types derived from HWC HAL types. Some of these are stand-alone,
// while others are also wrapped in translator classes below.
using ColorMode = int32_t;  // android_color_mode_t;
using Config = hwc2_config_t;
using ColorTransform =
    std::underlying_type<android_color_transform_t>::type;          // int32_t;
using Dataspace = std::underlying_type<android_dataspace_t>::type;  // int32_t;
using DisplayId = hwc2_display_t;
using DisplayRequest = std::underlying_type<HWC2::DisplayRequest>::type;
using Hdr = std::underlying_type<android_hdr_t>::type;  // int32_t;
using Layer = hwc2_layer_t;
using PixelFormat =
    std::underlying_type<android_pixel_format_t>::type;  // int32_t;

// Type traits and casting utilities.

// SFINAE utility to evaluate type expressions.
template <typename...>
using TestTypeExpression = void;

// Traits type to determine the underlying type of an enum, integer,
// or wrapper class.
template <typename T, typename = typename std::is_enum<T>::type,
          typename = typename std::is_integral<T>::type, typename = void>
struct UnderlyingType {
  using Type = T;
};
// Partial specialization that matches enum types. Captures the underlying type
// of the enum in member type Type.
template <typename T>
struct UnderlyingType<T, std::true_type, std::false_type> {
  using Type = typename std::underlying_type<T>::type;
};
// Partial specialization that matches integral types. Captures the type of the
// integer in member type Type.
template <typename T>
struct UnderlyingType<T, std::false_type, std::true_type> {
  using Type = T;
};
// Partial specialization that matches the wrapper types below. Captures
// wrapper member type ValueType in member type Type.
template <typename T>
struct UnderlyingType<T, std::false_type, std::false_type,
                      TestTypeExpression<typename T::ValueType>> {
  using Type = typename T::ValueType;
};

// Enable if T is an enum with underlying type U.
template <typename T, typename U, typename ReturnType = void>
using EnableIfMatchingEnum = typename std::enable_if<
    std::is_enum<T>::value &&
        std::is_same<U, typename UnderlyingType<T>::Type>::value,
    ReturnType>::type;

// Enable if T and U are the same size/alignment and have the same underlying
// type. Handles enum, integral, and wrapper classes below.
template <typename T, typename U, typename Return = void>
using EnableIfSafeCast = typename std::enable_if<
    sizeof(T) == sizeof(U) && alignof(T) == alignof(U) &&
        std::is_same<typename UnderlyingType<T>::Type,
                     typename UnderlyingType<U>::Type>::value,
    Return>::type;

// Safely cast between std::vectors of matching enum/integer/wraper types.
// Normally this is not possible with pendantic compiler type checks. However,
// given the same size, alignment, and underlying type this is safe due to
// allocator requirements and array-like element access guarantees.
template <typename T, typename U>
EnableIfSafeCast<T, U, std::vector<T>*> VectorCast(std::vector<U>* in) {
  return reinterpret_cast<std::vector<T>*>(in);
}

// Translator classes that wrap specific HWC types to make translating
// between different types (especially enum class) in code cleaner.

// Base type for the enum wrappers below. This type provides type definitions
// and implicit conversion logic common to each wrapper type.
template <typename EnumType>
struct Wrapper {
  // Alias type of this instantiantion of Wrapper. Useful for inheriting
  // constructors in subclasses via "using Base::Base;" statements.
  using Base = Wrapper<EnumType>;

  // The enum type wrapped by this instantiation of Wrapper.
  using BaseType = EnumType;

  // The underlying type of the base enum type.
  using ValueType = typename UnderlyingType<BaseType>::Type;

  // A default constructor is not defined here. Subclasses should define one
  // as appropriate to define the correct inital value for the enum type.

  // Default copy constructor.
  Wrapper(const Wrapper&) = default;

  // Implicit conversion from ValueType.
  // NOLINTNEXTLINE(google-explicit-constructor)
  Wrapper(ValueType value) : value(value) {}

  // Implicit conversion from BaseType.
  // NOLINTNEXTLINE(google-explicit-constructor)
  Wrapper(BaseType value) : value(static_cast<ValueType>(value)) {}

  // Implicit conversion from an enum type of the same underlying type.
  template <typename T, typename = EnableIfMatchingEnum<T, ValueType>>
  // NOLINTNEXTLINE(google-explicit-constructor)
  Wrapper(const T& value) : value(static_cast<ValueType>(value)) {}

  // Implicit conversion to BaseType.
  // NOLINTNEXTLINE(google-explicit-constructor)
  operator BaseType() const { return static_cast<BaseType>(value); }

  // Implicit conversion to ValueType.
  // NOLINTNEXTLINE(google-explicit-constructor)
  operator ValueType() const { return value; }

  template <typename T, typename = EnableIfMatchingEnum<T, ValueType>>
  T cast() const {
    return static_cast<T>(value);
  }

  // Converts to string using HWC2 stringification of BaseType.
  std::string to_string() const {
    return HWC2::to_string(static_cast<BaseType>(value));
  }

  bool operator!=(const Wrapper& other) const { return value != other.value; }
  bool operator!=(ValueType other_value) const { return value != other_value; }
  bool operator!=(BaseType other_value) const {
    return static_cast<BaseType>(value) != other_value;
  }
  bool operator==(const Wrapper& other) const { return value == other.value; }
  bool operator==(ValueType other_value) const { return value == other_value; }
  bool operator==(BaseType other_value) const {
    return static_cast<BaseType>(value) == other_value;
  }

  ValueType value;
};

struct Attribute final : public Wrapper<HWC2::Attribute> {
  enum : ValueType {
    Invalid = HWC2_ATTRIBUTE_INVALID,
    Width = HWC2_ATTRIBUTE_WIDTH,
    Height = HWC2_ATTRIBUTE_HEIGHT,
    VsyncPeriod = HWC2_ATTRIBUTE_VSYNC_PERIOD,
    DpiX = HWC2_ATTRIBUTE_DPI_X,
    DpiY = HWC2_ATTRIBUTE_DPI_Y,
  };

  Attribute() : Base(Invalid) {}
  using Base::Base;
};

struct BlendMode final : public Wrapper<HWC2::BlendMode> {
  enum : ValueType {
    Invalid = HWC2_BLEND_MODE_INVALID,
    None = HWC2_BLEND_MODE_NONE,
    Premultiplied = HWC2_BLEND_MODE_PREMULTIPLIED,
    Coverage = HWC2_BLEND_MODE_COVERAGE,
  };

  BlendMode() : Base(Invalid) {}
  using Base::Base;
};

struct Composition final : public Wrapper<HWC2::Composition> {
  enum : ValueType {
    Invalid = HWC2_COMPOSITION_INVALID,
    Client = HWC2_COMPOSITION_CLIENT,
    Device = HWC2_COMPOSITION_DEVICE,
    SolidColor = HWC2_COMPOSITION_SOLID_COLOR,
    Cursor = HWC2_COMPOSITION_CURSOR,
    Sideband = HWC2_COMPOSITION_SIDEBAND,
  };

  Composition() : Base(Invalid) {}
  using Base::Base;
};

struct DisplayType final : public Wrapper<HWC2::DisplayType> {
  enum : ValueType {
    Invalid = HWC2_DISPLAY_TYPE_INVALID,
    Physical = HWC2_DISPLAY_TYPE_PHYSICAL,
    Virtual = HWC2_DISPLAY_TYPE_VIRTUAL,
  };

  DisplayType() : Base(Invalid) {}
  using Base::Base;
};

struct Error final : public Wrapper<HWC2::Error> {
  enum : ValueType {
    None = HWC2_ERROR_NONE,
    BadConfig = HWC2_ERROR_BAD_CONFIG,
    BadDisplay = HWC2_ERROR_BAD_DISPLAY,
    BadLayer = HWC2_ERROR_BAD_LAYER,
    BadParameter = HWC2_ERROR_BAD_PARAMETER,
    HasChanges = HWC2_ERROR_HAS_CHANGES,
    NoResources = HWC2_ERROR_NO_RESOURCES,
    NotValidated = HWC2_ERROR_NOT_VALIDATED,
    Unsupported = HWC2_ERROR_UNSUPPORTED,
  };

  Error() : Base(None) {}
  using Base::Base;
};

struct LayerRequest final : public Wrapper<HWC2::LayerRequest> {
  enum : ValueType {
    ClearClientTarget = HWC2_LAYER_REQUEST_CLEAR_CLIENT_TARGET,
  };

  LayerRequest() : Base(0) {}
  using Base::Base;
};

struct PowerMode final : public Wrapper<HWC2::PowerMode> {
  enum : ValueType {
    Off = HWC2_POWER_MODE_OFF,
    DozeSuspend = HWC2_POWER_MODE_DOZE_SUSPEND,
    Doze = HWC2_POWER_MODE_DOZE,
    On = HWC2_POWER_MODE_ON,
  };

  PowerMode() : Base(Off) {}
  using Base::Base;
};

struct Transform final : public Wrapper<HWC2::Transform> {
  enum : ValueType {
    None = 0,
    FlipH = HWC_TRANSFORM_FLIP_H,
    FlipV = HWC_TRANSFORM_FLIP_V,
    Rotate90 = HWC_TRANSFORM_ROT_90,
    Rotate180 = HWC_TRANSFORM_ROT_180,
    Rotate270 = HWC_TRANSFORM_ROT_270,
    FlipHRotate90 = HWC_TRANSFORM_FLIP_H_ROT_90,
    FlipVRotate90 = HWC_TRANSFORM_FLIP_V_ROT_90,
  };

  Transform() : Base(None) {}
  using Base::Base;
};

struct Vsync final : public Wrapper<HWC2::Vsync> {
  enum : ValueType {
    Invalid = HWC2_VSYNC_INVALID,
    Enable = HWC2_VSYNC_ENABLE,
    Disable = HWC2_VSYNC_DISABLE,
  };

  Vsync() : Base(Invalid) {}
  using Base::Base;
};

// Utility color type.
struct Color final {
  Color(const Color&) = default;
  Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
  // NOLINTNEXTLINE(google-explicit-constructor)
  Color(hwc_color_t color) : r(color.r), g(color.g), b(color.b), a(color.a) {}

  // NOLINTNEXTLINE(google-explicit-constructor)
  operator hwc_color_t() const { return {r, g, b, a}; }

  uint8_t r __attribute__((aligned(1)));
  uint8_t g __attribute__((aligned(1)));
  uint8_t b __attribute__((aligned(1)));
  uint8_t a __attribute__((aligned(1)));
};

// Utility rectangle type.
struct Rect final {
  // TODO(eieio): Implicit conversion to/from Android rect types.

  int32_t left __attribute__((aligned(4)));
  int32_t top __attribute__((aligned(4)));
  int32_t right __attribute__((aligned(4)));
  int32_t bottom __attribute__((aligned(4)));
};

}  // namespace HWC

#endif  // ANDROID_LIBVRFLINGER_HWCTYPES_H
