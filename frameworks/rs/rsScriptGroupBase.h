#ifndef ANDROID_RS_SCRIPT_GROUP_BASE_H
#define ANDROID_RS_SCRIPT_GROUP_BASE_H

#include "rsObjectBase.h"

namespace android {
namespace renderscript {

class ScriptGroupBase : public ObjectBase {
 public:
  explicit ScriptGroupBase(Context* rsc) : ObjectBase(rsc) {}
  virtual ~ScriptGroupBase() {}

  virtual void serialize(Context *rsc, OStream *stream) const {}

  virtual RsA3DClassID getClassId() const {
    return RS_A3D_CLASS_ID_SCRIPT_GROUP;
  }

  enum SG_API_Version {
    SG_V1 = 10,
    SG_V2 = 20,
  };

  virtual void execute(Context *rsc) = 0;
  virtual SG_API_Version getApiVersion() const = 0;

  struct Hal {
    void * drv;

    struct DriverInfo {
    };
    DriverInfo info;
  };
  Hal mHal;
};

}  // namespace renderscript
}  // namespace android

#endif  // ANDROID_RS_SCRIPT_GROUP_BASE_H
