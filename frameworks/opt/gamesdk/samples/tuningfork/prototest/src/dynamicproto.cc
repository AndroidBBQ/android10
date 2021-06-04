
#include "dynamicproto.h"

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>

#include <iostream>
#include <memory>
#include <exception>

using namespace google::protobuf::compiler;
using namespace google::protobuf;

namespace {

class ErrorCollector : public MultiFileErrorCollector {
 public:
  void AddError(const std::string& file_name, int line, int column,
                        const std::string& message) override {
    std::cerr << "Error in: " << file_name << " line: " << line << " column: "
              << column << " message: " << message << std::endl;
  }
};

DiskSourceTree source_tree;
ErrorCollector error_collector;
std::unique_ptr<Importer> importer;
DynamicMessageFactory factory; // This needs to be kept around

} // namespace

namespace dynamicproto {

void warnNotInitialized() {
  std::cerr << "Warning: dynamicproto has not been initalized" << std::endl;
}

void init(const std::vector<std::string>& include_paths) {
  for (auto& path: include_paths) {
    source_tree.MapPath("", path);
  }
  importer = std::unique_ptr<Importer>(new Importer(&source_tree,
                                                    &error_collector));
}

void fileDescriptorDependencies(const FileDescriptor* fdesc,
                                std::vector<const FileDescriptor*>& result) {
  if(fdesc) {
    size_t n = fdesc->dependency_count();
    for (size_t i=0; i<n; ++i) {
      fileDescriptorDependencies(fdesc->dependency(i), result);
    }
    result.push_back(fdesc);
  }
}

// Get a FileDescriptor for the file and all its dependencies,
//  the latter of which appear at the beginning of the vector.
std::vector<const FileDescriptor*>
fileDescriptors(const std::string& file_name) {
  std::vector<const FileDescriptor*> result;
  if(importer) {
    const FileDescriptor* fdesc = importer->Import(file_name);
    fileDescriptorDependencies(fdesc, result);
  }
  else {
    warnNotInitialized();
  }
  return result;
}

// Find extensions of full_name in desc and add them to result.
void extensionsOf(const FileDescriptor* desc, const std::string& full_name,
                  std::vector<const FieldDescriptor*>& result) {
  // TODO: look in all messages that may contain inner extensions
  if(desc) {
      int n = desc->extension_count();
      for (int i=0;i<n;++i) {
        auto e = desc->extension(i);
        if(e->containing_type()->full_name()==full_name)
          result.push_back(e);
      }
      n = desc->dependency_count();
      for (int i=0;i<n;++i) {
        extensionsOf(desc->dependency(i), full_name, result);
      }
  }
}

std::vector<const EnumDescriptor*> enums(const FileDescriptor* extDesc) {
  std::vector<const EnumDescriptor*> result;
  if(extDesc) {
      int n = extDesc->enum_type_count();
      for (int i=0;i<n;++i) {
        auto e = extDesc->enum_type(i);
        result.push_back(e);
      }
  }
  return result;
}

void print(const EnumDescriptor* e, std::ostream& o) {
  o << "enum " << e->name() << " { ";
  int n = e->value_count();
  bool first = true;
  for (int i=0; i<n; ++i) {
    if(first)
      first = false;
    else
      o << ", ";
    o << e->value(i)->name() << "=" << (i+1);
  }
  o << "}";
}

Message* newMessage(const FileDescriptor* fdesc, const std::string& name) {
  auto desc = fdesc->FindMessageTypeByName(name);
  if(desc) {
    auto p = factory.GetPrototype(desc);
    if(p)
      return p->New();
  }
  size_t n = fdesc->dependency_count();
  for (size_t i=0; i<n; ++i) {
    auto m = newMessage(fdesc->dependency(i), name);
    if(m) return m;
  }
  return nullptr;
}

} // namespace dynamicproto
