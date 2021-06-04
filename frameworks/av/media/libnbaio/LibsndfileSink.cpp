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

#define LOG_TAG "LibsndfileSink"
//#define LOG_NDEBUG 0

#include <utils/Errors.h>
#include <utils/Log.h>
#include <media/nbaio/LibsndfileSink.h>

namespace android {

LibsndfileSink::LibsndfileSink(SNDFILE *sndfile, const SF_INFO &sfinfo) :
    NBAIO_Sink(Format_from_SR_C(sfinfo.samplerate, sfinfo.channels)),
    mSndfile(sndfile)
{
}

LibsndfileSink::~LibsndfileSink()
{
    // do not close mSndfile; we don't own it
}

ssize_t LibsndfileSink::write(const void *buffer, size_t count)
{
    if (!mNegotiated) {
        return (ssize_t) NEGOTIATE;
    }
    if (mSndfile == NULL) {
        return (ssize_t) NO_INIT;
    }
    sf_count_t actual = sf_writef_short(mSndfile, (short *) buffer, (sf_count_t) count);
    mFramesWritten += actual;
    return actual;
}

}   // namespace android
