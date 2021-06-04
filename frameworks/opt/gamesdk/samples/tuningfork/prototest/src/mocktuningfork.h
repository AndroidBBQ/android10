#include "proto/tuningfork.pb.h"

#include <functional>

namespace mocktuningfork {

using ::com::google::tuningfork::FidelityParams;

void init(const ::com::google::tuningfork::Settings& settings,
          const std::function<void(const FidelityParams&)>& callback);
void set(const ::com::google::tuningfork::Annotation& a);

#define SYS_CPU 0
#define SYS_GPU 1
void tick(int instrumentKey);

} // namespace mocktuningfork
