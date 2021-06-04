#ifndef ANDROID_DVR_VSYNC_SERVICE_H_
#define ANDROID_DVR_VSYNC_SERVICE_H_

#include <binder/IInterface.h>

namespace android {
namespace dvr {

class IVsyncCallback : public IInterface {
 public:
  DECLARE_META_INTERFACE(VsyncCallback)

  enum {
    ON_VSYNC = IBinder::FIRST_CALL_TRANSACTION
  };

  virtual status_t onVsync(int64_t vsync_timestamp) = 0;
};

class BnVsyncCallback : public BnInterface<IVsyncCallback> {
 public:
  virtual status_t onTransact(uint32_t code, const Parcel& data,
                              Parcel* reply, uint32_t flags = 0);
};

// Register a callback with IVsyncService to be notified of vsync events and
// timestamps. There's also a shared memory vsync buffer defined in
// dvr_shared_buffers.h. IVsyncService has advantages over the vsync shared
// memory buffer that make it preferable in certain situations:
//
// 1. The shared memory buffer lifetime is controlled by VrCore. IVsyncService
// is always available as long as surface flinger is running.
//
// 2. IVsyncService will make a binder callback when a vsync event occurs. This
// allows the client to not write code to implement periodic "get the latest
// vsync" calls, which is necessary with the vsync shared memory buffer.
//
// 3. The IVsyncService provides the real vsync timestamp reported by hardware
// composer, whereas the vsync shared memory buffer only has predicted vsync
// times.
class IVsyncService : public IInterface {
public:
  DECLARE_META_INTERFACE(VsyncService)

  static const char* GetServiceName() { return "vrflinger_vsync"; }

  enum {
    REGISTER_CALLBACK = IBinder::FIRST_CALL_TRANSACTION,
    UNREGISTER_CALLBACK
  };

  virtual status_t registerCallback(const sp<IVsyncCallback> callback) = 0;
  virtual status_t unregisterCallback(const sp<IVsyncCallback> callback) = 0;
};

class BnVsyncService : public BnInterface<IVsyncService> {
 public:
  virtual status_t onTransact(uint32_t code, const Parcel& data,
                              Parcel* reply, uint32_t flags = 0);
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_VSYNC_SERVICE_H_
