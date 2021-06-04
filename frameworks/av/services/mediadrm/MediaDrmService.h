/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_MEDIADRMSERVICE_H
#define ANDROID_MEDIADRMSERVICE_H

#include <arpa/inet.h>

#include <utils/threads.h>

#include <media/Metadata.h>
#include <media/stagefright/foundation/ABase.h>
#include <mediadrm/IMediaDrmService.h>

namespace android {

class MediaDrmService : public BnMediaDrmService
{
public:
    static void instantiate();

    // IMediaDrmService interface
    virtual sp<ICrypto> makeCrypto();
    virtual sp<IDrm> makeDrm();
private:
    MediaDrmService() {}
    virtual ~MediaDrmService() {}
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_MEDIADRMSERVICE_H
