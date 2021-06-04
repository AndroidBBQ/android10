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

#ifndef META_DATA_H_

#define META_DATA_H_

#include <sys/types.h>

#include <stdint.h>

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <media/stagefright/MetaDataBase.h>

namespace android {

class MetaData final : public MetaDataBase, public RefBase {
public:
    MetaData();
    MetaData(const MetaData &from);
    MetaData(const MetaDataBase &from);

protected:
    virtual ~MetaData();

private:
    friend class BnMediaSource;
    friend class BpMediaSource;
    friend class BpMediaExtractor;
    static sp<MetaData> createFromParcel(const Parcel &parcel);
};

}  // namespace android

#endif  // META_DATA_H_
