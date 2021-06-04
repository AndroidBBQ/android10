/*
 * Copyright (C) 2017 The Android Open Source Project
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
#define LOG_TAG "HlsSampleDecryptor"

#include "HlsSampleDecryptor.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/Utils.h>


namespace android {

HlsSampleDecryptor::HlsSampleDecryptor()
    : mValidKeyInfo(false) {
}

HlsSampleDecryptor::HlsSampleDecryptor(const sp<AMessage> &sampleAesKeyItem)
    : mValidKeyInfo(false) {

    signalNewSampleAesKey(sampleAesKeyItem);
}

void HlsSampleDecryptor::signalNewSampleAesKey(const sp<AMessage> &sampleAesKeyItem) {

    if (sampleAesKeyItem == NULL) {
        mValidKeyInfo = false;
        ALOGW("signalNewSampleAesKey: sampleAesKeyItem is NULL");
        return;
    }

    sp<ABuffer> keyDataBuffer, initVecBuffer;
    sampleAesKeyItem->findBuffer("keyData", &keyDataBuffer);
    sampleAesKeyItem->findBuffer("initVec", &initVecBuffer);

    if (keyDataBuffer != NULL && keyDataBuffer->size() == AES_BLOCK_SIZE &&
        initVecBuffer != NULL && initVecBuffer->size() == AES_BLOCK_SIZE) {

        ALOGV("signalNewSampleAesKey: Key: %s  IV: %s",
              aesBlockToStr(keyDataBuffer->data()).c_str(),
              aesBlockToStr(initVecBuffer->data()).c_str());

        uint8_t KeyData[AES_BLOCK_SIZE];
        memcpy(KeyData, keyDataBuffer->data(), AES_BLOCK_SIZE);
        memcpy(mAESInitVec, initVecBuffer->data(), AES_BLOCK_SIZE);

        mValidKeyInfo = (AES_set_decrypt_key(KeyData, 8*AES_BLOCK_SIZE/*128*/, &mAesKey) == 0);
        if (!mValidKeyInfo) {
            ALOGE("signalNewSampleAesKey: failed to set AES decryption key.");
        }

    } else {
        // Media scanner might try extract/parse the TS files without knowing the key.
        // Otherwise, shouldn't get here (unless an invalid playlist has swaped SAMPLE-AES with
        // NONE method while still sample-encrypted stream is parsed).

        mValidKeyInfo = false;
        ALOGE("signalNewSampleAesKey Can't decrypt; keyDataBuffer: %p(%zu) initVecBuffer: %p(%zu)",
              keyDataBuffer.get(), (keyDataBuffer.get() == NULL)? -1 : keyDataBuffer->size(),
              initVecBuffer.get(), (initVecBuffer.get() == NULL)? -1 : initVecBuffer->size());
    }
}

size_t HlsSampleDecryptor::processNal(uint8_t *nalData, size_t nalSize) {

    unsigned nalType = nalData[0] & 0x1f;
    if (!mValidKeyInfo) {
        ALOGV("processNal[%d]: (%p)/%zu Skipping due to invalid key", nalType, nalData, nalSize);
        return nalSize;
    }

    bool isEncrypted = (nalSize > VIDEO_CLEAR_LEAD + AES_BLOCK_SIZE);
    ALOGV("processNal[%d]: (%p)/%zu isEncrypted: %d", nalType, nalData, nalSize, isEncrypted);

    if (isEncrypted) {
        // Encrypted NALUs have extra start code emulation prevention that must be
        // stripped out before we can decrypt it.
        size_t newSize = unescapeStream(nalData, nalSize);

        ALOGV("processNal:unescapeStream[%d]: %zu -> %zu", nalType, nalSize, newSize);
        nalSize = newSize;

        //Encrypted_nal_unit () {
        //    nal_unit_type_byte                // 1 byte
        //    unencrypted_leader                // 31 bytes
        //    while (bytes_remaining() > 0) {
        //        if (bytes_remaining() > 16) {
        //            encrypted_block           // 16 bytes
        //        }
        //        unencrypted_block           // MIN(144, bytes_remaining()) bytes
        //    }
        //}

        size_t offset = VIDEO_CLEAR_LEAD;
        size_t remainingBytes = nalSize - VIDEO_CLEAR_LEAD;

        // a copy of initVec as decryptBlock updates it
        unsigned char AESInitVec[AES_BLOCK_SIZE];
        memcpy(AESInitVec, mAESInitVec, AES_BLOCK_SIZE);

        while (remainingBytes > 0) {
            // encrypted_block: protected block uses 10% skip encryption
            if (remainingBytes > AES_BLOCK_SIZE) {
                uint8_t *encrypted = nalData + offset;
                status_t ret = decryptBlock(encrypted, AES_BLOCK_SIZE, AESInitVec);
                if (ret != OK) {
                    ALOGE("processNal failed with %d", ret);
                    return nalSize; // revisit this
                }

                offset += AES_BLOCK_SIZE;
                remainingBytes -= AES_BLOCK_SIZE;
            }

            // unencrypted_block
            size_t clearBytes = std::min(remainingBytes, (size_t)(9 * AES_BLOCK_SIZE));

            offset += clearBytes;
            remainingBytes -= clearBytes;
        } // while

    } else { // isEncrypted == false
        ALOGV("processNal[%d]: Unencrypted NALU  (%p)/%zu", nalType, nalData, nalSize);
    }

    return nalSize;
}

