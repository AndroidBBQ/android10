#ifndef ANDROID_PDX_RPC_REMOTE_METHOD_H_
#define ANDROID_PDX_RPC_REMOTE_METHOD_H_

#include <tuple>
#include <type_traits>

#include <pdx/client.h>
#include <pdx/rpc/argument_encoder.h>
#include <pdx/rpc/message_buffer.h>
#include <pdx/rpc/payload.h>
#include <pdx/rpc/remote_method_type.h>
#include <pdx/service.h>
#include <pdx/status.h>

namespace android {
namespace pdx {
namespace rpc {

#ifdef __clang__
// Stand-in type to avoid Clang compiler bug. Clang currently has a bug where
// performing parameter pack expansion for arguments with empty packs causes a
// compiler crash. Provide a substitute void type and specializations/overloads
// of CheckArgumentTypes and DispatchRemoteMethod to work around this problem.
struct Void {};

// Evaluates to true if the method type is <any>(Void), false otherwise.
template <typename RemoteMethodType>
using IsVoidMethod = typename std::integral_constant<
    bool, RemoteMethodType::Traits::Arity == 1 &&
              std::is_same<typename RemoteMethodType::Traits::template Arg<0>,
                           Void>::value>;

// Utility to determine if a method is of type <any>(Void).
template <typename RemoteMethodType>
using EnableIfVoidMethod =
    typename std::enable_if<IsVoidMethod<RemoteMethodType>::value>::type;

// Utility to determine if a method is not of type <any>(Void).
template <typename RemoteMethodType>
using EnableIfNotVoidMethod =
    typename std::enable_if<!IsVoidMethod<RemoteMethodType>::value>::type;

#else
// GCC works fine with void argument types, always enable the regular
// implementation of DispatchRemoteMethod.
using Void = void;
template <typename RemoteMethodType>
using EnableIfVoidMethod = void;
template <typename RemoteMethodType>
using EnableIfNotVoidMethod = void;
#endif

// Helper type trait to specialize InvokeRemoteMethods for return types that
// can be obtained directly from Transaction::Send<T>() without deserializing
// reply payload.
template <typename T>
struct IsDirectReturn : std::false_type {};

template <>
struct IsDirectReturn<void> : std::true_type {};

template <>
struct IsDirectReturn<int> : std::true_type {};

template <>
struct IsDirectReturn<LocalHandle> : std::true_type {};

template <>
struct IsDirectReturn<LocalChannelHandle> : std::true_type {};

template <typename Return, typename Type = void>
using EnableIfDirectReturn =
    typename std::enable_if<IsDirectReturn<Return>::value, Type>::type;

template <typename Return, typename Type = void>
using EnableIfNotDirectReturn =
    typename std::enable_if<!IsDirectReturn<Return>::value, Type>::type;

// Utility class to invoke a method with arguments packed in a tuple.
template <typename Class, typename T>
class UnpackArguments;

// Utility class to invoke a method with arguments packed in a tuple.
template <typename Class, typename Return, typename... Args>
class UnpackArguments<Class, Return(Args...)> {
 public:
  using ArgsTupleType = std::tuple<typename std::decay<Args>::type...>;
  using MethodType = Return (Class::*)(Message&, Args...);

  UnpackArguments(Class& instance, MethodType method, Message& message,
                  ArgsTupleType& parameters)
      : instance_(instance),
        method_(method),
        message_(message),
        parameters_(parameters) {}

  // Invokes method_ on intance_ with the packed arguments from parameters_.
  Return Invoke() {
    constexpr auto Arity = sizeof...(Args);
    return static_cast<Return>(InvokeHelper(MakeIndexSequence<Arity>{}));
  }

 private:
  Class& instance_;
  MethodType method_;
  Message& message_;
  ArgsTupleType& parameters_;

  template <std::size_t... Index>
  Return InvokeHelper(IndexSequence<Index...>) {
    return static_cast<Return>((instance_.*method_)(
        message_,
        std::get<Index>(std::forward<ArgsTupleType>(parameters_))...));
  }

