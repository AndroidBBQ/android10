// We don't know about the user's annotations, only the base annotations
#include "mocktuningfork.h"
#include "gameengine.h"
#include <iostream>

namespace gameengine {

namespace {
Annotation current_annotation;
}

using namespace com::google::tuningfork;

void init(const Settings& s,
          const std::function<void(const FidelityParams&)>& callback ) {
  mocktuningfork::init(s,callback);
  current_annotation.SetExtension(loading_state, LoadingState::LOADING);
  current_annotation.SetExtension(level, 0);
}
void init(const std::string& settings,
          const std::function<void(const std::string&)>& callback ) {
  Settings s;
  s.ParseFromString(settings);
  mocktuningfork::init(s, [&](const FidelityParams& p) {
                            std::string fp_ser;
                            p.SerializeToString(&fp_ser);
                            callback(fp_ser); }
    );
  current_annotation.SetExtension(loading_state, LoadingState::LOADING);
  current_annotation.SetExtension(level, 0);
}
void set(const std::string& s) {
  Annotation t;
  t.ParseFromString(s);
  current_annotation.MergeFrom(t);
}
void set(const Annotation& a) {
  current_annotation.MergeFrom(a);
}
void tick() {
  // Some logic for updating the annotation - it usually wouldn't happen every
  //  tick
  auto l = current_annotation.GetExtension(level);
  current_annotation.SetExtension(level, l+1);
  if(l>0)
    current_annotation.SetExtension(loading_state, LoadingState::NOT_LOADING);
  mocktuningfork::set(current_annotation);
  mocktuningfork::tick(SYS_CPU);
}

} // namespace gamengine
