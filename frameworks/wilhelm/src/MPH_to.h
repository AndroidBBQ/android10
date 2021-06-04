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

#ifndef __MPH_to_H
#define __MPH_to_H

/** \brief Map minimal perfect hash of an interface ID to its class index. */

extern const signed char
    MPH_to_3DGroup[MPH_MAX],
    MPH_to_AudioPlayer[MPH_MAX],
    MPH_to_AudioRecorder[MPH_MAX],
    MPH_to_Engine[MPH_MAX],
    MPH_to_LEDDevice[MPH_MAX],
    MPH_to_Listener[MPH_MAX],
    MPH_to_MetadataExtractor[MPH_MAX],
    MPH_to_MidiPlayer[MPH_MAX],
    MPH_to_OutputMix[MPH_MAX],
    MPH_to_Vibra[MPH_MAX],
    MPH_to_MediaPlayer[MPH_MAX];

/** \brief Maximum number of interfaces on a single object. */
#define MAX_INDEX 32

#endif // !defined(__MPH_to_H)