  UnpackArguments(const UnpackArguments&) = delete;
  void operator=(const UnpackArguments&) = delete;
};

// Returns an error code from a remote method to the client. May be called
// either during dispatch of the remote method handler or at a later time if the
// message is moved for delayed response.
inline void RemoteMethodError(Message& message, int error_code) {
  const auto status = message.ReplyError(error_code);
  ALOGE_IF(!status, "RemoteMethodError: Failed to reply to message: %s",
           status.GetErrorMessage().c_str());
}

// Returns a value from a remote method to the client. The usual method to
// return a value during dispatch of a remote method is to simply use a return
// statement in the handler. If the message is moved however, these methods may
// be used to return a value at a later time, outside of initial dispatch.

// Overload for direct return types.
template <typename RemoteMethodType, typename Return>
EnableIfDirectReturn<typename RemoteMethodType::Return> RemoteMethodReturn(
    Message& message, const Return& return_value) {
  const auto status = message.Reply(return_value);
  ALOGE_IF(!status, "RemoteMethodReturn: Failed to reply to message: %s",
           status.GetErrorMessage().c_str());
}

// Overload for non-direct return types.
template <typename RemoteMethodType, typename Return>
EnableIfNotDirectReturn<typename RemoteMethodType::Return> RemoteMethodReturn(
    Message& message, const Return& return_value) {
  using Signature = typename RemoteMethodType::template RewriteReturn<Return>;
  rpc::ServicePayload<ReplyBuffer> payload(message);
  MakeArgumentEncoder<Signature>(&payload).EncodeReturn(return_value);

  auto ret = message.WriteAll(payload.Data(), payload.Size());
  auto status = message.Reply(ret);
  ALOGE_IF(!status, "RemoteMethodReturn: Failed to reply to message: %s",
           status.GetErrorMessage().c_str());
}

// Overload for Status<void> return types.
template <typename RemoteMethodType>
void RemoteMethodReturn(Message& message, const Status<void>& return_value) {
  if (return_value)
    RemoteMethodReturn<RemoteMethodType>(message, 0);
  else
    RemoteMethodError(message, return_value.error());
}

// Overload for Status<T> return types. This overload forwards the underlying
// value or error within the Status<T>.
template <typename RemoteMethodType, typename Return>
void RemoteMethodReturn(Message& message, const Status<Return>& return_value) {
  if (return_value)
    RemoteMethodReturn<RemoteMethodType, Return>(message, return_value.get());
  else
    RemoteMethodError(message, return_value.error());
}

// Dispatches a method by deserializing arguments from the given Message, with
// compile-time interface check. Overload for void return types.
template <typename RemoteMethodType, typename Class, typename... Args,
          typename = EnableIfNotVoidMethod<RemoteMethodType>>
void DispatchRemoteMethod(Class& instance,
                          void (Class::*method)(Message&, Args...),
                          Message& message,
                          std::size_t max_capacity = InitialBufferCapacity) {
  using Signature = typename RemoteMethodType::template RewriteArgs<Args...>;
  rpc::ServicePayload<ReceiveBuffer> payload(message);
  payload.Resize(max_capacity);

  Status<size_t> read_status = message.Read(payload.Data(), payload.Size());
  if (!read_status) {
    RemoteMethodError(message, read_status.error());
    return;
  }

  payload.Resize(read_status.get());

  ErrorType error;
  auto decoder = MakeArgumentDecoder<Signature>(&payload);
  auto arguments = decoder.DecodeArguments(&error);
  if (error) {
    RemoteMethodError(message, EIO);
    return;
  }

  UnpackArguments<Class, Signature>(instance, method, message, arguments)
      .Invoke();
  // Return to the caller unless the message was moved.
  if (message)
    RemoteMethodReturn<RemoteMethodType>(message, 0);
}

// Dispatches a method by deserializing arguments from the given Message, with
// compile-time interface signature check. Overload for generic return types.
template <typename RemoteMethodType, typename Class, typename Return,
          typename... Args, typename = EnableIfNotVoidMethod<RemoteMethodType>>
void DispatchRemoteMethod(Class& instance,
                          Return (Class::*method)(Message&, Args...),
                          Message& message,
                          std::size_t max_capacity = InitialBufferCapacity) {
  using Signature =
      typename RemoteMethodType::template RewriteSignature<Return, Args...>;
  rpc::ServicePayload<ReceiveBuffer> payload(message);
  payload.Resize(max_capacity);

  Status<size_t> read_status = message.Read(payload.Data(), payload.Size());
  if (!read_status) {
    RemoteMethodError(message, read_status.error());
    return;
  }

  payload.Resize(read_status.get());

  ErrorType error;
  auto decoder = MakeArgumentDecoder<Signature>(&payload);
  auto arguments = decoder.DecodeArguments(&error);
  if (error) {
    RemoteMethodError(message, EIO);
    return;
  }

  auto return_value =
      UnpackArguments<Class, Signature>(instance, method, message, arguments)
          .Invoke();
  // Return the value to the caller unless the message was moved.
  if (message)
    RemoteMethodReturn<RemoteMethodType>(message, return_value);
}

// Dispatches a method by deserializing arguments from the given Message, with
// compile-time interface signature check. Overload for Status<T> return types.
template <typename RemoteMethodType, typename Class, typename Return,
          typename... Args, typename = EnableIfNotVoidMethod<RemoteMethodType>>
void DispatchRemoteMethod(Class& instance,
                          Status<Return> (Class::*method)(Message&, Args...),
                          Message& message,
                          std::size_t max_capacity = InitialBufferCapacity) {
  using Signature =
      typename RemoteMethodType::template RewriteSignature<Return, Args...>;
  using InvokeSignature =
      typename RemoteMethodType::template RewriteSignatureWrapReturn<
          Status, Return, Args...>;
  rpc::ServicePayload<ReceiveBuffer> payload(message);
  payload.Resize(max_capacity);

  Status<size_t> read_status = message.Read(payload.Data(), payload.Size());
  if (!read_status) {
    RemoteMethodError(message, read_status.error());
    return;
  }

  payload.Resize(read_status.get());

  ErrorType error;
  auto decoder = MakeArgumentDecoder<Signature>(&payload);
  auto arguments = decoder.DecodeArguments(&error);
  if (error) {
    RemoteMethodError(message, EIO);
    return;
  }

  auto return_value = UnpackArguments<Class, InvokeSignature>(
                          instance, method, message, arguments)
                          .Invoke();
  // Return the value to the caller unless the message was moved.
  if (message)
    RemoteMethodReturn<RemoteMethodType>(message, return_value);
}

#ifdef __clang__
// Overloads to handle Void argument type without exploding clang.

// Overload for void return type.
template <typename RemoteMethodType, typename Class,
          typename = EnableIfVoidMethod<RemoteMethodType>>
void DispatchRemoteMethod(Class& instance, void (Class::*method)(Message&),
                          Message& message) {
  (instance.*method)(message);
  // Return to the caller unless the message was moved.
  if (message)
    RemoteMethodReturn<RemoteMethodType>(message, 0);
}

// Overload for generic return type.
template <typename RemoteMethodType, typename Class, typename Return,
          typename = EnableIfVoidMethod<RemoteMethodType>>
void DispatchRemoteMethod(Class& instance, Return (Class::*method)(Message&),
                          Message& message) {
  auto return_value = (instance.*method)(message);
  // Return the value to the caller unless the message was moved.
  if (message)
    RemoteMethodReturn<RemoteMethodType>(message, return_value);
}

// Overload for Status<T> return type.
template <typename RemoteMethodType, typename Class, typename Return,
          typename = EnableIfVoidMethod<RemoteMethodType>>
void DispatchRemoteMethod(Class& instance,
                          Status<Return> (Class::*method)(Message&),
                          Message& message) {
  auto return_value = (instance.*method)(message);
  // Return the value to the caller unless the message was moved.
  if (message)
    RemoteMethodReturn<RemoteMethodType>(message, return_value);
}
#endif

}  // namespace rpc

// Definitions for template methods declared in pdx/client.h.

template <int Opcode, typename T>
struct CheckArgumentTypes;

template <int Opcode, typename Return, typename... Args>
struct CheckArgumentTypes<Opcode, Return(Args...)> {
  template <typename R>
  static typename rpc::EnableIfDirectReturn<R, Status<R>> Invoke(Client& client,
                                                                 Args... args) {
    Transaction trans{client};
    rpc::ClientPayload<rpc::SendBuffer> payload{trans};
    rpc::MakeArgumentEncoder<Return(Args...)>(&payload).EncodeArguments(
        std::forward<Args>(args)...);
    return trans.Send<R>(Opcode, payload.Data(), payload.Size(), nullptr, 0);
  }

