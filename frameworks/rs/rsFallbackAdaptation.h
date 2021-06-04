/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef ANDROID_RS_FALLBACK_ADAPTATION_H
#define ANDROID_RS_FALLBACK_ADAPTATION_H

struct dispatchTable;

/*
 * RsFallbackAdaptation is a singleton class, for accessing
 * CPU fallback implementation.
 */
class RsFallbackAdaptation
{
public:
    static RsFallbackAdaptation& GetInstance();
    static const dispatchTable* GetEntryFuncs();

private:
    RsFallbackAdaptation();

    // Dispatch table entries for CPU fallback. The dispatch table is initialized by the
    // constructor of this singleton class, to avoid concurrency issues.
    static dispatchTable mEntryFuncs;
};

#endif // ANDROID_RS_FALLBACK_ADAPTATION_H
