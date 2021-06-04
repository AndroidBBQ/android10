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

#ifndef RSOV_ELEMENT_H
#define RSOV_ELEMENT_H

#include "rs_hal.h"

extern bool rsovElementInit(const android::renderscript::Context *rsc,
                            const android::renderscript::Element *);

extern void rsovElementDestroy(const android::renderscript::Context *rsc,
                               const android::renderscript::Element *);

extern void rsovElementUpdateCachedObject(
    const android::renderscript::Context *rsc,
    const android::renderscript::Element *,
    android::renderscript::rs_element *obj);

#endif  // RSOV_ELEMENT_H
