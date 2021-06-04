/*
 * Copyright (C) 2018 The Android Open Source Project
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

#define LOG_TAG "Camera3-DistMapper"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <algorithm>
#include <cmath>

#include "device3/DistortionMapper.h"

namespace android {

namespace camera3 {

/**
 * Metadata keys to correct when adjusting coordinates for distortion correction
 */

// Both capture request and result
constexpr std::array<uint32_t, 3> DistortionMapper::kMeteringRegionsToCorrect = {
    ANDROID_CONTROL_AF_REGIONS,
    ANDROID_CONTROL_AE_REGIONS,
    ANDROID_CONTROL_AWB_REGIONS
};

// Only capture request
constexpr std::array<uint32_t, 1> DistortionMapper::kRequestRectsToCorrect = {
    ANDROID_SCALER_CROP_REGION,
};

// Only for capture result
constexpr std::array<uint32_t, 1> DistortionMapper::kResultRectsToCorrect = {
    ANDROID_SCALER_CROP_REGION,
};

// Only for capture result
constexpr std::array<uint32_t, 2> DistortionMapper::kResultPointsToCorrectNoClamp = {
    ANDROID_STATISTICS_FACE_RECTANGLES, // Says rectangles, is really points
    ANDROID_STATISTICS_FACE_LANDMARKS,
};


DistortionMapper::DistortionMapper() : mValidMapping(false), mValidGrids(false) {
}

bool DistortionMapper::isDistortionSupported(const CameraMetadata &result) {
    bool isDistortionCorrectionSupported = false;
    camera_metadata_ro_entry_t distortionCorrectionModes =
            result.find(ANDROID_DISTORTION_CORRECTION_AVAILABLE_MODES);
    for (size_t i = 0; i < distortionCorrectionModes.count; i++) {
        if (distortionCorrectionModes.data.u8[i] !=
                ANDROID_DISTORTION_CORRECTION_MODE_OFF) {
            isDistortionCorrectionSupported = true;
            break;
        }
    }
    return isDistortionCorrectionSupported;
}

status_t DistortionMapper::setupStaticInfo(const CameraMetadata &deviceInfo) {
    std::lock_guard<std::mutex> lock(mMutex);
    camera_metadata_ro_entry_t array;

    array = deviceInfo.find(ANDROID_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE);
    if (array.count != 4) return BAD_VALUE;

    float arrayX = static_cast<float>(array.data.i32[0]);
    float arrayY = static_cast<float>(array.data.i32[1]);
    mArrayWidth = static_cast<float>(array.data.i32[2]);
    mArrayHeight = static_cast<float>(array.data.i32[3]);

    array = deviceInfo.find(ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE);
    if (array.count != 4) return BAD_VALUE;

    float activeX = static_cast<float>(array.data.i32[0]);
    float activeY = static_cast<float>(array.data.i32[1]);
    mActiveWidth = static_cast<float>(array.data.i32[2]);
    mActiveHeight = static_cast<float>(array.data.i32[3]);

    mArrayDiffX = activeX - arrayX;
    mArrayDiffY = activeY - arrayY;

    return updateCalibration(deviceInfo);
}

bool DistortionMapper::calibrationValid() const {
    std::lock_guard<std::mutex> lock(mMutex);

    return mValidMapping;
}

status_t DistortionMapper::correctCaptureRequest(CameraMetadata *request) {
    std::lock_guard<std::mutex> lock(mMutex);
    status_t res;

    if (!mValidMapping) return OK;

    camera_metadata_entry_t e;
    e = request->find(ANDROID_DISTORTION_CORRECTION_MODE);
    if (e.count != 0 && e.data.u8[0] != ANDROID_DISTORTION_CORRECTION_MODE_OFF) {
        for (auto region : kMeteringRegionsToCorrect) {
            e = request->find(region);
            for (size_t j = 0; j < e.count; j += 5) {
                int32_t weight = e.data.i32[j + 4];
                if (weight == 0) {
                    continue;
                }
                res = mapCorrectedToRaw(e.data.i32 + j, 2, /*clamp*/true);
                if (res != OK) return res;
            }
        }
        for (auto rect : kRequestRectsToCorrect) {
            e = request->find(rect);
            res = mapCorrectedRectToRaw(e.data.i32, e.count / 4, /*clamp*/true);
            if (res != OK) return res;
        }
    }

    return OK;
}

