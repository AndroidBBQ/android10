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

#include "rsovCore.h"

#include "rsAllocation.h"
#include "rsContext.h"
#include "rsScript.h"
#include "rsScriptGroup.h"
#include "rsd_cpu.h"

using android::renderscript::Allocation;
using android::renderscript::Context;
using android::renderscript::RsdCpuReference;
using android::renderscript::ScriptGroup;
using android::renderscript::ScriptGroupBase;
using android::renderscript::ScriptKernelID;
using android::renderscript::rs_script_group;

bool rsovScriptGroupInit(const Context *rsc, ScriptGroupBase *sg) {
  // Always falls back to CPU implmentation of ScriptGroup
  RSoVHal *dc = (RSoVHal *)rsc->mHal.drv;

  sg->mHal.drv = dc->mCpuRef->createScriptGroup(sg);
  return sg->mHal.drv != nullptr;
}

void rsovScriptGroupSetInput(const Context *rsc, const ScriptGroup *sg,
                             const ScriptKernelID *kid, Allocation *) {}

void rsovScriptGroupSetOutput(const Context *rsc, const ScriptGroup *sg,
                              const ScriptKernelID *kid, Allocation *) {}

void rsovScriptGroupExecute(const Context *rsc, const ScriptGroupBase *sg) {
  RsdCpuReference::CpuScriptGroupBase *sgi =
      (RsdCpuReference::CpuScriptGroupBase *)sg->mHal.drv;
  sgi->execute();
}

void rsovScriptGroupDestroy(const Context *rsc, const ScriptGroupBase *sg) {
  RsdCpuReference::CpuScriptGroupBase *sgi =
      (RsdCpuReference::CpuScriptGroupBase *)sg->mHal.drv;
  delete sgi;
}
