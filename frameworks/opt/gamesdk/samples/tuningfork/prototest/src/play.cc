// Mock of the play store functionality that creates a fidelity parameter set
//  for an experiment.
// Assume we don't include the user's proto, but do have access to the file at
//  runtime

#include "dynamicproto.h"

#include <iostream>

using namespace google::protobuf;

void usage() {
  std::cerr << "Usage: play <protofile>" << std::endl;
}

int main(int argc, char *argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  if(argc<2) {
    usage();
    return -1;
  }
  dynamicproto::init({".", "proto"});
  auto descs = dynamicproto::fileDescriptors(argv[1]);

  if(descs.size()<2) {
    std::cerr << "The proto file has errors" << std::endl;
    return -2;
  }

  std::vector<const FieldDescriptor*> exts;
  const std::string FIDELITYPARAMS = "com.google.tuningfork.FidelityParams";
  dynamicproto::extensionsOf(descs.back(), FIDELITYPARAMS, exts);
  std::cerr << "Found " << exts.size() << " extensions of " << FIDELITYPARAMS
            << " in " << descs.back()->name() << " and its dependencies"
            << std::endl;

  auto m = dynamicproto::newMessage(descs.front(), "FidelityParams");
  if(m) {
    const Reflection* r = m->GetReflection();
    if(r) {
      std::cerr << "Enter values for FidelityParams:\n";
      for(auto e: exts) {
        std::cerr << "Name: " << e->name() << " type: ";
        switch(e->type()) {
          case FieldDescriptor::TYPE_ENUM:
            {
              dynamicproto::print(e->enum_type(), std::cerr);
              std::cerr << "\n";
              int val;
              std::cin >> val;
              r->SetEnumValue(m, e, val);
            }
            break;
          case FieldDescriptor::TYPE_INT32:
            std::cerr << e->type_name() << "\n";
            int val;
            std::cin >> val;
            r->SetInt32(m, e, val);
            break;
          default:
            std::cerr << e->type_name() << " *ERROR* not supported\n";
        }
      }

      m->SerializeToOstream(&std::cout);
    }
  }

  // This would be something similar if we had the user's header at compile time
  // FidelityParams p;
  // p.SetExtension(lod, (::LOD)((rand()%4)+1));
  // p.SerializeToOstream(&std::cout);
}
