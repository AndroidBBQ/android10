#ifndef ANDROID_PDX_RPC_COPY_CV_REFERENCE_H_
#define ANDROID_PDX_RPC_COPY_CV_REFERENCE_H_

#include <type_traits>

namespace android {
namespace pdx {
namespace rpc {

// Copies const, void, and reference qualifiers from type T to type U, such that
// the new type U' carries the same cv-reference qualifiers as T, with the same
// underlying type as U.
template <typename T, typename U>
class CopyCVReference {
 private:
  using R = typename std::remove_reference<T>::type;
  using U1 =
      typename std::conditional<std::is_const<R>::value,
                                typename std::add_const<U>::type, U>::type;
  using U2 =
      typename std::conditional<std::is_volatile<R>::value,
                                typename std::add_volatile<U1>::type, U1>::type;
  using U3 =
      typename std::conditional<std::is_lvalue_reference<T>::value,
                                typename std::add_lvalue_reference<U2>::type,
                                U2>::type;
  using U4 =
      typename std::conditional<std::is_rvalue_reference<T>::value,
                                typename std::add_rvalue_reference<U3>::type,
                                U3>::type;

 public:
  using Type = U4;
};

template <typename T, typename U>
using CopyCVReferenceType = typename CopyCVReference<T, U>::Type;

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  //  ANDROID_PDX_RPC_COPY_CV_REFERENCE_H_
