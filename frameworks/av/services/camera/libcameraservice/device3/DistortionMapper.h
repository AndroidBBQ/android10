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

#ifndef ANDROID_SERVERS_DISTORTIONMAPPER_H
#define ANDROID_SERVERS_DISTORTIONMAPPER_H

#include <utils/Errors.h>
#include <array>
#include <mutex>

#include "camera/CameraMetadata.h"

namespace android {

namespace camera3 {

/**
 * Utilities to transform between raw (distorted) and warped (corrected) coordinate systems
 * for cameras that support geometric distortion
 */
class DistortionMapper {
  public:
    DistortionMapper();

    /**
     * Check whether distortion correction is supported by the camera HAL
     */
    static bool isDistortionSupported(const CameraMetadata &deviceInfo);

    /**
     * Update static lens calibration info from camera characteristics
     */
    status_t setupStaticInfo(const CameraMetadata &deviceInfo);

    /**
     * Return whether distortion correction can be applied currently
     */
    bool calibrationValid() const;

    /**
     * Correct capture request if distortion correction is enabled
     */
    status_t correctCaptureRequest(CameraMetadata *request);

    /**
     * Correct capture result if distortion correction is enabled
     */
    status_t correctCaptureResult(CameraMetadata *request);


  public: // Visible for testing. Not guarded by mutex; do not use concurrently
    /**
     * Update lens calibration from capture results or equivalent
     */
    status_t updateCalibration(const CameraMetadata &result);

    /**
     * Transform from distorted (original) to corrected (warped) coordinates.
     * Coordinates are transformed in-place
     *
     *   coordPairs: A pointer to an array of consecutive (x,y) points
     *   coordCount: Number of (x,y) pairs to transform
     *   clamp: Whether to clamp the result to the bounds of the active array
     *   simple: Whether to do complex correction or just a simple linear map
     */
    status_t mapRawToCorrected(int32_t *coordPairs, int coordCount, bool clamp,
            bool simple = true);

    /**
     * Transform from distorted (original) to corrected (warped) coordinates.
     * Coordinates are transformed in-place
     *
     *   rects: A pointer to an array of consecutive (x,y, w, h) rectangles
     *   rectCount: Number of rectangles to transform
     *   clamp: Whether to clamp the result to the bounds of the active array
     *   simple: Whether to do complex correction or just a simple linear map
     */
    status_t mapRawRectToCorrected(int32_t *rects, int rectCount, bool clamp,
            bool simple = true);

    /**
     * Transform from corrected (warped) to distorted (original) coordinates.
     * Coordinates are transformed in-place
     *
     *   coordPairs: A pointer to an array of consecutive (x,y) points
     *   coordCount: Number of (x,y) pairs to transform
     *   clamp: Whether to clamp the result to the bounds of the precorrection active array
     *   simple: Whether to do complex correction or just a simple linear map
     */
    status_t mapCorrectedToRaw(int32_t* coordPairs, int coordCount, bool clamp,
            bool simple = true) const;

    /**
     * Transform from corrected (warped) to distorted (original) coordinates.
     * Coordinates are transformed in-place
     *
     *   rects: A pointer to an array of consecutive (x,y, w, h) rectangles
     *   rectCount: Number of rectangles to transform
     *   clamp: Whether to clamp the result to the bounds of the precorrection active array
     *   simple: Whether to do complex correction or just a simple linear map
     */
    status_t mapCorrectedRectToRaw(int32_t *rects, int rectCount, bool clamp,
            bool simple = true) const;

    struct GridQuad {
        // Source grid quad, or null
        const GridQuad *src;
        // x,y coordinates of corners, in
        // clockwise order
        std::array<float, 8> coords;
    };

    // Find which grid quad encloses the point; returns null if none do
    static const GridQuad* findEnclosingQuad(
            const int32_t pt[2], const std::vector<GridQuad>& grid);

    // Calculate 'horizontal' interpolation coordinate for the point and the quad
    // Assumes the point P is within the quad Q.
    // Given quad with points P1-P4, and edges E12-E41, and considering the edge segments as
    // functions of U: E12(u), where E12(0) = P1 and E12(1) = P2, then we want to find a u
    // such that the edge E12(u) -> E43(u) contains point P.
    // This can be determined by checking if the cross product of vector [E12(u)-E43(u)] and
    // vector [E12(u)-P] is zero. Solving the equation
    // [E12(u)-E43(u)] x [E12(u)-P] = 0 gives a quadratic equation in u; the solution in the range
    // 0 to 1 is the one chosen.
    // If calculateU is true, then an interpolation coordinate for edges E12 and E43 is found;
    // if it is false, then an interpolation coordinate for edges E14 and E23 is found.
    static float calculateUorV(const int32_t pt[2], const GridQuad& quad, bool calculateU);

  private:
    mutable std::mutex mMutex;

    // Number of quads in each dimension of the mapping grids
    constexpr static size_t kGridSize = 15;
    // Margin to expand the grid by to ensure it doesn't clip the domain
    constexpr static float kGridMargin = 0.05f;
    // Fuzziness for float inequality tests
    constexpr static float kFloatFuzz = 1e-4;

    // Metadata key lists to correct

    // Both capture request and result
    static const std::array<uint32_t, 3> kMeteringRegionsToCorrect;

    // Only capture request
    static const std::array<uint32_t, 1> kRequestRectsToCorrect;

    // Only capture result
    static const std::array<uint32_t, 1> kResultRectsToCorrect;

    // Only for capture results; don't clamp
    static const std::array<uint32_t, 2> kResultPointsToCorrectNoClamp;

    // Single implementation for various mapCorrectedToRaw methods
    template<typename T>
    status_t mapCorrectedToRawImpl(T* coordPairs, int coordCount, bool clamp, bool simple) const;

    // Simple linear interpolation option
    template<typename T>
    status_t mapCorrectedToRawImplSimple(T* coordPairs, int coordCount, bool clamp) const;

    status_t mapRawToCorrectedSimple(int32_t *coordPairs, int coordCount, bool clamp) const;

    // Utility to create reverse mapping grids
    status_t buildGrids();


    bool mValidMapping;
    bool mValidGrids;

    // intrisic parameters, in pixels
    float mFx, mFy, mCx, mCy, mS;
    // pre-calculated inverses for speed
    float mInvFx, mInvFy;
    // radial/tangential distortion parameters
    float mK[5];

    // pre-correction active array dimensions
    float mArrayWidth, mArrayHeight;
    // active array dimensions
    float mActiveWidth, mActiveHeight;
    // corner offsets between pre-correction and active arrays
    float mArrayDiffX, mArrayDiffY;

    std::vector<GridQuad> mCorrectedGrid;
    std::vector<GridQuad> mDistortedGrid;

}; // class DistortionMapper

} // namespace camera3

} // namespace android

#endif
