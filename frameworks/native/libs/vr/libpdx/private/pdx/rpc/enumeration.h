#ifndef ANDROID_PDX_RPC_ENUMERATION_H_
#define ANDROID_PDX_RPC_ENUMERATION_H_

#include <pdx/rpc/sequence.h>

namespace android {
namespace pdx {
namespace rpc {

// Utility for manipulating lists of types. Provides operations to lookup an
// element by type or index.

namespace detail {

// Helper type that captures type and index for each element of a type
// enumeration.
template <std::size_t I, typename T>
struct IndexedElement {
  using Type = T;
  static constexpr std::size_t Index = I;
};

// Helper type that captures an IndexSequence and corresponding list of types.
template <typename Is, typename... Ts>
struct ElementIndexer;

// Partial specialization that generates an instantiation of IndexElement<I, T>
// for each element of a type enumeration using inheritance. Once a type
// enumeration is instantiated this way the compiler is able to deduce either I
// or T from the other using the method below.
template <std::size_t... Is, typename... Ts>
struct ElementIndexer<IndexSequence<Is...>, Ts...> : IndexedElement<Is, Ts>... {
};

// Helper function that causes the compiler to deduce an IndexedElement<I, T>
// given T.
template <typename T, std::size_t I>
static IndexedElement<I, T> SelectElementByType(IndexedElement<I, T>);

// Helper function that causes the compiler to deduce an IndexedElement<I, T>
// given I.
template <std::size_t I, typename T>
static IndexedElement<I, T> SelectElementByIndex(IndexedElement<I, T>);

}  // namespace detail

// Deduces the IndexedElement<I, T> given T and a type sequence Ts. This may be
// used to determine the index of T within Ts at compile time.
template <typename T, typename... Ts>
using ElementForType = decltype(detail::SelectElementByType<T>(
    detail::ElementIndexer<typename IndexSequenceFor<Ts...>::type, Ts...>{}));

// Deduces the IndexedElement<I, T> given I and a type sequence Ts. This may be
// used to determine the type of the element at index I within Ts at compile
// time. Tuple operations may also be used to accomplish the same task, however
// this implementation is provided here for symmetry.
template <std::size_t I, typename... Ts>
using ElementForIndex = decltype(detail::SelectElementByIndex<I>(
    detail::ElementIndexer<typename IndexSequenceFor<Ts...>::type, Ts...>{}));

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_ENUMERATION_H_
