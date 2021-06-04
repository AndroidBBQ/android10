/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef A_MPEG2_TS_ASSEMBLER_H_

#define A_MPEG2_TS_ASSEMBLER_H_

#include "ARTPAssembler.h"

namespace android {

struct AMessage;
struct AString;

struct AMPEG2TSAssembler : public ARTPAssembler {
    AMPEG2TSAssembler(
            const sp<AMessage> &notify,
            const char *desc, const AString &params);

protected:
    virtual ~AMPEG2TSAssembler();

    virtual AssemblyStatus assembleMore(const sp<ARTPSource> &source);
    virtual void onByeReceived();
    virtual void packetLost();

private:
    sp<AMessage> mNotifyMsg;
    bool mNextExpectedSeqNoValid;
    uint32_t mNextExpectedSeqNo;

    AssemblyStatus addPacket(const sp<ARTPSource> &source);

    DISALLOW_EVIL_CONSTRUCTORS(AMPEG2TSAssembler);
};

}  // namespace android

#endif  // A_MPEG2_TS_ASSEMBLER_H_

