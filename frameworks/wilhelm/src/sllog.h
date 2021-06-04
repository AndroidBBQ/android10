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

// Logging

// In order of decreasing priority, the log priority levels are:
//    Assert
//    E(rror)
//    W(arn)
//    I(nfo)
//    D(ebug)
//    V(erbose)
// Debug and verbose are usually compiled out except during development.

/** These values match the definitions in system/core/include/cutils/log.h */
#define SLAndroidLogLevel_Unknown 0
#define SLAndroidLogLevel_Default 1
#define SLAndroidLogLevel_Verbose 2
#define SLAndroidLogLevel_Debug   3
#define SLAndroidLogLevel_Info    4
#define SLAndroidLogLevel_Warn    5
#define SLAndroidLogLevel_Error   6
#define SLAndroidLogLevel_Fatal   7
#define SLAndroidLogLevel_Silent  8

// USE_LOG is the minimum log priority level that is enabled at build time.
// It is configured in Android.mk but can be overridden per source file.
#ifndef USE_LOG
#define USE_LOG SLAndroidLogLevel_Info
#endif


#ifdef ANDROID

// The usual Android LOGx macros are not available, so we use the lower-level APIs.
extern const char slLogTag[];

// can't disable SL_LOGF
#define SL_LOGF(...) __android_log_print(SLAndroidLogLevel_Fatal, slLogTag, __VA_ARGS__)

#if (USE_LOG <= SLAndroidLogLevel_Error)
#define SL_LOGE(...) __android_log_print(SLAndroidLogLevel_Error, slLogTag, __VA_ARGS__)
#else
#define SL_LOGE(...) do { } while (0)
#endif

#if (USE_LOG <= SLAndroidLogLevel_Warn)
#define SL_LOGW(...) __android_log_print(SLAndroidLogLevel_Warn, slLogTag, __VA_ARGS__)
#else
#define SL_LOGW(...) do { } while (0)
#endif

#if (USE_LOG <= SLAndroidLogLevel_Info)
#define SL_LOGI(...) __android_log_print(SLAndroidLogLevel_Info, slLogTag, __VA_ARGS__)
#else
#define SL_LOGI(...) do { } while (0)
#endif

#if (USE_LOG <= SLAndroidLogLevel_Debug)
#define SL_LOGD(...) __android_log_print(SLAndroidLogLevel_Debug, slLogTag, __VA_ARGS__)
#else
#define SL_LOGD(...) do { } while (0)
#endif

#if (USE_LOG <= SLAndroidLogLevel_Verbose)
#define SL_LOGV(...) __android_log_print(SLAndroidLogLevel_Verbose, slLogTag, __VA_ARGS__)
#else
#define SL_LOGV(...) do { } while (0)
#endif

#else   // !defined(ANDROID)

#if (USE_LOG <= SLAndroidLogLevel_Error)
#define SL_LOGE(...) do { fprintf(stderr, "SL_LOGE: %s:%s:%d ", __FILE__, __FUNCTION__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } while(0)
#else
#define SL_LOGE(...) do { } while (0)
#endif

#if (USE_LOG <= SLAndroidLogLevel_Warn)
#define SL_LOGW(...) do { fprintf(stderr, "SL_LOGW: %s:%s:%d ", __FILE__, __FUNCTION__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } while(0)
#else
#define SL_LOGW(...) do { } while (0)
#endif

#if (USE_LOG <= SLAndroidLogLevel_Info)
#define SL_LOGI(...) do { fprintf(stderr, "SL_LOGI: %s:%s:%d ", __FILE__, __FUNCTION__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } while(0)
#else
#define SL_LOGI(...) do { } while (0)
#endif

#if (USE_LOG <= SLAndroidLogLevel_Debug)
#define SL_LOGD(...) do { fprintf(stderr, "SL_LOGD: %s:%s:%d ", __FILE__, __FUNCTION__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } while(0)
#else
#define SL_LOGD(...) do { } while (0)
#endif

#if (USE_LOG <= SLAndroidLogLevel_Verbose)
#define SL_LOGV(...) do { fprintf(stderr, "SL_LOGV: %s:%s:%d ", __FILE__, __FUNCTION__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } while(0)
#else
#define SL_LOGV(...) do { } while (0)
#endif

#endif  // ANDROID
