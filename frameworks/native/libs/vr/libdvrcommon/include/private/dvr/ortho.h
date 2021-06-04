#ifndef ANDROID_DVR_ORTHO_H_
#define ANDROID_DVR_ORTHO_H_

#include <private/dvr/types.h>

namespace android {
namespace dvr {

template <class T>
Eigen::AffineMatrix<T, 4> OrthoMatrix(T left, T right, T bottom, T top,
                                      T znear, T zfar) {
  Eigen::AffineMatrix<T, 4> result;
  const T t2 = static_cast<T>(2);
  const T a = t2 / (right - left);
  const T b = t2 / (top - bottom);
  const T c = t2 / (zfar - znear);
  const T xoff = -(right + left) / (right - left);
  const T yoff = -(top + bottom) / (top - bottom);
  const T zoff = -(zfar + znear) / (zfar - znear);
  const T t1 = static_cast<T>(1);
  result.matrix() << a, 0, 0, xoff,
            0, b, 0, yoff,
            0, 0, c, zoff,
            0, 0, 0, t1;
  return result;
}

}  // namespace android
}  // namespace dvr

#endif  // ANDROID_DVR_ORTHO_H_
