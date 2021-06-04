/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "CameraParams2"
// #define LOG_NDEBUG 0
#include <utils/Log.h>

#include <string.h>
#include <stdlib.h>
#include <camera/CameraParameters2.h>

namespace android {

CameraParameters2::CameraParameters2()
                : mMap()
{
}

CameraParameters2::~CameraParameters2()
{
}

String8 CameraParameters2::flatten() const
{
    String8 flattened("");
    size_t size = mMap.size();

    for (size_t i = 0; i < size; i++) {
        String8 k, v;
        k = mMap.keyAt(i);
        v = mMap.valueAt(i);

        flattened += k;
        flattened += "=";
        flattened += v;
        if (i != size-1)
            flattened += ";";
    }

    ALOGV("%s: Flattened params = %s", __FUNCTION__, flattened.string());

    return flattened;
}

void CameraParameters2::unflatten(const String8 &params)
{
    const char *a = params.string();
    const char *b;

    mMap.clear();

    for (;;) {
        // Find the bounds of the key name.
        b = strchr(a, '=');
        if (b == 0)
            break;

        // Create the key string.
        String8 k(a, (size_t)(b-a));

        // Find the value.
        a = b+1;
        b = strchr(a, ';');
        if (b == 0) {
            // If there's no semicolon, this is the last item.
            String8 v(a);
            mMap.add(k, v);
            break;
        }

        String8 v(a, (size_t)(b-a));
        mMap.add(k, v);
        a = b+1;
    }
}


void CameraParameters2::set(const char *key, const char *value)
{
    // XXX i think i can do this with strspn()
    if (strchr(key, '=') || strchr(key, ';')) {
        //XXX ALOGE("Key \"%s\"contains invalid character (= or ;)", key);
        return;
    }

    if (strchr(value, '=') || strchr(value, ';')) {
        //XXX ALOGE("Value \"%s\"contains invalid character (= or ;)", value);
        return;
    }

    // Replacing a value updates the key's order to be the new largest order
    ssize_t res = mMap.replaceValueFor(String8(key), String8(value));
    LOG_ALWAYS_FATAL_IF(res < 0, "replaceValueFor(%s,%s) failed", key, value);
}

void CameraParameters2::set(const char *key, int value)
{
    char str[16];
    sprintf(str, "%d", value);
    set(key, str);
}

void CameraParameters2::setFloat(const char *key, float value)
{
    char str[16];  // 14 should be enough. We overestimate to be safe.
    snprintf(str, sizeof(str), "%g", value);
    set(key, str);
}

const char *CameraParameters2::get(const char *key) const
{
    ssize_t idx = mMap.indexOfKey(String8(key));
    if (idx < 0) {
        return NULL;
    } else {
        return mMap.valueAt(idx).string();
    }
}

int CameraParameters2::getInt(const char *key) const
{
    const char *v = get(key);
    if (v == 0)
        return -1;
    return strtol(v, 0, 0);
}

float CameraParameters2::getFloat(const char *key) const
{
    const char *v = get(key);
    if (v == 0) return -1;
    return strtof(v, 0);
}

status_t CameraParameters2::compareSetOrder(const char *key1, const char *key2,
        int *order) const {
    if (key1 == NULL) {
        ALOGE("%s: key1 must not be NULL", __FUNCTION__);
        return BAD_VALUE;
    } else if (key2 == NULL) {
        ALOGE("%s: key2 must not be NULL", __FUNCTION__);
        return BAD_VALUE;
    } else if (order == NULL) {
        ALOGE("%s: order must not be NULL", __FUNCTION__);
        return BAD_VALUE;
    }

    ssize_t index1 = mMap.indexOfKey(String8(key1));
    ssize_t index2 = mMap.indexOfKey(String8(key2));
    if (index1 < 0) {
        ALOGW("%s: Key1 (%s) was not set", __FUNCTION__, key1);
        return NAME_NOT_FOUND;
    } else if (index2 < 0) {
        ALOGW("%s: Key2 (%s) was not set", __FUNCTION__, key2);
        return NAME_NOT_FOUND;
    }

    *order = (index1 == index2) ? 0  :
             (index1 < index2)  ? -1 :
             1;

    return OK;
}

void CameraParameters2::remove(const char *key)
{
    mMap.removeItem(String8(key));
}

// Parse string like "640x480" or "10000,20000"
static int parse_pair(const char *str, int *first, int *second, char delim,
                      char **endptr = NULL)
{
    // Find the first integer.
    char *end;
    int w = (int)strtol(str, &end, 10);
    // If a delimeter does not immediately follow, give up.
    if (*end != delim) {
        ALOGE("Cannot find delimeter (%c) in str=%s", delim, str);
        return -1;
    }

    // Find the second integer, immediately after the delimeter.
    int h = (int)strtol(end+1, &end, 10);

    *first = w;
    *second = h;

    if (endptr) {
        *endptr = end;
    }

    return 0;
}

static void parseSizesList(const char *sizesStr, Vector<Size> &sizes)
{
    if (sizesStr == 0) {
        return;
    }

    char *sizeStartPtr = (char *)sizesStr;

    while (true) {
        int width, height;
        int success = parse_pair(sizeStartPtr, &width, &height, 'x',
                                 &sizeStartPtr);
        if (success == -1 || (*sizeStartPtr != ',' && *sizeStartPtr != '\0')) {
            ALOGE("Picture sizes string \"%s\" contains invalid character.", sizesStr);
            return;
        }
        sizes.push(Size(width, height));

        if (*sizeStartPtr == '\0') {
            return;
        }
        sizeStartPtr++;
    }
}

void CameraParameters2::setPreviewSize(int width, int height)
{
    char str[32];
    sprintf(str, "%dx%d", width, height);
    set(CameraParameters::KEY_PREVIEW_SIZE, str);
}

void CameraParameters2::getPreviewSize(int *width, int *height) const
{
    *width = *height = -1;
    // Get the current string, if it doesn't exist, leave the -1x-1
    const char *p = get(CameraParameters::KEY_PREVIEW_SIZE);
    if (p == 0)  return;
    parse_pair(p, width, height, 'x');
}

void CameraParameters2::getPreferredPreviewSizeForVideo(int *width, int *height) const
{
    *width = *height = -1;
    const char *p = get(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO);
    if (p == 0)  return;
    parse_pair(p, width, height, 'x');
}

void CameraParameters2::getSupportedPreviewSizes(Vector<Size> &sizes) const
{
    const char *previewSizesStr = get(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES);
    parseSizesList(previewSizesStr, sizes);
}

void CameraParameters2::setVideoSize(int width, int height)
{
    char str[32];
    sprintf(str, "%dx%d", width, height);
    set(CameraParameters::KEY_VIDEO_SIZE, str);
}

void CameraParameters2::getVideoSize(int *width, int *height) const
{
    *width = *height = -1;
    const char *p = get(CameraParameters::KEY_VIDEO_SIZE);
    if (p == 0) return;
    parse_pair(p, width, height, 'x');
}

void CameraParameters2::getSupportedVideoSizes(Vector<Size> &sizes) const
{
    const char *videoSizesStr = get(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES);
    parseSizesList(videoSizesStr, sizes);
}

void CameraParameters2::setPreviewFrameRate(int fps)
{
    set(CameraParameters::KEY_PREVIEW_FRAME_RATE, fps);
}

int CameraParameters2::getPreviewFrameRate() const
{
    return getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE);
}

