/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Proxy for media player implementations

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaAnalyticsService"
#include <utils/Log.h>

#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>
#include <pwd.h>

#include <cutils/atomic.h>
#include <cutils/properties.h> // for property_get

#include <utils/misc.h>

#include <android/content/pm/IPackageManagerNative.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/MemoryHeapBase.h>
#include <binder/MemoryBase.h>
#include <gui/Surface.h>
#include <utils/Errors.h>  // for status_t
#include <utils/List.h>
#include <utils/String8.h>
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <utils/Vector.h>

#include <media/IMediaHTTPService.h>
#include <media/IRemoteDisplay.h>
#include <media/IRemoteDisplayClient.h>
#include <media/MediaPlayerInterface.h>
#include <media/mediarecorder.h>
#include <media/MediaMetadataRetrieverInterface.h>
#include <media/Metadata.h>
#include <media/AudioTrack.h>
#include <media/MemoryLeakTrackUtil.h>
#include <media/stagefright/MediaCodecList.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/Utils.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooperRoster.h>
#include <mediautils/BatteryNotifier.h>

//#include <memunreachable/memunreachable.h>
#include <system/audio.h>

#include <private/android_filesystem_config.h>

#include "MediaAnalyticsService.h"

namespace android {

// individual records kept in memory: age or count
// age: <= 28 hours (1 1/6 days)
// count: hard limit of # records
// (0 for either of these disables that threshold)
//
static constexpr nsecs_t kMaxRecordAgeNs =  28 * 3600 * (1000*1000*1000ll);
// 2019/6: average daily per device is currently 375-ish;
// setting this to 2000 is large enough to catch most devices
// we'll lose some data on very very media-active devices, but only for
// the gms collection; statsd will have already covered those for us.
// This also retains enough information to help with bugreports
static constexpr int kMaxRecords    = 2000;

// max we expire in a single call, to constrain how long we hold the
// mutex, which also constrains how long a client might wait.
static constexpr int kMaxExpiredAtOnce = 50;

// TODO: need to look at tuning kMaxRecords and friends for low-memory devices

static const char *kServiceName = "media.metrics";

void MediaAnalyticsService::instantiate() {
    defaultServiceManager()->addService(
            String16(kServiceName), new MediaAnalyticsService());
}

MediaAnalyticsService::MediaAnalyticsService()
        : mMaxRecords(kMaxRecords),
          mMaxRecordAgeNs(kMaxRecordAgeNs),
          mMaxRecordsExpiredAtOnce(kMaxExpiredAtOnce),
          mDumpProto(MediaAnalyticsItem::PROTO_V1),
          mDumpProtoDefault(MediaAnalyticsItem::PROTO_V1) {

    ALOGD("MediaAnalyticsService created");

    mItemsSubmitted = 0;
    mItemsFinalized = 0;
    mItemsDiscarded = 0;
    mItemsDiscardedExpire = 0;
    mItemsDiscardedCount = 0;

    mLastSessionID = 0;
    // recover any persistency we set up
    // etc
}

MediaAnalyticsService::~MediaAnalyticsService() {
        ALOGD("MediaAnalyticsService destroyed");

    while (mItems.size() > 0) {
        MediaAnalyticsItem * oitem = *(mItems.begin());
        mItems.erase(mItems.begin());
        delete oitem;
        mItemsDiscarded++;
        mItemsDiscardedCount++;
    }
}


MediaAnalyticsItem::SessionID_t MediaAnalyticsService::generateUniqueSessionID() {
    // generate a new sessionid

    Mutex::Autolock _l(mLock_ids);
    return (++mLastSessionID);
}

// caller surrenders ownership of 'item'
MediaAnalyticsItem::SessionID_t MediaAnalyticsService::submit(MediaAnalyticsItem *item, bool forcenew)
{
    UNUSED(forcenew);

    // fill in a sessionID if we do not yet have one
    if (item->getSessionID() <= MediaAnalyticsItem::SessionIDNone) {
        item->setSessionID(generateUniqueSessionID());
    }

    // we control these, generally not trusting user input
    nsecs_t now = systemTime(SYSTEM_TIME_REALTIME);
    // round nsecs to seconds
    now = ((now + 500000000) / 1000000000) * 1000000000;
    item->setTimestamp(now);
    int pid = IPCThreadState::self()->getCallingPid();
    int uid = IPCThreadState::self()->getCallingUid();

    int uid_given = item->getUid();
    int pid_given = item->getPid();

    // although we do make exceptions for some trusted client uids
    bool isTrusted = false;

    ALOGV("caller has uid=%d, embedded uid=%d", uid, uid_given);

    switch (uid)  {
        case AID_MEDIA:
        case AID_MEDIA_CODEC:
        case AID_MEDIA_EX:
        case AID_MEDIA_DRM:
            // trusted source, only override default values
            isTrusted = true;
            if (uid_given == (-1)) {
                item->setUid(uid);
            }
            if (pid_given == (-1)) {
                item->setPid(pid);
            }
            break;
        default:
            isTrusted = false;
            item->setPid(pid);
            item->setUid(uid);
            break;
    }

    // Overwrite package name and version if the caller was untrusted.
    if (!isTrusted) {
      setPkgInfo(item, item->getUid(), true, true);
    } else if (item->getPkgName().empty()) {
      // empty, so fill out both parts
      setPkgInfo(item, item->getUid(), true, true);
    } else {
      // trusted, provided a package, do nothing
    }

    ALOGV("given uid %d; sanitized uid: %d sanitized pkg: %s "
          "sanitized pkg version: %"  PRId64,
          uid_given, item->getUid(),
          item->getPkgName().c_str(),
          item->getPkgVersionCode());

    mItemsSubmitted++;

    // validate the record; we discard if we don't like it
    if (contentValid(item, isTrusted) == false) {
        delete item;
        return MediaAnalyticsItem::SessionIDInvalid;
    }

    // XXX: if we have a sessionid in the new record, look to make
    // sure it doesn't appear in the finalized list.

    if (item->count() == 0) {
        ALOGV("dropping empty record...");
        delete item;
        item = NULL;
        return MediaAnalyticsItem::SessionIDInvalid;
    }

    // save the new record
    //
    // send a copy to statsd
    dump2Statsd(item);

    // and keep our copy for dumpsys
    MediaAnalyticsItem::SessionID_t id = item->getSessionID();
    saveItem(item);
    mItemsFinalized++;

    return id;
}


status_t MediaAnalyticsService::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 512;
    char buffer[SIZE];
    String8 result;

