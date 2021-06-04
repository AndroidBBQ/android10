#include "mocktuningfork.h"

#include <iostream>
#include <fstream>

namespace mocktuningfork {

using ::com::google::tuningfork::Annotation;
using ::com::google::tuningfork::Settings;
using ::com::google::tuningfork::FidelityParams;

Annotation current_annotation;
FidelityParams latest_params;

const std::string fparams_fname = "fidelityparams.pbin";

void init(const Settings& /*settings*/,
          const std::function<void(const FidelityParams&)>& callback) {
  std::ifstream fin(fparams_fname);
  if(fin.good()) {
    std::cout << "Tuning fork got params from Play ( " << fparams_fname
              << " )" << std::endl;
    latest_params.ParseFromIstream(&fin);
    callback(latest_params);
  }
  else {
    std::cerr << "Tuning fork couldn't get parameters from play."
              << " Run ./play first." << std::endl;
  }
}

void set(const Annotation& a) {
  current_annotation = a;
}

void tick(int ikey) {
  std::cout << "TF tick {ikey: " << ikey << ", ann: ";
  std::cout << current_annotation.DebugString();
  std::cout << "}" << std::endl;
}

} // namespace mocktuningfork
