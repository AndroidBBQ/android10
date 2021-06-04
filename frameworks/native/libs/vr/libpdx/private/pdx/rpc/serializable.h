#ifndef ANDROID_PDX_RPC_SERIALIZABLE_H_
#define ANDROID_PDX_RPC_SERIALIZABLE_H_

#include <cstddef>
#include <string>
#include <tuple>

#include <pdx/message_reader.h>
#include <pdx/message_writer.h>

#include "macros.h"
#include "serialization.h"

namespace android {
namespace pdx {
namespace rpc {

// This file provides utilities to define serializable types for communication
// between clients and services. Supporting efficient, typed communication
// protocols is the primary goal, NOT providing a general-purpose solution for
// all your C++ serialization needs. Features that are not aligned to the goals
// are not supported, such as static/const member serialization and serializable
// types with virtual methods (requiring a virtual destructor).

// Captures the type and value of a pointer to member. Pointer to members are
// essentially compile-time constant offsets that can be stored in the type
// system without adding to the size of the structures they describe. This
// library uses this property to implement a limited form of reflection for
// serialization/deserialization functions.
template <typename T, T>
struct MemberPointer;

template <typename Type, typename Class, Type Class::*Pointer>
struct MemberPointer<Type Class::*, Pointer> {
  // Type of the member pointer this type represents.
  using PointerType = Type Class::*;

  // Resolves a pointer to member with the given instance, yielding a
  // reference to the member in that instance.
  static Type& Resolve(Class& instance) { return (instance.*Pointer); }
  static const Type& Resolve(const Class& instance) {
    return (instance.*Pointer);
  }
};

// Describes a set of members to be serialized/deserialized by this library. The
// parameter pack MemberPointers takes a list of MemberPointer types that
// describe each member to participate in serialization/deserialization.
template <typename T, typename... MemberPointers>
struct SerializableMembersType {
  using Type = T;

  // The number of member pointers described by this type.
  enum : std::size_t { MemberCount = sizeof...(MemberPointers) };

  // The member pointers described by this type.
  using Members = std::tuple<MemberPointers...>;

  // Accessor for individual member pointer types.
  template <std::size_t Index>
  using At = typename std::tuple_element<Index, Members>::type;
};

// Classes must do the following to correctly define a serializable type:
//     1. Define a type called "SerializableMembers" as a template instantiation
//        of SerializableMembersType, describing the members of the class to
//        participate in serialization (presumably all of them). Use the macro
//        PDX_SERIALIZABLE_MEMBERS(...) below to aid the correct type
//        definition. This type should be private to prevent leaking member
//        access information.
//     2. Make SerializableTraits and HasSerilizableMembers types a friend of
//        the class. The macro PDX_SERIALIZABLE_MEMEBRS(...) takes care of
//        this automatically.
//     3. Define a public default constructor, if necessary. Deserialization
//        requires instances to be default-constructible.
//
// Example usage:
//     class MySerializableType : public AnotherBaseType {
//      public:
//       MySerializableType();
//       ...
//      private:
//       int a;
//       string b;
//       PDX_SERIALIZABLE_MEMBERS(MySerializableType, a, b);
//     };
//
// Note that const and static member serialization is not supported.

template <typename T>
class SerializableTraits {
 public:
  // Gets the serialized size of type T.
  static std::size_t GetSerializedSize(const T& value) {
    return GetEncodingSize(EncodeArrayType(SerializableMembers::MemberCount)) +
           GetMembersSize<SerializableMembers>(value);
  }

  // Serializes type T.
  static void SerializeObject(const T& value, MessageWriter* writer,
                              void*& buffer) {
    SerializeArrayEncoding(EncodeArrayType(SerializableMembers::MemberCount),
                           SerializableMembers::MemberCount, buffer);
    SerializeMembers<SerializableMembers>(value, writer, buffer);
  }

  // Deserializes type T.
  static ErrorType DeserializeObject(T* value, MessageReader* reader,
                                     const void*& start, const void* end) {
    EncodingType encoding;
    std::size_t size;

    if (const auto error =
            DeserializeArrayType(&encoding, &size, reader, start, end)) {
      return error;
    } else if (size != SerializableMembers::MemberCount) {
      return ErrorCode::UNEXPECTED_TYPE_SIZE;
    } else {
      return DeserializeMembers<SerializableMembers>(value, reader, start, end);
    }
  }

 private:
  using SerializableMembers = typename T::SerializableMembers;
};

// Utility macro to define a MemberPointer type for a member name.
#define PDX_MEMBER_POINTER(type, member) \
  ::android::pdx::rpc::MemberPointer<decltype(&type::member), &type::member>

// Defines a list of MemberPointer types given a list of member names.
#define PDX_MEMBERS(type, ... /*members*/) \
  PDX_FOR_EACH_BINARY_LIST(PDX_MEMBER_POINTER, type, __VA_ARGS__)

// Defines the serializable members of a type given a list of member names and
// befriends SerializableTraits and HasSerializableMembers for the class. This
// macro handles requirements #1 and #2 above.
#define PDX_SERIALIZABLE_MEMBERS(type, ... /*members*/)                     \
  template <typename T>                                                     \
  friend class ::android::pdx::rpc::SerializableTraits;                     \
  template <typename, typename>                                             \
  friend struct ::android::pdx::rpc::HasSerializableMembers;                \
  using SerializableMembers = ::android::pdx::rpc::SerializableMembersType< \
      type, PDX_MEMBERS(type, __VA_ARGS__)>

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_SERIALIZABLE_H_
