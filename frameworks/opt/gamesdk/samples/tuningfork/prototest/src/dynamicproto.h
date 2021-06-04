#pragma once

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

// Utility functions for dealing with protos loaded from file rather than
//   compiled with protoc
namespace dynamicproto {

using namespace google::protobuf;

// init must be called before the other functions, telling the protobuf
//   factory where to look for imports.
void init(const std::vector<std::string>& include_dirs);

// Return the file descriptor for file_name and all its imports. That for
// file_name is last in the vector.
std::vector<const FileDescriptor*> fileDescriptors(const std::string& file_name);

// Find extensions of fully_qualified_name in desc and add them to exts
void extensionsOf(const FileDescriptor* desc,
                  const std::string& fully_qualified_name,
                  std::vector<const FieldDescriptor*>& exts);

// Get all the enums in desc
std::vector<const EnumDescriptor*> enums(const FileDescriptor* desc);

// Print the name and all possible values of e to o.
void print(const EnumDescriptor* e, std::ostream& o);

// Create a new message defined in desc or one of its imports. message_type
//   is *not* qualified.
Message* newMessage(const FileDescriptor* desc, const std::string& message_type);

} // namespace dynamicproto
