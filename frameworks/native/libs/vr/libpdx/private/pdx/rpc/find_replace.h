#ifndef ANDROID_PDX_RPC_FIND_REPLACE_H_
#define ANDROID_PDX_RPC_FIND_REPLACE_H_

#include <type_traits>

#include <pdx/rpc/copy_cv_reference.h>

namespace android {
namespace pdx {
namespace rpc {

// Utility class to capture types to find and replace.
template <typename Find, typename Replace>
struct FindReplace;

template <typename T, typename U>
using IsSameBaseType = typename std::is_same<typename std::decay<T>::type,
                                             typename std::decay<U>::type>;

// Replaces the type Subject with type Replace if type Subject is the same type
// as type Find, excluding cv-reference qualifiers in the match.
template <typename Find, typename Replace, typename Subject>
using ReplaceType =
    typename std::conditional<IsSameBaseType<Find, Subject>::value,
                              CopyCVReferenceType<Subject, Replace>,
                              Subject>::type;

// Determines whether the type Find (excluding cv-reference qualifiers) is in
// the given parameter pack.
template <typename Find, typename... Types>
struct ContainsType : std::true_type {};

template <typename Find, typename First, typename... Rest>
struct ContainsType<Find, First, Rest...>
    : std::conditional<IsSameBaseType<Find, First>::value, std::true_type,
                       ContainsType<Find, Rest...>>::type {};

template <typename Find>
struct ContainsType<Find> : std::false_type {};

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  //  ANDROID_PDX_RPC_FIND_REPLACE_H_
