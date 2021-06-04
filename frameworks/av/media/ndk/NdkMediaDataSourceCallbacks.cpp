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

//#define LOG_NDEBUG 0
#define LOG_TAG "NdkMediaDataSourceCallbacks"

#include "NdkMediaDataSourceCallbacksPriv.h"
#include <media/DataSource.h>
#include <media/NdkMediaErrorPriv.h>

namespace android {

ssize_t DataSource_getSize(void *userdata) {
    DataSource *source = static_cast<DataSource *>(userdata);
    off64_t size = -1;
    source->getSize(&size);
    return size;
}

ssize_t DataSource_readAt(void *userdata, off64_t offset, void * buf, size_t size) {
    DataSource *source = static_cast<DataSource *>(userdata);
    return source->readAt(offset, buf, size);
}

void DataSource_close(void *userdata) {
    DataSource *source = static_cast<DataSource *>(userdata);
    source->close();
}

ssize_t DataSource_getAvailableSize(void *userdata, off64_t offset) {
    off64_t size = -1;
    DataSource *source = static_cast<DataSource *>(userdata);
    source->getAvailableSize(offset, &size);
    return  size;
}

}  // namespace android
