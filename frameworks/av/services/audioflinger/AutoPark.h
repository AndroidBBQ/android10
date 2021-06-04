/*
 * Copyright (C) 2016 The Android Open Source Project
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

namespace android {

// T is FastMixer or FastCapture
template<typename T> class AutoPark {
public:

    // Park the specific FastThread, which can be nullptr, in hot idle if not currently idling
    explicit AutoPark(const sp<T>& fastThread) : mFastThread(fastThread)
    {
        mPreviousCommand = FastThreadState::HOT_IDLE;
        if (fastThread != nullptr) {
            auto sq = mFastThread->sq();
            FastThreadState *state = sq->begin();
            if (!(state->mCommand & FastThreadState::IDLE)) {
                mPreviousCommand = state->mCommand;
                state->mCommand = FastThreadState::HOT_IDLE;
                sq->end();
                sq->push(sq->BLOCK_UNTIL_ACKED);
            } else {
                sq->end(false /*didModify*/);
            }
        }
    }

    // Remove the FastThread from hot idle if necessary
    ~AutoPark()
    {
        if (!(mPreviousCommand & FastThreadState::IDLE)) {
            ALOG_ASSERT(mFastThread != nullptr);
            auto sq = mFastThread->sq();
            FastThreadState *state = sq->begin();
            ALOG_ASSERT(state->mCommand == FastThreadState::HOT_IDLE);
            state->mCommand = mPreviousCommand;
            sq->end();
            sq->push(sq->BLOCK_UNTIL_PUSHED);
        }
    }

private:
    const sp<T>                 mFastThread;
    // if !&IDLE, holds the FastThread state to restore after new parameters processed
    FastThreadState::Command    mPreviousCommand;
};  // class AutoPark

}   // namespace
