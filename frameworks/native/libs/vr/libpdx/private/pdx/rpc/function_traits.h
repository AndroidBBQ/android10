#ifndef ANDROID_PDX_RPC_FUNCTION_TRAITS_H_
#define ANDROID_PDX_RPC_FUNCTION_TRAITS_H_

#include <type_traits>

#include <pdx/rpc/type_operators.h>

namespace android {
namespace pdx {
namespace rpc {

// Utility type to capture return and argument types of a function signature.
// Examples:
//     typedef SignatureType<int(int)> SignatureType;
//     using SignatureType = SignatureType<int(int)>;
template <typename T>
using SignatureType = T;

// Utility class to extract return and argument types from function types.
// Provides nested types for return value, arguments, and full signature. Also
// provides accessor types for individual arguments, argument-arity, and type
// substitution.
template <typename T>
struct FunctionTraits;

template <typename Return_, typename... Args_>
struct FunctionTraits<Return_(Args_...)> {
  using Return = Return_;
  using Args = std::tuple<Args_...>;
  using Signature = SignatureType<Return_(Args_...)>;

  enum : std::size_t { Arity = sizeof...(Args_) };

  template <std::size_t Index>
  using Arg = typename std::tuple_element<Index, Args>::type;

  template <typename... Params>
  using RewriteArgs =
      SignatureType<Return_(ConditionalRewrite<Args_, Params>...)>;

  template <typename ReturnType, typename... Params>
  using RewriteSignature =
      SignatureType<ConditionalRewrite<Return_, ReturnType>(
          ConditionalRewrite<Args_, Params>...)>;

  template <template <typename> class Wrapper, typename ReturnType,
            typename... Params>
  using RewriteSignatureWrapReturn =
      SignatureType<Wrapper<ConditionalRewrite<Return_, ReturnType>>(
          ConditionalRewrite<Args_, Params>...)>;

  template <typename ReturnType>
  using RewriteReturn =
      SignatureType<ConditionalRewrite<Return_, ReturnType>(Args_...)>;
};

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  //  ANDROID_PDX_RPC_FUNCTION_TRAITS_H_
