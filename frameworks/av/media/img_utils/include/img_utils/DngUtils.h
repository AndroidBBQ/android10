/*
 * Copyright 2014 The Android Open Source Project
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

#ifndef IMG_UTILS_DNG_UTILS_H
#define IMG_UTILS_DNG_UTILS_H

#include <img_utils/ByteArrayOutput.h>
#include <img_utils/EndianUtils.h>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/RefBase.h>

#include <cutils/compiler.h>
#include <stdint.h>

namespace android {
namespace img_utils {

#define NELEMS(x) ((int) (sizeof(x) / sizeof((x)[0])))
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/**
 * Utility class for building values for the OpcodeList tags specified
 * in the Adobe DNG 1.4 spec.
 */
class ANDROID_API OpcodeListBuilder : public LightRefBase<OpcodeListBuilder> {
    public:
        // Note that the Adobe DNG 1.4 spec for Bayer phase (defined for the
        // FixBadPixelsConstant and FixBadPixelsList opcodes) is incorrect. It's
        // inconsistent with the DNG SDK (cf. dng_negative::SetBayerMosaic and
        // dng_opcode_FixBadPixelsList::IsGreen), and Adobe confirms that the
        // spec should be updated to match the SDK.
        enum CfaLayout {
            CFA_GRBG = 0,
            CFA_RGGB,
            CFA_BGGR,
            CFA_GBRG,
            CFA_NONE,
        };

        OpcodeListBuilder();
        virtual ~OpcodeListBuilder();

        /**
         * Get the total size of this opcode list in bytes.
         */
        virtual size_t getSize() const;

        /**
         * Get the number of opcodes defined in this list.
         */
        virtual uint32_t getCount() const;

        /**
         * Write the opcode list into the given buffer.  This buffer
         * must be able to hold at least as many elements as returned
         * by calling the getSize() method.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t buildOpList(/*out*/ uint8_t* buf) const;

        /**
         * Add GainMap opcode(s) for the given metadata parameters.  The given
         * CFA layout must match the layout of the shading map passed into the
         * lensShadingMap parameter.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t addGainMapsForMetadata(uint32_t lsmWidth,
                                                uint32_t lsmHeight,
                                                uint32_t activeAreaTop,
                                                uint32_t activeAreaLeft,
                                                uint32_t activeAreaBottom,
                                                uint32_t activeAreaRight,
                                                CfaLayout cfa,
                                                const float* lensShadingMap);

        /**
         * Add a GainMap opcode with the given fields.  The mapGains array
         * must have mapPointsV * mapPointsH * mapPlanes elements.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t addGainMap(uint32_t top,
                                    uint32_t left,
                                    uint32_t bottom,
                                    uint32_t right,
                                    uint32_t plane,
                                    uint32_t planes,
                                    uint32_t rowPitch,
                                    uint32_t colPitch,
                                    uint32_t mapPointsV,
                                    uint32_t mapPointsH,
                                    double mapSpacingV,
                                    double mapSpacingH,
                                    double mapOriginV,
                                    double mapOriginH,
                                    uint32_t mapPlanes,
                                    const float* mapGains);

        /**
         * Add WarpRectilinear opcode for the given metadata parameters.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t addWarpRectilinearForMetadata(const float* kCoeffs,
                                                       uint32_t activeArrayWidth,
                                                       uint32_t activeArrayHeight,
                                                       float opticalCenterX,
                                                       float opticalCenterY);

        /**
         * Add a WarpRectilinear opcode.
         *
         * numPlanes - Number of planes included in this opcode.
         * opticalCenterX, opticalCenterY - Normalized x,y coordinates of the sensor optical
         *          center relative to the top,left pixel of the produced images (e.g. [0.5, 0.5]
         *          gives a sensor optical center in the image center.
         * kCoeffs - A list of coefficients for the polynomial equation representing the distortion
         *          correction.  For each plane, 6 coefficients must be included:
         *          {k_r0, k_r1, k_r2, k_r3, k_t0, k_t1}.  See the DNG 1.4 specification for an
         *          outline of the polynomial used here.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t addWarpRectilinear(uint32_t numPlanes,
                                            double opticalCenterX,
                                            double opticalCenterY,
                                            const double* kCoeffs);


        /**
         * Add FixBadPixelsList opcode for the given metadata parameters.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t addBadPixelListForMetadata(const uint32_t* hotPixels,
                                                    uint32_t xyPairCount,
                                                    uint32_t colorFilterArrangement);

        /**
         * Add FixBadPixelsList opcode.
         *
         * bayerPhase - 0=top-left of image is red, 1=top-left of image is green pixel in red row,
         *              2=top-left of image is green pixel in blue row, 3=top-left of image is
         *              blue.
         * badPointCount - number of (x,y) pairs of bad pixels are given in badPointRowColPairs.
         * badRectCount - number of (top, left, bottom, right) tuples are given in
         *              badRectTopLeftBottomRightTuples
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t addBadPixelList(uint32_t bayerPhase,
                                         uint32_t badPointCount,
                                         uint32_t badRectCount,
                                         const uint32_t* badPointRowColPairs,
                                         const uint32_t* badRectTopLeftBottomRightTuples);

        // TODO: Add other Opcode methods
    protected:
        static const uint32_t FLAG_OPTIONAL = 0x1u;
        static const uint32_t FLAG_OPTIONAL_FOR_PREVIEW = 0x2u;

        // Opcode IDs
        enum {
            WARP_RECTILINEAR_ID = 1,
            FIX_BAD_PIXELS_LIST = 5,
            GAIN_MAP_ID = 9,
        };

        // LSM mosaic indices
        enum {
            LSM_R_IND = 0,
            LSM_GE_IND = 1,
            LSM_GO_IND = 2,
            LSM_B_IND = 3,
        };

        uint32_t mCount;
        ByteArrayOutput mOpList;
        EndianOutput mEndianOut;

        status_t addOpcodePreamble(uint32_t opcodeId);

    private:
        /**
         * Add Bayer GainMap opcode(s) for the given metadata parameters.
         * CFA layout must match the layout of the shading map passed into the
         * lensShadingMap parameter.
         *
         * Returns OK on success, or a negative error code.
         */
        status_t addBayerGainMapsForMetadata(uint32_t lsmWidth,
                                                uint32_t lsmHeight,
                                                uint32_t activeAreaWidth,
                                                uint32_t activeAreaHeight,
                                                CfaLayout cfa,
                                                const float* lensShadingMap);

        /**
         * Add Bayer GainMap opcode(s) for the given metadata parameters.
         * CFA layout must match the layout of the shading map passed into the
         * lensShadingMap parameter.
         *
         * Returns OK on success, or a negative error code.
         */
        status_t addMonochromeGainMapsForMetadata(uint32_t lsmWidth,
                                                uint32_t lsmHeight,
                                                uint32_t activeAreaWidth,
                                                uint32_t activeAreaHeight,
                                                const float* lensShadingMap);
};

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_DNG_UTILS_H*/
