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


#ifndef ANDROID_MEDIAANALYTICSSERVICE_H
#define ANDROID_MEDIAANALYTICSSERVICE_H

#include <arpa/inet.h>

#include <utils/threads.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/List.h>

#include <future>

#include <media/IMediaAnalyticsService.h>

namespace android {

class MediaAnalyticsService : public BnMediaAnalyticsService
{

 public:

    // on this side, caller surrenders ownership
    virtual int64_t submit(MediaAnalyticsItem *item, bool forcenew);

    static  void            instantiate();
    virtual status_t        dump(int fd, const Vector<String16>& args);

                            MediaAnalyticsService();
    virtual                 ~MediaAnalyticsService();

    bool processExpirations();

 private:
    MediaAnalyticsItem::SessionID_t generateUniqueSessionID();

    // statistics about our analytics
    int64_t mItemsSubmitted;
    int64_t mItemsFinalized;
    int64_t mItemsDiscarded;
    int64_t mItemsDiscardedExpire;
    int64_t mItemsDiscardedCount;
    MediaAnalyticsItem::SessionID_t mLastSessionID;

    // partitioned a bit so we don't over serialize
    mutable Mutex           mLock;
    mutable Mutex           mLock_ids;
    mutable Mutex           mLock_mappings;

    // limit how many records we'll retain
    // by count (in each queue (open, finalized))
    int32_t mMaxRecords;
    // by time (none older than this long agan
    nsecs_t mMaxRecordAgeNs;
    // max to expire per expirations_l() invocation
    int32_t mMaxRecordsExpiredAtOnce;
    //
    // # of sets of summaries
    int32_t mMaxRecordSets;
    // nsecs until we start a new record set
    nsecs_t mNewSetInterval;

    // input validation after arrival from client
    bool contentValid(MediaAnalyticsItem *item, bool isTrusted);
    bool rateLimited(MediaAnalyticsItem *);

    // (oldest at front) so it prints nicely for dumpsys
    List<MediaAnalyticsItem *> mItems;
    void saveItem(MediaAnalyticsItem *);

    bool expirations_l(MediaAnalyticsItem *);
    std::future<bool> mExpireFuture;

    // support for generating output
    int mDumpProto;
    int mDumpProtoDefault;
    String8 dumpQueue();
    String8 dumpQueue(nsecs_t, const char *only);

    void dumpHeaders(String8 &result, nsecs_t ts_since);
    void dumpSummaries(String8 &result, nsecs_t ts_since, const char * only);
    void dumpRecent(String8 &result, nsecs_t ts_since, const char * only);

    // mapping uids to package names
    struct UidToPkgMap {
        uid_t uid;
        std::string pkg;
        std::string installer;
        int64_t versionCode;
        nsecs_t expiration;
    };

    KeyedVector<uid_t,struct UidToPkgMap>  mPkgMappings;
    void setPkgInfo(MediaAnalyticsItem *item, uid_t uid, bool setName, bool setVersion);

};

// hook to send things off to the statsd subsystem
extern bool dump2Statsd(MediaAnalyticsItem *item);

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_MEDIAANALYTICSSERVICE_H
