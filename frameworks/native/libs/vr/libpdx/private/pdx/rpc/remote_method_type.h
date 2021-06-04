#ifndef ANDROID_PDX_RPC_REMOTE_METHOD_TYPE_H_
#define ANDROID_PDX_RPC_REMOTE_METHOD_TYPE_H_

#include <cstddef>
#include <tuple>
#include <type_traits>

#include <pdx/rpc/enumeration.h>
#include <pdx/rpc/function_traits.h>

namespace android {
namespace pdx {
namespace rpc {

// Utility class binding a remote method opcode to its function signature.
// Describes the interface between RPC clients and services for a single method.
template <int Opcode_, typename Signature_>
struct RemoteMethodType {
  typedef FunctionTraits<Signature_> Traits;

  enum : int { Opcode = Opcode_ };

  typedef typename Traits::Signature Signature;
  typedef typename Traits::Return Return;
  typedef typename Traits::Args Args;

  template <typename... Params>
  using RewriteArgs = typename Traits::template RewriteArgs<Params...>;

  template <typename ReturnType, typename... Params>
  using RewriteSignature =
      typename Traits::template RewriteSignature<ReturnType, Params...>;

  template <template <typename> class Wrapper, typename ReturnType,
            typename... Params>
  using RewriteSignatureWrapReturn =
      typename Traits::template RewriteSignatureWrapReturn<Wrapper, ReturnType,
                                                           Params...>;

  template <typename ReturnType>
  using RewriteReturn = typename Traits::template RewriteReturn<ReturnType>;
};

// Utility class representing a set of related RemoteMethodTypes. Describes the
// interface between RPC clients and services as a set of methods.
template <typename... MethodTypes>
struct RemoteAPI {
  typedef std::tuple<MethodTypes...> Methods;
  enum : std::size_t { Length = sizeof...(MethodTypes) };

  template <std::size_t Index>
  using Method = typename std::tuple_element<Index, Methods>::type;

  template <typename MethodType>
  static constexpr std::size_t MethodIndex() {
    return ElementForType<MethodType, MethodTypes...>::Index;
  }
};

// Macro to simplify defining remote method signatures. Remote method signatures
// are specified by defining a RemoteMethodType for each remote method.
#define PDX_REMOTE_METHOD(name, opcode, ... /*signature*/) \
  using name = ::android::pdx::rpc::RemoteMethodType<opcode, __VA_ARGS__>

// Macro to simplify defining a set of remote method signatures.
#define PDX_REMOTE_API(name, ... /*methods*/) \
  using name = ::android::pdx::rpc::RemoteAPI<__VA_ARGS__>

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_REMOTE_METHOD_TYPE_H_
