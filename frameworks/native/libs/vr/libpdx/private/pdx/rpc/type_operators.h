#ifndef ANDROID_PDX_RPC_TYPE_OPERATORS_H_
#define ANDROID_PDX_RPC_TYPE_OPERATORS_H_

#include <array>
#include <map>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <pdx/channel_handle.h>
#include <pdx/file_handle.h>
#include <pdx/rpc/array_wrapper.h>
#include <pdx/rpc/buffer_wrapper.h>
#include <pdx/rpc/copy_cv_reference.h>
#include <pdx/rpc/pointer_wrapper.h>
#include <pdx/rpc/string_wrapper.h>

namespace android {
namespace pdx {
namespace rpc {

// Simplifies type expressions.
template <typename T>
using Decay = typename std::decay<T>::type;

// Compares the underlying type of A and B.
template <typename A, typename B>
using IsEquivalent = typename std::is_same<Decay<A>, Decay<B>>::type;

// Logical AND over template parameter pack.
template <typename... T>
struct And : std::false_type {};
template <typename A, typename B>
struct And<A, B> : std::integral_constant<bool, A::value && B::value> {};
template <typename A, typename B, typename... Rest>
struct And<A, B, Rest...> : And<A, And<B, Rest...>> {};

// Determines whether A is convertible to B (serializes to the same format)
// using these rules:
//    1. std:vector<T, Any...> is convertible to ArrayWrapper<T>.
//    2. ArrayWrapper<T> is convertible to std:vector<T, Any...>.
//    3. std::basic_string<T, Any...> is convertible to StringWrapper<T>.
//    4. StringWrapper<T> is convertible to std::basic_string<T, Any...>.
//    5. BufferWrapper<T*> is convertible to BufferWrapper<std::vector<T,
//    Any...>>.
//    6. BufferWrapper<std::vector<T, ...>> is convertible to BufferWrapper<T*>.
//    7. The value type T of A and B must match.

// Compares A and B for convertibility. This base type determines convertibility
// by equivalence of the underlying types of A and B. Specializations of this
// type handle the rules for which complex types are convertible.
template <typename A, typename B>
struct IsConvertible : IsEquivalent<A, B> {};

// Compares TT<A, ...> and TT<B, ...>; these are convertible if A and B are
// convertible.
template <template <typename, typename...> class TT, typename A, typename B,
          typename... AnyA, typename... AnyB>
struct IsConvertible<TT<A, AnyA...>, TT<B, AnyB...>>
    : IsConvertible<Decay<A>, Decay<B>> {};

// Compares TT<KeyA, ValueA, ...> and TT<KeyB, ValueB, ...>; these are
// convertible if KeyA and KeyB are
// convertible and ValueA and ValueB are convertible.
template <template <typename, typename, typename...> class TT, typename KeyA,
          typename ValueA, typename KeyB, typename ValueB, typename... AnyA,
          typename... AnyB>
struct IsConvertible<TT<KeyA, ValueA, AnyA...>, TT<KeyB, ValueB, AnyB...>>
    : And<IsConvertible<Decay<KeyA>, Decay<KeyB>>,
          IsConvertible<Decay<ValueA>, Decay<ValueB>>> {};

// Compares two std::pairs to see if the corresponding elements are convertible.
template <typename A, typename B, typename C, typename D>
struct IsConvertible<std::pair<A, B>, std::pair<C, D>>
    : And<IsConvertible<Decay<A>, Decay<C>>,
          IsConvertible<Decay<B>, Decay<D>>> {};

// Compares std::pair with a two-element std::tuple to see if the corresponding
// elements are convertible.
template <typename A, typename B, typename C, typename D>
struct IsConvertible<std::pair<A, B>, std::tuple<C, D>>
    : And<IsConvertible<Decay<A>, Decay<C>>,
          IsConvertible<Decay<B>, Decay<D>>> {};
template <typename A, typename B, typename C, typename D>
struct IsConvertible<std::tuple<A, B>, std::pair<C, D>>
    : And<IsConvertible<Decay<A>, Decay<C>>,
          IsConvertible<Decay<B>, Decay<D>>> {};

// Compares two std::tuples to see if the corresponding elements are
// convertible.
template <typename... A, typename... B>
struct IsConvertible<std::tuple<A...>, std::tuple<B...>>
    : And<IsConvertible<Decay<A>, Decay<B>>...> {};

// Compares std::vector, std::array, and ArrayWrapper; these are convertible if
// the value types are convertible.
template <typename A, typename B, typename... Any>
struct IsConvertible<std::vector<A, Any...>, ArrayWrapper<B>>
    : IsConvertible<Decay<A>, Decay<B>> {};
template <typename A, typename B, typename... Any>
struct IsConvertible<ArrayWrapper<A>, std::vector<B, Any...>>
    : IsConvertible<Decay<A>, Decay<B>> {};
template <typename A, typename B, typename... Any, std::size_t Size>
struct IsConvertible<std::vector<A, Any...>, std::array<B, Size>>
    : IsConvertible<Decay<A>, Decay<B>> {};
template <typename A, typename B, typename... Any, std::size_t Size>
struct IsConvertible<std::array<A, Size>, std::vector<B, Any...>>
    : IsConvertible<Decay<A>, Decay<B>> {};
template <typename A, typename B, std::size_t Size>
struct IsConvertible<ArrayWrapper<A>, std::array<B, Size>>
    : IsConvertible<Decay<A>, Decay<B>> {};
template <typename A, typename B, std::size_t Size>
struct IsConvertible<std::array<A, Size>, ArrayWrapper<B>>
    : IsConvertible<Decay<A>, Decay<B>> {};

// Compares std::map and std::unordered_map; these are convertible if the keys
// are convertible and the values are convertible.
template <typename KeyA, typename ValueA, typename KeyB, typename ValueB,
          typename... AnyA, typename... AnyB>
struct IsConvertible<std::map<KeyA, ValueA, AnyA...>,
                     std::unordered_map<KeyB, ValueB, AnyB...>>
    : And<IsConvertible<Decay<KeyA>, Decay<KeyB>>,
          IsConvertible<Decay<ValueA>, Decay<ValueB>>> {};
template <typename KeyA, typename ValueA, typename KeyB, typename ValueB,
          typename... AnyA, typename... AnyB>
struct IsConvertible<std::unordered_map<KeyA, ValueA, AnyA...>,
                     std::map<KeyB, ValueB, AnyB...>>
    : And<IsConvertible<Decay<KeyA>, Decay<KeyB>>,
          IsConvertible<Decay<ValueA>, Decay<ValueB>>> {};

// Compares BufferWrapper<A*> and BufferWrapper<std::vector<B>>; these are
// convertible if A and B are equivalent. Allocator types are not relevant to
// convertibility.
template <typename A, typename B, typename Allocator>
struct IsConvertible<BufferWrapper<A*>,
                     BufferWrapper<std::vector<B, Allocator>>>
    : IsEquivalent<A, B> {};
template <typename A, typename B, typename Allocator>
struct IsConvertible<BufferWrapper<std::vector<A, Allocator>>,
                     BufferWrapper<B*>> : IsEquivalent<A, B> {};
template <typename A, typename B, typename AllocatorA, typename AllocatorB>
struct IsConvertible<BufferWrapper<std::vector<A, AllocatorA>>,
                     BufferWrapper<std::vector<B, AllocatorB>>>
    : IsEquivalent<A, B> {};
template <typename A, typename B>
struct IsConvertible<BufferWrapper<A*>, BufferWrapper<B*>>
    : IsEquivalent<A, B> {};

// Compares std::basic_string<A, ...> and StringWrapper<B>; these are
// convertible if A and B are equivalent.
template <typename A, typename B, typename... Any>
struct IsConvertible<std::basic_string<A, Any...>, StringWrapper<B>>
    : IsEquivalent<A, B> {};
template <typename A, typename B, typename... Any>
struct IsConvertible<StringWrapper<A>, std::basic_string<B, Any...>>
    : IsEquivalent<A, B> {};

// Compares PointerWrapper<A> and B; these are convertible if A and B are
// convertible.
template <typename A, typename B>
struct IsConvertible<PointerWrapper<A>, B> : IsConvertible<Decay<A>, Decay<B>> {
};
template <typename A, typename B>
struct IsConvertible<A, PointerWrapper<B>> : IsConvertible<Decay<A>, Decay<B>> {
};

// LocalHandle is convertible to RemoteHandle on the service side. This means
// that a RemoteHandle may be supplied by a service when the protocol calls for
// a LocalHandle return value. The other way around is not safe and can leak
// file descriptors. The ServicePayload class enforces this policy by only
// supporting RemoteHandle for pushed handles.
template <>
struct IsConvertible<LocalHandle, RemoteHandle> : std::true_type {};
template <>
struct IsConvertible<LocalHandle, BorrowedHandle> : std::true_type {};

template <>
struct IsConvertible<LocalChannelHandle, RemoteChannelHandle> : std::true_type {
};
template <>
struct IsConvertible<LocalChannelHandle, BorrowedChannelHandle>
    : std::true_type {};

// Conditionally "rewrites" type A as type B, including cv-reference qualifiers,
// iff A is convertible to B.
template <typename A, typename B>
using ConditionalRewrite =
    typename std::conditional<IsConvertible<Decay<A>, Decay<B>>::value,
                              CopyCVReferenceType<A, B>, A>::type;

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  //  ANDROID_PDX_RPC_TYPE_OPERATORS_H_
