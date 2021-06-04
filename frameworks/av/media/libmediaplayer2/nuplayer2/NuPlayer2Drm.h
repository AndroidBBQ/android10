/*
 * Copyright 2017 The Android Open Source Project
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

#ifndef NUPLAYER2_DRM_H_
#define NUPLAYER2_DRM_H_

#include <media/NdkMediaExtractor.h>
#include <media/stagefright/foundation/ABuffer.h>

#include <utils/String8.h>
#include <utils/Vector.h>

#include "mediaplayer2.pb.h"

using android::media::MediaPlayer2Proto::PlayerMessage;

namespace android {

    struct DrmUUID {
        static const int UUID_SIZE = 16;

        DrmUUID() {
            memset(this->uuid, 0, sizeof(uuid));
        }

        // to allow defining Vector/KeyedVector of UUID type
        DrmUUID(const DrmUUID &a) {
            memcpy(this->uuid, a.uuid, sizeof(uuid));
        }

        // to allow defining Vector/KeyedVector of UUID type
        DrmUUID(const uint8_t uuid_in[UUID_SIZE]) {
            memcpy(this->uuid, uuid_in, sizeof(uuid));
        }

        const uint8_t *ptr() const {
            return uuid;
        }

        String8 toHexString() const {
            return arrayToHex(uuid, UUID_SIZE);
        }

        static String8 toHexString(const uint8_t uuid_in[UUID_SIZE]) {
            return arrayToHex(uuid_in, UUID_SIZE);
        }

        static String8 arrayToHex(const uint8_t *array, int bytes) {
            String8 result;
            for (int i = 0; i < bytes; i++) {
                result.appendFormat("%02x", array[i]);
            }

            return result;
        }

    protected:
        uint8_t uuid[UUID_SIZE];
    };


    struct NuPlayer2Drm {

        // static helpers - internal

    protected:
        static Vector<DrmUUID> parsePSSH(const void *pssh, size_t psshsize);
        static Vector<DrmUUID> getSupportedDrmSchemes(const void *pssh, size_t psshsize);

        // static helpers - public

    public:
        static sp<ABuffer> retrieveDrmInfo(const void *pssh, uint32_t psshsize);
        static status_t retrieveDrmInfo(PsshInfo *, PlayerMessage *);

    };  // NuPlayer2Drm

}   // android

#endif     //NUPLAYER2_DRM_H_
