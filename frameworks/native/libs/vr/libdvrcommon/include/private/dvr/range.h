#ifndef ANDROID_DVR_RANGE_H_
#define ANDROID_DVR_RANGE_H_

#include <private/dvr/eigen.h>

namespace android {
namespace dvr {

// TODO(skiazyk): Replace all instances of this with Eigen::AlignedBox

// Container of two points that define a 2D range.
template <class T, int d>
struct Range {
  // Construct an uninitialized Range.
  Range() {}
  Range(Eigen::Vector<T, d> p1, Eigen::Vector<T, d> p2) : p1(p1), p2(p2) {}

  static Range<T, d> FromSize(Eigen::Vector<T, d> p1,
                              Eigen::Vector<T, d> size) {
    return Range<T, d>(p1, p1 + size);
  }

  bool operator==(const Range<T, d>& rhs) const {
    return p1 == rhs.p1 && p2 == rhs.p2;
  }

  Eigen::Vector<T, d> GetMinPoint() const { return p1; }

  Eigen::Vector<T, d> GetMaxPoint() const { return p2; }

  Eigen::Vector<T, d> GetSize() const { return p2 - p1; }

  Eigen::Vector<T, d> p1;
  Eigen::Vector<T, d> p2;
};

typedef Range<int, 2> Range2i;
typedef Range<float, 2> Range2f;

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_RANGE_H_
