#ifndef ANDROID_DVR_MATRIX_HELPERS_H_
#define ANDROID_DVR_MATRIX_HELPERS_H_

#include <private/dvr/eigen.h>
#include <private/dvr/types.h>

namespace android {
namespace dvr {

// A helper function for creating a mat4 directly.
inline mat4 MakeMat4(float m00, float m01, float m02, float m03, float m10,
                     float m11, float m12, float m13, float m20, float m21,
                     float m22, float m23, float m30, float m31, float m32,
                     float m33) {
  Eigen::Matrix4f matrix;

  matrix << m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30,
      m31, m32, m33;

  return mat4(matrix);
}

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_LOG_HELPERS_H_
