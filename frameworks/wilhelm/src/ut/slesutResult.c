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

#include <stdlib.h>
#include <SLES/OpenSLES.h>
#include "OpenSLESUT.h"


/** \brief Maximum result return code */

#define SLESUT_RESULT_MAX (SL_RESULT_CONTROL_LOST + 1)

/** \brief Array of strings correponding to each result code */

static const char * const slesutResultStrings[SLESUT_RESULT_MAX] = {
    "SL_RESULT_SUCCESS",
    "SL_RESULT_PRECONDITIONS_VIOLATED",
    "SL_RESULT_PARAMETER_INVALID",
    "SL_RESULT_MEMORY_FAILURE",
    "SL_RESULT_RESOURCE_ERROR",
    "SL_RESULT_RESOURCE_LOST",
    "SL_RESULT_IO_ERROR",
    "SL_RESULT_BUFFER_INSUFFICIENT",
    "SL_RESULT_CONTENT_CORRUPTED",
    "SL_RESULT_CONTENT_UNSUPPORTED",
    "SL_RESULT_CONTENT_NOT_FOUND",
    "SL_RESULT_PERMISSION_DENIED",
    "SL_RESULT_FEATURE_UNSUPPORTED",
    "SL_RESULT_INTERNAL_ERROR",
    "SL_RESULT_UNKNOWN_ERROR",
    "SL_RESULT_OPERATION_ABORTED",
    "SL_RESULT_CONTROL_LOST"
};


/** \brief Convert a result code to a string or NULL. */

const char *slesutResultToString(SLresult result)
{
    // note that SLresult is unsigned
    return result < SLESUT_RESULT_MAX ? slesutResultStrings[result] : NULL;
}
