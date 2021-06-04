#include "private/dvr/trusted_uids.h"

#include <mutex>
#include <unordered_map>

#include <binder/IPermissionController.h>
#include <binder/IServiceManager.h>
#include <private/android_filesystem_config.h>
#include <utils/String16.h>
#include <vr/vr_manager/vr_manager.h>

namespace android {
namespace dvr {

bool IsTrustedUid(uid_t uid, bool use_cache) {
  static std::unordered_map<uid_t, bool> uid_cache;
  static std::mutex uid_cache_mutex;

  // Whitelist requests from the system UID.
  // These are already whitelisted by the permission service, but it might not
  // be available if the ActivityManagerService is up during boot.
  // This ensures the correct result for system services while booting up.
  if (uid == AID_SYSTEM)
    return true;

  std::lock_guard<std::mutex> lock(uid_cache_mutex);

  if (use_cache) {
    auto it = uid_cache.find(uid);
    if (it != uid_cache.end())
      return it->second;
  }

  sp<IBinder> binder = defaultServiceManager()->getService(String16("permission"));
  if (binder == 0) {
    ALOGW("Could not access permission service");
    return false;
  }

  // Note: we ignore the pid because it's only used to automatically reply
  // true if the caller is the Activity Manager Service.
  bool trusted = interface_cast<IPermissionController>(binder)->checkPermission(
      String16("android.permission.RESTRICTED_VR_ACCESS"), -1, uid);

  // Cache the information for this uid to avoid future Java calls.
  uid_cache[uid] = trusted;
  return trusted;
}

}  // namespace dvr
}  // namespace android
