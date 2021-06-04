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

#ifndef DESCRAMBLER_API_H_
#define DESCRAMBLER_API_H_

#include <media/stagefright/MediaErrors.h>
#include <media/cas/CasAPI.h>

namespace android {

struct AString;
struct DescramblerPlugin;

struct DescramblerFactory {
    DescramblerFactory() {}
    virtual ~DescramblerFactory() {}

    // Determine if the plugin can handle the CA scheme identified by CA_system_id.
    virtual bool isSystemIdSupported(
            int32_t CA_system_id) const = 0;

    // Construct a new instance of a DescramblerPlugin given a CA_system_id
    virtual status_t createPlugin(
            int32_t CA_system_id, DescramblerPlugin **plugin) = 0;

private:
    DescramblerFactory(const DescramblerFactory &);
    DescramblerFactory &operator=(const DescramblerFactory &);
};

struct DescramblerPlugin {
    enum ScramblingControl {
        kScrambling_Unscrambled = 0,
        kScrambling_Reserved    = 1,
        kScrambling_EvenKey     = 2,
        kScrambling_OddKey      = 3,
        kScrambling_Mask_Key    = 0x3,

        // Hint that the descrambling request is for a PES header only
        kScrambling_Flag_PesHeader = (1 << 31),
    };

    struct SubSample {
        uint32_t mNumBytesOfClearData;
        uint32_t mNumBytesOfEncryptedData;
    };

    DescramblerPlugin() {}
    virtual ~DescramblerPlugin() {}

    // If this method returns false, a non-secure decoder will be used to
    // decode the data after decryption. The decrypt API below will have
    // to support insecure decryption of the data (secure = false) for
    // media data of the given mime type.
    virtual bool requiresSecureDecoderComponent(const char *mime) const = 0;

    // A MediaCas session may be associated with a MediaCrypto session.  The
    // associated MediaCas session is used to load decryption keys
    // into the crypto/cas plugin.  The keys are then referenced by key-id
    // in the 'key' parameter to the decrypt() method.
    // Should return NO_ERROR on success, ERROR_CAS_SESSION_NOT_OPENED if
    // the session is not opened and a code from MediaErrors.h otherwise.
    virtual status_t setMediaCasSession(const CasSessionId& sessionId) = 0;

    // If the error returned falls into the range
    // ERROR_CAS_VENDOR_MIN..ERROR_CAS_VENDOR_MAX, errorDetailMsg should be
    // filled in with an appropriate string.
    // At the java level these special errors will then trigger a
    // MediaCodec.CryptoException that gives clients access to both
    // the error code and the errorDetailMsg.
    // Returns a non-negative result to indicate the number of bytes written
    // to the dstPtr, or a negative result to indicate an error.
    virtual ssize_t descramble(
            bool secure,
            ScramblingControl scramblingControl,
            size_t numSubSamples,
            const SubSample *subSamples,
            const void *srcPtr,
            int32_t srcOffset,
            void *dstPtr,
            int32_t dstOffset,
            AString *errorDetailMsg) = 0;

private:
    DescramblerPlugin(const DescramblerPlugin &);
    DescramblerPlugin &operator=(const DescramblerPlugin &);
};

}  // namespace android

extern "C" {
    extern android::DescramblerFactory *createDescramblerFactory();
}

#endif  // DESCRAMBLER_API_H_
