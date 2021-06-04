/*
 * Copyright 2018, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef A_MEDIA_DATA_SOURCE_CALLBACKS_H

#define A_MEDIA_DATA_SOURCE_CALLBACKS_H

#include <media/DataSource.h>
#include <media/NdkMediaDataSource.h>
#include <media/NdkMediaError.h>
#include <sys/types.h>

namespace android {

ssize_t DataSource_getSize(void *userdata);

ssize_t DataSource_readAt(void *userdata, off64_t offset, void * buf, size_t size);

void DataSource_close(void *userdata);

ssize_t DataSource_getAvailableSize(void *userdata, off64_t offset);

static inline AMediaDataSource* convertDataSourceToAMediaDataSource(const sp<DataSource> &source) {
    if (source == NULL) {
        return NULL;
    }
    AMediaDataSource *mSource = AMediaDataSource_new();
    AMediaDataSource_setUserdata(mSource, source.get());
    AMediaDataSource_setReadAt(mSource, DataSource_readAt);
    AMediaDataSource_setGetSize(mSource, DataSource_getSize);
    AMediaDataSource_setClose(mSource, DataSource_close);
    AMediaDataSource_setGetAvailableSize(mSource, DataSource_getAvailableSize);
    return mSource;
}

}  // namespace android

#endif  // A_MEDIA_DATA_SOURCE_CALLBACKS_H
