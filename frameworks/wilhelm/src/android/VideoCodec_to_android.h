/*
 * Copyright (C) 2011 The Android Open Source Project
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

namespace android {

/**************************************************************************
 * Important note: the functions below are helper functions for the
 *   implementation of the XAVideoDecoderCapabilitiesItf interface on
 *   Android. They are ONLY exposing the "hardware" video decoders
 *   as dynamically discovered on the platform. A hardware video decoder
 *   is defined as one where video decoding takes advantage of a hardware-
 *   specific feature of the platform, such as DSP or particular GPU.
 *   "Software" decoders, such as the ones available as default implementations
 *   in the Android tree, as not exposed here, but are rather listed in
 *   the CDD.
 */

/*
 * Initializes the list of supported video codecs, with their associated profiles and levels
 * Return
 *     false if it failed to list the available codecs, true otherwise
 */
extern bool android_videoCodec_expose();

/*
 * Frees all resources associated with the listing and query of the available video codecs
 */
extern void android_videoCodec_deinit();

/*
 * Returns the number of video codecs supported on the platform.
 * Returns 0 if android_videoCodec_expose() hasn't been called before (and returned true)
 */
extern XAuint32 android_videoCodec_getNbDecoders();

/*
 * Retrieve the supported decoder IDs
 * Pre-condition
 *    nbDecoders <= number of decoders
 *    pDecoderIds != NULL
 *    android_videoCodec_expose() has been called before (and returned true)
 */
extern void android_videoCodec_getDecoderIds(XAuint32 nbDecoders, XAuint32 *pDecoderIds);

/*
 * Retrieves the number of profile / level combinations for a given codec
 * Pre-condition
 *    pNb != NULL
 *    android_videoCodec_expose() has been called before (and returned true)
 * Return
 *    SL_RESULT_SUCCESS if the number of combinations was successfully retrieved and at least
 *                         one profile/level is supported (*pNb > 0)
 *    SL_RESULT_PARAMETER_INVALID otherwise
 */
extern SLresult android_videoCodec_getProfileLevelCombinationNb(XAuint32 decoderId, XAuint32 *pNb);

/*
 * Retrieves the profile / level at index plIndex in the list of profile / level combinations
 *    supported by the given codec
 * Pre-condition
 *    pDescr != NULL
 *    android_videoCodec_expose() has been called before (and returned true)
 * Post-condition
 *    pDescr->codecId == decoderId
 * Return
 *    SL_RESULT_SUCCESS if the profile/level was successfully retrieved
 *    SL_RESULT_PARAMETER_INVALID otherwise: invalid codec or invalid profile/level index
 */
extern SLresult android_videoCodec_getProfileLevelCombination(XAuint32 decoderId,
        XAuint32 plIndex, XAVideoCodecDescriptor *pDescr);

}; // namespace android
