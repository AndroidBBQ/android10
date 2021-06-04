/*
 * Copyright (C) 2010 The Android Open Source Project
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

/* trace debugging */

#include "sles_allinclusive.h"

#ifdef USE_TRACE

// This should be the only global variable
static unsigned slTraceEnabled = SL_TRACE_DEFAULT;


void slTraceSetEnabled(unsigned enabled)
{
    slTraceEnabled = enabled;
}


void slTraceEnterGlobal(const char *function)
{
    if (SL_TRACE_ENTER & slTraceEnabled) {
        SL_LOGD("Entering %s", function);
    }
}


void slTraceLeaveGlobal(const char *function, SLresult result)
{
    if (SL_RESULT_SUCCESS == result) {
        if (SL_TRACE_LEAVE_SUCCESS & slTraceEnabled) {
            SL_LOGD("Leaving %s", function);
        }
    } else {
        if (SL_TRACE_LEAVE_FAILURE & slTraceEnabled) {
            const char *str = slesutResultToString(result);
            if (NULL != str) {
                SL_LOGW("Leaving %s (%s)", function, str);
            } else {
                SL_LOGW("Leaving %s (0x%X)", function, result);
            }
        }
    }
}


void slTraceEnterInterface(const char *function)
{
    if (!(SL_TRACE_ENTER & slTraceEnabled)) {
        return;
    }
    if (*function == 'I') {
        ++function;
    }
    const char *underscore = function;
    while (*underscore != '\0') {
        if (*underscore == '_') {
            if (/*(strcmp(function, "BufferQueue_Enqueue") &&
                strcmp(function, "BufferQueue_GetState") &&
                strcmp(function, "OutputMixExt_FillBuffer")) &&*/
                true) {
                SL_LOGD("Entering %.*s::%s", (int) (underscore - function), function,
                    &underscore[1]);
            }
            return;
        }
        ++underscore;
    }
    SL_LOGV("Entering %s", function);
}


void slTraceLeaveInterface(const char *function, SLresult result)
{
    if (!((SL_TRACE_LEAVE_SUCCESS | SL_TRACE_LEAVE_FAILURE) & slTraceEnabled)) {
        return;
    }
    if (*function == 'I') {
        ++function;
    }
    const char *underscore = function;
    while (*underscore != '\0') {
        if (*underscore == '_') {
            break;
        }
        ++underscore;
    }
    if (SL_RESULT_SUCCESS == result) {
        if (SL_TRACE_LEAVE_SUCCESS & slTraceEnabled) {
            if (*underscore == '_') {
                SL_LOGD("Leaving %.*s::%s", (int) (underscore - function), function,
                    &underscore[1]);
            } else {
                SL_LOGD("Leaving %s", function);
            }
        }
    } else {
        if (SL_TRACE_LEAVE_FAILURE & slTraceEnabled) {
            const char *str = slesutResultToString(result);
            if (*underscore == '_') {
                if (NULL != str) {
                    SL_LOGW("Leaving %.*s::%s (%s)", (int) (underscore - function), function,
                        &underscore[1], str);
                } else {
                    SL_LOGW("Leaving %.*s::%s (0x%X)", (int) (underscore - function), function,
                        &underscore[1], result);
                }
            } else {
                if (NULL != str) {
                    SL_LOGW("Leaving %s (%s)", function, str);
                } else {
                    SL_LOGW("Leaving %s (0x%X)", function, result);
                }
            }
        }
    }
}


void slTraceEnterInterfaceVoid(const char *function)
{
    if (SL_TRACE_ENTER & slTraceEnabled) {
        slTraceEnterInterface(function);
    }
}


void slTraceLeaveInterfaceVoid(const char *function)
{
    if (SL_TRACE_LEAVE_VOID & slTraceEnabled) {
        slTraceLeaveInterface(function, SL_RESULT_SUCCESS);
    }
}

#else

void slTraceSetEnabled(unsigned enabled)
{
}

#endif // USE_TRACE
