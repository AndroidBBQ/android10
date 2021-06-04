#ifndef ANDROID_DVR_TRUSTED_UIDS_H_
#define ANDROID_DVR_TRUSTED_UIDS_H_

#include <sys/types.h>

namespace android {
namespace dvr {

/**
 * Tells if a provided UID can be trusted to access restricted VR APIs.
 *
 * UID trust is based on the android.permission.RESTRICTED_VR_ACCESS permission.
 * AID_SYSTEM and AID_ROOT are automatically trusted by Android.
 *
 * UIDs are guaranteed not to be reused until the next reboot even in case
 * of package reinstall. For performance reasons this method caches results by
 * default, as otherwise every check would trigger a Java call.
 *
 * This function is thread-safe.
 *
 * @param uid The uid to check.
 * @param use_cache If true any cached result for the provided uid will be
 *     reused. If false this call will reach the Application Manager Service
 *     in Java to get updated values. Any updates will be stored in the cache.
 * @return true if the uid is trusted, false if not or if the VR Manager Service
 *         could not be reached to verify the uid.
 */
bool IsTrustedUid(uid_t uid, bool use_cache = true);

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_TRUSTED_UIDS_H_