    if (checkCallingPermission(String16("android.permission.DUMP")) == false) {
        snprintf(buffer, SIZE, "Permission Denial: "
                "can't dump MediaAnalyticsService from pid=%d, uid=%d\n",
                IPCThreadState::self()->getCallingPid(),
                IPCThreadState::self()->getCallingUid());
        result.append(buffer);
        write(fd, result.string(), result.size());
        return NO_ERROR;
    }

    // crack any parameters
    String16 protoOption("-proto");
    int chosenProto = mDumpProtoDefault;
    String16 clearOption("-clear");
    bool clear = false;
    String16 sinceOption("-since");
    nsecs_t ts_since = 0;
    String16 helpOption("-help");
    String16 onlyOption("-only");
    std::string only;
    int n = args.size();

    for (int i = 0; i < n; i++) {
        String8 myarg(args[i]);
        if (args[i] == clearOption) {
            clear = true;
        } else if (args[i] == protoOption) {
            i++;
            if (i < n) {
                String8 value(args[i]);
                int proto = MediaAnalyticsItem::PROTO_V0;
                char *endp;
                const char *p = value.string();
                proto = strtol(p, &endp, 10);
                if (endp != p || *endp == '\0') {
                    if (proto < MediaAnalyticsItem::PROTO_FIRST) {
                        proto = MediaAnalyticsItem::PROTO_FIRST;
                    } else if (proto > MediaAnalyticsItem::PROTO_LAST) {
                        proto = MediaAnalyticsItem::PROTO_LAST;
                    }
                    chosenProto = proto;
                } else {
                    result.append("unable to parse value for -proto\n\n");
                }
            } else {
                result.append("missing value for -proto\n\n");
            }
        } else if (args[i] == sinceOption) {
            i++;
            if (i < n) {
                String8 value(args[i]);
                char *endp;
                const char *p = value.string();
                ts_since = strtoll(p, &endp, 10);
                if (endp == p || *endp != '\0') {
                    ts_since = 0;
                }
            } else {
                ts_since = 0;
            }
            // command line is milliseconds; internal units are nano-seconds
            ts_since *= 1000*1000;
        } else if (args[i] == onlyOption) {
            i++;
            if (i < n) {
                String8 value(args[i]);
                only = value.string();
            }
        } else if (args[i] == helpOption) {
            result.append("Recognized parameters:\n");
            result.append("-help        this help message\n");
            result.append("-proto #     dump using protocol #");
            result.append("-clear       clears out saved records\n");
            result.append("-only X      process records for component X\n");
            result.append("-since X     include records since X\n");
            result.append("             (X is milliseconds since the UNIX epoch)\n");
            write(fd, result.string(), result.size());
            return NO_ERROR;
        }
    }

