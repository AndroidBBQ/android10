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
#define LOG_TAG "Camera2-JpegCompressor"

#include <utils/Log.h>
#include <ui/GraphicBufferMapper.h>

#include "JpegCompressor.h"

namespace android {
namespace camera2 {

JpegCompressor::JpegCompressor():
        Thread(false),
        mIsBusy(false),
        mCaptureTime(0) {
}

JpegCompressor::~JpegCompressor() {
    ALOGV("%s", __FUNCTION__);
    Mutex::Autolock lock(mMutex);
}

status_t JpegCompressor::start(const Vector<CpuConsumer::LockedBuffer*>& buffers,
        nsecs_t captureTime) {
    ALOGV("%s", __FUNCTION__);
    Mutex::Autolock busyLock(mBusyMutex);

    if (mIsBusy) {
        ALOGE("%s: Already processing a buffer!", __FUNCTION__);
        return INVALID_OPERATION;
    }

    mIsBusy = true;

    mBuffers = buffers;
    mCaptureTime = captureTime;

    status_t res;
    res = run("JpegCompressor");
    if (res != OK) {
        ALOGE("%s: Unable to start up compression thread: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        //delete mBuffers;  // necessary?
    }
    return res;
}

status_t JpegCompressor::cancel() {
    ALOGV("%s", __FUNCTION__);
    requestExitAndWait();
    return OK;
}

status_t JpegCompressor::readyToRun() {
    ALOGV("%s", __FUNCTION__);
    return OK;
}

bool JpegCompressor::threadLoop() {
    ALOGV("%s", __FUNCTION__);

    mAuxBuffer = mBuffers[0];    // input
    mJpegBuffer = mBuffers[1];    // output

    // Set up error management
    mJpegErrorInfo = NULL;
    JpegError error;
    error.parent = this;

    mCInfo.err = jpeg_std_error(&error);
    mCInfo.err->error_exit = jpegErrorHandler;

    jpeg_create_compress(&mCInfo);
    if (checkError("Error initializing compression")) return false;

    // Route compressed data straight to output stream buffer
    JpegDestination jpegDestMgr;
    jpegDestMgr.parent = this;
    jpegDestMgr.init_destination = jpegInitDestination;
    jpegDestMgr.empty_output_buffer = jpegEmptyOutputBuffer;
    jpegDestMgr.term_destination = jpegTermDestination;

    mCInfo.dest = &jpegDestMgr;

    // Set up compression parameters
    mCInfo.image_width = mAuxBuffer->width;
    mCInfo.image_height = mAuxBuffer->height;
    mCInfo.input_components = 1; // 3;
    mCInfo.in_color_space = JCS_GRAYSCALE; // JCS_RGB

    ALOGV("%s: image_width = %d, image_height = %d", __FUNCTION__, mCInfo.image_width, mCInfo.image_height);

    jpeg_set_defaults(&mCInfo);
    if (checkError("Error configuring defaults")) return false;

    // Do compression
    jpeg_start_compress(&mCInfo, TRUE);
    if (checkError("Error starting compression")) return false;

    size_t rowStride = mAuxBuffer->stride;// * 3;
    const size_t kChunkSize = 32;
    while (mCInfo.next_scanline < mCInfo.image_height) {
        JSAMPROW chunk[kChunkSize];
        for (size_t i = 0 ; i < kChunkSize; i++) {
            chunk[i] = (JSAMPROW)
                    (mAuxBuffer->data + (i + mCInfo.next_scanline) * rowStride);
        }
        jpeg_write_scanlines(&mCInfo, chunk, kChunkSize);
        if (checkError("Error while compressing")) return false;
        if (exitPending()) {
            ALOGV("%s: Cancel called, exiting early", __FUNCTION__);
            cleanUp();
            return false;
        }
    }

    jpeg_finish_compress(&mCInfo);
    if (checkError("Error while finishing compression")) return false;

    cleanUp();
    return false;
}

bool JpegCompressor::isBusy() {
    ALOGV("%s", __FUNCTION__);
    Mutex::Autolock busyLock(mBusyMutex);
    return mIsBusy;
}

// old function -- TODO: update for new buffer type
bool JpegCompressor::isStreamInUse(uint32_t /*id*/) {
    ALOGV("%s", __FUNCTION__);
    Mutex::Autolock lock(mBusyMutex);

    if (mBuffers.size() && mIsBusy) {
        for (size_t i = 0; i < mBuffers.size(); i++) {
//            if ( mBuffers[i].streamId == (int)id ) return true;
        }
    }
    return false;
}

bool JpegCompressor::waitForDone(nsecs_t timeout) {
    ALOGV("%s", __FUNCTION__);
    Mutex::Autolock lock(mBusyMutex);
    status_t res = OK;
    if (mIsBusy) {
        res = mDone.waitRelative(mBusyMutex, timeout);
    }
    return (res == OK);
}

bool JpegCompressor::checkError(const char *msg) {
    ALOGV("%s", __FUNCTION__);
    if (mJpegErrorInfo) {
        char errBuffer[JMSG_LENGTH_MAX];
        mJpegErrorInfo->err->format_message(mJpegErrorInfo, errBuffer);
        ALOGE("%s: %s: %s",
                __FUNCTION__, msg, errBuffer);
        cleanUp();
        mJpegErrorInfo = NULL;
        return true;
    }
    return false;
}

void JpegCompressor::cleanUp() {
    ALOGV("%s", __FUNCTION__);
    jpeg_destroy_compress(&mCInfo);
    Mutex::Autolock lock(mBusyMutex);
    mIsBusy = false;
    mDone.signal();
}

void JpegCompressor::jpegErrorHandler(j_common_ptr cinfo) {
    ALOGV("%s", __FUNCTION__);
    JpegError *error = static_cast<JpegError*>(cinfo->err);
    error->parent->mJpegErrorInfo = cinfo;
}

void JpegCompressor::jpegInitDestination(j_compress_ptr cinfo) {
    ALOGV("%s", __FUNCTION__);
    JpegDestination *dest= static_cast<JpegDestination*>(cinfo->dest);
    ALOGV("%s: Setting destination to %p, size %zu",
            __FUNCTION__, dest->parent->mJpegBuffer->data, kMaxJpegSize);
    dest->next_output_byte = (JOCTET*)(dest->parent->mJpegBuffer->data);
    dest->free_in_buffer = kMaxJpegSize;
}

boolean JpegCompressor::jpegEmptyOutputBuffer(j_compress_ptr /*cinfo*/) {
    ALOGV("%s", __FUNCTION__);
    ALOGE("%s: JPEG destination buffer overflow!",
            __FUNCTION__);
    return true;
}

void JpegCompressor::jpegTermDestination(j_compress_ptr cinfo) {
    (void) cinfo; // TODO: clean up
    ALOGV("%s", __FUNCTION__);
    ALOGV("%s: Done writing JPEG data. %zu bytes left in buffer",
            __FUNCTION__, cinfo->dest->free_in_buffer);
}

}; // namespace camera2
}; // namespace android
