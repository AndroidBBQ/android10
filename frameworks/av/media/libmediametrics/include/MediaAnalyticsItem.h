/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_MEDIA_MEDIAANALYTICSITEM_H
#define ANDROID_MEDIA_MEDIAANALYTICSITEM_H

#include <string>
#include <sys/types.h>

#include <cutils/properties.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <utils/Timers.h>

namespace android {

class IMediaAnalyticsService;
class Parcel;

// the class interface
//

class MediaAnalyticsItem {

    friend class MediaAnalyticsService;
    friend class IMediaAnalyticsService;
    friend class MediaMetricsJNI;
    friend class MetricsSummarizer;
    friend class MediaMetricsDeathNotifier;

    public:

            enum Type {
                kTypeNone = 0,
                kTypeInt32 = 1,
                kTypeInt64 = 2,
                kTypeDouble = 3,
                kTypeCString = 4,
                kTypeRate = 5,
            };

        // sessionid
        // unique within device, within boot,
        typedef int64_t SessionID_t;
        static constexpr SessionID_t SessionIDInvalid = -1;
        static constexpr SessionID_t SessionIDNone = 0;

        // Key: the record descriminator
        // values for the record discriminator
        // values can be "component/component"
        // basic values: "video", "audio", "drm"
        // XXX: need to better define the format
        typedef std::string Key;
        static const Key kKeyNone;              // ""
        static const Key kKeyAny;               // "*"

        // Attr: names for attributes within a record
        // format "prop1" or "prop/subprop"
        // XXX: need to better define the format
        typedef const char *Attr;


        enum {
            PROTO_V0 = 0,
            PROTO_FIRST = PROTO_V0,
            PROTO_V1 = 1,
            PROTO_LAST = PROTO_V1,
        };

    private:
        // use the ::create() method instead
        MediaAnalyticsItem();
        MediaAnalyticsItem(Key);
        MediaAnalyticsItem(const MediaAnalyticsItem&);
        MediaAnalyticsItem &operator=(const MediaAnalyticsItem&);

    public:

        static MediaAnalyticsItem* create(Key key);
        static MediaAnalyticsItem* create();

        // access functions for the class
        ~MediaAnalyticsItem();

        // SessionID ties multiple submissions for same key together
        // so that if video "height" and "width" are known at one point
        // and "framerate" is only known later, they can be be brought
        // together.
        MediaAnalyticsItem &setSessionID(SessionID_t);
        MediaAnalyticsItem &clearSessionID();
        SessionID_t getSessionID() const;
        // generates and stores a new ID iff mSessionID == SessionIDNone
        SessionID_t generateSessionID();

        // reset all contents, discarding any extra data
        void clear();
        MediaAnalyticsItem *dup();

        // set the key discriminator for the record.
        // most often initialized as part of the constructor
        MediaAnalyticsItem &setKey(MediaAnalyticsItem::Key);
        MediaAnalyticsItem::Key getKey();

        // # of attributes in the record
        int32_t count() const;

        // set values appropriately
        void setInt32(Attr, int32_t value);
        void setInt64(Attr, int64_t value);
        void setDouble(Attr, double value);
        void setRate(Attr, int64_t count, int64_t duration);
        void setCString(Attr, const char *value);

        // fused get/add/set; if attr wasn't there, it's a simple set.
        // type-mismatch counts as "wasn't there".
        void addInt32(Attr, int32_t value);
        void addInt64(Attr, int64_t value);
        void addDouble(Attr, double value);
        void addRate(Attr, int64_t count, int64_t duration);

        // find & extract values
        // return indicates whether attr exists (and thus value filled in)
        // NULL parameter value suppresses storage of value.
        bool getInt32(Attr, int32_t *value);
        bool getInt64(Attr, int64_t *value);
        bool getDouble(Attr, double *value);
        bool getRate(Attr, int64_t *count, int64_t *duration, double *rate);
        // Caller owns the returned string
        bool getCString(Attr, char **value);
        bool getString(Attr, std::string *value);

        // parameter indicates whether to close any existing open
        // record with same key before establishing a new record
        // caller retains ownership of 'this'.
        bool selfrecord(bool);
        bool selfrecord();

        // remove indicated attributes and their values
        // filterNot() could also be called keepOnly()
        // return value is # attributes removed
        // XXX: perhaps 'remove' instead of 'filter'
        // XXX: filterNot would become 'keep'
        int32_t filter(int count, Attr attrs[]);
        int32_t filterNot(int count, Attr attrs[]);
        int32_t filter(Attr attr);

        // below here are used on server side or to talk to server
        // clients need not worry about these.

        // timestamp, pid, and uid only used on server side
        // timestamp is in 'nanoseconds, unix time'
        MediaAnalyticsItem &setTimestamp(nsecs_t);
        nsecs_t getTimestamp() const;

        MediaAnalyticsItem &setPid(pid_t);
        pid_t getPid() const;

        MediaAnalyticsItem &setUid(uid_t);
        uid_t getUid() const;

        MediaAnalyticsItem &setPkgName(const std::string &pkgName);
        std::string getPkgName() const { return mPkgName; }

        MediaAnalyticsItem &setPkgVersionCode(int64_t);
        int64_t getPkgVersionCode() const;

        // our serialization code for binder calls
        int32_t writeToParcel(Parcel *);
        int32_t readFromParcel(const Parcel&);

        // supports the stable interface
        bool dumpAttributes(char **pbuffer, size_t *plength);

        std::string toString();
        std::string toString(int version);
        const char *toCString();
        const char *toCString(int version);

        // are we collecting analytics data
        static bool isEnabled();

    private:
        // handle Parcel version 0
        int32_t writeToParcel0(Parcel *);
        int32_t readFromParcel0(const Parcel&);

    protected:

        // merge fields from arg into this
        // with rules for first/last/add, etc
        // XXX: document semantics and how they are indicated
        // caller continues to own 'incoming'
        bool merge(MediaAnalyticsItem *incoming);

        // enabled 1, disabled 0
        static const char * const EnabledProperty;
        static const char * const EnabledPropertyPersist;
        static const int   EnabledProperty_default;

    private:

        // to help validate that A doesn't mess with B's records
        pid_t     mPid;
        uid_t     mUid;
        std::string   mPkgName;
        int64_t   mPkgVersionCode;

        // let's reuse a binder connection
        static sp<IMediaAnalyticsService> sAnalyticsService;
        static sp<IMediaAnalyticsService> getInstance();
        static void dropInstance();

        // tracking information
        SessionID_t mSessionID;         // grouping similar records
        nsecs_t mTimestamp;             // ns, system_time_monotonic

        // will this record accept further updates
        bool mFinalized;

        Key mKey;

        struct Prop {

            Type mType;
            const char *mName;
            size_t mNameLen;    // the strlen(), doesn't include the null
            union {
                    int32_t int32Value;
                    int64_t int64Value;
                    double doubleValue;
                    char *CStringValue;
                    struct { int64_t count, duration; } rate;
            } u;
            void setName(const char *name, size_t len);
        };

        void initProp(Prop *item);
        void clearProp(Prop *item);
        void clearPropValue(Prop *item);
        void copyProp(Prop *dst, const Prop *src);
        enum {
            kGrowProps = 10
        };
        bool growProps(int increment = kGrowProps);
        size_t findPropIndex(const char *name, size_t len);
        Prop *findProp(const char *name);
        Prop *allocateProp(const char *name);
        bool removeProp(const char *name);

        size_t mPropCount;
        size_t mPropSize;
        Prop *mProps;
};

} // namespace android

#endif
