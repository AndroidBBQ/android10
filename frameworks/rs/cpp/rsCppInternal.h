/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef ANDROID_RS_CPP_INTERNAL_H
#define ANDROID_RS_CPP_INTERNAL_H


#define LOG_TAG "rsC++"
#include "rsCppUtils.h"
#include "rsInternalDefines.h"
#include "rsDispatch.h"

#define tryDispatch(rs, dispatch)               \
    if ((rs)->getError() == RS_SUCCESS) {       \
        dispatch;                               \
    }

#define createDispatch(rs, dispatch) \
    rs->getError() == RS_SUCCESS ? (dispatch) : nullptr

#undef LOG_TAG
#undef LOG_NDEBUG
// Uncomment the following line to enable verbose debugging.
// #define LOG_NDEBUG 0
#define LOG_TAG "rsC++"

#endif
