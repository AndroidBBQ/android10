#ifndef ANDROID_PDX_RPC_SEQUENCE_H_
#define ANDROID_PDX_RPC_SEQUENCE_H_

#include <cstdint>

namespace android {
namespace pdx {
namespace rpc {

// Provides a C++11 implementation of C++14 index_sequence and
// make_index_sequence for compatibility with common compilers. This
// implementation may be conditionally replaced with compiler-provided versions
// when C++14 support is available.

// Utility to capture a sequence of unsigned indices.
template <std::size_t... I>
struct IndexSequence {
  using type = IndexSequence;
  using value_type = std::size_t;
  static constexpr std::size_t size() { return sizeof...(I); }
};

namespace detail {

// Helper class to merge and renumber sequence parts in log N instantiations.
template <typename S1, typename S2>
struct MergeSequencesAndRenumber;

template <std::size_t... I1, std::size_t... I2>
struct MergeSequencesAndRenumber<IndexSequence<I1...>, IndexSequence<I2...>>
    : IndexSequence<I1..., (sizeof...(I1) + I2)...> {};

}  // namespace detail

// Utility to build an IndexSequence with N indices.
template <std::size_t N>
struct MakeIndexSequence : detail::MergeSequencesAndRenumber<
                               typename MakeIndexSequence<N / 2>::type,
                               typename MakeIndexSequence<N - N / 2>::type> {};

// Identity sequences.
template <>
struct MakeIndexSequence<0> : IndexSequence<> {};
template <>
struct MakeIndexSequence<1> : IndexSequence<0> {};

// Utility to build an IndexSequence with indices for each element of a
// parameter pack.
template <typename... T>
using IndexSequenceFor = MakeIndexSequence<sizeof...(T)>;

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_SEQUENCE_H_
