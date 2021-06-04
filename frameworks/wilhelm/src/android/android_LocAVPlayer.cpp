/*
 * Copyright (C) 2011 The Android Open Source Project
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

//#define USE_LOG SLAndroidLogLevel_Verbose

#include "sles_allinclusive.h"

#include <media/IMediaHTTPService.h>
#include <media/IMediaPlayerService.h>

#include "android_LocAVPlayer.h"

#include "HTTPHelper.h"

namespace android {

//--------------------------------------------------------------------------------------------------
LocAVPlayer::LocAVPlayer(const AudioPlayback_Parameters* params, bool hasVideo) :
        GenericMediaPlayer(params, hasVideo)
{
    SL_LOGD("LocAVPlayer::LocAVPlayer()");

}


LocAVPlayer::~LocAVPlayer() {
    SL_LOGD("LocAVPlayer::~LocAVPlayer()");

}


//--------------------------------------------------
// Event handlers
void LocAVPlayer::onPrepare() {
    SL_LOGD("LocAVPlayer::onPrepare()");
    sp<IMediaPlayerService> mediaPlayerService(getMediaPlayerService());
    if (mediaPlayerService != NULL) {
        switch (mDataLocatorType) {
        case kDataLocatorUri:
            mPlayer = mediaPlayerService->create(mPlayerClient /*IMediaPlayerClient*/,
                    mPlaybackParams.sessionId);
            if (mPlayer == NULL) {
                SL_LOGE("media player service failed to create player by URI");
            } else {
                sp <IMediaHTTPService> mediaHTTPService;
#ifndef __BRILLO__
                // As Brillo doesn't have a Java layer, we don't have to call this
                // function since it would return NULL anyways. Not having this
                // function call allows us to significantly reduce the size of the
                // Brillo checkout.
                mediaHTTPService = CreateHTTPServiceInCurrentJavaContext();
#endif
                status_t status =  mPlayer->setDataSource(
                    mediaHTTPService, mDataLocator.uriRef, NULL /*headers*/);
                if (status != NO_ERROR) {
                    SL_LOGE("setDataSource failed");
                    mPlayer.clear();
                }
            }
            break;
        case kDataLocatorFd:
            mPlayer = mediaPlayerService->create(mPlayerClient /*IMediaPlayerClient*/,
                    mPlaybackParams.sessionId);
            if (mPlayer == NULL) {
                SL_LOGE("media player service failed to create player by FD");
            } else if (mPlayer->setDataSource(mDataLocator.fdi.fd, mDataLocator.fdi.offset,
                    mDataLocator.fdi.length) != NO_ERROR) {
                SL_LOGE("setDataSource failed");
                mPlayer.clear();
            }
            // Binder dups the fd for use by mediaserver, so if we own the fd then OK to close now
            if (mDataLocator.fdi.mCloseAfterUse) {
                (void) ::close(mDataLocator.fdi.fd);
                mDataLocator.fdi.fd = -1;
                mDataLocator.fdi.mCloseAfterUse = false;
            }
            break;
        case kDataLocatorNone:
            SL_LOGE("no data locator for MediaPlayer object");
            break;
        default:
            SL_LOGE("unsupported data locator %d for MediaPlayer object", mDataLocatorType);
            break;
        }
    }
    if (mPlayer == NULL) {
        mStateFlags |= kFlagPreparedUnsuccessfully;
    }
    // blocks until mPlayer is prepared
    GenericMediaPlayer::onPrepare();
    SL_LOGD("LocAVPlayer::onPrepare() done");
}

} // namespace android
