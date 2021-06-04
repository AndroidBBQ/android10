/*
 * Copyright (C) 2014 The Android Open Source Project
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
#pragma once
#include "aacdecoder_lib.h"

typedef enum
{
    DRC_PRES_MODE_WRAP_DESIRED_TARGET         = 0x0000,
    DRC_PRES_MODE_WRAP_DESIRED_ATT_FACTOR     = 0x0001,
    DRC_PRES_MODE_WRAP_DESIRED_BOOST_FACTOR   = 0x0002,
    DRC_PRES_MODE_WRAP_DESIRED_HEAVY          = 0x0003,
    DRC_PRES_MODE_WRAP_ENCODER_TARGET         = 0x0004
} DRC_PRES_MODE_WRAP_PARAM;


class CDrcPresModeWrapper {
public:
    CDrcPresModeWrapper();
    ~CDrcPresModeWrapper();
    void setDecoderHandle(const HANDLE_AACDECODER handle);
    void setParam(const DRC_PRES_MODE_WRAP_PARAM param, const int value);
    void submitStreamData(CStreamInfo*);
    void update();

protected:
    HANDLE_AACDECODER mHandleDecoder;
    int mDesTarget;
    int mDesAttFactor;
    int mDesBoostFactor;
    int mDesHeavy;

    int mEncoderTarget;

    int mLastTarget;
    int mLastAttFactor;
    int mLastBoostFactor;
    int mLastHeavy;

    SCHAR mStreamPRL;
    SCHAR mStreamDRCPresMode;
    INT mStreamNrAACChan;
    INT mStreamNrOutChan;

    bool mIsDownmix;
    bool mIsMonoDownmix;
    bool mIsStereoDownmix;

    bool mDataUpdate;
};