  template <typename R>
  static typename rpc::EnableIfNotDirectReturn<R, Status<R>> Invoke(
      Client& client, Args... args) {
    Transaction trans{client};

    rpc::ClientPayload<rpc::SendBuffer> send_payload{trans};
    rpc::MakeArgumentEncoder<Return(Args...)>(&send_payload)
        .EncodeArguments(std::forward<Args>(args)...);

    rpc::ClientPayload<rpc::ReplyBuffer> reply_payload{trans};
    reply_payload.Resize(reply_payload.Capacity());

    Status<R> result;
    auto status =
        trans.Send<void>(Opcode, send_payload.Data(), send_payload.Size(),
                         reply_payload.Data(), reply_payload.Size());
    if (!status) {
      result.SetError(status.error());
    } else {
      R return_value;
      rpc::ErrorType error =
          rpc::MakeArgumentDecoder<Return(Args...)>(&reply_payload)
              .DecodeReturn(&return_value);

      switch (error.error_code()) {
        case rpc::ErrorCode::NO_ERROR:
          result.SetValue(std::move(return_value));
          break;

        // This error is returned when ArrayWrapper/StringWrapper is too
        // small to receive the payload.
        case rpc::ErrorCode::INSUFFICIENT_DESTINATION_SIZE:
          result.SetError(ENOBUFS);
          break;

        default:
          result.SetError(EIO);
          break;
      }
    }
    return result;
  }