status_t DistortionMapper::correctCaptureResult(CameraMetadata *result) {
    std::lock_guard<std::mutex> lock(mMutex);
    status_t res;

    if (!mValidMapping) return OK;

    res = updateCalibration(*result);
    if (res != OK) {
        ALOGE("Failure to update lens calibration information");
        return INVALID_OPERATION;
    }

    camera_metadata_entry_t e;
    e = result->find(ANDROID_DISTORTION_CORRECTION_MODE);
    if (e.count != 0 && e.data.u8[0] != ANDROID_DISTORTION_CORRECTION_MODE_OFF) {
        for (auto region : kMeteringRegionsToCorrect) {
            e = result->find(region);
            for (size_t j = 0; j < e.count; j += 5) {
                int32_t weight = e.data.i32[j + 4];
                if (weight == 0) {
                    continue;
                }
                res = mapRawToCorrected(e.data.i32 + j, 2, /*clamp*/true);
                if (res != OK) return res;
            }
        }
        for (auto rect : kResultRectsToCorrect) {
            e = result->find(rect);
            res = mapRawRectToCorrected(e.data.i32, e.count / 4, /*clamp*/true);
            if (res != OK) return res;
        }
        for (auto pts : kResultPointsToCorrectNoClamp) {
            e = result->find(pts);
            res = mapRawToCorrected(e.data.i32, e.count / 2, /*clamp*/false);
            if (res != OK) return res;
        }
    }

    return OK;
}

// Utility methods; not guarded by mutex

status_t DistortionMapper::updateCalibration(const CameraMetadata &result) {
    camera_metadata_ro_entry_t calib, distortion;

    calib = result.find(ANDROID_LENS_INTRINSIC_CALIBRATION);
    distortion = result.find(ANDROID_LENS_DISTORTION);

    if (calib.count != 5) return BAD_VALUE;
    if (distortion.count != 5) return BAD_VALUE;

    // Skip redoing work if no change to calibration fields
    if (mValidMapping &&
            mFx == calib.data.f[0] &&
            mFy == calib.data.f[1] &&
            mCx == calib.data.f[2] &&
            mCy == calib.data.f[3] &&
            mS == calib.data.f[4]) {
        bool noChange = true;
        for (size_t i = 0; i < distortion.count; i++) {
            if (mK[i] != distortion.data.f[i]) {
                noChange = false;
                break;
            }
        }
        if (noChange) return OK;
    }

    mFx = calib.data.f[0];
    mFy = calib.data.f[1];
    mCx = calib.data.f[2];
    mCy = calib.data.f[3];
    mS = calib.data.f[4];

    mInvFx = 1 / mFx;
    mInvFy = 1 / mFy;

    for (size_t i = 0; i < distortion.count; i++) {
        mK[i] = distortion.data.f[i];
    }

    mValidMapping = true;
    // Need to recalculate grid
    mValidGrids = false;

    return OK;
}

