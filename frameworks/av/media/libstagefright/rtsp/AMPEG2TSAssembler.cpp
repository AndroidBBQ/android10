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

//#define LOG_NDEBUG 0
#define LOG_TAG "AMPEG2TSAssembler"
#include <utils/Log.h>

#include "AMPEG2TSAssembler.h"

#include "ARTPSource.h"
#include "ASessionDescription.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>

namespace android {

AMPEG2TSAssembler::AMPEG2TSAssembler(
        const sp<AMessage> &notify,
        const char * /* desc */,
        const AString & /* params */)
    : mNotifyMsg(notify),
      mNextExpectedSeqNoValid(false),
      mNextExpectedSeqNo(0) {
}

AMPEG2TSAssembler::~AMPEG2TSAssembler() {
}

ARTPAssembler::AssemblyStatus AMPEG2TSAssembler::assembleMore(
        const sp<ARTPSource> &source) {
    return addPacket(source);
}

ARTPAssembler::AssemblyStatus AMPEG2TSAssembler::addPacket(
        const sp<ARTPSource> &source) {
    List<sp<ABuffer> > *queue = source->queue();

    if (queue->empty()) {
        return NOT_ENOUGH_DATA;
    }

    if (mNextExpectedSeqNoValid) {
        List<sp<ABuffer> >::iterator it = queue->begin();
        while (it != queue->end()) {
            if ((uint32_t)(*it)->int32Data() >= mNextExpectedSeqNo) {
                break;
            }

            it = queue->erase(it);
        }

        if (queue->empty()) {
            return NOT_ENOUGH_DATA;
        }
    }

    sp<ABuffer> buffer = *queue->begin();

    if (!mNextExpectedSeqNoValid) {
        mNextExpectedSeqNoValid = true;
        mNextExpectedSeqNo = (uint32_t)buffer->int32Data();
    } else if ((uint32_t)buffer->int32Data() != mNextExpectedSeqNo) {
        ALOGV("Not the sequence number I expected");

        return WRONG_SEQUENCE_NUMBER;
    }

    // hexdump(buffer->data(), buffer->size());

    if ((buffer->size() % 188) > 0) {
        queue->erase(queue->begin());
        ++mNextExpectedSeqNo;

        ALOGV("Not a multiple of transport packet size.");

        return MALFORMED_PACKET;
    }

    sp<AMessage> msg = mNotifyMsg->dup();
    msg->setBuffer("access-unit", buffer);
    msg->post();

    queue->erase(queue->begin());
    ++mNextExpectedSeqNo;

    return OK;
}

void AMPEG2TSAssembler::packetLost() {
    CHECK(mNextExpectedSeqNoValid);
    ++mNextExpectedSeqNo;
}

void AMPEG2TSAssembler::onByeReceived() {
    sp<AMessage> msg = mNotifyMsg->dup();
    msg->setInt32("eos", true);
    msg->post();
}

}  // namespace android