  template <typename R>
  static typename rpc::EnableIfDirectReturn<R, Status<void>> InvokeInPlace(
      Client& client, R* return_value, Args... args) {
    Transaction trans{client};

    rpc::ClientPayload<rpc::SendBuffer> send_payload{trans};
    rpc::MakeArgumentEncoder<Return(Args...)>(&send_payload)
        .EncodeArguments(std::forward<Args>(args)...);

    Status<void> result;
    auto status = trans.Send<R>(Opcode, send_payload.Data(),
                                send_payload.Size(), nullptr, 0);
    if (status) {
      *return_value = status.take();
      result.SetValue();
    } else {
      result.SetError(status.error());
    }
    return result;
  }

  template <typename R>
  static typename rpc::EnableIfNotDirectReturn<R, Status<void>> InvokeInPlace(
      Client& client, R* return_value, Args... args) {
    Transaction trans{client};

    rpc::ClientPayload<rpc::SendBuffer> send_payload{trans};
    rpc::MakeArgumentEncoder<Return(Args...)>(&send_payload)
        .EncodeArguments(std::forward<Args>(args)...);

    rpc::ClientPayload<rpc::ReplyBuffer> reply_payload{trans};
    reply_payload.Resize(reply_payload.Capacity());

    auto result =
        trans.Send<void>(Opcode, send_payload.Data(), send_payload.Size(),
                         reply_payload.Data(), reply_payload.Size());
    if (result) {
      rpc::ErrorType error =
          rpc::MakeArgumentDecoder<Return(Args...)>(&reply_payload)
              .DecodeReturn(return_value);

      switch (error.error_code()) {
        case rpc::ErrorCode::NO_ERROR:
          result.SetValue();
          break;

        // This error is returned when ArrayWrapper/StringWrapper is too
        // small to receive the payload.
        case rpc::ErrorCode::INSUFFICIENT_DESTINATION_SIZE:
          result.SetError(ENOBUFS);
          break;

        default:
          result.SetError(EIO);
          break;
      }
    }
    return result;
  }
};

// Invokes the remote method with opcode and signature described by
// |RemoteMethodType|.
template <typename RemoteMethodType, typename... Args>
Status<typename RemoteMethodType::Return> Client::InvokeRemoteMethod(
    Args&&... args) {
  return CheckArgumentTypes<
      RemoteMethodType::Opcode,
      typename RemoteMethodType::template RewriteArgs<Args...>>::
      template Invoke<typename RemoteMethodType::Return>(
          *this, std::forward<Args>(args)...);
}

template <typename RemoteMethodType, typename Return, typename... Args>
Status<void> Client::InvokeRemoteMethodInPlace(Return* return_value,
                                               Args&&... args) {
  return CheckArgumentTypes<
      RemoteMethodType::Opcode,
      typename RemoteMethodType::template RewriteSignature<Return, Args...>>::
      template InvokeInPlace(*this, return_value, std::forward<Args>(args)...);
}

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_REMOTE_METHOD_H_
