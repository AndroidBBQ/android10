#ifndef ANDROID_PDX_RPC_ARGUMENT_ENCODER_H_
#define ANDROID_PDX_RPC_ARGUMENT_ENCODER_H_

#include <cstdint>
#include <tuple>
#include <type_traits>

#include <pdx/rpc/serialization.h>
#include <pdx/service.h>

namespace android {
namespace pdx {
namespace rpc {

// Provides automatic serialization of argument lists and return
// values by analyzing the supplied function signature types.
// Examples:
//     ArgumentEncoder<int(int, float)> encoder(writer);
//     encoder.EncodeArguments(1, 1.0);

template <typename T>
class ArgumentEncoder;

// Specialization of ArgumentEncoder for void return types.
template <typename... Args>
class ArgumentEncoder<void(Args...)> {
 public:
  explicit ArgumentEncoder(MessageWriter* writer) : writer_{writer} {}

  // Serializes the arguments as a tuple.
  void EncodeArguments(Args... args) {
    Serialize(std::forward_as_tuple(args...), writer_);
  }

 private:
  MessageWriter* writer_;
};

// Specialization of ArgumentEncoder for non-void return types.
template <typename Return, typename... Args>
class ArgumentEncoder<Return(Args...)> {
 public:
  // Simplified types with reference and cv removed.
  using ReturnType = typename std::decay<Return>::type;

  explicit ArgumentEncoder(MessageWriter* writer) : writer_{writer} {}

  // Serializes the arguments as a tuple.
  void EncodeArguments(Args... args) {
    Serialize(std::forward_as_tuple(args...), writer_);
  }

  // Serializes the return value for rvalue references.
  void EncodeReturn(const ReturnType& return_value) {
    Serialize(return_value, writer_);
  }

 private:
  MessageWriter* writer_;
};

// Utility to build an ArgumentEncoder from a function pointer and a message
// writer.
template <typename Return, typename... Args>
inline ArgumentEncoder<Return(Args...)> MakeArgumentEncoder(
    Return (*)(Args...), MessageWriter* writer) {
  return ArgumentEncoder<Return(Args...)>(writer);
}

// Utility to build an ArgumentEncoder from a method pointer and a message
// writer.
template <typename Class, typename Return, typename... Args>
inline ArgumentEncoder<Return(Args...)> MakeArgumentEncoder(
    Return (Class::*)(Args...), MessageWriter* writer) {
  return ArgumentEncoder<Return(Args...)>(writer);
}

// Utility to build an ArgumentEncoder from a const method pointer and a
// message writer.
template <typename Class, typename Return, typename... Args>
inline ArgumentEncoder<Return(Args...)> MakeArgumentEncoder(
    Return (Class::*)(Args...) const, MessageWriter* writer) {
  return ArgumentEncoder<Return(Args...)>(writer);
}

// Utility to build an ArgumentEncoder from a function type and a message
// writer.
template <typename Signature>
inline ArgumentEncoder<Signature> MakeArgumentEncoder(MessageWriter* writer) {
  return ArgumentEncoder<Signature>(writer);
}

//////////////////////////////////////////////////////////////////////////////
// Provides automatic deserialization of argument lists and return
// values by analyzing the supplied function signature types.
// Examples:
//     auto decoder = MakeArgumentDecoder<std::string(void)>(reader);
//     ErrorType error = decoder.DecodeReturn(&return_value);

template <typename T>
class ArgumentDecoder;

// Specialization of ArgumentDecoder for void return types.
template <typename... Args>
class ArgumentDecoder<void(Args...)> {
 public:
  // Simplified types with reference and cv removed.
  using ArgsTupleType = std::tuple<typename std::decay<Args>::type...>;

  explicit ArgumentDecoder(MessageReader* reader) : reader_{reader} {}

  // Deserializes arguments into a tuple.
  ArgsTupleType DecodeArguments(ErrorType* error) {
    ArgsTupleType value;
    *error = Deserialize(&value, reader_);
    return value;
  }

 private:
  MessageReader* reader_;
};

// Specialization of ArgumentDecoder for non-void return types.
template <typename Return, typename... Args>
class ArgumentDecoder<Return(Args...)> {
 public:
  // Simplified types with reference and cv removed.
  using ArgsTupleType = std::tuple<typename std::decay<Args>::type...>;
  using ReturnType = typename std::decay<Return>::type;

  explicit ArgumentDecoder(MessageReader* reader) : reader_{reader} {}

  // Deserializes arguments into a tuple.
  ArgsTupleType DecodeArguments(ErrorType* error) {
    ArgsTupleType value;
    *error = Deserialize(&value, reader_);
    return value;
  }

  // Deserializes the return value.
  ErrorType DecodeReturn(ReturnType* value) {
    return Deserialize(value, reader_);
  }

 private:
  MessageReader* reader_;
};

// Utility to build an ArgumentDecoder from a function pointer and a message
// reader.
template <typename Return, typename... Args>
inline ArgumentDecoder<Return(Args...)> MakeArgumentDecoder(
    Return (*)(Args...), MessageReader* reader) {
  return ArgumentDecoder<Return(Args...)>(reader);
}

// Utility to build an ArgumentDecoder from a method pointer and a message
// reader.
template <typename Class, typename Return, typename... Args>
inline ArgumentDecoder<Return(Args...)> MakeArgumentDecoder(
    Return (Class::*)(Args...), MessageReader* reader) {
  return ArgumentDecoder<Return(Args...)>(reader);
}

// Utility to build an ArgumentDecoder from a const method pointer and a
// message reader.
template <typename Class, typename Return, typename... Args>
inline ArgumentDecoder<Return(Args...)> MakeArgumentDecoder(
    Return (Class::*)(Args...) const, MessageReader* reader) {
  return ArgumentDecoder<Return(Args...)>(reader);
}

// Utility to build an ArgumentDecoder from a function type and a message
// reader.
template <typename Signature>
inline ArgumentDecoder<Signature> MakeArgumentDecoder(MessageReader* reader) {
  return ArgumentDecoder<Signature>(reader);
}

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_ARGUMENT_ENCODER_H_
