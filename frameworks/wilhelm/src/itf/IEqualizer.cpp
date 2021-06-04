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

/* Equalizer implementation */

#include "sles_allinclusive.h"
#ifdef ANDROID
#include <system/audio_effects/effect_equalizer.h>
#endif

#define MAX_EQ_PRESETS 3

#if !defined(ANDROID)
static const struct EqualizerBand EqualizerBands[MAX_EQ_BANDS] = {
    {1000, 1500, 2000},
    {2000, 3000, 4000},
    {4000, 5500, 7000},
    {7000, 8000, 9000}
};

static const struct EqualizerPreset {
    const char *mName;
    SLmillibel mLevels[MAX_EQ_BANDS];
} EqualizerPresets[MAX_EQ_PRESETS] = {
    {"Default", {0, 0, 0, 0}},
    {"Bass", {500, 200, 100, 0}},
    {"Treble", {0, 100, 200, 500}}
};
#endif


#if defined(ANDROID)
/**
 * returns true if this interface is not associated with an initialized Equalizer effect
 */
static inline bool NO_EQ(IEqualizer* v) {
    return (v->mEqEffect == 0);
}
#endif


static SLresult IEqualizer_SetEnabled(SLEqualizerItf self, SLboolean enabled)
{
    SL_ENTER_INTERFACE

    IEqualizer *thiz = (IEqualizer *) self;
    interface_lock_exclusive(thiz);
    thiz->mEnabled = (SLboolean) enabled;
#if !defined(ANDROID)
    result = SL_RESULT_SUCCESS;
#else
    if (NO_EQ(thiz)) {
        result = SL_RESULT_CONTROL_LOST;
    } else {
        android::status_t status =
                thiz->mEqEffect->setEnabled((bool) thiz->mEnabled);
        result = android_fx_statusToResult(status);
    }
#endif
    interface_unlock_exclusive(thiz);

    SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_IsEnabled(SLEqualizerItf self, SLboolean *pEnabled)
{
    SL_ENTER_INTERFACE

    if (NULL == pEnabled) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEqualizer *thiz = (IEqualizer *) self;
        interface_lock_exclusive(thiz);
 #if !defined(ANDROID)
        SLboolean enabled = thiz->mEnabled;
        *pEnabled = enabled;
        result = SL_RESULT_SUCCESS;
 #else
        if (NO_EQ(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            *pEnabled = (SLboolean) thiz->mEqEffect->getEnabled();
            result = SL_RESULT_SUCCESS;
        }
 #endif
        interface_unlock_exclusive(thiz);
    }

      SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_GetNumberOfBands(SLEqualizerItf self, SLuint16 *pNumBands)
{
    SL_ENTER_INTERFACE

    if (NULL == pNumBands) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEqualizer *thiz = (IEqualizer *) self;
        // Note: no lock, but OK because it is const
        *pNumBands = thiz->mNumBands;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_GetBandLevelRange(SLEqualizerItf self, SLmillibel *pMin,
    SLmillibel *pMax)
{
    SL_ENTER_INTERFACE

    if (NULL == pMin && NULL == pMax) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEqualizer *thiz = (IEqualizer *) self;
        // Note: no lock, but OK because it is const
        if (NULL != pMin)
            *pMin = thiz->mBandLevelRangeMin;
        if (NULL != pMax)
            *pMax = thiz->mBandLevelRangeMax;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_SetBandLevel(SLEqualizerItf self, SLuint16 band, SLmillibel level)
{
    SL_ENTER_INTERFACE

    IEqualizer *thiz = (IEqualizer *) self;
    if (!(thiz->mBandLevelRangeMin <= level && level <= thiz->mBandLevelRangeMax) ||
            (band >= thiz->mNumBands)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        interface_lock_exclusive(thiz);
#if !defined(ANDROID)
        thiz->mLevels[band] = level;
        thiz->mPreset = SL_EQUALIZER_UNDEFINED;
        result = SL_RESULT_SUCCESS;
#else
        if (NO_EQ(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            android::status_t status =
                android_eq_setParam(thiz->mEqEffect, EQ_PARAM_BAND_LEVEL, band, &level);
            result = android_fx_statusToResult(status);
        }
#endif
        interface_unlock_exclusive(thiz);
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_GetBandLevel(SLEqualizerItf self, SLuint16 band, SLmillibel *pLevel)
{
    SL_ENTER_INTERFACE

    if (NULL == pLevel) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEqualizer *thiz = (IEqualizer *) self;
        // const, no lock needed
        if (band >= thiz->mNumBands) {
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            SLmillibel level = 0;
            interface_lock_shared(thiz);
#if !defined(ANDROID)
            level = thiz->mLevels[band];
            result = SL_RESULT_SUCCESS;
#else
            if (NO_EQ(thiz)) {
                result = SL_RESULT_CONTROL_LOST;
            } else {
                android::status_t status =
                    android_eq_getParam(thiz->mEqEffect, EQ_PARAM_BAND_LEVEL, band, &level);
                result = android_fx_statusToResult(status);
            }
#endif
            interface_unlock_shared(thiz);
            *pLevel = level;
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_GetCenterFreq(SLEqualizerItf self, SLuint16 band, SLmilliHertz *pCenter)
{
    SL_ENTER_INTERFACE

    if (NULL == pCenter) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEqualizer *thiz = (IEqualizer *) self;
        if (band >= thiz->mNumBands) {
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
#if !defined(ANDROID)
            // Note: no lock, but OK because it is const
            *pCenter = thiz->mBands[band].mCenter;
            result = SL_RESULT_SUCCESS;
#else
            SLmilliHertz center = 0;
            interface_lock_shared(thiz);
            if (NO_EQ(thiz)) {
                result = SL_RESULT_CONTROL_LOST;
            } else {
                android::status_t status =
                    android_eq_getParam(thiz->mEqEffect, EQ_PARAM_CENTER_FREQ, band, &center);
                result = android_fx_statusToResult(status);
            }
            interface_unlock_shared(thiz);
            *pCenter = center;
#endif
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_GetBandFreqRange(SLEqualizerItf self, SLuint16 band,
    SLmilliHertz *pMin, SLmilliHertz *pMax)
{
    SL_ENTER_INTERFACE

    if (NULL == pMin && NULL == pMax) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEqualizer *thiz = (IEqualizer *) self;
        if (band >= thiz->mNumBands) {
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
#if !defined(ANDROID)
            // Note: no lock, but OK because it is const
            if (NULL != pMin)
                *pMin = thiz->mBands[band].mMin;
            if (NULL != pMax)
                *pMax = thiz->mBands[band].mMax;
            result = SL_RESULT_SUCCESS;
#else
            SLmilliHertz range[2] = {0, 0}; // SLmilliHertz is SLuint32
            interface_lock_shared(thiz);
            if (NO_EQ(thiz)) {
                result = SL_RESULT_CONTROL_LOST;
            } else {
                android::status_t status =
                    android_eq_getParam(thiz->mEqEffect, EQ_PARAM_BAND_FREQ_RANGE, band, range);
                result = android_fx_statusToResult(status);
            }
            interface_unlock_shared(thiz);
            if (NULL != pMin) {
                *pMin = range[0];
            }
            if (NULL != pMax) {
                *pMax = range[1];
            }
#endif
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_GetBand(SLEqualizerItf self, SLmilliHertz frequency, SLuint16 *pBand)
{
    SL_ENTER_INTERFACE

    if (NULL == pBand) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEqualizer *thiz = (IEqualizer *) self;
#if !defined(ANDROID)
        // search for band whose center frequency has the closest ratio to 1.0
        // assumes bands are unsorted (a pessimistic assumption)
        // assumes bands can overlap (a pessimistic assumption)
        // assumes a small number of bands, so no need for a fancier algorithm
        const struct EqualizerBand *band;
        float floatFreq = (float) frequency;
        float bestRatio = 0.0;
        SLuint16 bestBand = SL_EQUALIZER_UNDEFINED;
        for (band = thiz->mBands; band < &thiz->mBands[thiz->mNumBands]; ++band) {
            if (!(band->mMin <= frequency && frequency <= band->mMax))
                continue;
            assert(band->mMin <= band->mCenter && band->mCenter <= band->mMax);
            assert(band->mCenter != 0);
            float ratio = frequency <= band->mCenter ?
                floatFreq / band->mCenter : band->mCenter / floatFreq;
            if (ratio > bestRatio) {
                bestRatio = ratio;
                bestBand = band - thiz->mBands;
            }
        }
        *pBand = bestBand;
        result = SL_RESULT_SUCCESS;
#else
        uint16_t band = 0;
        interface_lock_shared(thiz);
        if (NO_EQ(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            android::status_t status =
                android_eq_getParam(thiz->mEqEffect, EQ_PARAM_GET_BAND, frequency, &band);
            result = android_fx_statusToResult(status);
        }
        interface_unlock_shared(thiz);
        *pBand = (SLuint16)band;
#endif
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_GetCurrentPreset(SLEqualizerItf self, SLuint16 *pPreset)
{
    SL_ENTER_INTERFACE

    if (NULL == pPreset) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEqualizer *thiz = (IEqualizer *) self;
        interface_lock_shared(thiz);
#if !defined(ANDROID)
        SLuint16 preset = thiz->mPreset;
        interface_unlock_shared(thiz);
        *pPreset = preset;
        result = SL_RESULT_SUCCESS;
#else
        uint16_t preset = 0;
        if (NO_EQ(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            android::status_t status =
                    android_eq_getParam(thiz->mEqEffect, EQ_PARAM_CUR_PRESET, 0, &preset);
            result = android_fx_statusToResult(status);
        }
        interface_unlock_shared(thiz);

        *pPreset = (SLuint16) preset;
#endif

    }

    SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_UsePreset(SLEqualizerItf self, SLuint16 index)
{
    SL_ENTER_INTERFACE
    SL_LOGV("Equalizer::UsePreset index=%u", index);

    IEqualizer *thiz = (IEqualizer *) self;
    if (index >= thiz->mNumPresets) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        interface_lock_exclusive(thiz);
#if !defined(ANDROID)
        SLuint16 band;
        for (band = 0; band < thiz->mNumBands; ++band)
            thiz->mLevels[band] = EqualizerPresets[index].mLevels[band];
        thiz->mPreset = index;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
#else
        if (NO_EQ(thiz)) {
            result = SL_RESULT_CONTROL_LOST;
        } else {
            android::status_t status =
                android_eq_setParam(thiz->mEqEffect, EQ_PARAM_CUR_PRESET, 0, &index);
            result = android_fx_statusToResult(status);
        }
        interface_unlock_shared(thiz);
#endif
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_GetNumberOfPresets(SLEqualizerItf self, SLuint16 *pNumPresets)
{
    SL_ENTER_INTERFACE

    if (NULL == pNumPresets) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEqualizer *thiz = (IEqualizer *) self;
        // Note: no lock, but OK because it is const
        *pNumPresets = thiz->mNumPresets;

        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEqualizer_GetPresetName(SLEqualizerItf self, SLuint16 index, const SLchar **ppName)
{
    SL_ENTER_INTERFACE

    if (NULL == ppName) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IEqualizer *thiz = (IEqualizer *) self;
#if !defined(ANDROID)
        if (index >= thiz->mNumPresets) {
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            *ppName = (SLchar *) thiz->mPresets[index].mName;
            result = SL_RESULT_SUCCESS;
        }
#else
        if (index >= thiz->mNumPresets) {
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            // FIXME query preset name rather than retrieve it from the engine.
            //       In SL ES 1.0.1, the strings must exist for the lifetime of the engine.
            //       Starting in 1.1, this will change and we don't need to hold onto the strings
            //       for so long as they will copied into application space.
            *ppName = (SLchar *) thiz->mThis->mEngine->mEqPresetNames[index];
            result = SL_RESULT_SUCCESS;
        }
#endif
    }

    SL_LEAVE_INTERFACE
}


static const struct SLEqualizerItf_ IEqualizer_Itf = {
    IEqualizer_SetEnabled,
    IEqualizer_IsEnabled,
    IEqualizer_GetNumberOfBands,
    IEqualizer_GetBandLevelRange,
    IEqualizer_SetBandLevel,
    IEqualizer_GetBandLevel,
    IEqualizer_GetCenterFreq,
    IEqualizer_GetBandFreqRange,
    IEqualizer_GetBand,
    IEqualizer_GetCurrentPreset,
    IEqualizer_UsePreset,
    IEqualizer_GetNumberOfPresets,
    IEqualizer_GetPresetName
};

void IEqualizer_init(void *self)
{
    IEqualizer *thiz = (IEqualizer *) self;
    thiz->mItf = &IEqualizer_Itf;
    thiz->mEnabled = SL_BOOLEAN_FALSE;
    thiz->mPreset = SL_EQUALIZER_UNDEFINED;
#if 0 < MAX_EQ_BANDS
    unsigned band;
    for (band = 0; band < MAX_EQ_BANDS; ++band)
        thiz->mLevels[band] = 0;
#endif
    // const fields
    thiz->mNumPresets = 0;
    thiz->mNumBands = 0;
#if !defined(ANDROID)
    thiz->mBands = EqualizerBands;
    thiz->mPresets = EqualizerPresets;
#endif
    thiz->mBandLevelRangeMin = 0;
    thiz->mBandLevelRangeMax = 0;
#if defined(ANDROID)
    memset(&thiz->mEqDescriptor, 0, sizeof(effect_descriptor_t));
    // placement new (explicit constructor)
    (void) new (&thiz->mEqEffect) android::sp<android::AudioEffect>();
#endif
}

void IEqualizer_deinit(void *self)
{
#if defined(ANDROID)
    IEqualizer *thiz = (IEqualizer *) self;
    // explicit destructor
    thiz->mEqEffect.~sp();
#endif
}

bool IEqualizer_Expose(void *self)
{
#if defined(ANDROID)
    IEqualizer *thiz = (IEqualizer *) self;
    if (!android_fx_initEffectDescriptor(SL_IID_EQUALIZER, &thiz->mEqDescriptor)) {
        SL_LOGE("Equalizer initialization failed");
        thiz->mNumPresets = 0;
        thiz->mNumBands = 0;
        thiz->mBandLevelRangeMin = 0;
        thiz->mBandLevelRangeMax = 0;
        return false;
    }
#endif
    return true;
}
