#ifndef ANDROID_DVR_POSE_H_
#define ANDROID_DVR_POSE_H_

#include <private/dvr/eigen.h>

namespace android {
namespace dvr {

// Encapsulates a 3D pose (rotation and position).
//
// @tparam T Data type for storing the position coordinate and rotation
//     quaternion.
template <typename T>
class Pose {
 public:
  // Creates identity pose.
  Pose()
      : rotation_(Eigen::Quaternion<T>::Identity()),
        position_(Eigen::Vector3<T>::Zero()) {}

  // Initializes a pose with given rotation and position.
  //
  // rotation Initial rotation.
  // position Initial position.
  Pose(Eigen::Quaternion<T> rotation, Eigen::Vector3<T> position)
      : rotation_(rotation), position_(position) {}

  void Invert() {
    rotation_ = rotation_.inverse();
    position_ = rotation_ * -position_;
  }

  Pose Inverse() const {
    Pose result(*this);
    result.Invert();
    return result;
  }

  // Compute the composition of this pose with another, storing the result
  // in the current object
  void ComposeInPlace(const Pose& other) {
    position_ = position_ + rotation_ * other.position_;
    rotation_ = rotation_ * other.rotation_;
  }

  // Computes the composition of this pose with another, and returns the result
  Pose Compose(const Pose& other) const {
    Pose result(*this);
    result.ComposeInPlace(other);
    return result;
  }

  Eigen::Vector3<T> TransformPoint(const Eigen::Vector3<T>& v) const {
    return rotation_ * v + position_;
  }

  Eigen::Vector3<T> Transform(const Eigen::Vector3<T>& v) const {
    return rotation_ * v;
  }

  Pose& operator*=(const Pose& other) {
    ComposeInPlace(other);
    return *this;
  }

  Pose operator*(const Pose& other) const { return Compose(other); }

  // Gets the rotation of the 3D pose.
  Eigen::Quaternion<T> GetRotation() const { return rotation_; }

  // Gets the position of the 3D pose.
  Eigen::Vector3<T> GetPosition() const { return position_; }

  // Sets the rotation of the 3D pose.
  void SetRotation(Eigen::Quaternion<T> rotation) { rotation_ = rotation; }

  // Sets the position of the 3D pose.
  void SetPosition(Eigen::Vector3<T> position) { position_ = position; }

  // Gets a 4x4 matrix representing a transform from the reference space (that
  // the rotation and position of the pose are relative to) to the object space.
  Eigen::AffineMatrix<T, 4> GetObjectFromReferenceMatrix() const;

  // Gets a 4x4 matrix representing a transform from the object space to the
  // reference space (that the rotation and position of the pose are relative
  // to).
  Eigen::AffineMatrix<T, 4> GetReferenceFromObjectMatrix() const;

 private:
  Eigen::Quaternion<T> rotation_;
  Eigen::Vector3<T> position_;
};

template <typename T>
Eigen::AffineMatrix<T, 4> Pose<T>::GetObjectFromReferenceMatrix() const {
  // The transfrom from the reference is the inverse of the pose.
  Eigen::AffineMatrix<T, 4> matrix(rotation_.inverse().toRotationMatrix());
  return matrix.translate(-position_);
}

template <typename T>
Eigen::AffineMatrix<T, 4> Pose<T>::GetReferenceFromObjectMatrix() const {
  // The transfrom to the reference.
  Eigen::AffineMatrix<T, 4> matrix(rotation_.toRotationMatrix());
  return matrix.pretranslate(position_);
}

//------------------------------------------------------------------------------
// Type-specific typedefs.
//------------------------------------------------------------------------------

using Posef = Pose<float>;
using Posed = Pose<double>;

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_POSE_H_