    Mutex::Autolock _l(mLock);
    // mutex between insertion and dumping the contents

    mDumpProto = chosenProto;

    // we ALWAYS dump this piece
    snprintf(buffer, SIZE, "Dump of the %s process:\n", kServiceName);
    result.append(buffer);

    dumpHeaders(result, ts_since);

    dumpRecent(result, ts_since, only.c_str());


    if (clear) {
        // remove everything from the finalized queue
        while (mItems.size() > 0) {
            MediaAnalyticsItem * oitem = *(mItems.begin());
            mItems.erase(mItems.begin());
            delete oitem;
            mItemsDiscarded++;
        }

        // shall we clear the summary data too?

    }

    write(fd, result.string(), result.size());
    return NO_ERROR;
}

// dump headers
void MediaAnalyticsService::dumpHeaders(String8 &result, nsecs_t ts_since)
{
    const size_t SIZE = 512;
    char buffer[SIZE];

    snprintf(buffer, SIZE, "Protocol Version: %d\n", mDumpProto);
    result.append(buffer);

    int enabled = MediaAnalyticsItem::isEnabled();
    if (enabled) {
        snprintf(buffer, SIZE, "Metrics gathering: enabled\n");
    } else {
        snprintf(buffer, SIZE, "Metrics gathering: DISABLED via property\n");
    }
    result.append(buffer);

    snprintf(buffer, SIZE,
        "Since Boot: Submissions: %8" PRId64
            " Accepted: %8" PRId64 "\n",
        mItemsSubmitted, mItemsFinalized);
    result.append(buffer);
    snprintf(buffer, SIZE,
        "Records Discarded: %8" PRId64
            " (by Count: %" PRId64 " by Expiration: %" PRId64 ")\n",
         mItemsDiscarded, mItemsDiscardedCount, mItemsDiscardedExpire);
    result.append(buffer);
    if (ts_since != 0) {
        snprintf(buffer, SIZE,
            "Emitting Queue entries more recent than: %" PRId64 "\n",
            (int64_t) ts_since);
        result.append(buffer);
    }
}

// the recent, detailed queues
void MediaAnalyticsService::dumpRecent(String8 &result, nsecs_t ts_since, const char * only)
{
    const size_t SIZE = 512;
    char buffer[SIZE];

    if (only != NULL && *only == '\0') {
        only = NULL;
    }

    // show the recently recorded records
    snprintf(buffer, sizeof(buffer), "\nFinalized Metrics (oldest first):\n");
    result.append(buffer);
    result.append(this->dumpQueue(ts_since, only));

    // show who is connected and injecting records?
    // talk about # records fed to the 'readers'
    // talk about # records we discarded, perhaps "discarded w/o reading" too
}

// caller has locked mLock...
String8 MediaAnalyticsService::dumpQueue() {
    return dumpQueue((nsecs_t) 0, NULL);
}

String8 MediaAnalyticsService::dumpQueue(nsecs_t ts_since, const char * only) {
    String8 result;
    int slot = 0;

    if (mItems.empty()) {
            result.append("empty\n");
    } else {
        List<MediaAnalyticsItem *>::iterator it = mItems.begin();
        for (; it != mItems.end(); it++) {
            nsecs_t when = (*it)->getTimestamp();
            if (when < ts_since) {
                continue;
            }
            if (only != NULL &&
                strcmp(only, (*it)->getKey().c_str()) != 0) {
                ALOGV("Omit '%s', it's not '%s'", (*it)->getKey().c_str(), only);
                continue;
            }
            std::string entry = (*it)->toString(mDumpProto);
            result.appendFormat("%5d: %s\n", slot, entry.c_str());
            slot++;
        }
    }

    return result;
}

//
// Our Cheap in-core, non-persistent records management.