status_t DistortionMapper::mapRawToCorrected(int32_t *coordPairs, int coordCount,
        bool clamp, bool simple) {
    if (!mValidMapping) return INVALID_OPERATION;

    if (simple) return mapRawToCorrectedSimple(coordPairs, coordCount, clamp);

    if (!mValidGrids) {
        status_t res = buildGrids();
        if (res != OK) return res;
    }

    for (int i = 0; i < coordCount * 2; i += 2) {
        const GridQuad *quad = findEnclosingQuad(coordPairs + i, mDistortedGrid);
        if (quad == nullptr) {
            ALOGE("Raw to corrected mapping failure: No quad found for (%d, %d)",
                    *(coordPairs + i), *(coordPairs + i + 1));
            return INVALID_OPERATION;
        }
        ALOGV("src xy: %d, %d, enclosing quad: (%f, %f), (%f, %f), (%f, %f), (%f, %f)",
                coordPairs[i], coordPairs[i+1],
                quad->coords[0], quad->coords[1],
                quad->coords[2], quad->coords[3],
                quad->coords[4], quad->coords[5],
                quad->coords[6], quad->coords[7]);

        const GridQuad *corrQuad = quad->src;
        if (corrQuad == nullptr) {
            ALOGE("Raw to corrected mapping failure: No src quad found");
            return INVALID_OPERATION;
        }
        ALOGV("              corr quad: (%f, %f), (%f, %f), (%f, %f), (%f, %f)",
                corrQuad->coords[0], corrQuad->coords[1],
                corrQuad->coords[2], corrQuad->coords[3],
                corrQuad->coords[4], corrQuad->coords[5],
                corrQuad->coords[6], corrQuad->coords[7]);

        float u = calculateUorV(coordPairs + i, *quad, /*calculateU*/ true);
        float v = calculateUorV(coordPairs + i, *quad, /*calculateU*/ false);

        ALOGV("uv: %f, %f", u, v);

        // Interpolate along top edge of corrected quad (which are axis-aligned) for x
        float corrX = corrQuad->coords[0] + u * (corrQuad->coords[2] - corrQuad->coords[0]);
        // Interpolate along left edge of corrected quad (which are axis-aligned) for y
        float corrY = corrQuad->coords[1] + v * (corrQuad->coords[7] - corrQuad->coords[1]);

        // Clamp to within active array
        if (clamp) {
            corrX = std::min(mActiveWidth - 1, std::max(0.f, corrX));
            corrY = std::min(mActiveHeight - 1, std::max(0.f, corrY));
        }

        coordPairs[i] = static_cast<int32_t>(std::round(corrX));
        coordPairs[i + 1] = static_cast<int32_t>(std::round(corrY));
    }

    return OK;
}

status_t DistortionMapper::mapRawToCorrectedSimple(int32_t *coordPairs, int coordCount,
        bool clamp) const {
    if (!mValidMapping) return INVALID_OPERATION;

    float scaleX = mActiveWidth / mArrayWidth;
    float scaleY = mActiveHeight / mArrayHeight;
    for (int i = 0; i < coordCount * 2; i += 2) {
        float x = coordPairs[i];
        float y = coordPairs[i + 1];
        float corrX = x * scaleX;
        float corrY = y * scaleY;
        if (clamp) {
            corrX = std::min(mActiveWidth - 1, std::max(0.f, corrX));
            corrY = std::min(mActiveHeight - 1, std::max(0.f, corrY));
        }
        coordPairs[i] = static_cast<int32_t>(std::round(corrX));
        coordPairs[i + 1] = static_cast<int32_t>(std::round(corrY));
    }

    return OK;
}

status_t DistortionMapper::mapRawRectToCorrected(int32_t *rects, int rectCount, bool clamp,
        bool simple) {
    if (!mValidMapping) return INVALID_OPERATION;
    for (int i = 0; i < rectCount * 4; i += 4) {
        // Map from (l, t, width, height) to (l, t, r, b)
        int32_t coords[4] = {
            rects[i],
            rects[i + 1],
            rects[i] + rects[i + 2] - 1,
            rects[i + 1] + rects[i + 3] - 1
        };

        mapRawToCorrected(coords, 2, clamp, simple);

        // Map back to (l, t, width, height)
        rects[i] = coords[0];
        rects[i + 1] = coords[1];
        rects[i + 2] = coords[2] - coords[0] + 1;
        rects[i + 3] = coords[3] - coords[1] + 1;
    }

    return OK;
}

status_t DistortionMapper::mapCorrectedToRaw(int32_t *coordPairs, int coordCount, bool clamp,
        bool simple) const {
    return mapCorrectedToRawImpl(coordPairs, coordCount, clamp, simple);
}

