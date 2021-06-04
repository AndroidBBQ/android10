#include "proto/tuningfork_extensions.pb.h"
#include <iostream>
#include "mocktuningfork.h"

using ::com::google::tuningfork::Annotation;
using ::com::google::tuningfork::Settings;
using ::com::google::tuningfork::FidelityParams;

using ::google::protobuf::internal::EnumTypeTraits;

FidelityParams fparams;

void fidelityParamsCallback(const FidelityParams& p) {
  fparams = p;
  std::cout << "Game got fidelity parameters from TF: "
            << fparams.DebugString() << std::endl;
}

void runWithNoGameEngine() {
  Settings settings;
  mocktuningfork::init(settings, fidelityParamsCallback);
  Annotation ann;
  ann.SetExtension(boss_alive, true);
  for(size_t i=1; i<5; ++i) {
    ann.SetExtension(car, (EnumTypeTraits<Car, Car_IsValid>::ConstType)(i));
    mocktuningfork::set(ann);
    mocktuningfork::tick(SYS_CPU);
  }
}

int main(int /*argc*/, char * /*argv*/[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  std::cout << "** Without Game engine ** \n";
  runWithNoGameEngine();

  return 0;
}
