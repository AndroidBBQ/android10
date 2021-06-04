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

#ifndef ANDROID_MEDIA_EXTRACTOR_SERVICE_H
#define ANDROID_MEDIA_EXTRACTOR_SERVICE_H

#include <binder/BinderService.h>
#include <media/IMediaExtractorService.h>
#include <media/IMediaExtractor.h>

namespace android {

class MediaExtractorService : public BinderService<MediaExtractorService>, public BnMediaExtractorService
{
    friend class BinderService<MediaExtractorService>;    // for MediaExtractorService()
public:
    MediaExtractorService();
    virtual ~MediaExtractorService() { }
    virtual void onFirstRef() { }

    static const char*  getServiceName() { return "media.extractor"; }

    virtual sp<IMediaExtractor> makeExtractor(const sp<IDataSource> &source, const char *mime);

    virtual sp<IDataSource> makeIDataSource(int fd, int64_t offset, int64_t length);

    virtual std::unordered_set<std::string> getSupportedTypes();

    virtual status_t    dump(int fd, const Vector<String16>& args);

    virtual status_t    onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                uint32_t flags);

private:
    Mutex               mLock;
};

}   // namespace android

#endif  // ANDROID_MEDIA_EXTRACTOR_SERVICE_H
