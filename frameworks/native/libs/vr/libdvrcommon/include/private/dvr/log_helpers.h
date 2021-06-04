#ifndef ANDROID_DVR_LOG_HELPERS_H_
#define ANDROID_DVR_LOG_HELPERS_H_

#include <iomanip>
#include <ostream>

#include <private/dvr/eigen.h>
#include <private/dvr/field_of_view.h>

namespace android {
namespace dvr {

template <typename T>
inline std::ostream& operator<<(std::ostream& out,
                                const Eigen::Vector<T, 2>& vec) {
  return out << "vec2(" << vec.x() << ',' << vec.y() << ')';
}

template <typename T>
inline std::ostream& operator<<(std::ostream& out,
                                const Eigen::Vector<T, 3>& vec) {
  return out << "vec3(" << vec.x() << ',' << vec.y() << ',' << vec.z() << ')';
}

template <typename T>
inline std::ostream& operator<<(std::ostream& out,
                                const Eigen::Vector<T, 4>& vec) {
  return out << "vec4(" << vec.x() << ',' << vec.y() << ',' << vec.z() << ','
             << vec.w() << ')';
}

template <typename T>
inline std::ostream& operator<<(std::ostream& out,
                                const Eigen::AffineMatrix<T, 4>& mat) {
  out << std::setfill(' ') << std::setprecision(4) << std::fixed
      << std::showpos;
  out << "\nmat4[";
  out << std::setw(10) << mat(0, 0) << " " << std::setw(10) << mat(0, 1) << " "
      << std::setw(10) << mat(0, 2) << " " << std::setw(10) << mat(0, 3);
  out << "]\n    [";
  out << std::setw(10) << mat(1, 0) << " " << std::setw(10) << mat(1, 1) << " "
      << std::setw(10) << mat(1, 2) << " " << std::setw(10) << mat(1, 3);
  out << "]\n    [";
  out << std::setw(10) << mat(2, 0) << " " << std::setw(10) << mat(2, 1) << " "
      << std::setw(10) << mat(2, 2) << " " << std::setw(10) << mat(2, 3);
  out << "]\n    [";
  out << std::setw(10) << mat(3, 0) << " " << std::setw(10) << mat(3, 1) << " "
      << std::setw(10) << mat(3, 2) << " " << std::setw(10) << mat(3, 3);
  out << "]\n";

  return out;
}

inline std::ostream& operator<<(std::ostream& out, const FieldOfView& fov) {
  return out << "fov(" << (fov.GetLeft() * 180.0f / M_PI) << ','
             << (fov.GetRight() * 180.0f / M_PI) << ','
             << (fov.GetBottom() * 180.0f / M_PI) << ','
             << (fov.GetTop() * 180.0f / M_PI) << ')';
}

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_LOG_HELPERS_H_
