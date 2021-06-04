#ifndef ANDROID_DVR_EIGEN_H_
#define ANDROID_DVR_EIGEN_H_

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace Eigen {

// Eigen doesn't take advantage of C++ template typedefs, but we can
template <class T, int N>
using Vector = Matrix<T, N, 1>;

template <class T>
using Vector2 = Vector<T, 2>;

template <class T>
using Vector3 = Vector<T, 3>;

template <class T>
using Vector4 = Vector<T, 4>;

template <class T, int N>
using RowVector = Matrix<T, 1, N>;

template <class T>
using RowVector2 = RowVector<T, 2>;

template <class T>
using RowVector3 = RowVector<T, 3>;

template <class T>
using RowVector4 = RowVector<T, 4>;

// In Eigen, the type you should be using for transformation matrices is the
// `Transform` class, instead of a raw `Matrix`.
// The `Projective` option means this will not make any assumptions about the
// last row of the object, making this suitable for use as general OpenGL
// projection matrices (which is the most common use-case). The one caveat
// is that in order to apply this transformation to non-homogeneous vectors
// (e.g., vec3), you must use the `.linear()` method to get the affine part of
// the matrix.
//
// Example:
//   mat4 transform;
//   vec3 position;
//   vec3 transformed = transform.linear() * position;
//
// Note, the use of N-1 is because the parameter passed to Eigen is the ambient
// dimension of the transformation, not the size of the matrix iself.
// However graphics programmers sometimes get upset when they see a 3 next
// to a matrix when they expect a 4, so I'm hoping this will avoid that.
template <class T, int N>
using AffineMatrix = Transform<T, N-1, Projective>;

}  // namespace Eigen

#endif  // ANDROID_DVR_EIGEN_H_
