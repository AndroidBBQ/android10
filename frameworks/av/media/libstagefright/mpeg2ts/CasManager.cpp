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
#define LOG_TAG "CasManager"
#include "CasManager.h"

#include <android/hardware/cas/1.0/ICas.h>
#include <android/hardware/cas/1.0/IMediaCasService.h>
#include <android/hardware/cas/native/1.0/IDescrambler.h>
#include <hidl/HidlSupport.h>
#include <media/stagefright/foundation/ABitReader.h>
#include <utils/Log.h>

namespace android {

using hardware::hidl_vec;
using hardware::Return;
using namespace hardware::cas::V1_0;
using namespace hardware::cas::native::V1_0;

struct ATSParser::CasManager::ProgramCasManager : public RefBase {
    ProgramCasManager(unsigned programNumber, const CADescriptor &descriptor);
    ProgramCasManager(unsigned programNumber);

    bool addStream(unsigned elementaryPID, const CADescriptor &descriptor);

    status_t setMediaCas(const sp<ICas> &cas, PidToSessionMap &sessionMap);

    bool getCasSession(unsigned elementaryPID,
            sp<IDescrambler> *descrambler, std::vector<uint8_t> *sessionId) const;

    void closeAllSessions(const sp<ICas>& cas);

private:
    struct CasSession {
        CasSession() {}
        CasSession(const CADescriptor &descriptor) :
            mCADescriptor(descriptor) {}

        CADescriptor mCADescriptor;
        std::vector<uint8_t> mSessionId;
        sp<IDescrambler> mDescrambler;
    };
    status_t initSession(
            const sp<ICas>& cas,
            PidToSessionMap &sessionMap,
            CasSession *session);
    void closeSession(const sp<ICas>& cas, const CasSession &casSession);

