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

#define LOG_NDEBUG 0
#define LOG_TAG "MockCasPlugin"

#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaErrors.h>
#include <utils/Log.h>

#include "MockCasPlugin.h"
#include "MockSessionLibrary.h"

android::CasFactory* createCasFactory() {
    return new android::MockCasFactory();
}

android::DescramblerFactory* createDescramblerFactory() {
    return new android::MockDescramblerFactory();
}

namespace android {

static const int32_t sMockId = 0xFFFF;

bool MockCasFactory::isSystemIdSupported(int32_t CA_system_id) const {
    return CA_system_id == sMockId;
}

status_t MockCasFactory::queryPlugins(
        std::vector<CasPluginDescriptor> *descriptors) const {
    descriptors->clear();
    descriptors->push_back({sMockId, String8("MockCAS")});
    return OK;
}

status_t MockCasFactory::createPlugin(
        int32_t CA_system_id,
        void* /*appData*/,
        CasPluginCallback /*callback*/,
        CasPlugin **plugin) {
    if (!isSystemIdSupported(CA_system_id)) {
        return BAD_VALUE;
    }

    *plugin = new MockCasPlugin();
    return OK;
}

status_t MockCasFactory::createPlugin(
        int32_t CA_system_id,
        void* /*appData*/,
        CasPluginCallbackExt /*callback*/,
        CasPlugin **plugin) {
    if (!isSystemIdSupported(CA_system_id)) {
        return BAD_VALUE;
    }

    *plugin = new MockCasPlugin();
    return OK;
}

///////////////////////////////////////////////////////////////////////////////

bool MockDescramblerFactory::isSystemIdSupported(int32_t CA_system_id) const {
    return CA_system_id == sMockId;
}

status_t MockDescramblerFactory::createPlugin(
        int32_t CA_system_id, DescramblerPlugin** plugin) {
    if (!isSystemIdSupported(CA_system_id)) {
        return BAD_VALUE;
    }

    *plugin = new MockDescramblerPlugin();
    return OK;
}

///////////////////////////////////////////////////////////////////////////////

static String8 arrayToString(const std::vector<uint8_t> &array) {
    String8 result;
    for (size_t i = 0; i < array.size(); i++) {
        result.appendFormat("%02x ", array[i]);
    }
    if (result.isEmpty()) {
        result.append("(null)");
    }
    return result;
}

MockCasPlugin::MockCasPlugin() {
    ALOGV("CTOR");
}

MockCasPlugin::~MockCasPlugin() {
    ALOGV("DTOR");
    MockSessionLibrary::get()->destroyPlugin(this);
}

status_t MockCasPlugin::setPrivateData(const CasData& /*data*/) {
    ALOGV("setPrivateData");
    return OK;
}

status_t MockCasPlugin::openSession(CasSessionId* sessionId) {
    ALOGV("openSession");
    return MockSessionLibrary::get()->addSession(this, sessionId);
}

status_t MockCasPlugin::closeSession(const CasSessionId &sessionId) {
    ALOGV("closeSession: sessionId=%s", arrayToString(sessionId).string());
    Mutex::Autolock lock(mLock);

    sp<MockCasSession> session =
            MockSessionLibrary::get()->findSession(sessionId);
    if (session == NULL) {
        return BAD_VALUE;
    }

    MockSessionLibrary::get()->destroySession(sessionId);
    return OK;
}

status_t MockCasPlugin::setSessionPrivateData(
        const CasSessionId &sessionId, const CasData& /*data*/) {
    ALOGV("setSessionPrivateData: sessionId=%s",
            arrayToString(sessionId).string());
    Mutex::Autolock lock(mLock);

    sp<MockCasSession> session =
            MockSessionLibrary::get()->findSession(sessionId);
    if (session == NULL) {
        return BAD_VALUE;
    }
    return OK;
}

status_t MockCasPlugin::processEcm(
        const CasSessionId &sessionId, const CasEcm& ecm) {
    ALOGV("processEcm: sessionId=%s", arrayToString(sessionId).string());
    Mutex::Autolock lock(mLock);

    sp<MockCasSession> session =
            MockSessionLibrary::get()->findSession(sessionId);
    if (session == NULL) {
        return BAD_VALUE;
    }
    ALOGV("ECM: size=%zu", ecm.size());
    ALOGV("ECM: data=%s", arrayToString(ecm).string());

    return OK;
}

status_t MockCasPlugin::processEmm(const CasEmm& emm) {
    ALOGV("processEmm");
    Mutex::Autolock lock(mLock);

    ALOGV("EMM: size=%zu", emm.size());
    ALOGV("EMM: data=%s", arrayToString(emm).string());

    return OK;
}

status_t MockCasPlugin::sendEvent(
        int32_t event, int /*arg*/, const CasData& /*eventData*/) {
    ALOGV("sendEvent: event=%d", event);
    Mutex::Autolock lock(mLock);

    return OK;
}

status_t MockCasPlugin::sendSessionEvent(
        const CasSessionId &sessionId, int32_t event,
        int /*arg*/, const CasData& /*eventData*/) {
    ALOGV("sendSessionEvent: sessionId=%s, event=%d",
          arrayToString(sessionId).string(), event);
    Mutex::Autolock lock(mLock);

    return OK;
}

status_t MockCasPlugin::provision(const String8 &str) {
    ALOGV("provision: provisionString=%s", str.string());
    Mutex::Autolock lock(mLock);

    return OK;
}

status_t MockCasPlugin::refreshEntitlements(
        int32_t /*refreshType*/, const CasData &refreshData) {
    ALOGV("refreshEntitlements: refreshData=%s", arrayToString(refreshData).string());
    Mutex::Autolock lock(mLock);

    return OK;
}

/////////////////////////////////////////////////////////////////
bool MockDescramblerPlugin::requiresSecureDecoderComponent(
        const char *mime) const {
    ALOGV("MockDescramblerPlugin::requiresSecureDecoderComponent"
            "(mime=%s)", mime);
    return false;
}

status_t MockDescramblerPlugin::setMediaCasSession(
        const CasSessionId &sessionId) {
    ALOGV("MockDescramblerPlugin::setMediaCasSession");
    sp<MockCasSession> session =
            MockSessionLibrary::get()->findSession(sessionId);

    if (session == NULL) {
        ALOGE("MockDescramblerPlugin: session not found");
        return ERROR_DRM_SESSION_NOT_OPENED;
    }

    return OK;
}

ssize_t MockDescramblerPlugin::descramble(
        bool secure,
        ScramblingControl scramblingControl,
        size_t numSubSamples,
        const SubSample *subSamples,
        const void *srcPtr,
        int32_t srcOffset,
        void *dstPtr,
        int32_t dstOffset,
        AString* /*errorDetailMsg*/) {
    ALOGV("MockDescramblerPlugin::descramble(secure=%d, sctrl=%d,"
          "subSamples=%s, srcPtr=%p, dstPtr=%p, srcOffset=%d, dstOffset=%d)",
          (int)secure, (int)scramblingControl,
          subSamplesToString(subSamples, numSubSamples).string(),
          srcPtr, dstPtr, srcOffset, dstOffset);

    return 0;
}

// Conversion utilities
String8 MockDescramblerPlugin::arrayToString(
        uint8_t const *array, size_t len) const
{
    String8 result("{ ");
    for (size_t i = 0; i < len; i++) {
        result.appendFormat("0x%02x ", array[i]);
    }
    result += "}";
    return result;
}

String8 MockDescramblerPlugin::subSamplesToString(
        SubSample const *subSamples, size_t numSubSamples) const
{
    String8 result;
    for (size_t i = 0; i < numSubSamples; i++) {
        result.appendFormat("[%zu] {clear:%u, encrypted:%u} ", i,
                            subSamples[i].mNumBytesOfClearData,
                            subSamples[i].mNumBytesOfEncryptedData);
    }
    return result;
}

} // namespace android

