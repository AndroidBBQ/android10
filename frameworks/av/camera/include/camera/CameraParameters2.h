/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_CAMERA_PARAMETERS2_H
#define ANDROID_HARDWARE_CAMERA_PARAMETERS2_H

#include <utils/Vector.h>
#include <utils/String8.h>
#include "CameraParameters.h"

namespace android {

/**
 * A copy of CameraParameters plus ABI-breaking changes. Needed
 * because some camera HALs directly link to CameraParameters and cannot
 * tolerate an ABI change.
 */
class CameraParameters2
{
public:
    CameraParameters2();
    CameraParameters2(const String8 &params) { unflatten(params); }
    ~CameraParameters2();

    String8 flatten() const;
    void unflatten(const String8 &params);

    void set(const char *key, const char *value);
    void set(const char *key, int value);
    void setFloat(const char *key, float value);
    // Look up string value by key.
    // -- The string remains valid until the next set/remove of the same key,
    //    or until the map gets cleared.
    const char *get(const char *key) const;
    int getInt(const char *key) const;
    float getFloat(const char *key) const;

    // Compare the order that key1 was set vs the order that key2 was set.
    //
    // Sets the order parameter to an integer less than, equal to, or greater
    // than zero if key1's set order was respectively, to be less than, to
    // match, or to be greater than key2's set order.
    //
    // Error codes:
    //  * NAME_NOT_FOUND - if either key has not been set previously
    //  * BAD_VALUE - if any of the parameters are NULL
    status_t compareSetOrder(const char *key1, const char *key2,
            /*out*/
            int *order) const;

    void remove(const char *key);

    void setPreviewSize(int width, int height);
    void getPreviewSize(int *width, int *height) const;
    void getSupportedPreviewSizes(Vector<Size> &sizes) const;

    // Set the dimensions in pixels to the given width and height
    // for video frames. The given width and height must be one
    // of the supported dimensions returned from
    // getSupportedVideoSizes(). Must not be called if
    // getSupportedVideoSizes() returns an empty Vector of Size.
    void setVideoSize(int width, int height);
    // Retrieve the current dimensions (width and height)
    // in pixels for video frames, which must be one of the
    // supported dimensions returned from getSupportedVideoSizes().
    // Must not be called if getSupportedVideoSizes() returns an
    // empty Vector of Size.
    void getVideoSize(int *width, int *height) const;
    // Retrieve a Vector of supported dimensions (width and height)
    // in pixels for video frames. If sizes returned from the method
    // is empty, the camera does not support calls to setVideoSize()
    // or getVideoSize(). In adddition, it also indicates that
    // the camera only has a single output, and does not have
    // separate output for video frames and preview frame.
    void getSupportedVideoSizes(Vector<Size> &sizes) const;
    // Retrieve the preferred preview size (width and height) in pixels
    // for video recording. The given width and height must be one of
    // supported preview sizes returned from getSupportedPreviewSizes().
    // Must not be called if getSupportedVideoSizes() returns an empty
    // Vector of Size. If getSupportedVideoSizes() returns an empty
    // Vector of Size, the width and height returned from this method
    // is invalid, and is "-1x-1".
    void getPreferredPreviewSizeForVideo(int *width, int *height) const;

    void setPreviewFrameRate(int fps);
    int getPreviewFrameRate() const;
    void getPreviewFpsRange(int *min_fps, int *max_fps) const;
    void setPreviewFpsRange(int min_fps, int max_fps);
    void setPreviewFormat(const char *format);
    const char *getPreviewFormat() const;
    void setPictureSize(int width, int height);
    void getPictureSize(int *width, int *height) const;
    void getSupportedPictureSizes(Vector<Size> &sizes) const;
    void setPictureFormat(const char *format);
    const char *getPictureFormat() const;

    void dump() const;
    status_t dump(int fd, const Vector<String16>& args) const;

private:

    // Quick and dirty map that maintains insertion order
    template <typename KeyT, typename ValueT>
    struct OrderedKeyedVector {

        ssize_t add(const KeyT& key, const ValueT& value) {
            return mList.add(Pair(key, value));
        }

        size_t size() const {
            return mList.size();
        }

        const KeyT& keyAt(size_t idx) const {
            return mList[idx].mKey;
        }

        const ValueT& valueAt(size_t idx) const {
            return mList[idx].mValue;
        }

        const ValueT& valueFor(const KeyT& key) const {
            ssize_t i = indexOfKey(key);
            LOG_ALWAYS_FATAL_IF(i<0, "%s: key not found", __PRETTY_FUNCTION__);

            return valueAt(i);
        }

        ssize_t indexOfKey(const KeyT& key) const {
                size_t vectorIdx = 0;
                for (; vectorIdx < mList.size(); ++vectorIdx) {
                    if (mList[vectorIdx].mKey == key) {
                        return (ssize_t) vectorIdx;
                    }
                }

                return NAME_NOT_FOUND;
        }

        ssize_t removeItem(const KeyT& key) {
            ssize_t vectorIdx = indexOfKey(key);

            if (vectorIdx < 0) {
                return vectorIdx;
            }

            return mList.removeAt(vectorIdx);
        }

        void clear() {
            mList.clear();
        }

        // Same as removing and re-adding. The key's index changes to max.
        ssize_t replaceValueFor(const KeyT& key, const ValueT& value) {
            removeItem(key);
            return add(key, value);
        }

    private:

        struct Pair {
            Pair() : mKey(), mValue() {}
            Pair(const KeyT& key, const ValueT& value) :
                    mKey(key),
                    mValue(value) {}
            KeyT   mKey;
            ValueT mValue;
        };

        Vector<Pair> mList;
    };

    /**
     * Order matters: Keys that are set() later are stored later in the map.
     *
     * If two keys have meaning that conflict, then the later-set key
     * wins.
     *
     * For example, preview FPS and preview FPS range conflict since only
     * we only want to use the FPS range if that's the last thing that was set.
     * So in that case, only use preview FPS range if it was set later than
     * the preview FPS.
     */
    OrderedKeyedVector<String8,String8>    mMap;
};

}; // namespace android

#endif
