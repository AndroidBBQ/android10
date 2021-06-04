#pragma once

#include "proto/eng_tuningfork.pb.h"

namespace gameengine {

using ::com::google::tuningfork::Annotation;
using ::com::google::tuningfork::Settings;
using ::com::google::tuningfork::FidelityParams;

// If the game logic is in C++, initialize like this:
void init(const Settings& s,
          const std::function<void(const FidelityParams&)>& callback );
// If the game logic is not C++, we need to serialize the protobufs:
void init(const std::string& settings,
          const std::function<void(const std::string&)>& callback );

// Set the annotation directly
void set(const Annotation& s);
// Set the annotation using a serialized representation
void set(const std::string& s);

void tick();

} // namespace gameengine