// we hold mLock when we get here
// if item != NULL, it's the item we just inserted
// true == more items eligible to be recovered
bool MediaAnalyticsService::expirations_l(MediaAnalyticsItem *item)
{
    bool more = false;
    int handled = 0;

    // keep removing old records the front until we're in-bounds (count)
    // since we invoke this with each insertion, it should be 0/1 iterations.
    if (mMaxRecords > 0) {
        while (mItems.size() > (size_t) mMaxRecords) {
            MediaAnalyticsItem * oitem = *(mItems.begin());
            if (oitem == item) {
                break;
            }
            if (handled >= mMaxRecordsExpiredAtOnce) {
                // unlikely in this loop
                more = true;
                break;
            }
            handled++;
            mItems.erase(mItems.begin());
            delete oitem;
            mItemsDiscarded++;
            mItemsDiscardedCount++;
        }
    }

    // keep removing old records the front until we're in-bounds (age)
    // limited to mMaxRecordsExpiredAtOnce items per invocation.
    if (mMaxRecordAgeNs > 0) {
        nsecs_t now = systemTime(SYSTEM_TIME_REALTIME);
        while (mItems.size() > 0) {
            MediaAnalyticsItem * oitem = *(mItems.begin());
            nsecs_t when = oitem->getTimestamp();
            if (oitem == item) {
                break;
            }
            // careful about timejumps too
            if ((now > when) && (now-when) <= mMaxRecordAgeNs) {
                // this (and the rest) are recent enough to keep
                break;
            }
            if (handled >= mMaxRecordsExpiredAtOnce) {
                // this represents "one too many"; tell caller there are
                // more to be reclaimed.
                more = true;
                break;
            }
            handled++;
            mItems.erase(mItems.begin());
            delete oitem;
            mItemsDiscarded++;
            mItemsDiscardedExpire++;
        }
    }

    // we only indicate whether there's more to clean;
    // caller chooses whether to schedule further cleanup.
    return more;
}

// process expirations in bite sized chunks, allowing new insertions through
// runs in a pthread specifically started for this (which then exits)
bool MediaAnalyticsService::processExpirations()
{
    bool more;
    do {
        sleep(1);
        {
            Mutex::Autolock _l(mLock);
            more = expirations_l(NULL);
            if (!more) {
                break;
            }
        }
    } while (more);
    return true;        // value is for std::future thread synchronization
}

