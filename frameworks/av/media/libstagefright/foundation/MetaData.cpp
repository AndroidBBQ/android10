/*
 * Copyright (C) 2009 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "MetaData"
#include <inttypes.h>
#include <binder/Parcel.h>
#include <utils/KeyedVector.h>
#include <utils/Log.h>

#include <stdlib.h>
#include <string.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MetaData.h>

namespace android {


MetaData::MetaData() {
}

MetaData::MetaData(const MetaData &from)
    : MetaDataBase(from) {
}
MetaData::MetaData(const MetaDataBase &from)
    : MetaDataBase(from) {
}

MetaData::~MetaData() {
}

/* static */
sp<MetaData> MetaData::createFromParcel(const Parcel &parcel) {

    sp<MetaData> meta = new MetaData();
    meta->updateFromParcel(parcel);
    return meta;
}

}  // namespace android

