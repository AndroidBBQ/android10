#ifndef ANDROID_DVR_FIELD_OF_VIEW_H_
#define ANDROID_DVR_FIELD_OF_VIEW_H_

#include <cmath>

#include <private/dvr/eigen.h>

namespace android {
namespace dvr {

// Encapsulates a generalized, asymmetric field of view with four half angles.
// Each half angle denotes the angle between the corresponding frustum plane.
// Together with a near and far plane, a FieldOfView forms the frustum of an
// off-axis perspective projection.
class FieldOfView {
 public:
  // The default constructor sets an angle of 0 (in any unit) for all four
  // half-angles.
  FieldOfView() : left_(0.0f), right_(0.0f), bottom_(0.0f), top_(0.0f) {}

  // Constructs a FieldOfView from four angles.
  FieldOfView(float left, float right, float bottom, float top)
      : left_(left), right_(right), bottom_(bottom), top_(top) {}

  explicit FieldOfView(const float* fov)
      : FieldOfView(fov[0], fov[1], fov[2], fov[3]) {}

  // Accessors for all four half-angles.
  float GetLeft() const { return left_; }
  float GetRight() const { return right_; }
  float GetBottom() const { return bottom_; }
  float GetTop() const { return top_; }

  // Setters for all four half-angles.
  void SetLeft(float left) { left_ = left; }
  void SetRight(float right) { right_ = right; }
  void SetBottom(float bottom) { bottom_ = bottom; }
  void SetTop(float top) { top_ = top; }

  Eigen::AffineMatrix<float, 4> GetProjectionMatrix(float z_near,
                                                    float z_far) const {
    float x_left = -std::tan(left_) * z_near;
    float x_right = std::tan(right_) * z_near;
    float y_bottom = -std::tan(bottom_) * z_near;
    float y_top = std::tan(top_) * z_near;

    float zero = 0.0f;
    if (x_left == x_right || y_bottom == y_top || z_near == z_far ||
        z_near <= zero || z_far <= zero) {
      return Eigen::AffineMatrix<float, 4>::Identity();
    }

    float x = (2 * z_near) / (x_right - x_left);
    float y = (2 * z_near) / (y_top - y_bottom);
    float a = (x_right + x_left) / (x_right - x_left);
    float b = (y_top + y_bottom) / (y_top - y_bottom);
    float c = (z_near + z_far) / (z_near - z_far);
    float d = (2 * z_near * z_far) / (z_near - z_far);

    // Note: Eigen matrix initialization syntax is always 'column-major'
    // even if the storage is row-major. Or in other words, just write the
    // matrix like you'd see in a math textbook.
    Eigen::AffineMatrix<float, 4> result;
    result.matrix() << x,  0,  a,  0,
                       0,  y,  b,  0,
                       0,  0,  c,  d,
                       0,  0, -1,  0;
    return result;
  }

  static FieldOfView FromProjectionMatrix(
      const Eigen::AffineMatrix<float, 4>& m) {
    // Compute tangents.
    float tan_vert_fov = 1.0f / m(1, 1);
    float tan_horz_fov = 1.0f / m(0, 0);
    float t = (m(1, 2) + 1.0f) * tan_vert_fov;
    float b = (m(1, 2) - 1.0f) * tan_vert_fov;
    float l = (m(0, 2) - 1.0f) * tan_horz_fov;
    float r = (m(0, 2) + 1.0f) * tan_horz_fov;

    return FieldOfView(std::atan(-l), std::atan(r), std::atan(-b),
                       std::atan(t));
  }

 private:
  float left_;
  float right_;
  float bottom_;
  float top_;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_FIELD_OF_VIEW_H_