void CameraParameters2::getPreviewFpsRange(int *min_fps, int *max_fps) const
{
    *min_fps = *max_fps = -1;
    const char *p = get(CameraParameters::KEY_PREVIEW_FPS_RANGE);
    if (p == 0) return;
    parse_pair(p, min_fps, max_fps, ',');
}

void CameraParameters2::setPreviewFpsRange(int min_fps, int max_fps)
{
    String8 str = String8::format("%d,%d", min_fps, max_fps);
    set(CameraParameters::KEY_PREVIEW_FPS_RANGE, str.string());
}

void CameraParameters2::setPreviewFormat(const char *format)
{
    set(CameraParameters::KEY_PREVIEW_FORMAT, format);
}

const char *CameraParameters2::getPreviewFormat() const
{
    return get(CameraParameters::KEY_PREVIEW_FORMAT);
}

void CameraParameters2::setPictureSize(int width, int height)
{
    char str[32];
    sprintf(str, "%dx%d", width, height);
    set(CameraParameters::KEY_PICTURE_SIZE, str);
}

void CameraParameters2::getPictureSize(int *width, int *height) const
{
    *width = *height = -1;
    // Get the current string, if it doesn't exist, leave the -1x-1
    const char *p = get(CameraParameters::KEY_PICTURE_SIZE);
    if (p == 0) return;
    parse_pair(p, width, height, 'x');
}

void CameraParameters2::getSupportedPictureSizes(Vector<Size> &sizes) const
{
    const char *pictureSizesStr = get(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES);
    parseSizesList(pictureSizesStr, sizes);
}

void CameraParameters2::setPictureFormat(const char *format)
{
    set(CameraParameters::KEY_PICTURE_FORMAT, format);
}

const char *CameraParameters2::getPictureFormat() const
{
    return get(CameraParameters::KEY_PICTURE_FORMAT);
}

void CameraParameters2::dump() const
{
    ALOGD("dump: mMap.size = %zu", mMap.size());
    for (size_t i = 0; i < mMap.size(); i++) {
        String8 k, v;
        k = mMap.keyAt(i);
        v = mMap.valueAt(i);
        ALOGD("%s: %s\n", k.string(), v.string());
    }
}

status_t CameraParameters2::dump(int fd, const Vector<String16>& args) const
{
    (void)args;
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, 255, "CameraParameters2::dump: mMap.size = %zu\n", mMap.size());
    result.append(buffer);
    for (size_t i = 0; i < mMap.size(); i++) {
        String8 k, v;
        k = mMap.keyAt(i);
        v = mMap.valueAt(i);
        snprintf(buffer, 255, "\t%s: %s\n", k.string(), v.string());
        result.append(buffer);
    }
    write(fd, result.string(), result.size());
    return NO_ERROR;
}

}; // namespace android
