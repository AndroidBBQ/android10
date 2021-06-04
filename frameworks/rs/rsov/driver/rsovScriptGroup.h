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

#ifndef RSOV_SCRIPT_GROUP_H
#define RSOV_SCRIPT_GROUP_H

#include "rs_hal.h"

bool rsovScriptGroupInit(const android::renderscript::Context *rsc,
                         android::renderscript::ScriptGroupBase *sg);
void rsovScriptGroupSetInput(const android::renderscript::Context *rsc,
                             const android::renderscript::ScriptGroup *sg,
                             const android::renderscript::ScriptKernelID *kid,
                             android::renderscript::Allocation *);
void rsovScriptGroupSetOutput(const android::renderscript::Context *rsc,
                              const android::renderscript::ScriptGroup *sg,
                              const android::renderscript::ScriptKernelID *kid,
                              android::renderscript::Allocation *);
void rsovScriptGroupExecute(const android::renderscript::Context *rsc,
                            const android::renderscript::ScriptGroupBase *sg);
void rsovScriptGroupDestroy(const android::renderscript::Context *rsc,
                            const android::renderscript::ScriptGroupBase *sg);

#endif  // RSOV_SCRIPT_GROUP_H