template<typename T>
status_t DistortionMapper::mapCorrectedToRawImpl(T *coordPairs, int coordCount, bool clamp,
        bool simple) const {
    if (!mValidMapping) return INVALID_OPERATION;

    if (simple) return mapCorrectedToRawImplSimple(coordPairs, coordCount, clamp);

    float activeCx = mCx - mArrayDiffX;
    float activeCy = mCy - mArrayDiffY;
    for (int i = 0; i < coordCount * 2; i += 2) {
        // Move to normalized space from active array space
        float ywi = (coordPairs[i + 1] - activeCy) * mInvFy;
        float xwi = (coordPairs[i] - activeCx - mS * ywi) * mInvFx;
        // Apply distortion model to calculate raw image coordinates
        float rSq = xwi * xwi + ywi * ywi;
        float Fr = 1.f + (mK[0] * rSq) + (mK[1] * rSq * rSq) + (mK[2] * rSq * rSq * rSq);
        float xc = xwi * Fr + (mK[3] * 2 * xwi * ywi) + mK[4] * (rSq + 2 * xwi * xwi);
        float yc = ywi * Fr + (mK[4] * 2 * xwi * ywi) + mK[3] * (rSq + 2 * ywi * ywi);
        // Move back to image space
        float xr = mFx * xc + mS * yc + mCx;
        float yr = mFy * yc + mCy;
        // Clamp to within pre-correction active array
        if (clamp) {
            xr = std::min(mArrayWidth - 1, std::max(0.f, xr));
            yr = std::min(mArrayHeight - 1, std::max(0.f, yr));
        }

        coordPairs[i] = static_cast<T>(std::round(xr));
        coordPairs[i + 1] = static_cast<T>(std::round(yr));
    }

    return OK;
}

template<typename T>
status_t DistortionMapper::mapCorrectedToRawImplSimple(T *coordPairs, int coordCount,
        bool clamp) const {
    if (!mValidMapping) return INVALID_OPERATION;

    float scaleX = mArrayWidth / mActiveWidth;
    float scaleY = mArrayHeight / mActiveHeight;
    for (int i = 0; i < coordCount * 2; i += 2) {
        float x = coordPairs[i];
        float y = coordPairs[i + 1];
        float rawX = x * scaleX;
        float rawY = y * scaleY;
        if (clamp) {
            rawX = std::min(mArrayWidth - 1, std::max(0.f, rawX));
            rawY = std::min(mArrayHeight - 1, std::max(0.f, rawY));
        }
        coordPairs[i] = static_cast<T>(std::round(rawX));
        coordPairs[i + 1] = static_cast<T>(std::round(rawY));
    }

    return OK;
}


status_t DistortionMapper::mapCorrectedRectToRaw(int32_t *rects, int rectCount, bool clamp,
        bool simple) const {
    if (!mValidMapping) return INVALID_OPERATION;

    for (int i = 0; i < rectCount * 4; i += 4) {
        // Map from (l, t, width, height) to (l, t, r, b)
        int32_t coords[4] = {
            rects[i],
            rects[i + 1],
            rects[i] + rects[i + 2] - 1,
            rects[i + 1] + rects[i + 3] - 1
        };

        mapCorrectedToRaw(coords, 2, clamp, simple);

        // Map back to (l, t, width, height)
        rects[i] = coords[0];
        rects[i + 1] = coords[1];
        rects[i + 2] = coords[2] - coords[0] + 1;
        rects[i + 3] = coords[3] - coords[1] + 1;
    }

    return OK;
}

status_t DistortionMapper::buildGrids() {
    if (mCorrectedGrid.size() != kGridSize * kGridSize) {
        mCorrectedGrid.resize(kGridSize * kGridSize);
        mDistortedGrid.resize(kGridSize * kGridSize);
    }

    float gridMargin = mArrayWidth * kGridMargin;
    float gridSpacingX = (mArrayWidth + 2 * gridMargin) / kGridSize;
    float gridSpacingY = (mArrayHeight + 2 * gridMargin) / kGridSize;

    size_t index = 0;
    float x = -gridMargin;
    for (size_t i = 0; i < kGridSize; i++, x += gridSpacingX) {
        float y = -gridMargin;
        for (size_t j = 0; j < kGridSize; j++, y += gridSpacingY, index++) {
            mCorrectedGrid[index].src = nullptr;
            mCorrectedGrid[index].coords = {
                x, y,
                x + gridSpacingX, y,
                x + gridSpacingX, y + gridSpacingY,
                x, y + gridSpacingY
            };
            mDistortedGrid[index].src = &mCorrectedGrid[index];
            mDistortedGrid[index].coords = mCorrectedGrid[index].coords;
            status_t res = mapCorrectedToRawImpl(mDistortedGrid[index].coords.data(), 4,
                    /*clamp*/false, /*simple*/false);
            if (res != OK) return res;
        }
    }

    mValidGrids = true;
    return OK;
}

