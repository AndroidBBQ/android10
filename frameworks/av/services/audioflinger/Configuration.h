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

// Put build-time configuration options here rather than Android.mk,
// so that the instantiate for AudioFlinger service will pick up the same options.

#ifndef ANDROID_AUDIOFLINGER_CONFIGURATION_H
#define ANDROID_AUDIOFLINGER_CONFIGURATION_H

// uncomment to enable detailed battery usage reporting (not debugged)
//#define ADD_BATTERY_DATA

// uncomment to enable the audio watchdog
//#define AUDIO_WATCHDOG

// uncomment to display CPU load adjusted for CPU frequency
//#define CPU_FREQUENCY_STATISTICS

// uncomment to enable fast threads to take performance samples for later statistical analysis
#define FAST_THREAD_STATISTICS

// uncomment for debugging timing problems related to StateQueue::push()
//#define STATE_QUEUE_DUMP

// uncomment to allow tee sink debugging to be enabled by property
//#define TEE_SINK

// uncomment to log CPU statistics every n wall clock seconds
//#define DEBUG_CPU_USAGE 10

// define FLOAT_EFFECT_CHAIN to request float effects (falls back to int16_t if unavailable)
#define FLOAT_EFFECT_CHAIN

#ifdef FLOAT_EFFECT_CHAIN
// define FLOAT_AUX to process aux effect buffers in float (FLOAT_EFFECT_CHAIN must be defined)
#define FLOAT_AUX

// define MULTICHANNEL_EFFECT_CHAIN to allow multichannel effects (FLOAT_EFFECT_CHAIN defined)
#define MULTICHANNEL_EFFECT_CHAIN
#endif

#endif // ANDROID_AUDIOFLINGER_CONFIGURATION_H