void HlsSampleDecryptor::processAAC(size_t adtsHdrSize, uint8_t *data, size_t size) {

    if (!mValidKeyInfo) {
        ALOGV("processAAC: (%p)/%zu Skipping due to invalid key", data, size);
        return;
    }

    // ADTS header is included in the size
    if (size < adtsHdrSize) {
        ALOGV("processAAC: size (%zu) < adtsHdrSize (%zu)", size, adtsHdrSize);
        android_errorWriteLog(0x534e4554, "128433933");
        return;
    }
    size_t offset = adtsHdrSize;
    size_t remainingBytes = size - adtsHdrSize;

    bool isEncrypted = (remainingBytes >= AUDIO_CLEAR_LEAD + AES_BLOCK_SIZE);
    ALOGV("processAAC: header: %zu data: %p(%zu) isEncrypted: %d",
          adtsHdrSize, data, size, isEncrypted);

    //Encrypted_AAC_Frame () {
    //    ADTS_Header                        // 7 or 9 bytes
    //    unencrypted_leader                 // 16 bytes
    //    while (bytes_remaining() >= 16) {
    //        encrypted_block                // 16 bytes
    //    }
    //    unencrypted_trailer                // 0-15 bytes
    //}

    // with lead bytes
    if (remainingBytes >= AUDIO_CLEAR_LEAD) {
        offset += AUDIO_CLEAR_LEAD;
        remainingBytes -= AUDIO_CLEAR_LEAD;

        // encrypted_block
        if (remainingBytes >= AES_BLOCK_SIZE) {

            size_t encryptedBytes = (remainingBytes / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
            unsigned char AESInitVec[AES_BLOCK_SIZE];
            memcpy(AESInitVec, mAESInitVec, AES_BLOCK_SIZE);

            // decrypting all blocks at once
            uint8_t *encrypted = data + offset;
            status_t ret = decryptBlock(encrypted, encryptedBytes, AESInitVec);
            if (ret != OK) {
                ALOGE("processAAC: decryptBlock failed with %d", ret);
                return;
            }

            offset += encryptedBytes;
            remainingBytes -= encryptedBytes;
        } // encrypted

        // unencrypted_trailer
        size_t clearBytes = remainingBytes;
        if (clearBytes > 0) {
            CHECK(clearBytes < AES_BLOCK_SIZE);
        }

    } else { // without lead bytes
        ALOGV("processAAC: Unencrypted frame (without lead bytes) size %zu = %zu (hdr) + %zu (rem)",
              size, adtsHdrSize, remainingBytes);
    }

}

void HlsSampleDecryptor::processAC3(uint8_t *data, size_t size) {

    if (!mValidKeyInfo) {
        ALOGV("processAC3: (%p)/%zu Skipping due to invalid key", data, size);
        return;
    }

    bool isEncrypted = (size >= AUDIO_CLEAR_LEAD + AES_BLOCK_SIZE);
    ALOGV("processAC3 %p(%zu) isEncrypted: %d", data, size, isEncrypted);

    //Encrypted_AC3_Frame () {
    //    unencrypted_leader                 // 16 bytes
    //    while (bytes_remaining() >= 16) {
    //        encrypted_block                // 16 bytes
    //    }
    //    unencrypted_trailer                // 0-15 bytes
    //}

    if (size >= AUDIO_CLEAR_LEAD) {
        // unencrypted_leader
        size_t offset = AUDIO_CLEAR_LEAD;
        size_t remainingBytes = size - AUDIO_CLEAR_LEAD;

        if (remainingBytes >= AES_BLOCK_SIZE) {

            size_t encryptedBytes = (remainingBytes / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;

            // encrypted_block
            unsigned char AESInitVec[AES_BLOCK_SIZE];
            memcpy(AESInitVec, mAESInitVec, AES_BLOCK_SIZE);

            // decrypting all blocks at once
            uint8_t *encrypted = data + offset;
            status_t ret = decryptBlock(encrypted, encryptedBytes, AESInitVec);
            if (ret != OK) {
                ALOGE("processAC3: decryptBlock failed with %d", ret);
                return;
            }

            offset += encryptedBytes;
            remainingBytes -= encryptedBytes;
        } // encrypted

        // unencrypted_trailer
        size_t clearBytes = remainingBytes;
        if (clearBytes > 0) {
            CHECK(clearBytes < AES_BLOCK_SIZE);
        }

    } else {
        ALOGV("processAC3: Unencrypted frame (without lead bytes) size %zu", size);
    }
}

// Unescapes data replacing occurrences of [0, 0, 3] with [0, 0] and returns the new size
size_t HlsSampleDecryptor::unescapeStream(uint8_t *data, size_t limit) const {
    Vector<size_t> scratchEscapePositions;
    size_t position = 0;

    while (position < limit) {
        position = findNextUnescapeIndex(data, position, limit);
        if (position < limit) {
            scratchEscapePositions.add(position);
            position += 3;
        }
    }

    size_t scratchEscapeCount = scratchEscapePositions.size();
    size_t escapedPosition = 0; // The position being read from.
    size_t unescapedPosition = 0; // The position being written to.
    for (size_t i = 0; i < scratchEscapeCount; i++) {
        size_t nextEscapePosition = scratchEscapePositions[i];
        //TODO: add 2 and get rid of the later = 0 assignments
        size_t copyLength = nextEscapePosition - escapedPosition;
        memmove(data+unescapedPosition, data+escapedPosition, copyLength);
        unescapedPosition += copyLength;
        data[unescapedPosition++] = 0;
        data[unescapedPosition++] = 0;
        escapedPosition += copyLength + 3;
    }

    size_t unescapedLength = limit - scratchEscapeCount;
    size_t remainingLength = unescapedLength - unescapedPosition;
    memmove(data+unescapedPosition, data+escapedPosition, remainingLength);

    return unescapedLength;
}

size_t HlsSampleDecryptor::findNextUnescapeIndex(uint8_t *data, size_t offset, size_t limit) const {
    for (size_t i = offset; i < limit - 2; i++) {
        //TODO: speed
        if (data[i] == 0x00 && data[i + 1] == 0x00 && data[i + 2] == 0x03) {
            return i;
        }
    }
    return limit;
}

status_t HlsSampleDecryptor::decryptBlock(uint8_t *buffer, size_t size,
        uint8_t AESInitVec[AES_BLOCK_SIZE]) {
    if (size == 0) {
        return OK;
    }

    if ((size % AES_BLOCK_SIZE) != 0) {
        ALOGE("decryptBlock: size (%zu) not a multiple of block size", size);
        return ERROR_MALFORMED;
    }

    ALOGV("decryptBlock: %p (%zu)", buffer, size);

    AES_cbc_encrypt(buffer, buffer, size, &mAesKey, AESInitVec, AES_DECRYPT);

    return OK;
}

AString HlsSampleDecryptor::aesBlockToStr(uint8_t block[AES_BLOCK_SIZE]) {
    AString result;

    if (block == NULL) {
        result = AString("null");
    } else {
        result = AStringPrintf("0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
            block[0], block[1], block[2], block[3], block[4], block[5], block[6], block[7],
            block[8], block[9], block[10], block[11], block[12], block[13], block[14], block[15]);
    }

    return result;
}


}  // namespace android
