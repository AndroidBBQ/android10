#include "include/private/dvr/vsync_service.h"

#include <binder/Parcel.h>
#include <log/log.h>

namespace android {
namespace dvr {

status_t BnVsyncCallback::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {
  switch (code) {
    case ON_VSYNC: {
      CHECK_INTERFACE(IVsyncCallback, data, reply);
      int64_t vsync_timestamp = 0;
      status_t result = data.readInt64(&vsync_timestamp);
      if (result != OK) {
        ALOGE("onVsync failed to readInt64: %d", result);
        return result;
      }
      onVsync(vsync_timestamp);
      return OK;
    }
    default: {
      return BBinder::onTransact(code, data, reply, flags);
    }
  }
}

class BpVsyncCallback : public BpInterface<IVsyncCallback> {
public:
  explicit BpVsyncCallback(const sp<IBinder>& impl)
      : BpInterface<IVsyncCallback>(impl) {}
  virtual ~BpVsyncCallback() {}

  virtual status_t onVsync(int64_t vsync_timestamp) {
    Parcel data, reply;
    status_t result = data.writeInterfaceToken(
        IVsyncCallback::getInterfaceDescriptor());
    if (result != OK) {
      ALOGE("onVsync failed to writeInterfaceToken: %d", result);
      return result;
    }
    result = data.writeInt64(vsync_timestamp);
    if (result != OK) {
      ALOGE("onVsync failed to writeInt64: %d", result);
      return result;
    }
    result = remote()->transact(
        BnVsyncCallback::ON_VSYNC, data, &reply, TF_ONE_WAY);
    if (result != OK) {
      ALOGE("onVsync failed to transact: %d", result);
      return result;
    }
    return result;
  }
};

IMPLEMENT_META_INTERFACE(VsyncCallback, "android.dvr.IVsyncCallback");


status_t BnVsyncService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {
  switch (code) {
    case REGISTER_CALLBACK: {
      CHECK_INTERFACE(IVsyncService, data, reply);
      sp<IBinder> callback;
      status_t result = data.readStrongBinder(&callback);
      if (result != OK) {
        ALOGE("registerCallback failed to readStrongBinder: %d", result);
        return result;
      }
      registerCallback(interface_cast<IVsyncCallback>(callback));
      return OK;
    }
    case UNREGISTER_CALLBACK: {
      CHECK_INTERFACE(IVsyncService, data, reply);
      sp<IBinder> callback;
      status_t result = data.readStrongBinder(&callback);
      if (result != OK) {
        ALOGE("unregisterCallback failed to readStrongBinder: %d", result);
        return result;
      }
      unregisterCallback(interface_cast<IVsyncCallback>(callback));
      return OK;
    }
    default: {
      return BBinder::onTransact(code, data, reply, flags);
    }
  }
}

class BpVsyncService : public BpInterface<IVsyncService> {
public:
  explicit BpVsyncService(const sp<IBinder>& impl)
      : BpInterface<IVsyncService>(impl) {}
  virtual ~BpVsyncService() {}

  virtual status_t registerCallback(const sp<IVsyncCallback> callback) {
    Parcel data, reply;
    status_t result = data.writeInterfaceToken(
        IVsyncService::getInterfaceDescriptor());
    if (result != OK) {
      ALOGE("registerCallback failed to writeInterfaceToken: %d", result);
      return result;
    }
    result = data.writeStrongBinder(IInterface::asBinder(callback));
    if (result != OK) {
      ALOGE("registerCallback failed to writeStrongBinder: %d", result);
      return result;
    }
    result = remote()->transact(
        BnVsyncService::REGISTER_CALLBACK, data, &reply);
    if (result != OK) {
      ALOGE("registerCallback failed to transact: %d", result);
      return result;
    }
    return result;
  }

  virtual status_t unregisterCallback(const sp<IVsyncCallback> callback) {
    Parcel data, reply;
    status_t result = data.writeInterfaceToken(
        IVsyncService::getInterfaceDescriptor());
    if (result != OK) {
      ALOGE("unregisterCallback failed to writeInterfaceToken: %d", result);
      return result;
    }
    result = data.writeStrongBinder(IInterface::asBinder(callback));
    if (result != OK) {
      ALOGE("unregisterCallback failed to writeStrongBinder: %d", result);
      return result;
    }
    result = remote()->transact(
        BnVsyncService::UNREGISTER_CALLBACK, data, &reply);
    if (result != OK) {
      ALOGE("unregisterCallback failed to transact: %d", result);
      return result;
    }
    return result;
  }
};

IMPLEMENT_META_INTERFACE(VsyncService, "android.dvr.IVsyncService");

}  // namespace dvr
}  // namespace android
