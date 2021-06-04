/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef ANDROID_IMEDIAEXTRACTORSERVICE_H
#define ANDROID_IMEDIAEXTRACTORSERVICE_H

#include <unordered_set>

#include <binder/IInterface.h>
#include <binder/IMemory.h>
#include <binder/Parcel.h>
#include <media/IDataSource.h>
#include <media/IMediaExtractor.h>

namespace android {

class IMediaExtractorService: public IInterface
{
public:
    DECLARE_META_INTERFACE(MediaExtractorService);

    virtual sp<IMediaExtractor> makeExtractor(const sp<IDataSource> &source, const char *mime) = 0;

    virtual sp<IDataSource> makeIDataSource(int fd, int64_t offset, int64_t length) = 0;

    virtual std::unordered_set<std::string> getSupportedTypes() = 0;
};

class BnMediaExtractorService: public BnInterface<IMediaExtractorService>
{
public:
    virtual status_t    onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                uint32_t flags = 0);
};

}   // namespace android

#endif  // ANDROID_IMEDIAEXTRACTORSERVICE_H
