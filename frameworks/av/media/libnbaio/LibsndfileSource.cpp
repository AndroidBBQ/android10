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

#define LOG_TAG "LibsndfileSource"
//#define LOG_NDEBUG 0

#include <utils/Errors.h>
#include <utils/Log.h>
#include <media/nbaio/LibsndfileSource.h>

namespace android {

LibsndfileSource::LibsndfileSource(SNDFILE *sndfile, const SF_INFO &sfinfo, bool loop) :
    NBAIO_Source(Format_from_SR_C(sfinfo.samplerate, sfinfo.channels)),
    mSndfile(sndfile),
    mEstimatedFramesUntilEOF(sfinfo.frames),
    mLooping(loop && sfinfo.seekable),
    mReadAnyFramesThisLoopCycle(false)
{
}

LibsndfileSource::~LibsndfileSource()
{
    // do not close mSndfile; we don't own it
}

ssize_t LibsndfileSource::availableToRead()
{
    // after we reach the presumed EOF, report infinity just in case there's actually more
    return !mLooping && mEstimatedFramesUntilEOF > 0 ? mEstimatedFramesUntilEOF : SSIZE_MAX;
}

ssize_t LibsndfileSource::read(void *buffer, size_t count)
{
    if (!mNegotiated) {
        return (ssize_t) NEGOTIATE;
    }
    if (mSndfile == NULL) {
        return (ssize_t) NO_INIT;
    }
    sf_count_t actual = sf_readf_short(mSndfile, (short *) buffer, (sf_count_t) count);
    // Detect EOF by zero frames read, not by mFramesUntilEOF as it could be inaccurate
    if (actual == 0) {
        if (mLooping) {
            if (mReadAnyFramesThisLoopCycle) {
                (void) sf_seek(mSndfile, (sf_count_t) 0, SEEK_SET);
                mReadAnyFramesThisLoopCycle = false;
            } else {
                // We didn't read any frames during the current loop cycle, so disable
                // further looping to prevent the caller from busy waiting at read().
                // This is especially important when looping an empty file.
                mLooping = false;
            }
        }
    } else {
        mFramesRead += actual;
        if (actual >= mEstimatedFramesUntilEOF) {
            mEstimatedFramesUntilEOF = 0;
        } else {
            mEstimatedFramesUntilEOF -= actual;
        }
        mReadAnyFramesThisLoopCycle = true;
    }
    return actual;
}

}   // namespace android
