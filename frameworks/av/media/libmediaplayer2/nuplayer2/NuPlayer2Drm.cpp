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

//#define LOG_NDEBUG 0
#define LOG_TAG "NuPlayer2Drm"

#include "NuPlayer2Drm.h"

#include <media/NdkWrapper.h>
#include <utils/Log.h>
#include <sstream>

namespace android {

Vector<DrmUUID> NuPlayer2Drm::parsePSSH(const void *pssh, size_t psshsize)
{
    Vector<DrmUUID> drmSchemes, empty;
    const int DATALEN_SIZE = 4;

    // the format of the buffer is 1 or more of:
    //    {
    //        16 byte uuid
    //        4 byte data length N
    //        N bytes of data
    //    }
    // Determine the number of entries in the source data.
    // Since we got the data from stagefright, we trust it is valid and properly formatted.

    const uint8_t *data = (const uint8_t*)pssh;
    size_t len = psshsize;
    size_t numentries = 0;
    while (len > 0) {
        if (len < DrmUUID::UUID_SIZE) {
            ALOGE("ParsePSSH: invalid PSSH data");
            return empty;
        }

        const uint8_t *uuidPtr = data;

        // skip uuid
        data += DrmUUID::UUID_SIZE;
        len -= DrmUUID::UUID_SIZE;

        // get data length
        if (len < DATALEN_SIZE) {
            ALOGE("ParsePSSH: invalid PSSH data");
            return empty;
        }

        uint32_t datalen = *((uint32_t*)data);
        data += DATALEN_SIZE;
        len -= DATALEN_SIZE;

        if (len < datalen) {
            ALOGE("ParsePSSH: invalid PSSH data");
            return empty;
        }

        // skip the data
        data += datalen;
        len -= datalen;

        DrmUUID _uuid(uuidPtr);
        drmSchemes.add(_uuid);

        ALOGV("ParsePSSH[%zu]: %s: %s", numentries,
                _uuid.toHexString().string(),
                DrmUUID::arrayToHex(data, datalen).string()
             );

        numentries++;
    }

    return drmSchemes;
}

Vector<DrmUUID> NuPlayer2Drm::getSupportedDrmSchemes(const void *pssh, size_t psshsize)
{
    Vector<DrmUUID> psshDRMs = parsePSSH(pssh, psshsize);

    Vector<DrmUUID> supportedDRMs;
    for (size_t i = 0; i < psshDRMs.size(); i++) {
        DrmUUID uuid = psshDRMs[i];
        if (AMediaDrmWrapper::isCryptoSchemeSupported(uuid.ptr(), NULL)) {
            supportedDRMs.add(uuid);
        }
    }

    ALOGV("getSupportedDrmSchemes: psshDRMs: %zu supportedDRMs: %zu",
            psshDRMs.size(), supportedDRMs.size());

    return supportedDRMs;
}

sp<ABuffer> NuPlayer2Drm::retrieveDrmInfo(const void *pssh, uint32_t psshsize)
{
    std::ostringstream buf;

    // 1) PSSH bytes
    buf.write(reinterpret_cast<const char *>(&psshsize), sizeof(psshsize));
    buf.write(reinterpret_cast<const char *>(pssh), psshsize);

    ALOGV("retrieveDrmInfo: MEDIA2_DRM_INFO  PSSH: size: %u %s", psshsize,
            DrmUUID::arrayToHex((uint8_t*)pssh, psshsize).string());

    // 2) supportedDRMs
    Vector<DrmUUID> supportedDRMs = getSupportedDrmSchemes(pssh, psshsize);
    uint32_t n = supportedDRMs.size();
    buf.write(reinterpret_cast<char *>(&n), sizeof(n));
    for (size_t i = 0; i < n; i++) {
        DrmUUID uuid = supportedDRMs[i];
        buf.write(reinterpret_cast<const char *>(&n), sizeof(n));
        buf.write(reinterpret_cast<const char *>(uuid.ptr()), DrmUUID::UUID_SIZE);

        ALOGV("retrieveDrmInfo: MEDIA2_DRM_INFO  supportedScheme[%zu] %s", i,
                uuid.toHexString().string());
    }

    sp<ABuffer> drmInfoBuffer = ABuffer::CreateAsCopy(buf.str().c_str(), buf.tellp());
    return drmInfoBuffer;
}

status_t NuPlayer2Drm::retrieveDrmInfo(PsshInfo *psshInfo, PlayerMessage *playerMsg)
{
    std::ostringstream pssh, drmInfo;

    // 0) Generate PSSH bytes
    for (size_t i = 0; i < psshInfo->numentries; i++) {
        PsshEntry *entry = &psshInfo->entries[i];
        uint32_t datalen = entry->datalen;
        pssh.write(reinterpret_cast<const char *>(&entry->uuid), sizeof(entry->uuid));
        pssh.write(reinterpret_cast<const char *>(&datalen), sizeof(datalen));
        pssh.write(reinterpret_cast<const char *>(entry->data), datalen);
    }

    uint32_t psshSize = pssh.tellp();
    std::string psshBase = pssh.str();
    const auto* psshPtr = reinterpret_cast<const uint8_t*>(psshBase.c_str());
    ALOGV("retrieveDrmInfo: MEDIA_DRM_INFO  PSSH: size: %u %s", psshSize,
            DrmUUID::arrayToHex(psshPtr, psshSize).string());

    // 1) Write PSSH bytes
    playerMsg->add_values()->set_bytes_value(
            reinterpret_cast<const char *>(pssh.str().c_str()), psshSize);

    // 2) Write supportedDRMs
    uint32_t numentries = psshInfo->numentries;
    playerMsg->add_values()->set_int32_value(numentries);
    for (size_t i = 0; i < numentries; i++) {
        PsshEntry *entry = &psshInfo->entries[i];
        playerMsg->add_values()->set_bytes_value(
                reinterpret_cast<const char *>(&entry->uuid), sizeof(entry->uuid));
        ALOGV("retrieveDrmInfo: MEDIA_DRM_INFO  supportedScheme[%zu] %s", i,
                DrmUUID::arrayToHex((const uint8_t*)&entry->uuid, sizeof(AMediaUUID)).string());
    }
    return OK;
}

}   // namespace android
