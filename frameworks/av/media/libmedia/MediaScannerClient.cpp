/*
 * Copyright (C) 2009 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaScannerClient"
#include <utils/Log.h>

#include <media/mediascanner.h>

#include <media/CharacterEncodingDetector.h>
#include <media/StringArray.h>

namespace android {

MediaScannerClient::MediaScannerClient() {
}

MediaScannerClient::~MediaScannerClient() {
}

void MediaScannerClient::setLocale(const char* locale)
{
    mLocale = locale; // not currently used
}

void MediaScannerClient::beginFile() {
}

status_t MediaScannerClient::addStringTag(const char* name, const char* value)
{
    handleStringTag(name, value);
    return OK;
}

void MediaScannerClient::endFile() {
}

}  // namespace android
