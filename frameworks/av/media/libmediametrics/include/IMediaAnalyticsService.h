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

#ifndef ANDROID_IMEDIAANALYTICSSERVICE_H
#define ANDROID_IMEDIAANALYTICSSERVICE_H

#include <utils/String8.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <sys/types.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/RefBase.h>
#include <utils/List.h>

#include <binder/IServiceManager.h>

#include <media/MediaAnalyticsItem.h>
// nope...#include <media/MediaAnalytics.h>

namespace android {

class IMediaAnalyticsService: public IInterface
{
public:
    DECLARE_META_INTERFACE(MediaAnalyticsService);

    // generate a unique sessionID to use across multiple requests
    // 'unique' is within this device, since last reboot
    virtual MediaAnalyticsItem::SessionID_t generateUniqueSessionID() = 0;

    // submit the indicated record to the mediaanalytics service, where
    // it will be merged (if appropriate) with incomplete records that
    // share the same key and sessionid.
    // 'forcenew' marks any matching incomplete record as complete before
    // inserting this new record.
    // returns the sessionID associated with that item.
    // caller continues to own the passed item
    virtual MediaAnalyticsItem::SessionID_t submit(MediaAnalyticsItem *item, bool forcenew) = 0;

};

// ----------------------------------------------------------------------------

class BnMediaAnalyticsService: public BnInterface<IMediaAnalyticsService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace android

#endif // ANDROID_IMEDIASTATISTICSSERVICE_H
