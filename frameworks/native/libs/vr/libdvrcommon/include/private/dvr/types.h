#ifndef ANDROID_DVR_TYPES_H_
#define ANDROID_DVR_TYPES_H_

// All basic types used by VR code.

#include <private/dvr/eigen.h>
#include <private/dvr/field_of_view.h>
#include <private/dvr/pose.h>
#include <private/dvr/range.h>

namespace android {
namespace dvr {

enum RgbColorChannel { kRed, kGreen, kBlue };

// EyeType: 0 for left, 1 for right.
enum EyeType { kLeftEye = 0, kRightEye = 1 };

// In the context of VR, vector types are used as much as base types.

using vec2f = Eigen::Vector2f;
using vec2d = Eigen::Vector2d;
using vec2i = Eigen::Vector2i;
using vec2 = vec2f;

using vec3f = Eigen::Vector3f;
using vec3d = Eigen::Vector3d;
using vec3i = Eigen::Vector3i;
using vec3 = vec3f;

using vec4f = Eigen::Vector4f;
using vec4d = Eigen::Vector4d;
using vec4i = Eigen::Vector4i;
using vec4 = vec4f;

using mat3f = Eigen::AffineMatrix<float, 3>;
using mat3d = Eigen::AffineMatrix<double, 3>;
using mat3 = mat3f;

using mat4f = Eigen::AffineMatrix<float, 4>;
using mat4d = Eigen::AffineMatrix<double, 4>;
using mat4 = mat4f;

using quatf = Eigen::Quaternionf;
using quatd = Eigen::Quaterniond;
using quat = quatf;

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_TYPES_H_
