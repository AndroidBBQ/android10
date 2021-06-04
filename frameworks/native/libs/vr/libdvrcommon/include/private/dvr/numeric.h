#ifndef ANDROID_DVR_NUMERIC_H_
#define ANDROID_DVR_NUMERIC_H_

#include <cmath>
#include <limits>
#include <random>
#include <type_traits>

#include <private/dvr/eigen.h>
#include <private/dvr/types.h>

namespace android {
namespace dvr {

template <typename FT>
static inline FT ToDeg(FT f) {
  return f * static_cast<FT>(180.0 / M_PI);
}

template <typename FT>
static inline FT ToRad(FT f) {
  return f * static_cast<FT>(M_PI / 180.0);
}

// Adjusts `x` to the periodic range `[lo, hi]` (to normalize angle values
// for example).
template <typename T>
T NormalizePeriodicRange(T x, T lo, T hi) {
  T range_size = hi - lo;

  while (x < lo) {
    x += range_size;
  }

  while (x > hi) {
    x -= range_size;
  }

  return x;
}

// Normalizes a measurement in radians.
// @param x the angle to be normalized
// @param centre the point around which to normalize the range
// @return the value of x, normalized to the range [centre - 180, centre + 180]
template <typename T>
T NormalizeDegrees(T x, T centre = static_cast<T>(180.0)) {
  return NormalizePeriodicRange(x, centre - static_cast<T>(180.0),
                                centre + static_cast<T>(180.0));
}

// Normalizes a measurement in radians.
// @param x the angle to be normalized
// @param centre the point around which to normalize the range
// @return the value of x, normalized to the range
//         [centre - M_PI, centre + M_PI]
// @remark the centre parameter is to make it possible to specify a different
//         periodic range. This is useful if you are planning on comparing two
//         angles close to 0 or M_PI, so that one might not accidentally end
//         up on the other side of the range
template <typename T>
T NormalizeRadians(T x, T centre = static_cast<T>(M_PI)) {
  return NormalizePeriodicRange(x, centre - static_cast<T>(M_PI),
                                centre + static_cast<T>(M_PI));
}

static inline vec2i Round(const vec2& v) {
  return vec2i(roundf(v.x()), roundf(v.y()));
}

static inline vec2i Scale(const vec2i& v, float scale) {
  return vec2i(roundf(static_cast<float>(v.x()) * scale),
               roundf(static_cast<float>(v.y()) * scale));
}

// Re-maps `x` from `[lba,uba]` to `[lbb,ubb]`.
template <typename T>
T ConvertRange(T x, T lba, T uba, T lbb, T ubb) {
  return (((x - lba) * (ubb - lbb)) / (uba - lba)) + lbb;
}

template <typename R1, typename R2>
static inline vec2 MapPoint(const vec2& pt, const R1& from, const R2& to) {
  vec2 normalized((pt - vec2(from.p1)).array() / vec2(from.GetSize()).array());
  return (normalized * vec2(to.GetSize())) + vec2(to.p1);
}

template <typename T>
inline bool IsZero(const T& v,
                   const T& tol = std::numeric_limits<T>::epsilon()) {
  return std::abs(v) <= tol;
}

template <typename T>
inline bool IsEqual(const T& a, const T& b,
                    const T& tol = std::numeric_limits<T>::epsilon()) {
  return std::abs(b - a) <= tol;
}

template <typename T>
T Square(const T& x) {
  return x * x;
}

template <typename T>
T RandomInRange(T lo, T hi,
                typename
                std::enable_if<std::is_floating_point<T>::value>::type* = 0) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<T> distro(lo, hi);
  return distro(gen);
}

template <typename T>
T RandomInRange(T lo, T hi,
                typename
                std::enable_if<std::is_integral<T>::value>::type* = 0) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<T> distro(lo, hi);
  return distro(gen);
}

template <typename Derived1, typename Derived2>
Derived1 RandomInRange(
    const Eigen::MatrixBase<Derived1>& lo,
    const Eigen::MatrixBase<Derived2>& hi) {
  EIGEN_STATIC_ASSERT_SAME_MATRIX_SIZE(Derived1, Derived2);

  Derived1 result = Eigen::MatrixBase<Derived1>::Zero();

  for (int row = 0; row < result.rows(); ++row) {
    for (int col = 0; col < result.cols(); ++col) {
      result(row, col) = RandomInRange(lo(row, col), hi(row, col));
    }
  }

  return result;
}

template <typename T>
T RandomRange(T x) {
  return RandomInRange(-x, x);
}

template <typename T>
T Clamp(T x, T lo, T hi) {
  return std::min(std::max(x, lo), hi);
}

inline mat3 ScaleMatrix(const vec2& scale_xy) {
  return mat3(Eigen::Scaling(scale_xy[0], scale_xy[1], 1.0f));
}

inline mat3 TranslationMatrix(const vec2& translation) {
  return mat3(Eigen::Translation2f(translation));
}

inline mat4 TranslationMatrix(const vec3& translation) {
  return mat4(Eigen::Translation3f(translation));
}

inline vec2 TransformPoint(const mat3& m, const vec2& p) {
  return m.linear() * p + m.translation();
}

inline vec2 TransformVector(const mat3& m, const vec2& p) {
  return m.linear() * p;
}

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_NUMERIC_H_
