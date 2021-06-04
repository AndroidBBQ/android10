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

#include "ATSParser.h"
#include <utils/KeyedVector.h>
#include <set>

namespace android {
namespace hardware {
namespace cas {
namespace native {
namespace V1_0 {
struct IDescrambler;
}}}}
using hardware::cas::native::V1_0::IDescrambler;

struct ATSParser::CasManager : public RefBase {
    CasManager();
    virtual ~CasManager();

    status_t setMediaCas(const sp<ICas> &cas);

    bool addProgram(
            unsigned programNumber, const CADescriptor &descriptor);

    bool addStream(
            unsigned programNumber, unsigned elementaryPID,
            const CADescriptor &descriptor);

    bool getCasInfo(
            unsigned programNumber, unsigned elementaryPID,
            int32_t *systemId, sp<IDescrambler> *descrambler,
            std::vector<uint8_t> *sessionId) const;

    bool isCAPid(unsigned pid);

    bool parsePID(ABitReader *br, unsigned pid);

private:
    typedef KeyedVector<unsigned, std::vector<uint8_t> > PidToSessionMap;
    struct ProgramCasManager;

    bool setSystemId(int32_t CA_system_ID);

    int32_t mSystemId;
    sp<ICas> mICas;
    KeyedVector<unsigned, sp<ProgramCasManager> > mProgramCasMap;
    PidToSessionMap mCAPidToSessionIdMap;
    std::set<uint32_t> mCAPidSet;
};

}  // namespace android