    unsigned mProgramNumber;
    bool mHasProgramCas;
    CasSession mProgramCas;
    KeyedVector<unsigned, CasSession> mStreamPidToCasMap;
};

ATSParser::CasManager::ProgramCasManager::ProgramCasManager(
        unsigned programNumber, const CADescriptor &descriptor) :
    mProgramNumber(programNumber),
    mHasProgramCas(true),
    mProgramCas(descriptor) {}

ATSParser::CasManager::ProgramCasManager::ProgramCasManager(
        unsigned programNumber) :
    mProgramNumber(programNumber),
    mHasProgramCas(false) {}

bool ATSParser::CasManager::ProgramCasManager::addStream(
        unsigned elementaryPID, const CADescriptor &descriptor) {
    ssize_t index = mStreamPidToCasMap.indexOfKey(elementaryPID);
    if (index >= 0) {
        return false;
    }
    ALOGV("addStream: program=%d, elementaryPID=%d, CA_system_ID=0x%x",
            mProgramNumber, elementaryPID, descriptor.mSystemID);
    mStreamPidToCasMap.add(elementaryPID, CasSession(descriptor));
    return true;
}

status_t ATSParser::CasManager::ProgramCasManager::setMediaCas(
        const sp<ICas> &cas, PidToSessionMap &sessionMap) {
    if (mHasProgramCas) {
        return initSession(cas, sessionMap, &mProgramCas);
    }
    // TODO: share session among streams that has identical CA_descriptors.
    // For now, we open one session for each stream that has CA_descriptor.
    for (size_t index = 0; index < mStreamPidToCasMap.size(); index++) {
        status_t err = initSession(
                cas, sessionMap, &mStreamPidToCasMap.editValueAt(index));
        if (err != OK) {
            return err;
        }
    }
    return OK;
}

bool ATSParser::CasManager::ProgramCasManager::getCasSession(
        unsigned elementaryPID, sp<IDescrambler> *descrambler,
        std::vector<uint8_t> *sessionId) const {
    if (mHasProgramCas) {
        *descrambler = mProgramCas.mDescrambler;
        *sessionId = mProgramCas.mSessionId;
        return true;
    }
    ssize_t index = mStreamPidToCasMap.indexOfKey(elementaryPID);
    if (index < 0) {
        return false;
    }

    *descrambler = mStreamPidToCasMap[index].mDescrambler;
    *sessionId = mStreamPidToCasMap[index].mSessionId;
    return true;
}

status_t ATSParser::CasManager::ProgramCasManager::initSession(
         const sp<ICas>& cas,
         PidToSessionMap &sessionMap,
         CasSession *session) {
    sp<IMediaCasService> casService = IMediaCasService::getService("default");
    if (casService == NULL) {
        ALOGE("Cannot obtain IMediaCasService");
        return NO_INIT;
    }

    Status status;
    sp<IDescrambler> descrambler;
    sp<IDescramblerBase> descramblerBase;
    Return<Status> returnStatus(Status::OK);
    Return<sp<IDescramblerBase> > returnDescrambler(NULL);
    std::vector<uint8_t> sessionId;
    const CADescriptor &descriptor = session->mCADescriptor;

    auto returnVoid = cas->openSession(
            [&status, &sessionId] (Status _status, const hidl_vec<uint8_t>& _sessionId) {
                status = _status;
                sessionId = _sessionId;
            });
    if (!returnVoid.isOk() || status != Status::OK) {
        ALOGE("Failed to open session: trans=%s, status=%d",
                returnVoid.description().c_str(), status);
        goto l_fail;
    }

    returnStatus = cas->setSessionPrivateData(sessionId, descriptor.mPrivateData);
    if (!returnStatus.isOk() || returnStatus != Status::OK) {
        ALOGE("Failed to set private data: trans=%s, status=%d",
                returnStatus.description().c_str(), (Status)returnStatus);
        goto l_fail;
    }

    returnDescrambler = casService->createDescrambler(descriptor.mSystemID);
    if (!returnDescrambler.isOk()) {
        ALOGE("Failed to create descrambler: trans=%s",
                returnDescrambler.description().c_str());
        goto l_fail;
    }
    descramblerBase = (sp<IDescramblerBase>) returnDescrambler;
    if (descramblerBase == NULL) {
        ALOGE("Failed to create descrambler: null ptr");
        goto l_fail;
    }

    returnStatus = descramblerBase->setMediaCasSession(sessionId);
    if (!returnStatus.isOk() || (Status) returnStatus != Status::OK) {
        ALOGE("Failed to init descrambler: : trans=%s, status=%d",
                returnStatus.description().c_str(), (Status) returnStatus);
        goto l_fail;
    }

    descrambler = IDescrambler::castFrom(descramblerBase);
    if (descrambler == NULL) {
        ALOGE("Failed to cast from IDescramblerBase to IDescrambler");
        goto l_fail;
    }

    session->mSessionId = sessionId;
    session->mDescrambler = descrambler;
    sessionMap.add(descriptor.mPID, sessionId);

    return OK;

l_fail:
    if (!sessionId.empty()) {
        cas->closeSession(sessionId);
    }
    if (descramblerBase != NULL) {
        descramblerBase->release();
    }
    return NO_INIT;
}

void ATSParser::CasManager::ProgramCasManager::closeSession(
        const sp<ICas>& cas, const CasSession &casSession) {
    if (casSession.mDescrambler != NULL) {
        casSession.mDescrambler->release();
    }
    if (!casSession.mSessionId.empty()) {
        cas->closeSession(casSession.mSessionId);
    }
}

void ATSParser::CasManager::ProgramCasManager::closeAllSessions(
        const sp<ICas>& cas) {
    if (mHasProgramCas) {
        closeSession(cas, mProgramCas);
    }
    for (size_t index = 0; index < mStreamPidToCasMap.size(); index++) {
        closeSession(cas, mStreamPidToCasMap.editValueAt(index));
    }
}

////////////////////////////////////////////////////////////////////////////////

ATSParser::CasManager::CasManager() : mSystemId(-1) {}

ATSParser::CasManager::~CasManager() {
    // Explictly close the sessions opened by us, since the CAS object is owned
    // by the app and may not go away after the parser is destroyed, and the app
    // may not have information about the sessions.
    if (mICas != NULL) {
        for (size_t index = 0; index < mProgramCasMap.size(); index++) {
            mProgramCasMap.editValueAt(index)->closeAllSessions(mICas);
        }
    }
}

bool ATSParser::CasManager::setSystemId(int32_t CA_system_ID) {
    if (mSystemId == -1) {
        // Verify the CA_system_ID is within range on the first program
        if (CA_system_ID < 0 || CA_system_ID > 0xffff) {
            ALOGE("Invalid CA_system_id: %d", CA_system_ID);
            return false;
        }
        mSystemId = CA_system_ID;
    } else if (mSystemId != CA_system_ID) {
        // All sessions need to be under the same CA system
        ALOGE("Multiple CA systems not allowed: %d vs %d",
                mSystemId, CA_system_ID);
        return false;
    }
    return true;
}

status_t ATSParser::CasManager::setMediaCas(const sp<ICas> &cas) {
    if (cas == NULL) {
        ALOGE("setMediaCas: received NULL object");
        return BAD_VALUE;
    }
    if (mICas != NULL) {
        ALOGW("setMediaCas: already set");
        return ALREADY_EXISTS;
    }
    for (size_t index = 0; index < mProgramCasMap.size(); index++) {
        status_t err;
        if ((err = mProgramCasMap.editValueAt(
                index)->setMediaCas(cas, mCAPidToSessionIdMap)) != OK) {
            return err;
        }
    }
    mICas = cas;
    return OK;
}

bool ATSParser::CasManager::addProgram(
        unsigned programNumber, const CADescriptor &descriptor) {
    if (!setSystemId(descriptor.mSystemID)) {
        return false;
    }

    ssize_t index = mProgramCasMap.indexOfKey(programNumber);
    if (index < 0) {
        ALOGV("addProgram: programNumber=%d, CA_system_ID=0x%x",
                programNumber, descriptor.mSystemID);
        mProgramCasMap.add(programNumber,
                new ProgramCasManager(programNumber, descriptor));
        mCAPidSet.insert(descriptor.mPID);
    }
    return true;
}

bool ATSParser::CasManager::addStream(
        unsigned programNumber, unsigned elementaryPID,
        const CADescriptor &descriptor) {
    if (!setSystemId(descriptor.mSystemID)) {
        return false;
    }

    ssize_t index = mProgramCasMap.indexOfKey(programNumber);
    sp<ProgramCasManager> programCasManager;
    if (index < 0) {
        ALOGV("addProgram (no CADescriptor): programNumber=%d", programNumber);
        programCasManager = new ProgramCasManager(programNumber);
        mProgramCasMap.add(programNumber, programCasManager);
    } else {
        programCasManager = mProgramCasMap.editValueAt(index);
    }
    if (programCasManager->addStream(elementaryPID, descriptor)) {
        mCAPidSet.insert(descriptor.mPID);
    }
    return true;
}

bool ATSParser::CasManager::getCasInfo(
        unsigned programNumber, unsigned elementaryPID,
        int32_t *systemId, sp<IDescrambler> *descrambler,
        std::vector<uint8_t> *sessionId) const {
    ssize_t index = mProgramCasMap.indexOfKey(programNumber);
    if (index < 0) {
        return false;
    }
    *systemId = mSystemId;
    return mProgramCasMap[index]->getCasSession(
            elementaryPID, descrambler, sessionId);
}

bool ATSParser::CasManager::isCAPid(unsigned pid) {
    return mCAPidSet.find(pid) != mCAPidSet.end();
}

bool ATSParser::CasManager::parsePID(ABitReader *br, unsigned pid) {
    ssize_t index = mCAPidToSessionIdMap.indexOfKey(pid);
    if (index < 0) {
        return false;
    }
    hidl_vec<uint8_t> ecm;
    ecm.setToExternal((uint8_t*)br->data(), br->numBitsLeft() / 8);
    auto returnStatus = mICas->processEcm(mCAPidToSessionIdMap[index], ecm);
    if (!returnStatus.isOk() || (Status) returnStatus != Status::OK) {
        ALOGE("Failed to process ECM: trans=%s, status=%d",
                returnStatus.description().c_str(), (Status) returnStatus);
    }
    return true; // handled
}

}  // namespace android