// insert appropriately into queue
void MediaAnalyticsService::saveItem(MediaAnalyticsItem * item)
{

    Mutex::Autolock _l(mLock);
    // mutex between insertion and dumping the contents

    // we want to dump 'in FIFO order', so insert at the end
    mItems.push_back(item);

    // clean old stuff from the queue
    bool more = expirations_l(item);

    // consider scheduling some asynchronous cleaning, if not running
    if (more) {
        if (!mExpireFuture.valid()
            || mExpireFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {

            mExpireFuture = std::async(std::launch::async, [this]()
                                       {return this->processExpirations();});
        }
    }
}

static std::string allowedKeys[] =
{
    "audiopolicy",
    "audiorecord",
    "audiothread",
    "audiotrack",
    "codec",
    "extractor",
    "nuplayer",
};

static const int nAllowedKeys = sizeof(allowedKeys) / sizeof(allowedKeys[0]);

// are the contents good
bool MediaAnalyticsService::contentValid(MediaAnalyticsItem *item, bool isTrusted) {

    // untrusted uids can only send us a limited set of keys
    if (isTrusted == false) {
        // restrict to a specific set of keys
        std::string key = item->getKey();

        size_t i;
        for(i = 0; i < nAllowedKeys; i++) {
            if (key == allowedKeys[i]) {
                break;
            }
        }
        if (i == nAllowedKeys) {
            ALOGD("Ignoring (key): %s", item->toString().c_str());
            return false;
        }
    }

    // internal consistency

    return true;
}

// are we rate limited, normally false
bool MediaAnalyticsService::rateLimited(MediaAnalyticsItem *) {

    return false;
}

// how long we hold package info before we re-fetch it
#define PKG_EXPIRATION_NS (30*60*1000000000ll)   // 30 minutes, in nsecs

// give me the package name, perhaps going to find it
// manages its own mutex operations internally
void MediaAnalyticsService::setPkgInfo(MediaAnalyticsItem *item, uid_t uid, bool setName, bool setVersion)
{
    ALOGV("asking for packagename to go with uid=%d", uid);

    if (!setName && !setVersion) {
        // setting nothing? strange
        return;
    }

    nsecs_t now = systemTime(SYSTEM_TIME_REALTIME);
    struct UidToPkgMap mapping;
    mapping.uid = (uid_t)(-1);

    {
        Mutex::Autolock _l(mLock_mappings);
        int i = mPkgMappings.indexOfKey(uid);
        if (i >= 0) {
            mapping = mPkgMappings.valueAt(i);
            ALOGV("Expiration? uid %d expiration %" PRId64 " now %" PRId64,
                  uid, mapping.expiration, now);
            if (mapping.expiration <= now) {
                // purge the stale entry and fall into re-fetching
                ALOGV("entry for uid %d expired, now= %" PRId64 "", uid, now);
                mPkgMappings.removeItemsAt(i);
                mapping.uid = (uid_t)(-1);
            }
        }
    }

    // if we did not find it
    if (mapping.uid == (uid_t)(-1)) {
        std::string pkg;
        std::string installer = "";
        int64_t versionCode = 0;

        struct passwd *pw = getpwuid(uid);
        if (pw) {
            pkg = pw->pw_name;
        }

        // find the proper value

        sp<IBinder> binder = NULL;
        sp<IServiceManager> sm = defaultServiceManager();
        if (sm == NULL) {
            ALOGE("defaultServiceManager failed");
        } else {
            binder = sm->getService(String16("package_native"));
            if (binder == NULL) {
                ALOGE("getService package_native failed");
            }
        }

        if (binder != NULL) {
            sp<content::pm::IPackageManagerNative> package_mgr =
                            interface_cast<content::pm::IPackageManagerNative>(binder);
            binder::Status status;

            std::vector<int> uids;
            std::vector<std::string> names;

            uids.push_back(uid);

            status = package_mgr->getNamesForUids(uids, &names);
            if (!status.isOk()) {
                ALOGE("package_native::getNamesForUids failed: %s",
                      status.exceptionMessage().c_str());
            } else {
                if (!names[0].empty()) {
                    pkg = names[0].c_str();
                }
            }

            // strip any leading "shared:" strings that came back
            if (pkg.compare(0, 7, "shared:") == 0) {
                pkg.erase(0, 7);
            }

            // determine how pkg was installed and the versionCode
            //
            if (pkg.empty()) {
                // no name for us to manage
            } else if (strchr(pkg.c_str(), '.') == NULL) {
                // not of form 'com.whatever...'; assume internal and ok
            } else if (strncmp(pkg.c_str(), "android.", 8) == 0) {
                // android.* packages are assumed fine
            } else {
                String16 pkgName16(pkg.c_str());
                status = package_mgr->getInstallerForPackage(pkgName16, &installer);
                if (!status.isOk()) {
                    ALOGE("package_native::getInstallerForPackage failed: %s",
                          status.exceptionMessage().c_str());
                }

                // skip if we didn't get an installer
                if (status.isOk()) {
                    status = package_mgr->getVersionCodeForPackage(pkgName16, &versionCode);
                    if (!status.isOk()) {
                        ALOGE("package_native::getVersionCodeForPackage failed: %s",
                          status.exceptionMessage().c_str());
                    }
                }


                ALOGV("package '%s' installed by '%s' versioncode %"  PRId64 " / %" PRIx64,
                      pkg.c_str(), installer.c_str(), versionCode, versionCode);

                if (strncmp(installer.c_str(), "com.android.", 12) == 0) {
                        // from play store, we keep info
                } else if (strncmp(installer.c_str(), "com.google.", 11) == 0) {
                        // some google source, we keep info
                } else if (strcmp(installer.c_str(), "preload") == 0) {
                        // preloads, we keep the info
                } else if (installer.c_str()[0] == '\0') {
                        // sideload (no installer); do not report
                        pkg = "";
                        versionCode = 0;
                } else {
                        // unknown installer; do not report
                        pkg = "";
                        versionCode = 0;
                }
            }
        }

        // add it to the map, to save a subsequent lookup
        if (!pkg.empty()) {
            Mutex::Autolock _l(mLock_mappings);
            ALOGV("Adding uid %d pkg '%s'", uid, pkg.c_str());
            ssize_t i = mPkgMappings.indexOfKey(uid);
            if (i < 0) {
                mapping.uid = uid;
                mapping.pkg = pkg;
                mapping.installer = installer.c_str();
                mapping.versionCode = versionCode;
                mapping.expiration = now + PKG_EXPIRATION_NS;
                ALOGV("expiration for uid %d set to %" PRId64 "", uid, mapping.expiration);

                mPkgMappings.add(uid, mapping);
            }
        }
    }

    if (mapping.uid != (uid_t)(-1)) {
        if (setName) {
            item->setPkgName(mapping.pkg);
        }
        if (setVersion) {
            item->setPkgVersionCode(mapping.versionCode);
        }
    }
}

} // namespace android
