/*
 * Copyright (C) 2018 The Android Open Source Project
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


/*
 * This file defines an NDK API.
 * Do not remove methods.
 * Do not change method signatures.
 * Do not change the value of constants.
 * Do not change the size of any of the classes defined in here.
 * Do not reference types that are not part of the NDK.
 * Do not #include files that aren't part of the NDK.
 */

#ifndef _NDK_MEDIA_DATASOURCE_PRIV_H
#define _NDK_MEDIA_DATASOURCE_PRIV_H

#include <sys/cdefs.h>
#include <sys/types.h>

#include <media/DataSource.h>
#include <media/MediaHTTPService.h>
#include <media/NdkMediaDataSource.h>
#include <utils/Mutex.h>
#include <utils/String8.h>

using namespace android;

struct NdkDataSource : public DataSource {

    NdkDataSource(AMediaDataSource *);

    virtual status_t initCheck() const;
    virtual uint32_t flags();
    virtual ssize_t readAt(off64_t offset, void *data, size_t size);
    virtual status_t getSize(off64_t *);
    virtual String8 toString();
    virtual String8 getMIMEType() const;
    virtual void close();
    virtual status_t getAvailableSize(off64_t offset, off64_t *size);

protected:
    virtual ~NdkDataSource();

private:

    Mutex mLock;
    AMediaDataSource *mDataSource;

};

sp<MediaHTTPService> createMediaHttpService(const char *uri, int version);

#endif // _NDK_MEDIA_DATASOURCE_PRIV_H