const DistortionMapper::GridQuad* DistortionMapper::findEnclosingQuad(
        const int32_t pt[2], const std::vector<GridQuad>& grid) {
    const float x = pt[0];
    const float y = pt[1];

    for (const GridQuad& quad : grid) {
        const float &x1 = quad.coords[0];
        const float &y1 = quad.coords[1];
        const float &x2 = quad.coords[2];
        const float &y2 = quad.coords[3];
        const float &x3 = quad.coords[4];
        const float &y3 = quad.coords[5];
        const float &x4 = quad.coords[6];
        const float &y4 = quad.coords[7];

        // Point-in-quad test:

        // Quad has corners P1-P4; if P is within the quad, then it is on the same side of all the
        // edges (or on top of one of the edges or corners), traversed in a consistent direction.
        // This means that the cross product of edge En = Pn->P(n+1 mod 4) and line Ep = Pn->P must
        // have the same sign (or be zero) for all edges.
        // For clockwise traversal, the sign should be negative or zero for Ep x En, indicating that
        // En is to the left of Ep, or overlapping.
        float s1 = (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1);
        if (s1 > 0) continue;
        float s2 = (x - x2) * (y3 - y2) - (y - y2) * (x3 - x2);
        if (s2 > 0) continue;
        float s3 = (x - x3) * (y4 - y3) - (y - y3) * (x4 - x3);
        if (s3 > 0) continue;
        float s4 = (x - x4) * (y1 - y4) - (y - y4) * (x1 - x4);
        if (s4 > 0) continue;

        return &quad;
    }
    return nullptr;
}

float DistortionMapper::calculateUorV(const int32_t pt[2], const GridQuad& quad, bool calculateU) {
    const float x = pt[0];
    const float y = pt[1];
    const float &x1 = quad.coords[0];
    const float &y1 = quad.coords[1];
    const float &x2 = calculateU ? quad.coords[2] : quad.coords[6];
    const float &y2 = calculateU ? quad.coords[3] : quad.coords[7];
    const float &x3 = quad.coords[4];
    const float &y3 = quad.coords[5];
    const float &x4 = calculateU ? quad.coords[6] : quad.coords[2];
    const float &y4 = calculateU ? quad.coords[7] : quad.coords[3];

    float a = (x1 - x2) * (y1 - y2 + y3 - y4) - (y1 - y2) * (x1 - x2 + x3 - x4);
    float b = (x - x1) * (y1 - y2 + y3 - y4) + (x1 - x2) * (y4 - y1) -
              (y - y1) * (x1 - x2 + x3 - x4) - (y1 - y2) * (x4 - x1);
    float c = (x - x1) * (y4 - y1) - (y - y1) * (x4 - x1);

    if (a == 0) {
        // One solution may happen if edges are parallel
        float u0 = -c / b;
        ALOGV("u0: %.9g, b: %f, c: %f", u0, b, c);
        return u0;
    }

    float det = b * b - 4 * a * c;
    if (det < 0) {
        // Sanity check - should not happen if pt is within the quad
        ALOGE("Bad determinant! a: %f, b: %f, c: %f, det: %f", a,b,c,det);
        return -1;
    }

    // Select more numerically stable solution
    float sqdet = b > 0 ? -std::sqrt(det) : std::sqrt(det);

    float u1 = (-b + sqdet) / (2 * a);
    ALOGV("u1: %.9g", u1);
    if (0 - kFloatFuzz < u1 && u1 < 1 + kFloatFuzz) return u1;

    float u2 = c / (a * u1);
    ALOGV("u2: %.9g", u2);
    if (0 - kFloatFuzz < u2 && u2 < 1 + kFloatFuzz) return u2;

    // Last resort, return the smaller-magnitude solution
    return fabs(u1) < fabs(u2) ? u1 : u2;
}

} // namespace camera3

} // namespace android
